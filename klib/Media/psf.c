#include <stdint.h>
#include <gui.h>
#include "debug.h"
#include "psf.h"

//For drawing and rendering PSF fonts

void testRead(void *psf){
    PSF1Header *psf1 = (PSF1Header*)psf;
    if(psf1->magic != PSF1_MAGIC){goto psftwo;}
    kdebug(DNONE,"PSF1\n");
    return;
    psftwo:
    PSF2Header *psf2 = (PSF2Header*)psf;
    if(psf2->magic != PSF2_MAGIC){kdebug(DNONE,"Invalid magic 0x%x\n",psf2->magic);return;}
    kdebug(DNONE,"PSF2\n");
    kdebug(DNONE,"Version 0x%x\n",psf2->version);
    kdebug(DNONE,"Unicode: ");
    if(psf2->flags&PSF2_HAS_UNICODE_TABLE){kdebug(DNONE,"Yes\n");}else{kdebug(DNONE,"No\n");}
    kdebug(DNONE,"0x%x byte characters\n",psf2->characterSz);
    kdebug(DNONE,"0x%x x 0x%x\n",psf2->width,psf2->height);
}