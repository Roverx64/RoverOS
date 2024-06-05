#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include <stdbool.h>
#include <bootMem.h>
#include "boot.h"
#include "mmap.h"

uint64_t mapMax = 0;
uint64_t mapPtr = 0;
mmapEntry *allocMap = NULL;

//AllocatePool wrapper
void *allocMem_int(const char *func, uint64_t ln, size_t sz, uint64_t virt, uint8_t type, bool temp, bool align, bool clear){
    uint64_t rsz = sz;
    if(align){sz += PAGE_SZ;}
    void *ptr = AllocatePool(sz);
    if(ptr == NULL){Print(L"MEM   : Allocation for %a failed on line 0x%lx\n",func,ln); for(;;){}}
    if(align){ptr = (void*)(KALIGN((uint64_t)ptr)+PAGE_SZ);}
    if(clear){ZeroMem((void*)ptr,rsz);}
    //Add to mmap if it is not a temp mapping
    if(temp){goto end;}
    allocMap[mapPtr].bytes = rsz;
    allocMap[mapPtr].phys = (uint64_t)ptr;
    allocMap[mapPtr].type = type;
    allocMap[mapPtr].virt = (virt == 0x0) ? (uint64_t)ptr : virt;
    ++mapPtr;
    if(mapPtr == mapMax){
        allocMap = (mmapEntry*)ReallocatePool(allocMap,sizeof(mmapEntry)*mapMax,sizeof(mmapEntry)*(mapMax+20));
        kinf.mem.sptr = (void*)allocMap;
        mapMax += 20;
    }
    end:
    if(MEM_DEBUG){Print(L"MEM   : Allocated 0x%lx bytes at 0x%lx for %a on line 0x%lx\n",sz,(uint64_t)ptr,func,ln);}
    return ptr;
}

//Paging functions

uint64_t *pml = NULL;
uint64_t bumpBase = 0x0;
uint64_t bumpPtr = 0x0;
uint64_t bumpSize = 0x0;

//Returns a new table
uint64_t pgAlloc(){
    if(bumpPtr == bumpSize){
        //Allocate new chunk
        bumpBase = (uint64_t)AllocatePool(0x400000+PAGE_SZ); //4MB bump
        bumpBase = KALIGN(bumpBase)+PAGE_SZ;
        bumpSize = 0x400000;
        bumpPtr = 0x0;
    }
    uint64_t ptr = (bumpBase+bumpPtr);
    ZeroMem((void*)ptr,0x1000);
    bumpPtr += 0x1000;
    return ptr;
}

EFI_MEMORY_DESCRIPTOR *mmapStart = NULL;
UINTN mmapEntries = 0;
UINTN mmapKey = 0;
UINTN mmapSz = 0;
uint32_t mmapVer = 0;

EFI_STATUS mapPage(uint64_t phys, uint64_t virt, bool write, bool execute){
    phys = KALIGN(phys);
    virt = KALIGN(virt);
    //PDPE
    uint64_t pmln = PML_ENTRY(virt);
    if(TABLE_BASE(pml[pmln]) == 0x0){pml[pmln] |= TABLE_BASE(pgAlloc());}
    uint64_t *pdpe = (uint64_t*)TABLE_BASE(pml[pmln]);
    //PDE
    uint64_t pdpen = PDPE_ENTRY(virt);
    if(TABLE_BASE(pdpe[pdpen]) == 0x0){pdpe[pdpen] |= TABLE_BASE(pgAlloc());}
    uint64_t *pde = (uint64_t*)TABLE_BASE(pdpe[pdpen]);
    //PTE
    uint64_t pden = PDE_ENTRY(virt);
    if(TABLE_BASE(pde[pden]) == 0x0){pde[pden] |= TABLE_BASE(pgAlloc());}
    uint64_t *pte = (uint64_t*)TABLE_BASE(pde[pden]);
    //Flags
    uint64_t flags = PG_PRESENT;
    if(write){flags |= PG_WRITE;}
    pml[pmln] |= flags;
    pdpe[pdpen] |= flags;
    pde[pden] |= flags;
    if(execute == false){flags |= PG_NX;}
    pte[PTE_ENTRY(virt)] = (flags|phys);
    return EFI_SUCCESS;
}

EFI_STATUS mapPages(uint64_t phys, uint64_t virt, uint32_t n, bool write, bool execute){
    if(n == 0){n = 1;}
    for(int i = 0; i < n; ++i){
        if(mapPage(phys+(i*PAGE_SZ),virt+(i*PAGE_SZ),write,execute) != EFI_SUCCESS){return !EFI_SUCCESS;}
    }
    return EFI_SUCCESS;
}

