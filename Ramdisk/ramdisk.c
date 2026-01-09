#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ftw.h>
#include "ramdisk.h"

FILE *disk;
struct ramdiskHeader header;

uint64_t fileSz(FILE *fl){
    fseek(fl,0x0,SEEK_END);
    uint64_t r = ftell(fl);
    fseek(fl,0x0,SEEK_SET);
    return r;
}

void copyFile(FILE *src, FILE *dest, uint64_t sz){
    char ch = 0x0;
    fseek(src,0,SEEK_SET);
    for(uint64_t i = 0; i < sz; ++i){
        ch = fgetc(src);
        fputc(ch,dest);
    }
}

int addFile(const char *fp, const struct stat *sb, int flags, struct FTW *buf){
    if(flags != FTW_F){return 0;}
    //Add the file
    FILE *fl = fopen(fp,"r");
    if(!fl){printf("Failed to open %s\n",fp); return 0;}
    uint64_t l = fileSz(fl);
    struct ramdiskFile fh;
    header.files += 1;
    fh.magic = RD_FILE_MAGIC;
    fh.size = l;
    fwrite(&fh,sizeof(fh),1,disk);
    //Get filename
    uint64_t sl = strlen(fp);
    while(((fp[sl] != '\\') && (fp[sl] != '/'))){--sl;}
    ++sl;
    char *name = (char*)malloc(sizeof(char)*(sl+1));
    //Write filename
    sprintf(name,"%s",(char*)(&fp[sl]));
    fprintf(disk,"%s%c",name,'\0');
    //Append file
    copyFile(fl,disk,l);
    //Cleanup
    printf("[Added %s - %llu bytes]\n",name,l);
    free(name);
    fclose(fl);
    return 0;
}

int main(int argc, char *argv[]){
    if(argc < 2){printf("Missing base path argument\n"); exit(EXIT_FAILURE);}
    printf("Base path %s\n",argv[1]);
    disk = fopen("./Ramdisk/init.rd","w+");
    if(!disk){printf("Failed to open ramdisk\n"); exit(EXIT_FAILURE);}
    //Write temp header
    fwrite(&header,sizeof(header),1,disk);
    //Write files
    if(nftw(argv[1],addFile,10,0) == -1){
        int err = errno;
        switch(err){
            case ENOTDIR:
            printf("'%s' is not a directory\n",argv[1]);
            break;
            case ENOENT:
            printf("No '%s' directory\n",argv[1]);
            break;
            default:
            printf("NFTW had an error - errno: %i\n",err);
            break;
        }
        fclose(disk);
        exit(EXIT_FAILURE);
    }
    //Overwrite header
    header.magic = RD_HEADER_MAGIC;
    fseek(disk,0x0,SEEK_SET);
    fwrite(&header,sizeof(header),1,disk);
    printf("[Wrote %i files|Total %llu bytes/%llu MB]\n",header.files,fileSz(disk),fileSz(disk)/1048576);
    fclose(disk);
    return 0;
}