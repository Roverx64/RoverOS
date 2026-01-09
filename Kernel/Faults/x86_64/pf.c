#include <stdint.h>
#include <kprint.h>
#include <cpu.h>
#include <paging.h>
#include <kmalloc.h>
#include <fdebug.h>
#include <task.h>
#include <vmm.h>

void pageFault(registers *reg){
    knone("#=============Page=Fault=============#\n");
    uint64_t faultAddr = 0x0;
    uint64_t cr3 = 0x0;
    asm volatile("movq %%cr3, %0" : "=r"(cr3));
    asm volatile("movq %%cr2, %0" : "=r"(faultAddr));
    knone("ADDR=0x%lx\n",faultAddr);
    uint8_t present = reg->ec&(1<<0);
    uint8_t write = reg->ec&(1<<1);
    uint8_t user = reg->ec&(1<<2);
    uint8_t resv = reg->ec&(1<<3);
    uint8_t fetch = reg->ec&(1<<4);
    knone("CR3=0x%lx\n",cr3);
    knone("CPL=");
    if(!user){knone("Supervisor\n");}
    else{knone("User\n");}
    uint64_t page = getPageInfo(cr3,faultAddr,false);
    if(page == 0x0){knone("Page not found in cr3\n"); goto cause;}
    knone("RAW=0x%lx\n",page);
    knone("PHYS=0x%lx\n",TABLE_BASE(page));
    knone("RIP=0x%lx\n",reg->rip);
    knone("FLAGS=");
    if(page&PG_PRESENT){knone("P");}else{knone("-");}
    if(page&PG_WRITE){knone("W");}else{knone("R");}
    if(page&PG_USER){knone("U");}else{knone("S");}
    if(page&PG_PWT){knone("T");}else{knone("-");}
    if(page&PG_PCD){knone("C");}else{knone("-");}
    if(page&PG_ACCESSED){knone("A");}else{knone("-");}
    if(page&PG_DIRTY){knone("D");}else{knone("-");}
    if(page&PG_GLOBAL){knone("G");}else{knone("-");}
    if(page&PG_PAT){knone("O");}else{knone("-");}
    if(page&PG_NX){knone("-");}else{knone("X");}
    knone("\n");
    cause:
    //Determine cause
    knone("Cause: ");
    if(resv){
        knone("Wrote 1 to reserved bit(s)\n");
        goto proc;
    }
    if(page == 0x0){
        knone("Non-existent page\n");
        goto proc;
    }
    if(!present){
        if(write){knone( "Wrote to ");}
        else{knone("Read from ");}
        knone("unmapped page\n");
        goto proc;
    }
    if(present && write && !(page&PG_WRITE)){
        knone("Wrote to read-only page\n");
        goto proc;
    }
    if(fetch){
        knone("Instruction fetch\n");
    }
    proc:
    //Walk page table
    knone("#===Page=Walk===#\n");
    uint64_t rawpe = getPageInfo(cr3,faultAddr,true);
    knone("#===============#\n");
    knone("#===Virtual=Mem===#\n");
    dumpVMarker(currentProcessVMarker());
    knone("#===============#\n");
    //Check mmap
    //TODO: Make this less complex
    if(rawpe != 0x0){
        uint64_t phys = TABLE_BASE(rawpe);
        struct mmapEntry *mentry = getMmapEntry(phys);
        struct mmapEntry *aentry = getAllocMapEntry(phys);
        if((mentry == NULL) && (aentry == NULL)){knone("No relevant mmap entry found\n"); goto cont;}
        if(aentry != NULL){mentry = aentry; knone("[ALLOC]");} //Allocation map is more important than the general mmap
        knone("[MMAP|%s|0x%lx->0x%lx|",mmapTypeString[mentry->type],mentry->phys,mentry->virt);
        if(mentry->flags&MMAP_FLAG_ALIGN){knone("A");}else{knone("-");}
        if(mentry->flags&MMAP_FLAG_PARSED){knone("P");}else{knone("-");}
        knone("|0x%lx bytes]\n",mentry->bytes);
        knone("#===============#\n");
        dumpMmap();
    }
    cont:
    //Print procmap info and dump kmalloc() chain if cpl=0 and PROC_HEAP type
    end:
    dumpRegisters(reg);
    //if((type == PROC_HEAP) && !user && (ent != NULL)){
    //    dumpKmallocChain(faultAddr);
    //}
    for(;;){asm("hlt");}
}