void idMapMem(){
    mmapStart = LibMemoryMap(&mmapEntries,&mmapKey,&mmapSz,&mmapVer);
    if(mmapStart == NULL){Print(L"PAGE  : LibMemoryMap() failed\n"); for(;;){}}
    //Map any memory that isn't reserved, unaccepted, or unusable
    EFI_MEMORY_DESCRIPTOR *desc = mmapStart;
    for(int i = 0; i+1 < mmapEntries; ++i){
        if((desc->Type == EfiReservedMemoryType) || (desc->Type == EfiUnusableMemory) || (desc->Type == EfiUnacceptedMemoryType)){goto end;}
        mapPages(desc->PhysicalStart,desc->PhysicalStart,(desc->NumberOfPages*EFI_PAGE_SIZE)/PAGE_SZ,true,false);
        end:
        desc = NEXT_DESC(desc,mmapSz);
    }
}

void initMem(){
    //Init mmap
    Print(L"MEM   : Intilizing allocation mmap\n");
    mapMax = 40;
    allocMap = (mmapEntry*)AllocatePool(sizeof(mmapEntry)*mapMax); //Can be expanded later
    kinf.mem.sptr = (void*)allocMap;
    ZeroMem((void*)allocMap,sizeof(mmapEntry)*mapMax);
    Print(L"MEM   : Allocated mmap to 0x%lx\n",(uint64_t)allocMap);
    //Setup paging
    Print(L"PAGE  : Initilizing paging structs\n");
    pml = (uint64_t*)pgAlloc();
    Print(L"PAGE  : Allocated PML to 0x%lx\n",(uint64_t)pml);
    Print(L"PAGE  : Identity mapping all memory\n");
    idMapMem();
    //Setup inital memory for buddy bump allocator
    Print(L"MEM   : Allocating pmm chunk\n");
    kinf.mem.chunkPtr = (uint64_t)allocMem(0x200000,0x0,ALLOC_TYPE_BUDDY,false,true,false);
    kinf.mem.chunkSize = 0x200000;
    mapPages(kinf.mem.chunkPtr,kinf.mem.chunkPtr,0x200000/PAGE_SZ,true,false);
    Print(L"MEM   : Allocated chunk to 0x%lx\n",kinf.mem.chunkPtr);
}

void setCR3(){
    asm volatile("movq %%rax, %%cr3" ::"a"(pml));
}

void testPath(uint64_t addr, bool efi){
    addr = KALIGN(addr);
    uint64_t temp = 0x0;
    uint64_t *tpml = pml;
    if(efi == true){asm volatile("movq %%cr3, %0" : "=r"(temp)); tpml = (uint64_t*)temp;}
    Print(L"TRACE : 0x%lx->",addr);
    //PML4E
    Print(L"PML[");
    if(!(tpml[PML_ENTRY(addr)]&PG_PRESENT)){Print(L"#PF]\n"); return;}
    Print(L"0x%lx]->",tpml[PML_ENTRY(addr)]);
    //PDPE
    Print(L"PDPE[");
    uint64_t *pdpe = (uint64_t*)TABLE_BASE(tpml[PML_ENTRY(addr)]);
    if(!(pdpe[PDPE_ENTRY(addr)]&PG_PRESENT)){Print(L"#PF]\n"); return;}
    Print(L"0x%lx]->",pdpe[PDPE_ENTRY(addr)]);
    //PDE
    Print(L"PDE[");
    uint64_t *pde = (uint64_t*)TABLE_BASE(pdpe[PDPE_ENTRY(addr)]);
    if(!(pde[PDE_ENTRY(addr)]&PG_PRESENT)){Print(L"#PF]\n"); return;}
    Print(L"0x%lx]->",pde[PDE_ENTRY(addr)]);
    //PTE
    Print(L"PTE[");
    uint64_t *pte = (uint64_t*)TABLE_BASE(pde[PDE_ENTRY(addr)]);
    if(!(pte[PTE_ENTRY(addr)]&PG_PRESENT)){Print(L"#PF]\n"); return;}
    Print(L"0x%lx]->0x%lx[",pte[PTE_ENTRY(addr)],TABLE_BASE(pte[PTE_ENTRY(addr)]));
    if(pte[PTE_ENTRY(addr)]&PG_WRITE){Print(L"W");}else{Print(L"R");}
    if(pte[PTE_ENTRY(addr)]&PG_USER){Print(L"U");}else{Print(L"S");}
    if(pte[PTE_ENTRY(addr)]&PG_NX){Print(L"-");}else{Print(L"X");}
    Print(L"]\n");
}

mmapEntry *kmmap = NULL;

