#include <stdint.h>
#include <string.h>
#include <malloc.h>

//Returns a pointer to the decompressed data
//The caller must free the pointer
void *decompressLZ77(void *src){
    void *dest = 0x0; //malloc(60);
    uint8 *compressed = (uint8*)src;
    //for(uint64 i = 0;;++i){
    //}
}

//Returns a pointer to the compressed data
//The caller must free the pointer
void *compressLZ77(void *src){

}