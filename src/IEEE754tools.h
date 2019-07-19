// Note this code is token from IEEE754 tools. It has a few bits stripped out because
// BSON only cares about LSB.
//
//    FILE: IEEE754tools.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.00
// PURPOSE: IEEE754 tools
//
// https://playground.arduino.cc//Main/IEEE754tools
//
// Released to the public domain
// not tested, use with care
//

#ifndef IEEE754tools_h
#define IEEE754tools_h

#include <stdint.h>
#include <math.h>

// IEEE754 float layout;
struct IEEEfloat
{
    uint32_t m:23;
    uint8_t e:8;
    uint8_t s:1;
};

// IEEE754 double layout;
struct IEEEdouble
{
    uint64_t m:52;
    uint16_t e:11;
    uint8_t s:1;
};

// Arduino UNO double layout:
// the UNO has no 64 bit double, it is only able to map 23 bits of the mantisse
// a filler is added.
struct _DBL
{
    uint32_t filler:29;
    uint32_t m:23;
    uint16_t e:11;
    uint8_t  s:1;
};

// for packing and unpacking a float
typedef union _FLOATCONV
{
    IEEEfloat p;
    float f;
    uint8_t b[4];
} _FLOATCONV;

// for packing and unpacking a double
typedef union _DBLCONV
{
    // IEEEdouble p;
    _DBL p;
    double d;           // !! is a 32bit float for UNO.
    uint8_t b[4];
} _DBLCONV;


//
// converts a float to a packed array of 8 bytes representing a 64 bit double
// restriction exponent and mantisse.
//
// float;  array of 8 bytes;  LSBFIRST;
//
void float2DoublePacked(float number, uint8_t* bar)
{
    _FLOATCONV fl;
    memset(&fl, 0x00, sizeof(fl));
    fl.f = number;
    _DBLCONV dbl;
    dbl.p.s = fl.p.s;
    dbl.p.e = fl.p.e-127 +1023;  // exponent adjust
    dbl.p.m = fl.p.m;

    for (int i=0; i<8; i++)
    {
        bar[i] = dbl.b[i];
    }
}

//
// converts a packed array of bytes into a 32bit float.
// there can be an exponent overflow
// the mantisse is truncated to 23 bits.
//
float doublePacked2Float(uint8_t* bar)
{
    _FLOATCONV fl;
    _DBLCONV dbl;

    for (int i=0; i<8; i++)
    {
        dbl.b[i] = bar[i];
    }


    int e = dbl.p.e-1023+127;  // exponent adjust
    // TODO check exponent overflow.
    if (e >=0 && e <= 255)
    {
        fl.p.s = dbl.p.s;
        fl.p.e = e;
        fl.p.m = dbl.p.m;  // note this one clips the mantisse
    }
    else fl.f = NAN;

    return fl.f;
}

#endif
