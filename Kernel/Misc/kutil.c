#include <stdint.h>
#include <kernel.h>
#include <kprint.h>
#include <fdebug.h>
#include <vmm.h>
#include <task.h>

void kpanicInt(const char *func, const char *file, const uint64_t line, const char *str, uint64_t ex){
    asm volatile("cli");
    knone("!!!#=========KERNEL=PANIC=========#!!!\n");
    knone("Cause: %s\n",str);
    knone("File: %s\n",file);
    knone("Caller: %s\n",func);
    knone("Line: 0x%lx\n",line);
    knone("EX: 0x%lx\n",ex);
    knone("!!!#==============================#!!!\n");
    uint64_t rbp;
    asm volatile("movq %%rbp, %0" : "=r"(rbp));
    dumpVMarker(currentProcessVMarker());
    stackTrace(rbp);
    for(;;){asm volatile("hlt");}
}

void kassertInt(const char *func, const char *file, const uint64_t line, int expression, const char *fail){
    if(expression){return;}
    kpanicInt(func,file,line,fail,0);
}