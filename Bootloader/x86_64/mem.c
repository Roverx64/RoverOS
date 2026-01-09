#include <sefi.h>
#include "bootMem.h"
#include "bootinfo.h"
#include "mmap.h"

extern struct bootInfo kinf;

uint64_t mapMax = 0;
uint64_t mapPtr = 0;
uint64_t allocEntries = 0;
mmapEntry *allocMap = NULL;

/*! allocPool() wrapper

This function keeps track of non-temporary allocations.

Returns NULL on failure.
*/
void *allocMem_int(const char *func, uint64_t ln, size_t sz, uint64_t virt, uint8_t type, bool temp, bool align, bool clear){
    if(align){sz += PAGE_SZ;}
    void *ptr = allocPool(sz);
    uint64_t bptr = (uint64_t)ptr;
    if(ptr == NULL){sprint(L"Allocation for %a failed on line 0x%lx\n",func,ln); for(;;){}}
    if(align){ptr = (void*)(KALIGN((uint64_t)ptr)+PAGE_SZ);}
    if(clear){clearMem((void*)ptr,sz);}
    //Add to mmap if it is not a temp mapping
    if(temp){goto end;}
    allocMap[mapPtr].bytes = sz;
    allocMap[mapPtr].phys = bptr;
    allocMap[mapPtr].type = type;
    allocMap[mapPtr].virt = (virt == 0x0) ? (uint64_t)ptr : virt;
    allocMap[mapPtr].magic = MMAP_MAGIC;
    if(align){allocMap[mapPtr].flags |= MMAP_FLAG_ALIGN;}
    ++allocEntries;
    ++mapPtr;
    if(mapPtr == mapMax){
        allocMap = (mmapEntry*)reallocPool(allocMap,sizeof(mmapEntry)*mapMax,sizeof(mmapEntry)*(mapMax+20));
        mapMax += 20;
    }
    end:
    if(MEM_DEBUG){sprint(L"Allocated 0x%lx bytes at 0x%lx for %a on line 0x%lx\n",sz,(uint64_t)ptr,func,ln);}
    return ptr;
}

//Paging functions

uint64_t *pml = NULL;
uint64_t bumpBase = 0x0;
uint64_t bumpPtr = 0x0;
uint64_t bumpSize = 0x0;

/*!
Allocates and returns a new page table.

Returns 0x0 upon failure.
*/
uint64_t pgAlloc(){
    if(bumpPtr == bumpSize){
        //Allocate new chunk
        bumpBase = (uint64_t)allocPool(0x400000+PAGE_SZ); //4MB bump
        bumpBase = KALIGN(bumpBase)+PAGE_SZ;
        bumpSize = 0x400000;
        bumpPtr = 0x0;
    }
    uint64_t ptr = (bumpBase+bumpPtr);
    clearMem((void*)ptr,0x1000);
    bumpPtr += 0x1000;
    return ptr;
}

EFI_MEMORY_DESCRIPTOR *mmapStart = NULL;
UINTN mmapEntries = 0;
UINTN mmapKey = 0;
UINTN mmapSz = 0;
uint32_t mmapVer = 0;

/*!
Maps a physical address to a virtual address
*/
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

/*!
Maps 'n' number of pages at once
*/
EFI_STATUS mapPages(uint64_t phys, uint64_t virt, uint32_t n, bool write, bool execute){
    if(n == 0){n = 1;}
    for(uint32_t i = 0; i < n; ++i){
        if(mapPage(phys+(i*PAGE_SZ),virt+(i*PAGE_SZ),write,execute) != EFI_SUCCESS){return !EFI_SUCCESS;}
    }
    return EFI_SUCCESS;
}

