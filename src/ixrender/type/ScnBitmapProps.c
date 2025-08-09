//
//  ScnBitmapProps.c
//  ixtli-render
//
//  Created by Marcos Ortega on 8/8/25.
//

#include "ixrender/type/ScnBitmapProps.h"

//

typedef struct STScnBitmapColorDesc {
    ScnUI8              bitsPerPx;
    const char*         name;
    ENScnBitmapColor    color;
} STScnBitmapColorDesc;

static const STScnBitmapColorDesc ScnBitmap_colorMap[] = {
    {0, "undef", ENScnBitmapColor_undef },
    {8, "ALPHA8", ENScnBitmapColor_ALPHA8},
    {8, "GRAY8", ENScnBitmapColor_GRAY8 },
    {16, "GRAYALPHA8", ENScnBitmapColor_GRAYALPHA8 },
    {24, "RGB8", ENScnBitmapColor_RGB8 },
    {32, "RGBA8", ENScnBitmapColor_RGBA8 },
};

//STScnBitmapProps

STScnBitmapProps ScnBitmapProps_build(const ScnSI32 width, const ScnSI32 height, const ENScnBitmapColor color){
    STScnBitmapProps r = STScnBitmapProps_Zero;
    if(color < (sizeof(ScnBitmap_colorMap) / sizeof(ScnBitmap_colorMap[0]))){
        const STScnBitmapColorDesc* d = &ScnBitmap_colorMap[color];
        if(d->bitsPerPx == 0){
            ScnMemory_setZeroSt(r);
        } else {
            r.bitsPerPx = d->bitsPerPx;
            {
                const ScnUI32 align = 4;
                const ScnUI32 bitsPerLine = (width * d->bitsPerPx);
                //aligned to byte and word
                r.bytesPerLine    = (((bitsPerLine / 8) + ((bitsPerLine % 8) != 0 ? 1 : 0)) + align - 1) / align * align;
            }
            r.color         = color;
            r.size.width    = width;
            r.size.height   = height;
        }
    }
    return r;
}
