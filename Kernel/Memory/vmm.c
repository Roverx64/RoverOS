#include <stdint.h>
#include <pmm.h>
#include <kprint.h>
#include <paging.h>
#include <vmm.h>
#include <task.h>
#include <kernel.h>
#include <kcache.h>
#include <kmalloc.h>
#include <pointer.h>
#include <dstack.h>

//Design inspiration from dreamportdev

extern uint64_t newPagetable(); //paging.c
extern kcache *createVMarkerCache(char *name, size_t objSize, uint16_t align, uint8_t type, uint64_t flags, kcacheConstructor *constructor, kcacheDestructor *destructor);

kcache *vmCache;
kcache *markerCache;

kcacheConstructor *vmmConstructor(){

}

kcacheDestructor *vmmDestructor(){

}

char *vtypeToString[VTYPE_MAX_TYPE+1] = {
    "FREE", "HEAP", "CODE",
    "DATA", "STACK", "OTHER",
    "USER", "LCODE", "LDATA",
    "VIMEM", "DEV", "FILE"
};


extern uint64_t __kernelStart;
extern uint64_t __kernelEnd;
extern uint64_t __heapStart;
extern uint64_t __heapEnd;

dstack *physStack;

void extendStack(){
    uint64_t sz = 0x0;
    uint64_t ptr = allocPhys(PAGE_SZ*10,MEMORY_TYPE_ANY,0,&sz);
    KASSERT((ptr != MEMORY_ALLOCATION_FAILED),"OOM\n");
    for(size_t i = 0; i < sz; i += PAGE_SZ){
        pushStack(physStack,ptr+i);
    }
}

uint64_t getPhysmem(){
    retry:
    bool f = false;
    uint64_t addr = popStack(physStack,&f);
    if(!f){extendStack(); goto retry;}
    //kinfo("Returning 0x%lx\n",addr);
    return addr;
}

//NOTE: Currently broken
uint64_t prefferedMinAddress(uint8_t mtype, uint32_t flags){
    //Avoid 1MB memory
    if(flags&VFLAG_KERNEL){
        if(mtype == VTYPE_HEAP){
            return (uint64_t)&__heapEnd; //Allocate at the end of the pre-allocated heap region
        }
    }
    return MIN_ADDR_LOW; //Avoid allocating in the 1MB region
}

/*!
    !D Create SLAB allocator for backing phys mem to virt mem
    !R None
*/
void initVmalloc(){
    markerCache = createVMarkerCache("vm_marker_cache",sizeof(vMarker),0,0,0,vmmConstructor,vmmDestructor);
    KASSERT((markerCache != NULL),"Failed to create vMarker kcache");
    physStack = createDstack(sizeof(uint64_t)*100);
    KASSERT((physStack != NULL),"Failed to create data stack\n");
    extendStack();
    kinfo("Created vmCache\n");
}

static uint64_t formFlags(uint32_t flags){
    uint64_t pf = 0;
    if(flags&VFLAG_WRITE){pf |= PAGE_FLAG_WRITE;}
    if(flags&VFLAG_EXEC){pf |= PAGE_FLAG_EXECUTE;}
    if(flags&VFLAG_USER){pf |= PAGE_FLAG_USER;}
    if(flags&VFLAG_UC){pf |= PAGE_FLAG_UC;}
    if(flags&VFLAG_MAKE){pf |= PAGE_FLAG_MAKE;}
    if(flags&VFLAG_WT){pf |= PAGE_FLAG_WT;}
    if(flags&VFLAG_WC){pf |= PAGE_FLAG_WC;}
    return pf;
}

vMarker *allocMarker(vMarker *head, size_t bytes, uint8_t type, uint32_t flags){
    uint32_t pages = bytes/PAGE_SZ;
    if(!pages){pages = 1;}
    size_t sz = pages*PAGE_SZ;
    vMarker *end = head;
    uint64_t minAddr = prefferedMinAddress(type,flags);
    //Find space between/after markers
    while(head){
        if(!head->next){head = NULL; break;}
        vMarker *n = (vMarker*)head->next;
        if(n->base > minAddr){goto nxt;} //Cannot allocate here
        if(head->base <= minAddr){goto nxt;}
        if((n->base-VMARKER_END(head)) >= sz){
            break;
        }
        nxt:
        head = n;
        end = n;
    }
    vMarker *marker = NULL;
    marker = (vMarker*)kcacheAlloc(markerCache,0);
    marker->pages = pages;
    marker->flags = flags;
    marker->type = type;
    if(!head){ //I.E no free space between marker
        marker->base = VMARKER_END(end);
        marker->next = NULL;
        end->next = (void*)marker;
    }
    else{ //Insert marker
        marker->base = VMARKER_END(head)+PAGE_SZ;
        marker->next = head->next;
        head->next = (void*)marker;
    }
    return marker;
}