/*!
Uses page mapping functions to map mmap entries.

('reserved', 'unaccepted, and 'unusable' are not mapped)
*/
void idMapMem(){
    mmapStart = getMmap(&mmapEntries,&mmapKey,&mmapSz,&mmapVer);
    if(mmapStart == NULL){sprint(L"LibMemoryMap() failed\n"); for(;;){}}
    //Map any memory that isn't reserved, unaccepted, or unusable
    EFI_MEMORY_DESCRIPTOR *desc = mmapStart;
    for(UINTN i = 0; i+1 < mmapEntries; ++i){
        if((desc->Type == EfiReservedMemoryType) || (desc->Type == EfiUnusableMemory) || (desc->Type == EfiUnacceptedMemoryType)){goto end;}
        mapPages(desc->PhysicalStart,desc->PhysicalStart,(desc->NumberOfPages*EFI_PAGE_SIZE)/PAGE_SZ,true,false);
        end:
        desc = NEXT_DESC(desc,mmapSz);
    }
}

/*!
Intilizes mmap and allocation functions.
*/
void initMem(){
    //Init mmap
    sprint(L"Intilizing allocation mmap\n");
    mapMax = 40;
    allocMap = (mmapEntry*)allocPool(sizeof(mmapEntry)*mapMax); //Can be expanded later
    clearMem((void*)allocMap,sizeof(mmapEntry)*mapMax);
    sprint(L"Allocated mmap to 0x%lx\n",(uint64_t)allocMap);
    //Setup paging
    sprint(L"Initilizing paging structs\n");
    pml = (uint64_t*)pgAlloc();
    sprint(L"Allocated PML to 0x%lx\n",(uint64_t)pml);
    sprint(L"Identity mapping all memory\n");
    idMapMem();
}

/*!
Changes the CR3 from EFI's pml4e to polarboot's plm4e.
This function must only be called after exiting bootservices.
*/
void setCR3(){
    asm volatile("movq %%rax, %%cr3" ::"a"(pml));
}

/*!
This is a debugging function that tests if a given address causes a page fault.
'efi' specifies if EFI's pml or if polarboot's pml should be tested.
*/
void testPath(uint64_t addr, bool efi){
    addr = KALIGN(addr);
    uint64_t temp = 0x0;
    uint64_t *tpml = pml;
    if(efi == true){asm volatile("movq %%cr3, %0" : "=r"(temp)); tpml = (uint64_t*)temp;}
    sprint(L"TRACE : 0x%lx->",addr);
    //PML4E
    sprint(L"PML[");
    if(!(tpml[PML_ENTRY(addr)]&PG_PRESENT)){sprint(L"#PF]\n"); return;}
    sprint(L"0x%lx]->",tpml[PML_ENTRY(addr)]);
    //PDPE
    sprint(L"PDPE[");
    uint64_t *pdpe = (uint64_t*)TABLE_BASE(tpml[PML_ENTRY(addr)]);
    if(!(pdpe[PDPE_ENTRY(addr)]&PG_PRESENT)){sprint(L"#PF]\n"); return;}
    sprint(L"0x%lx]->",pdpe[PDPE_ENTRY(addr)]);
    //PDE
    sprint(L"PDE[");
    uint64_t *pde = (uint64_t*)TABLE_BASE(pdpe[PDPE_ENTRY(addr)]);
    if(!(pde[PDE_ENTRY(addr)]&PG_PRESENT)){sprint(L"#PF]\n"); return;}
    sprint(L"0x%lx]->",pde[PDE_ENTRY(addr)]);
    //PTE
    sprint(L"PTE[");
    uint64_t *pte = (uint64_t*)TABLE_BASE(pde[PDE_ENTRY(addr)]);
    if(!(pte[PTE_ENTRY(addr)]&PG_PRESENT)){sprint(L"#PF]\n"); return;}
    sprint(L"0x%lx]->0x%lx[",pte[PTE_ENTRY(addr)],TABLE_BASE(pte[PTE_ENTRY(addr)]));
    if(pte[PTE_ENTRY(addr)]&PG_WRITE){sprint(L"W");}else{sprint(L"R");}
    if(pte[PTE_ENTRY(addr)]&PG_USER){sprint(L"U");}else{sprint(L"S");}
    if(pte[PTE_ENTRY(addr)]&PG_NX){sprint(L"-");}else{sprint(L"X");}
    sprint(L"]\n");
}

mmapEntry *kmmap = NULL;

