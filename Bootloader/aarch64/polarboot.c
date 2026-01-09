#include <stdint.h>
#include <uart.h>

extern bool parseTree(uint64_t *dtb);
typedef (*kfunc)(struct bootInfo*);
struct bootInfo kinf;

void bootstrap(uint64_t *dtb, uint64_t heap, uint64_t memsz, uint64_t loaderAddr, uint64_t loaderSize, uint64_t ramdisk){
    if(!initMem(heap,memsz,loaderAddr,loaderSize)){goto end;}
    //Parse device tree
    if(!parseTree(dtb)){goto end;}
    //Setup paging
    //Get kernel ELF from ramdisk
    //Load ELF
    uint64_t addr = loadKernel();
    //Jump
    end:
    for(;;){asm volatile("wfi");}
}