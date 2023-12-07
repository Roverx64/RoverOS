#pragma once
#include <stdint.h>

//https://developer.apple.com/fonts/TrueType-Reference-Manual/

#define NEXT_TTF_TABLE(pos, tlbsz) ((pos+tlbsz))

//Tables
typedef struct ttfFeat{
    uint32 version;
    uint16 featureNameCount;
    uint16 resv0; //Reserved
    uint32 resv1; //Reserved
}ttfFeat;

typedef struct tffFeatName{
    uint16 feature;
    uint16 nSettings;
    uint32 settingTable;
    uint16 featureFlags;
    int16 nameIndex;
}ttfFeatName;