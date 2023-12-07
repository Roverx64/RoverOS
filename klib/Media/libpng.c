#include <libpng.h>
#include <stdint.h>
#include <stdio.h>
#include <debug.h>
/*
void getChunk(FILE *fl, struct pngChunk *chunk){
    uint64 ret = ftell(fl);
    fread(chunk,sizeof(struct pngChunk),1,fl);
    fseek(fl,ret,SEEK_SET);
}

void readIHDR(FILE *fl){
    struct IHDR ihdr;
    fread(&ihdr,sizeof(ihdr),1,fl);
    if(rorder32(ihdr.chunk.type) != PNG_CHUNK_IHDR){
        kdebug(DNONE,"Invalid IHDR\n");
        return;
    }
    kdebug(DNONE,"#===IHDR===#\n");
    kdebug(DNONE,"0x%x x 0x%x\n",rorder32(ihdr.width),rorder32(ihdr.height));
    kdebug(DNONE,"Depth: 0x%hx\n",ihdr.depth);
    kdebug(DNONE,"Type: 0x%hx\n",ihdr.type);
    kdebug(DNONE,"Compression: 0x%hx\n",ihdr.compression);
    kdebug(DNONE,"Filter: 0x%hx\n",ihdr.filter);
    kdebug(DNONE,"Interlace: 0x%hx\n",ihdr.interlace);
}

void drawPNG(FILE *fl){
    kdebug(DNONE,"#==========PNG==========#\n");
    struct pngSignature sig;
    fread(&sig,sizeof(sig),1,fl);
    if(rorder64(sig.signature) != PNG_SIGNATURE){goto end;}
    kdebug(DNONE,"Found valid png signature\n");
    struct pngChunk chunk;
    getChunk(fl,&chunk);
    //Must be IHDR first
    if(rorder32(chunk.type) != PNG_CHUNK_IHDR){
        kdebug(DNONE,"Invalid PNG file. (IHDR must be first)\n");
        return;
    }
    readIHDR(fl);
    //Loop through chunks
    for(;;){
        getChunk(fl,&chunk);
        switch(rorder32(chunk.type)){
            case PNG_CHUNK_IDAT:
            break;
            case PNG_CHUNK_IEND:
            break;
            case PNG_CHUNK_PLTE:
            break;
        }
    }
    end:
    kdebug(DNONE,"#=======================#\n");
}
*/