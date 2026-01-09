#include <stdint.h>
#include <stdbool.h>
#include <vfs.h>
#include <libpng.h>
#include <kprint.h>

/*!
    Tests if an image is a PNG.
    Returns true if it is a PNG
*/
bool checkPNG(vFILE *fl){
    fileSeek(fl,0,VFS_SEEK_SET);
    struct pngSignature sig;
    readFile(fl,sizeof(struct pngSignature),&sig);
    if(sig.signature != PNG_SIGNATURE){kinfo("Invalid PNG sig (0x%lx)\n",sig.signature); return false;}
    return true;
}