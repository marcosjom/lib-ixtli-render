//
//  ScnTextureMaker.c
//  ixtli-render
//
//  Created by Marcos Ortega on 20/8/25.
//

#include "ScnTextureMaker.h"


ScnBitmapRef ScnTextureMaker_make(ScnContextRef ctx, const ScnUI16 width, const ScnUI16 height, const ENScnBitmapColor color, const ScnUI16 gridSz){
    ScnBitmapRef r = ScnBitmap_alloc(ctx);
    if(ScnBitmap_isNull(r)){
        return r ;
    }
    //create buffer
    if(!ScnBitmap_create(r, width, height, color)){
        ScnBitmap_releaseAndNull(&r);
        return r;
    }
    //create grid
    {
        void* buff = NULL;
        const STScnBitmapProps props = ScnBitmap_getProps(r, &buff);
        ScnUI16 cmpCount = 0;
        //ammount of components
        switch(props.color){
            case ENScnBitmapColor_ALPHA8:
            case ENScnBitmapColor_GRAY8:
                cmpCount = 1;
                break;
            case ENScnBitmapColor_GRAYALPHA8:
                cmpCount = 2;
                break;
            case ENScnBitmapColor_RGB8:
                cmpCount = 3;
                break;
            case ENScnBitmapColor_RGBA8:
                cmpCount = 4;
                break;
            default:
                break;
        }
        //validate color
        if(cmpCount <= 0){
            ScnBitmap_releaseAndNull(&r);
            return r;
        }
        //build grid
        {
            ScnUI16 iRow = 0, iCmp = 0, iCmpRow = 0;
            ScnUI8 c[4];
            while(iRow < props.size.height){
                ScnUI16 iByte = 0;
                ScnUI8* row = &((ScnUI8*)buff)[iRow * props.bytesPerLine];
                //initial state
                iCmp = iCmpRow;
                c[0] = (iCmp == 0 ? 255 : 0);
                c[1] = (iCmp == 1 ? 255 : 0);
                c[2] = (iCmp == 2 ? 255 : 0);
                c[3] = (iCmp == 3 ? 0 : 255);
                //fill line
                while(iByte < props.bytesPerLine){
                    row[iByte] = c[iByte % cmpCount];
                    ++iByte;
                    //change grid component
                    if((iByte % (gridSz * cmpCount)) == 0){
                        //change current component
                        c[iCmp] = (c[iCmp] == 0u ? 255u : 0u);
                        //change next coponent
                        iCmp = (iCmp + 1) % cmpCount;
                        c[iCmp] = (c[iCmp] == 0u ? 255u : 0u);
                    }
                }
                //next row
                ++iRow;
                //move grid forward to the right
                if((iRow % gridSz) == 0){
                    iCmpRow = (iCmpRow + 1) % cmpCount;
                }
            }
        }
    }
    return r;
}
