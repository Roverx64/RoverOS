#include <efi.h>
#include <efilib.h>
#include "boot.h"
#include "bootPaging.h"
#include "mmap.h"
#include "bootMmap.h"

uint64 *bpml = NULL;
uint64 slabStart;
uint64 slabSize;
uint64 slabOffset;

uint64 allocTable(){
    if(slabOffset >= slabSize){Print(L"SLAB : Out of space\n"); for(;;){asm("hlt");}}
    uint64 ptr = slabStart+slabOffset;
    slabOffset += sizeof(uint64)*512;
    ZeroMem((void*)ptr,sizeof(uint64)*512);
    return ptr;
}

EFI_STATUS mapPage(uint64 phys, uint64 virt, bool write, bool user, bool execute){
    phys = KALIGN(phys);
    virt = KALIGN(virt);
    //PDPE
    uint64 pmln = PML_ENTRY(virt);
    if(TABLE_BASE(bpml[pmln]) == 0x0){bpml[pmln] |= TABLE_BASE(allocTable());}
    uint64 *pdpe = (uint64*)TABLE_BASE(bpml[pmln]);
    //PDE
    uint64 pdpen = PDPE_ENTRY(virt);
    if(TABLE_BASE(pdpe[pdpen]) == 0x0){pdpe[pdpen] |= TABLE_BASE(allocTable());}
    uint64 *pde = (uint64*)TABLE_BASE(pdpe[pdpen]);
    //PTE
    uint64 pden = PDE_ENTRY(virt);
    if(TABLE_BASE(pde[pden]) == 0x0){pde[pden] |= TABLE_BASE(allocTable());}
    uint64 *pte = (uint64*)TABLE_BASE(pde[pden]);
    //Flags
    uint64 flags = PG_PRESENT;
    if(write){flags |= PG_WRITE;}
    bpml[pmln] |= flags;
    pdpe[pdpen] |= flags;
    pde[pden] |= flags;
    if(user){flags |= PG_USER;}
    if(execute == false){flags |= PG_NX;}
    pte[PTE_ENTRY(virt)] = (flags|phys);
    return EFI_SUCCESS;
}

EFI_STATUS mapPages(uint64 phys, uint64 virt, uint32 n, bool write, bool user, bool execute){
    if(n == 0){n = 1;}
    for(int i = 0; i < n; ++i){
        if(mapPage(phys+(i*PAGE_SZ),virt+(i*PAGE_SZ),write,user,execute) != EFI_SUCCESS){return !EFI_SUCCESS;}
    }
    return EFI_SUCCESS;
}

EFI_STATUS initPaging(){
    slabSize = (0x200000*8)+PAGE_SZ; //4MB slab
    slabStart = (uint64)AllocatePool(slabSize);
    slabStart = KALIGN(slabStart+PAGE_SZ); //Align slab
    slabOffset = 0x0;
    Print(L"PAGE  : Allocated 0x%lx byte slab at 0x%lx\n",slabSize,slabStart);
    bpml = (uint64*)allocTable();
    Print(L"PAGE  : PML at 0x%lx\n",(uint64)bpml);
    mapPages(slabStart,slabStart,slabSize/PAGE_SZ,true,false,false);
    Print(L"PAGE  : Identity mapped bpml\n");
    uint64 stack = 0x0;
    asm volatile("movq %%rbp, %0" : "=r"(stack));
    mapPage(stack,stack,true,false,false);
    Print(L"PAGE  : Identity mapped the stack at 0x%lx\n",stack);
    return EFI_SUCCESS;
}

void testPath(uint64 addr, bool efi){
    addr = KALIGN(addr);
    uint64 temp = 0x0;
    uint64 *tpml = bpml;
    if(efi){asm volatile("movq %%cr3, %0" : "=r"(temp)); tpml = (uint64*)temp;}
    Print(L"TRACE : ->");
    if((tpml[PML_ENTRY(addr)]&PG_PRESENT) == 0x0){Print(L"[PML #PF]\n"); return;}
    Print(L"[0x%x][0x%lx]->",PML_ENTRY(addr),tpml[PML_ENTRY(addr)]);

    uint64 *pdpe = (uint64*)TABLE_BASE(tpml[PML_ENTRY(addr)]);
    if(pdpe == 0x0){Print(L"[PDPE #PF]\n"); return;}
    if((pdpe[PDPE_ENTRY(addr)]&PG_PRESENT) == 0x0){Print(L"[PDPE #PF]\n"); return;}
    Print(L"[0x%x][0x%lx]->",PDPE_ENTRY(addr),pdpe[PDPE_ENTRY(addr)]);

    uint64 *pde = (uint64*)TABLE_BASE(pdpe[PDPE_ENTRY(addr)]);
    if(pde == 0x0){Print(L"[PDE #PF]\n"); return;}
    if((pde[PDE_ENTRY(addr)]&PG_PRESENT) == 0x0){Print(L"[PDE #PF]\n"); return;}
    Print(L"[0x%x][0x%lx]->",PDE_ENTRY(addr),pde[PDE_ENTRY(addr)]);

    uint64 *pte = (uint64*)TABLE_BASE(pde[PDE_ENTRY(addr)]);
    if(pte == 0x0){Print(L"[PTE-N #PF]\n"); return;}
    Print(L"[0x%x][0x%lx]->0x%lx[",PTE_ENTRY(addr),pte[PTE_ENTRY(addr)],TABLE_BASE(pte[PTE_ENTRY(addr)]));
    if(pte[PTE_ENTRY(addr)]&PG_PRESENT){Print(L"P");}else{Print(L"-");}
    if(pte[PTE_ENTRY(addr)]&PG_WRITE){Print(L"W");}else{Print(L"R");}
    if(pte[PTE_ENTRY(addr)]&PG_USER){Print(L"U");}else{Print(L"S");}
    if(pte[PTE_ENTRY(addr)]&PG_NX){Print(L"-");}else{Print(L"X");}
    Print(L"]\n");
}

