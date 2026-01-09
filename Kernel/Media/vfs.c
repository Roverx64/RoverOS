#include <stdint.h>
#include <stdbool.h>
#include <kernel.h>
#include <kmalloc.h>
#include <hashtable.h>
#include <kprint.h>
#include <kcache.h>
#include <bootinfo.h>
#include <ramdisk.h>
#include <string.h>
#include <vfs.h>

struct vfsRoot root;
struct vfsMountpoint **mount;
uint32_t slots = 0;
uint32_t devices = 0;

extern void initRamdisk(struct bootInfo *kinf);

/*!
    Initilizes RoverOS' VFS.
*/
void initVFS(struct bootInfo *kinf){
    kinfo("Initilizing VFS\n");
    mount = (struct vfsMountpoint**)kmalloc(sizeof(struct vfsMountpoint*)*2);
    //Create root mount
    root.mounted = 0;
    root.children = hashtableCreate(200);
    root.childCount = 0;
    devices = 0;
    slots = 1;
    initRamdisk(kinf);
    kinfo("Initilized VFS\n");
}

struct vfsMountpoint *mountVFS(uint8_t type, uint32_t childCount, char letter, uint16_t flags, vfsDriverReadFile *mfread){
    struct vfsMountpoint *mnt = (struct vfsMountpoint *)kmalloc(sizeof(struct vfsMountpoint));
    mnt->table = hashtableCreate((childCount == 0 ? 1 : childCount));
    mnt->childCount = 0;
    mnt->letter = letter;
    mnt->type = type;
    mnt->header.type = VFS_TYPE_MOUNT;
    mnt->header.flags = flags;
    mnt->header.length = sizeof(struct vfsMountpoint);
    mnt->mfread;
    hashtableInsert(root.children,(uint64_t)mnt,&letter,sizeof(char),0,false);
    kinfo("Mounted new VFS to '%c'\n",letter);
    return mnt;
}

/*!
    Adds a file to a specified parent.
    Parent may be a mount or directory type.
    Returns true on success
*/
bool addVFSFile(void *parent, char *name, uint64_t phys, uint32_t size, uint16_t flags){
    struct vfsHeader *header =  (struct vfsHeader*)parent;
    struct vfsFile *fl = (struct vfsFile*)kmalloc(sizeof(struct vfsFile));
    fl->name = name;
    fl->phys = phys;
    fl->size = size;
    fl->references = 0;
    fl->header.type = VFS_TYPE_FILE;
    fl->header.length = sizeof(struct vfsFile);
    fl->header.flags = flags;
    if(header->type == VFS_TYPE_MOUNT){
        struct vfsMountpoint *mnt = (struct vfsMountpoint *)parent;
        hashtableInsert(mnt->table,(uint64_t)fl,name,strlen(name),0,false);
        return true;
    }
    else if(header->type == VFS_TYPE_DIR){
        
    }
    return false;
}

/*!
    Returns 0 at the end of the string
*/
uint32_t pathNameLength(char *str, uint32_t base){
    uint32_t i = 0;
    while(str[base] != '\0'){
        if(str[base] == '/'){break;}
        ++base;
        ++i;
    }
    return i;
}

/*!
    Returns VFS structure at the end of path.
    Sets mnt to mount point. NULL if path is on root
    Returns NULL if path is invalid
*/
void *parsePath(char *path, struct vfsMountpoint **mnt){
    uint32_t base = 1;
    uint32_t slen = 0;
    struct vfsHeader *header = NULL;
    //Check root path
    slen = pathNameLength(path,base);
    hashtableFetch(root.children,&path[base],slen,(void**)&header);
    if(!header){return NULL;} //No folder/path in the hash table
    //Go down the path until none is found or file is met
    if(header->type == VFS_TYPE_FILE){*mnt = NULL; return header;}
    base += slen+1;
    nxt:
    slen = pathNameLength(path,base);
    if(slen == 0){return header;}
    hashtable *table = NULL;
    switch(header->type){
        case VFS_TYPE_MOUNT:
        table = ((struct vfsMountpoint*)header)->table;
        *mnt = (struct vfsMountpoint*)header;
        break;
        default:
        kerror("Unsupported VFS type 0x%x\n",(uint32_t)header->type);
        return NULL;
        break;
    }
    if(!hashtableFetch(table,&path[base],slen,(void**)&header)){return NULL;}
    base += slen+1; //+1 to skip the '/'
    if(header->type == VFS_TYPE_FILE){return header;}
    goto nxt;
}

/*!
    Unmounts a device
    Returns true on success
*/
bool unmountVFS(struct vfsMountpoint *mnt){
    bool found = false;
    for(uint32_t i = 0; i < devices; ++i){
        if((&mount[i]) != mnt){continue;}
        found = true;
        mount[i] = (struct vfsMountpoint*)NULL;
        break;
    }
    if(!found){return false;}
    //Unmount devices
    hashtableDestroy(mnt->table);
    //Free files 'n stuff
    --devices;
    return true;
}

/*!
    Returns NULL if path is invalid or bad flags
*/
vFILE *openFile(char *path, uint16_t flags){
    struct vfsMountpoint *mnt;
    struct vfsHeader *header = (struct vfsHeader*)parsePath(path,&mnt);
    if(!header){return NULL;}
    if(header->type != VFS_TYPE_FILE){return NULL;}
    //Check flags here
    vFILE *fl = (vFILE*)kmalloc(sizeof(vFILE));
    fl->file = (struct vfsFile*)header;
    fl->file->references += 1;
    fl->mount = mnt;
    if(mnt){mnt->references += 1;}
    return fl;
}

/*!
    Closes a file
*/
void closeFile(vFILE *fl){
    if(!fl){return;}
    fl->file->references -= 1;
    if(fl->file->references == 0){
        kinfo("Do file unload\n");
    } //Unload file
    kfree(fl);
}

size_t readFile(vFILE *fl, size_t sz, void *dst){
    if(!fl->mount){
        //Use EXT4 file functions here
        return 0x0;
    }
    size_t in = fl->mount->mfread(sz,fl->file,fl->fileOffset,dst);
    fl->fileOffset += in;
    return in;
}

/*!
    Returns offset the function set
*/
size_t fileSeek(vFILE *fl, size_t offset, uint8_t seek){
    switch(seek){
        case VFS_SEEK_CUR:
        fl->fileOffset = (offset+fl->fileOffset >= fl->file->size) ? fl->file->size : offset;
        break;
        case VFS_SEEK_END:
        fl->fileOffset = (offset+fl->fileOffset >= fl->file->size) ? 0 : fl->file->size-offset;
        break;
        case VFS_SEEK_SET:
        fl->fileOffset = (offset+fl->fileOffset >= fl->file->size) ? fl->file->size : offset;
        break;
        default:
        return !offset;
        break;
    }
    return fl->fileOffset;
}