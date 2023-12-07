#pragma once

//Global system information table
//Allows kernel C code to easily determine what hardware is supported

struct gsitTimers{

};

struct gsitS{
    struct gsitTimers timers;

};

extern struct hsit