void setCR3(){
    asm volatile("movq %%rax, %%cr3" ::"a"(bpml));
}

EFI_MEMORY_DESCRIPTOR *mmapStart = NULL;
UINTN mmapEntries = 0;
UINTN mmapKey = 0;
UINTN mmapSz = 0;
UINT32 mmapVer = 0;
mmapEntry *kmmap = NULL;

//Map required pages and construct kernel mmap
//TODO: Mark where the kernel is in the MMAP
EFI_STATUS initMMAP(){
    mmapStart = LibMemoryMap(&mmapEntries,&mmapKey,&mmapSz,&mmapVer);
    if(mmapStart == NULL){Print(L"MMAP  : LibMemoryMap() failed\n"); return !EFI_SUCCESS;}
    kmmap = (mmapEntry*)AllocatePool(sizeof(mmapEntry)*(mmapEntries+5));
    //Print(L"0x%lx ent\n",mmapEntries);
    if(kmmap == NULL){Print(L"MMAP  : AllocatePool() failed\n"); return !EFI_SUCCESS;}
    mapPages(KALIGN((uint64)kmmap),KALIGN((uint64)kmmap),(sizeof(mmapEntry)*(mmapEntries+5))/PAGE_SZ,true,false,false);
    mmapStart = LibMemoryMap(&mmapEntries,&mmapKey,&mmapSz,&mmapVer);
    if(mmapStart == NULL){Print(L"MMAP  : LibMemoryMap() failed\n"); return !EFI_SUCCESS;}
    EFI_MEMORY_DESCRIPTOR *desc = mmapStart;
    uint64 usableMemory = 0x0;
    uint64 total = 0x0;
    uint64 mp = 0x0;
    bool map = false;
    bool write = false;
    bool exec = false;
    uint64 mmioMem = 0;
    uint64 efiMem = 0;
    uint64 cvMem = 0;
    uint64 unMem = 0;
    uint64 acpiMem = 0;
    for(int i = 0; i+1 < mmapEntries; ++i){
        kmmap[mp].magic = MMAP_MAGIC;
        switch(desc->Type){
            case EfiMemoryMappedIOPortSpace:
            case EfiMemoryMappedIO:
            mmioMem += DESC_SZ(desc);
            kmmap[mp].type = MMAP_TYPE_MMIO;
            break;
            case EfiBootServicesCode:
            case EfiBootServicesData:
            efiMem += DESC_SZ(desc);
            cvMem -= DESC_SZ(desc);
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
            map = true; write = false; exec = true;
            kmmap[mp].type = MMAP_TYPE_FREE;
            usableMemory += DESC_SZ(desc);
            break;
            case EfiLoaderData:
            efiMem += DESC_SZ(desc);
            kmmap[mp].type = MMAP_TYPE_UEFI;
            usableMemory += DESC_SZ(desc);
            break;
            case EfiACPIReclaimMemory:
            case EfiPalCode:
            case EfiACPIMemoryNVS:
            acpiMem += DESC_SZ(desc);
            kmmap[mp].type = MMAP_TYPE_ACPI;
            break;
        }
        if(map){mapPages(desc->PhysicalStart,desc->PhysicalStart,desc->NumberOfPages,write,false,exec); map = false;}
        kmmap[mp].phys = desc->PhysicalStart;
        kmmap[mp].bytes = DESC_SZ(desc);
        cont:
        //Print(L"[%x|0x%x|0x%x|0x%lx]\n",desc->Type,DESC_SZ(desc),mp,desc->PhysicalStart);
        total += DESC_SZ(desc);
        ++mp;
        desc = NEXT_DESC(desc,mmapSz);
    }
    //Print(L"MMIO: 0x%lx\n",mmioMem);
    //Print(L"EFI: 0x%lx\n",efiMem);
    //Print(L"CV: 0x%lx\n",cvMem);
    //Print(L"UN: 0x%lx\n",unMem);
    //Print(L"ACPI: 0x%lx\n",acpiMem);
    kinf.mem.ptr = (void*)kmmap;
    kinf.mem.entries = mp;
    kinf.mem.totalMem = total-mmioMem;
    kinf.mem.usableMem = usableMemory;
    Print(L"MMAP  : At 0x%lx [0x%x|T: 0x%lx|U: 0x%lx]\n",(uint64)kinf.mem.ptr,mp,total-mmioMem,usableMemory);
    return EFI_SUCCESS;
}