/*!
    !D Allocates virtual memory and maps it to physical memory
    !R Pointer to virtual address on success or NULL on failure
*/
void *vmalloc(size_t bytes, uint8_t type, uint32_t flags){
    struct processTicket *proc = getCurrentTask();
    if(!proc){kerror("NULL proc\n"); return NULL;}
    vMarker *head = proc->vmem;
    if(!head){kerror("NULL head\n"); return NULL;}
    vMarker *marker = allocMarker(head,bytes,type,flags);
    if(!marker){kerror("NULL marker\n"); return NULL;}
    //Allocate phys memory to back the new allocation
    uint64_t virt = marker->base;
    uint32_t pages = bytes/PAGE_SZ;
    if(!pages){pages = 1;}
    for(uint32_t i = 0; i <= pages; ++i){
        uint64_t phys = getPhysmem();
        if(phys == MEMORY_ALLOCATION_FAILED){kpanic("Incomplete backing\n",0);}
        mapPage(proc->space,phys,virt+(i*PAGE_SZ),formFlags(flags));
    }
    return (void*)virt;
}

/*!
    !D Allocates virtual memory and maps it to specified phys address
    !R Pointer to virtual address on success or NULL on failure
*/
void *vmallocPhys(uint64_t phys, size_t bytes, uint8_t type,  uint32_t flags){
    struct processTicket *proc = getCurrentTask();
    if(!proc){kerror("NULL proc\n"); return NULL;}
    vMarker *head = proc->vmem;
    if(!head){kerror("NULL head\n"); return NULL;}
    vMarker *marker = allocMarker(head,bytes,type,flags);
    if(!marker){kerror("NULL marker\n"); return NULL;}
    uint64_t virt = marker->base;
    uint32_t pages = bytes/PAGE_SZ;
    if(!pages){pages = 1;}
    mapPages(proc->space,phys,virt,formFlags(flags),pages);
    return (void*)virt;
}

void vmFree(void *virt){
    uint64_t v = (uint64_t)virt;
    if(v == 0x0){return;}
    //Find block
}

/*!
    !D  Creates a blank pml4e and pageSpace.
    !R pageSpace on success, NULL on failure
*/
pageSpace *createPagespace(){
    pageSpace *space = (pageSpace*)kmalloc(sizeof(pageSpace));
    KASSERT((space != NULL),"kmalloc() failed");
    space->pml4e = (uint64_t*)newPagetable();
    KASSERT((space->pml4e != NULL),"kmalloc() failed");
    return space;
}

extern uint64_t virtToPhysInt(pageSpace *space, uint64_t virt);

/*!
    !D Converts a virtual to physical address
    !I virt: Virtual address to convert
    !R physical address on success, NULL on failure
*/
uint64_t virtToPhys(uint64_t virt){
    struct processTicket *tk = getCurrentTask();
    KASSERT((tk != NULL),"NULL ticket");
    KASSERT((tk->space != NULL),"NULL pageSpace");
    return virtToPhysInt(tk->space,virt);
}

void printVFlags(vMarker *mark){
    if(mark->flags&VFLAG_WRITE){knone("W");}else{knone("-");}
    if(mark->flags&VFLAG_EXEC){knone("X");}else{knone("-");}
    if(mark->flags&VFLAG_UC){knone("U");}else{knone("-");}
    if(mark->flags&VFLAG_MAKE){knone("M");}else{knone("-");}
    if(mark->flags&VFLAG_WT){knone("T");}else{knone("-");}
    if(mark->flags&VFLAG_KERNEL){knone("K");}else{knone("-");}
    if(mark->flags&VFLAG_WC){knone("C");}else{knone("-");}
}

void dumpVMarker(vMarker *head){
    while(head){
        knone("[0x%lx->0x%lx|0x%x pages|%s|",head->base,VMARKER_END(head),head->pages,vtypeToString[head->type]);
        printVFlags(head);
        knone("]\n");
        head = (vMarker*)head->next;
    }
}