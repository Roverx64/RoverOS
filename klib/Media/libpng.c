#include <libpng.h>
#include <stdint.h>
#include <debug.h>

void readIHDR(pngIHDR *ihdr){
    if(rorder32(ihdr->ident.type) != PNG_CHUNK_IHDR){
        kdebug(DNONE,"Invalid IHDR\n");
        return;
    }
    kdebug(DNONE,"[");
    kdebug(DNONE,"0x%llX x 0x%llX|",rorder32(ihdr->width),rorder32(ihdr->height));
    switch(ihdr->colorType){
        case PNG_COLOR_GREYSCALE:
        kdebug(DNONE,"Greyscale");
        break;
        case PNG_COLOR_RGB:
        kdebug(DNONE,"RGB");
        break;
        case PNG_COLOR_PALETTE:
        kdebug(DNONE,"Palette");
        break;
        case PNG_COLOR_GRAYALPHA:
        kdebug(DNONE,"Grey+alpha");
        break;
        case PNG_COLOR_RGBA:
        kdebug(DNONE,"RGBA");
        break;
        default:
        kdebug(DNONE,"Unknown");
        break;
    }

    kdebug(DNONE,"]\n");
}

void readIDAT(pngIDAT *idat){
    if(rorder32(idat->ident.type) != PNG_CHUNK_IDAT){
        kdebug(DNONE,"Invalid IDAT\n");
        return;
    }
}

uint32 getPixel(void *data){
    //Processes, decompresses, and returns a valid pixel
    //Use pallette if present
    uint8 *rgb = (uint8*)data;
    uint32 offset = 0x0;
    //Assume RGB colors for now
    offset += 3;
}

void readPNG(void *data){
    pngSignature *png = (pngSignature*)data;
    if(rorder64(png->signature) != PNG_SIGNATURE){kdebug(DINFO,"Invalid PNG signature 0x%llx\n",rorder64(png->signature)); return;}
    kdebug(DINFO,"Got valid PNG signature\n");
    //IHDR is always first
    struct chunkIdent *ident = (struct chunkIdent*)((uint64)data+sizeof(pngSignature));
    uint32 ntype = rorder32(ident->type);
    readIHDR((pngIHDR*)((uint64)data+sizeof(pngSignature)));
    //Loop through chunks
    for(int i = 0;i < 1; ++i){
        //ident = NEXT_CHUNK(ident);
        //kdebug(DNONE,"[%c%c%c%c]\n",ident->type&(0xFF<<0),ident->type&(0xFF<<8),ident->type&(0xFF<<16),ident->type&(0xFF<<32));
    }
}