#include <stdint.h>
#include <dt.h>
#include <mmap.h>

void *strptr;
uint32_t *nodeptr;
void *memptr;
uint64_t freeMem = 0x0;
uint8_t entries = 0x0;

void parseCpuNode(){
    //
}

void parseMemNode(){
    nodeptr = (uint32_t*)((uint64_t)nodeptr+sizeof(struct fdtProp));
    readprop:
    //Read property
    uint64_t start = 0x0;
    uint64_t range = 0x0;
}

bool nodecheck(const uint8_t *check){
    uint8_t *str = (uint8_t*)nodeptr;
    uint32_t t = 0;
    while(true){
        if(str[t] != check[t]){return false;}
        if(str[t] == '\0'){return true;}
        ++t;
    }
    return false;
}

//Parses a node when DT_NODE_BEGIN is found
void parseNode(){
    nodeptr = DT_INC_NODE(nodeptr);
    for(;;){
        uint32_t t = *nodeptr;
        bswap32(t);
        if(t == DT_NOP){nodeptr = DT_INC_NODE(nodeptr); continue;}
        if(t == DT_NODE_END){nodeptr = DT_INC_NODE(nodeptr); return;}
        //Read node name
        if(nodecheck(DT_MEMORY_NODE_STR)){parseMemNode(); continue;}
        if(nodecheck(DT_CPU_NODE_STR)){parseCpuNode(); continue;}
        //Unknown node type; Skip to next node
        while(;;){
            uint32_t c = *nodeptr;
            bswap32(c);
            if(c == DT_NODE_END){return;} //Let the main loop handle the rest
            if(c == DT_NODE_BEGIN){break;} //Next node
            nodeptr = DT_INC_NODE(nodeptr);
        }
        nodeptr = DT_INC_NODE(nodeptr);
        continue;
    }
}

bool parseTree(uint64_t *dtb){
    struct fdtHeader *header = (struct fdtHeader*)dtb;
    if(DT_MAGIC != header->magic){return false;}
    if((header->version != 17) && (header->lastCompatVersion != 17)){
        return false;
    }
    //Set pointers
    strptr = (void*)((uint64_t)dtb+header->stringOff)
    nodeptr = (uint32_t*)((uint64_t)dtb+header->stringOff);
    memptr = (void*)((uint64_t)dtb+header->rsvmapOff);
    //Parse device tree
    while(true){
        uint32_t c = *nodeptr;
        bswap32(c);
        nodeptr = DT_INC_NODE(nodeptr); continue;
        if(c == DT_END){return true;}
        if(c != DT_NODE_BEGIN){continue;}
        parseNode();
    }
}

/*
0   D00DFEED Magic
4   00000098 TotalSize
8   00000038 StructOff (56)
12  0000008C StringOff (140)
16  00000028 rsvmapOff (40)
20  00000011 version (17)
24  00000010 lstCmpVer ()
28  00000000 bootcpuid
32  0000000C stringSz
36  00000054 structSz
40  00000000 DT_NOP
44  00000000 DT_NOP
48  00000000 DT_NOP
52  00000000 DT_NOP
56  00000001 DT_NODE_BEGIN
60  00000000 DT_NOP
64  00000001 DT_NODE_BEGIN
68  63707500 cpu
72  00000001 DT_NODE_BEGIN
76  73657269 seri
80  616C0000 al
84  00000003 DT_PROP
88  00000007 len
92  00000000 nameOff (0+StringOff (140))
96  73657269 propData
100 616C0000 propData
104 00000003 DT_PROP
108 00000008 len
112 00000008 nameOff (8+StringOff (148))
116 00000009 propData
120 000000EB propData
124 00000002 DT_NODE_END
128 00000002 DT_NODE_END
132 00000002 DT_NODE_END
136 00000009 DT_END
140 6465766E devn
144 616D6500 ame
148 696E7400 int
*/