UINT16 *mtype[17] = {
    L"Reserved", L"efiLoaderCode", L"efiLoaderData",
    L"efiBootCode", L"efiBootData", L"efiRuntimeCode",
    L"efiRuntimeData", L"EfiConventionalMem", L"EfiUnusableMem",
    L"efiACPIReclaim", L"efiACPIMemNVS", L"efiMMIO",
    L"efiMMIOPort", L"efiPAL", L"efiPersistent",
    L"efiUnaccepted", L"efiMaxMemtype"
};

UINT16 *etype[MMAP_TYPE_MAX+1] = {
    L"IGNORE", L"FREE", L"UEFI", L"VESA",
    L"MMIO", L"ACPI", L"RESV",
    L"UNUSABLE", L"KERNEL", L"OTHER",
    L"ELF", L"PAGE", L"BOOT",
    L"PMM"
};

uint64_t convertMmapType(UINTN efiType){
    switch(efiType){
        case EfiReservedMemoryType:
        return MMAP_TYPE_RESV;
        break;
        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiConventionalMemory:
        return MMAP_TYPE_FREE;
        break;
        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
        return MMAP_TYPE_UEFI;
        break;
        case EfiACPIReclaimMemory:
        case EfiACPIMemoryNVS:
        return MMAP_TYPE_ACPI;
        break;
        case EfiMemoryMappedIO:
        case EfiMemoryMappedIOPortSpace:
        return MMAP_TYPE_MMIO;
        break;
        case EfiPalCode:
        case EfiPersistentMemory:
        case EfiUnacceptedMemoryType:
        case EfiMaxMemoryType:
        default:
        return MMAP_TYPE_OTHER;
        break;
    }
    return MMAP_TYPE_OTHER;
}

/*
    Makes code look a little nicer
*/
static inline void writeKmmapEntry(UINTN mp, EFI_MEMORY_DESCRIPTOR *desc){
    kmmap[mp].type = convertMmapType(desc->Type);
    kmmap[mp].phys = desc->PhysicalStart;
    kmmap[mp].virt = desc->VirtualStart;
    kmmap[mp].bytes = desc->NumberOfPages*EFI_PAGE_SIZE;
    kmmap[mp].magic = MMAP_MAGIC;
}

/*
    Returns true if merged into existing mmap entry.
*/
bool mergeDescToMMAP(UINTN mp, EFI_MEMORY_DESCRIPTOR *desc){
    uint64_t type = convertMmapType(desc->Type);
    uint64_t dMax = desc->PhysicalStart+(desc->NumberOfPages*EFI_PAGE_SIZE);
    if(desc->Type == EfiLoaderCode){
        mapPages(desc->PhysicalStart,desc->PhysicalStart,(desc->NumberOfPages*EFI_PAGE_SIZE)/PAGE_SZ,false,true);
    }
    if(desc->Type == EfiLoaderData){
        mapPages(desc->PhysicalStart,desc->PhysicalStart,(desc->NumberOfPages*EFI_PAGE_SIZE)/PAGE_SZ,true,false);
    }
    for(UINTN z = 0; z < mp; ++z){
        //Check if type matches
        if(kmmap[z].type != type){continue;}
        if(kmmap[z].phys > desc->PhysicalStart){continue;}
        if((kmmap[z].phys+kmmap[z].bytes+(desc->NumberOfPages*EFI_PAGE_SIZE) < dMax)){continue;}
        //Merge entry
        kmmap[mp].bytes += (desc->NumberOfPages*EFI_PAGE_SIZE);
        //sprint(L"(MERGE)->[0x%lx]->MMAP entry[%s][Phys: 0x%lx][Virt: 0x%lx][0x%lx pages][A: 0x%lx][Max: 0x%lx]\n",mp,mtype[desc->Type],desc->PhysicalStart,desc->VirtualStart,desc->NumberOfPages,desc->Attribute,desc->PhysicalStart+(desc->NumberOfPages*EFI_PAGE_SIZE)+EFI_PAGE_SIZE);
        return true;
    }
    return false;
}

