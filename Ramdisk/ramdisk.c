#include "ramdisk.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

FILE *disk;

uint64_t fileSz(FILE *fl){
    fseek(fl,0x0,SEEK_END);
    uint64_t r = ftell(fl);
    fseek(fl,0x0,SEEK_SET);
    return r;
}

void copyFile(FILE *src, FILE *dest, uint64_t sz){
    char ch = 0x0;
    fseek(src,0,SEEK_SET);
    for(int i = 0; i < sz; ++i){
        ch = fgetc(src);
        fputc(ch,dest);
    }
}

uint16_t fcount = 0;
void addFile(const char *subPath, const char *name, const char *ext, uint16_t flags){
    //Get full path
    uint32_t nLen = strlen(name);
    uint32_t xLen = strlen(ext);
    uint32_t fullLen = nLen+xLen+strlen(subPath);
    char *path = (char*)malloc(fullLen+16);
    sprintf(path,"./Ramdisk/FS/%s/%s.%s%c",subPath,name,ext,'\0');
    FILE *fl = fopen(path,"r");
    if(fl == NULL){printf("Failed to open %s\n",path); free(path); return;}
    struct ramdiskFile fhdr;
    fhdr.magic = RD_FILE_MAGIC;
    fhdr.size = fileSz(fl);
    fhdr.offset = sizeof(struct ramdiskFile)+nLen+xLen+fhdr.size+2;
    fhdr.flags = flags;
    fwrite(&fhdr,sizeof(fhdr),1,disk);
    fprintf(disk,"%s.%s%c",name,ext,'\0');
    copyFile(fl,disk,fhdr.size);
    fclose(fl);
    free(path);
    printf("[Wrote %li bytes to disk|%s.%s]\n",fhdr.size,name,ext);
    ++fcount;
}

int main(){
    disk = fopen("./Ramdisk/initrd.disk","w+");
    if(disk == NULL){printf("Failed to open ramdisk\n"); exit(EXIT_FAILURE);}
    struct ramdiskHeader header;
    //Write blank header
    fwrite(&header,sizeof(struct ramdiskHeader),1,disk);
    //Write files
    addFile("Media","test","png",0);
    addFile("Fonts","Abyssinica","ttf",0);
    //Overwrite header
    fseek(disk,0,SEEK_SET);
    header.magic = RD_HEADER_MAGIC;
    header.files = fcount;
    fwrite(&header,sizeof(struct ramdiskHeader),1,disk);
    printf("[Wrote %i files|Total %li bytes/%li Mb]\n",fcount,fileSz(disk),fileSz(disk)/1048576);
    fclose(disk);
    return 0;
}