//Constructs kernel mmap
EFI_STATUS initMMAP(){
    mmapStart = LibMemoryMap(&mmapEntries,&mmapKey,&mmapSz,&mmapVer);
    if(mmapStart == NULL){Print(L"MMAP  : LibMemoryMap() failed\n"); return !EFI_SUCCESS;}
    kmmap = (mmapEntry*)allocMem(sizeof(mmapEntry)*(mmapEntries+5),0x0,ALLOC_TYPE_NONE,true,true,true);
    if(kmmap == NULL){Print(L"MMAP  : AllocatePool() failed\n"); return !EFI_SUCCESS;}
    //mapPages(KALIGN((uint64_t)kmmap),KALIGN((uint64_t)kmmap),((sizeof(mmapEntry)*(mmapEntries+5))/PAGE_SZ)+1,true,false);
    mmapStart = LibMemoryMap(&mmapEntries,&mmapKey,&mmapSz,&mmapVer);
    if(mmapStart == NULL){Print(L"MMAP  : LibMemoryMap() failed\n"); return !EFI_SUCCESS;}
    EFI_MEMORY_DESCRIPTOR *desc = mmapStart;
    uint64_t usableMemory = 0x0;
    uint64_t total = 0x0;
    uint64_t mp = 0x0;
    uint64_t mmioMem = 0;
    uint64_t efiMem = 0;
    uint64_t cvMem = 0;
    uint64_t unMem = 0;
    uint64_t acpiMem = 0;
    for(int i = 0; i+1 < mmapEntries; ++i){
        kmmap[mp].magic = MMAP_MAGIC;
        switch(desc->Type){
            case EfiMemoryMappedIOPortSpace:
            case EfiMemoryMappedIO:
            mmioMem += DESC_SZ(desc);
            kmmap[mp].type = MMAP_TYPE_MMIO;
            break;
            case EfiBootServicesCode:
            efiMem += DESC_SZ(desc);
            cvMem -= DESC_SZ(desc);
            break;
            case EfiBootServicesData:
            efiMem += DESC_SZ(desc);
            cvMem -= DESC_SZ(desc);
            kmmap[mp].type = MMAP_TYPE_FREE;
            break;
            case EfiConventionalMemory:
            kmmap[mp].type = MMAP_TYPE_FREE;
            cvMem += DESC_SZ(desc);
            usableMemory += DESC_SZ(desc);
            break;
            case EfiReservedMemoryType:
            unMem += DESC_SZ(desc);
            kmmap[mp].type = MMAP_TYPE_RESV;
            break;
            default:
            case EfiUnusableMemory:
            unMem += DESC_SZ(desc);
            kmmap[mp].type = MMAP_TYPE_UNUSABLE;
            break;
            case EfiRuntimeServicesCode:
            case EfiRuntimeServicesData:
            efiMem += DESC_SZ(desc);
            kmmap[mp].type = MMAP_TYPE_UEFI;
            break;
            case EfiLoaderCode:
            efiMem += DESC_SZ(desc);
            kmmap[mp].type = MMAP_TYPE_FREE;
            usableMemory += DESC_SZ(desc);
            mapPages(desc->PhysicalStart,desc->PhysicalStart,DESC_SZ(desc)/PAGE_SZ,false,true);
            break;
            case EfiLoaderData:
            efiMem += DESC_SZ(desc);
            kmmap[mp].type = MMAP_TYPE_UEFI;
            usableMemory += DESC_SZ(desc);
            mapPages(desc->PhysicalStart,desc->PhysicalStart,DESC_SZ(desc)/PAGE_SZ,true,false);
            break;
            case EfiACPIReclaimMemory:
            case EfiPalCode:
            case EfiACPIMemoryNVS:
            acpiMem += DESC_SZ(desc);
            kmmap[mp].type = MMAP_TYPE_ACPI;
            break;
        }
        kmmap[mp].phys = desc->PhysicalStart;
        kmmap[mp].bytes = DESC_SZ(desc);
        //Check if entries can be merged
        //NOTE: This is very basic and assumes address are already ordered correctly
        if(mp == 0x0){goto end;}
        if(kmmap[mp-1].type != kmmap[mp].type){goto end;}
        if((kmmap[mp-1].phys+kmmap[mp-1].bytes) != kmmap[mp].phys){goto end;}
        //Merge here
        kmmap[mp-1].bytes += kmmap[mp].bytes;
        kmmap[mp].type = !MMAP_TYPE_FREE;
        --mp;
        end:
        total += DESC_SZ(desc);
        ++mp;
        desc = NEXT_DESC(desc,mmapSz);
    }
    kinf.mem.ptr = (void*)kmmap;
    kinf.mem.entries = mp;
    kinf.mem.totalMem = total-mmioMem;
    kinf.mem.usableMem = usableMemory;
    Print(L"MMAP  : At 0x%lx [0x%x entries|T: 0x%lx|U: 0x%lx]\n",(uint64_t)kinf.mem.ptr,mp,total-mmioMem,usableMemory);
    return EFI_SUCCESS;
}