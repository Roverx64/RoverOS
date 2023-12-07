#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ttf.h"
#include "debug.h"

//TODO: Move this into a driver/userspace service

/*For reading a TFF font file*/

void initTTF(FILE *font){
    //Read header
    ttfFeat feat;
    fread(&feat,sizeof(feat),1,font);
    kdebug(DINFO,"Ver: 0x%lx\n",feat.version);
}