/*!
    Merges kmmap entries that overlap with the same types
*/
void mergeKmmapEntry(UINTN mp, mmapEntry *entry){
    if(entry->type == MMAP_TYPE_IGNORE){return;}
    uint64_t eMax = entry->phys+entry->bytes;
    for(UINTN z = 0; z < mp; ++z){
        if(&kmmap[z] == entry){continue;}
        uint64_t kMax = kmmap[z].phys+kmmap[z].bytes;
        if(kmmap[z].type != entry->type){continue;}
        //Check for overlap
        if((eMax >= kmmap[z].phys) && (entry->phys < kmmap[z].phys)){goto merge;}
        if((kMax >= entry->phys) && (kmmap[z].phys < entry->phys)){goto merge;}
        continue;
        merge:
        kmmap[z].bytes = entry->bytes+kmmap[mp].bytes;
        entry->type = MMAP_TYPE_IGNORE;
        kmmap[z].phys = (kmmap[z].phys < entry->phys) ? kmmap[z].phys : entry->phys;
        return;
    }
}

/*!
Constructs the kernel's memory map.
This should be called once the bootloader is ready to handover control to the kernel.
No further memory allocations should be made after this is called.
*/
EFI_STATUS initMMAP(){
    mmapStart = getMmap(&mmapEntries,&mmapKey,&mmapSz,&mmapVer);
    if(mmapStart == NULL){sprint(L"LibMemoryMap() failed\n"); return !EFI_SUCCESS;}
    kmmap = (mmapEntry*)allocMem(sizeof(mmapEntry)*(mmapEntries+(allocEntries*2)+5),0x0,MMAP_TYPE_OTHER,true,true,true);
    if(kmmap == NULL){sprint(L"AllocatePool() failed\n"); return !EFI_SUCCESS;}
    mmapStart = getMmap(&mmapEntries,&mmapKey,&mmapSz,&mmapVer);
    if(mmapStart == NULL){sprint(L"LibMemoryMap() failed\n"); return !EFI_SUCCESS;}
    EFI_MEMORY_DESCRIPTOR *desc = mmapStart;
    UINTN mp = 0;
    writeKmmapEntry(mp,desc);
    if(desc->Type == EfiLoaderCode){
        mapPages(desc->PhysicalStart,desc->PhysicalStart,(desc->NumberOfPages*EFI_PAGE_SIZE)/PAGE_SZ,false,true);
    }
    if(desc->Type == EfiLoaderData){
        mapPages(desc->PhysicalStart,desc->PhysicalStart,(desc->NumberOfPages*EFI_PAGE_SIZE)/PAGE_SZ,true,false);
    }
    desc = NEXT_DESC(desc,mmapSz);
    for(UINTN z = 0; (z+1) < mmapEntries; ++z){
        if(!mergeDescToMMAP(mp,desc)){
            ++mp;
            writeKmmapEntry(mp,desc);
        }
        desc = NEXT_DESC(desc,mmapSz);
    }
    //Merge loose entries
    for(UINTN c = 0; c < mp; ++c){
        mergeKmmapEntry(mp,&kmmap[c]);
    }
    //Dump mmap
    for(UINTN z = 0; z < mp; ++z){
        if(kmmap[z].type == MMAP_TYPE_IGNORE){continue;}
        sprint(L"MMAP[%s][Phys: 0x%lx][Virt: 0x%lx][Bytes: 0x%lx][Max: 0x%lx]\n",etype[kmmap[z].type],kmmap[z].phys,kmmap[z].virt,kmmap[z].bytes,kmmap[z].bytes+kmmap[z].phys);
    }
    sprint(L"#========\n");
    for(UINTN z = 0; z < allocEntries; ++z){
        sprint(L"AMAP[%s][Phys 0x%lx][Virt 0x%lx][Bytes 0x%lx][Max 0x%lx]\n",etype[allocMap[z].type],allocMap[z].phys,allocMap[z].virt,allocMap[z].bytes,allocMap[z].phys+allocMap[z].bytes);
    }
    kinf.mem.aptr = (uint64_t)allocMap;
    kinf.mem.aentries = allocEntries;
    kinf.mem.entries = mp;
    kinf.mem.ptr = (uint64_t)kmmap;
    return EFI_SUCCESS;
}