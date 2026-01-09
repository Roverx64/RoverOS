#pragma once

#include <stdint.h>
#include <hashtable.h>
#include <stdbool.h>

#define VFS_TYPE_MOUNT 0
#define VFS_TYPE_FILE 1
#define VFS_TYPE_DIR 2
#define VFS_TYPE_SYM 3

#define VFS_MOUNT_RAM 0 //Mount point is a RAM based device
#define VFS_MOUNT_PHYS 1 //Mount point is a physical device

#define VFS_MOUNT_NO_LETTER '\0' //!< For devices that don't need one

#define VFS_FLAG_WRITE 1<<0 //!< Enables a file to be written to
#define VFS_FLAG_EXEC 1<<1 //!< Enables a file to be executed
#define VFS_FLAG_NO_UNLOAD 1<<2 //!< Disallows the OS from unloading this file from RAM

#define VFS_OPEN_FLAG_READ_ONLY 1<<0

#define VFS_SEEK_SET 0
#define VFS_SEEK_END 1
#define VFS_SEEK_CUR 2

//All file system drivers must have these functions supported

/*!
    size_t = Bytes requested
    vfsFile = file information
    size_t = Current file offset
    uint64_t = Pointer to destionation
*/
typedef size_t (vfsDriverReadFile)(size_t,struct vfsFile*,size_t,void*); //!< Returns number of bytes read in



/*!
    Shared vfsHeader
*/
struct vfsHeader{
    uint16_t type;
    uint16_t flags;
    uint16_t length;
};

/*!
    Drive mountpoint in VFS
*/
struct vfsMountpoint{
    struct vfsHeader header;
    uint8_t type;
    uint32_t childCount;
    hashtable *table; //Hash table of children
    char letter; //!< Letter identifier for mount point
    uint16_t references; //!< Number of open files on this point
    vfsDriverReadFile *mfread;
};

/*!
    File structure
*/
struct vfsFile{
    struct vfsHeader header;
    char *name; //File name
    uint64_t phys; //!< Phys location of data
    uint32_t size; //!< Size of file in bytes
    uint16_t references; //!< Open references to this file
};

/*!
    Directory structure
*/
struct vfsDirectory{
    struct vfsHeader header;
};

/*!
    Root mount point for everything
*/
struct vfsRoot{
    hashtable *children;
    uint32_t childCount;
    uint16_t mounted; //Number of mounted devices 
};

typedef struct vFileS{
    struct vfsFile *file;
    struct vfsMountpoint *mount; //!< NULL if root device is used
    size_t fileOffset;
}vFILE;

extern bool addVFSFile(void *parent, char *name, uint64_t phys, uint32_t size, uint16_t flags);
extern bool unmountVFS(struct vfsMountpoint *mount);
extern struct vfsMountpoint *mountVFS(uint8_t type, uint32_t childCount, char letter, uint16_t flags, vfsDriverReadFile *mfread);
extern vFILE *openFile(char *path, uint16_t flags);
extern size_t readFile(vFILE *fl, size_t sz, void *dst);
extern size_t fileSeek(vFILE *fl, size_t offset, uint8_t seek);