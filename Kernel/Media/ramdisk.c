#include <stdint.h>
#include <string.h>
#include <kprint.h>
#include <ramdisk.h>
#include <kernel.h>
#include <kmalloc.h>
#include <vfs.h>
#include "bootinfo.h"

//Required VFS functions
uint64_t ramdiskPbase; //Physical base address
uint64_t ramdiskVbase; //Virtual base address

size_t ramdiskReadFile(size_t sz, struct vfsFile *fl, size_t offset, void *dst){
    //Convert phys ptr to virt ptr
    uint64_t flptr = (fl->phys-ramdiskPbase)+ramdiskVbase+offset;
    size_t rin = 0;
    uint8_t *s = (uint8_t*)flptr;
    uint8_t *d = (uint8_t*)dst;
    while((offset+rin) < fl->size){
        d[rin] = s[rin];
        ++rin;
    }
    return rin;
}

void initRamdisk(struct bootInfo *kinf){
    //Parse ramdisk and initilize files
    kinfo("Intilizing ramdisk (0x%lx)\n",kinf->rdptr);
    if(kinf->rdptr == 0x0){kwarn("NULL ramdisk\n");}
    uint64_t physPtr = kinf->rdptr;
    uint64_t virtPtr = (uint64_t)vmallocPhys(kinf->rdptr,kinf->rdsz,VTYPE_FILE,VFLAG_MAKE);
    ramdiskVbase = virtPtr;
    ramdiskPbase = physPtr;
    KASSERT((virtPtr != 0x0),"Failed to allocate virt space for ramdisk");
    kinfo("Mapped 0x%lx->0x%lx\n",physPtr,virtPtr);
    struct ramdiskHeader *master = (struct ramdiskHeader*)virtPtr;
    if(master->magic != RD_HEADER_MAGIC){kerror("Bad ramdisk magic (0x%x)\n",master->magic); return;}
    struct ramdiskFile *file = (struct ramdiskFile*)(virtPtr+sizeof(struct ramdiskHeader));
    struct vfsMountpoint *mount = mountVFS(VFS_MOUNT_RAM,master->files,'R',VFS_FLAG_NO_UNLOAD,&ramdiskReadFile);
    if(!mount){kerror("Failed to mount ramdisk to R\n"); return;}
    physPtr += sizeof(struct ramdiskHeader);
    for(uint32_t i = 0; i < master->files; ++i){
        if(file->magic != RD_FILE_MAGIC){kerror("Invalid file magic (0x%x)\n",file->magic); break;}
        char *name = RD_FILE_NAME(file);
        uint32_t len = strlen(name);
        //Ramdisk should be identity mapped
        addVFSFile(mount,name,(uint64_t)RD_FILE_DATA(physPtr,len),file->size,VFS_FLAG_NO_UNLOAD);
        kinfo("Loaded file %s (0x%lx bytes) to VFS\n",name,file->size);
        file = (struct ramdiskFile*)RD_NEXT_FILE(file,file->size,len);
        physPtr = (uint64_t)RD_NEXT_FILE(physPtr,file->size,len);
    }
    kinfo("Initilized ramdisk\n");
}

//172896 | 2075046

/*

root = /

//Theoretical directories
//Note: all programs will be sandboxed to their directories unless granted permission to other directories
//Note2: Impliment tag based searching as well

/user/<username>/
               +/programs/<program name>/
                                        +/data
                                        +/bin
                                        +/documents
               +/documents
               +/media
               +/downloads
/sys/
   +/bin/
        +/boot
        +/drivers/
                 +/<driver>.kdm
        +/libs
        +/cmd
        +/headers
   +/resources/
             +/media
             +/fonts
   +/sandbox/
            +/<program name>/
                            +/permissions.lst (Directories/Files that the program may modify)
                            +/modify.lst (Files modified by the program)
   +/programs/ (System programs I.E file manager)
             +/<program name>/
/dev/ (NOTE: probably redo how devices are done)
    +/storage/
            +/<st#>
    +/serial/
            +/<sr#>
    +/PCI/
         +/<gpu#>
         +/xHCI
         +/aHCI
    +/Misc/
          +/<cpu#>
          +/<HPET#>
          +/<lapic#>

*/
