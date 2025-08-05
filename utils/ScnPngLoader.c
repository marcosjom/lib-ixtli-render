//
//  ScnPngLoader.c
//  lib-nbframework
//
//  Created by Marcos Ortega on 8/11/18.
//

#include "../utils/SncPngLoader.h"
#include "../utils/spng.h"
//#include "../utils/easyzlib.h"
#include "ixrender/type/ScnSize.h"

#include <fcntl.h>     //POSIX header
#include <unistd.h>    //for STDIN_FILENO, STDOUT_FILENO, and STDERR_FILENO

int ScnPngLoader_rw_fn(spng_ctx *ctx, void *user, void *dst_src, size_t length){
    int* file = (int*)user;
    size_t rr = read(*file, dst_src, length);
    printf("ScnPngLoader_rw_fn %ld of %ld bnytes.\n", rr, length);
    return (rr == length ? SPNG_OK : SPNG_IO_ERROR);
}

ScnBOOL ScnPngLoader_loadFromPath(ScnContextRef ctx, const char* path, STScnBitmapProps* dstProps, void** dstData, ScnUI32* dstDataSz){
    ScnBOOL r = ScnFALSE;
    void* data = NULL;
    ScnUI32 dataSz = 0;
    ENScnBitmapColor color = ENScnBitmapColor_undef;
    STScnBitmapProps props = STScnBitmapProps_Zero;
    {
        int fmode = O_RDONLY;
        int file = open(path, fmode, 0666);
        if(file < 0){
            printf("ERROR, ScnPngLoader_loadFromPath::fopen('%s') failed.\n", path);
        } else {
            int rr = 0;
            struct spng_ihdr ihdr;
            spng_ctx* sctx = spng_ctx_new(0);
            if(0 != (rr = spng_set_png_stream(sctx, ScnPngLoader_rw_fn, &file))){
                printf("ERROR, spng_set_png_stream failed(%d).\n", rr);
            } else if(0 != (rr = spng_decode_chunks(sctx))){
                printf("ERROR, spng_decode_chunks failed(%d).\n", rr);
            } else if(0 != (rr = spng_get_ihdr(sctx, &ihdr))){
                printf("ERROR, spng_get_ihdr failed(%d).\n", rr);
            } else {
                // Especificaciones PNG
                // TIPO_COLOR   BITS_PROFUNDIDAD   DESCRIPCION
                // 0            1, 2, 4, 8, 16     Gris
                // 2            8, 16              RGB
                // 3            1, 2, 4, 8         Paleta RGB (el alpha puede incluirse en un chunck tRNS)
                // 4            8, 16              Gris con alpha
                // 6            8, 16              RGBA
                enum spng_format fmt = SPNG_FMT_RAW;
                switch(ihdr.color_type) {
                    case 0: //Grayscale
                        if(ihdr.bit_depth != 1 && ihdr.bit_depth != 2 && ihdr.bit_depth != 4 && ihdr.bit_depth != 8 && ihdr.bit_depth != 16){
                            SCN_PRINTF_ERROR("PNG load, bitdepth(%d) not allowed for colorMode(%d) by specification.\n", ihdr.bit_depth, ihdr.color_type);
                        } else {
                            fmt     = SPNG_FMT_G8;
                            color   = ENScnBitmapColor_GRAY8;
                            props   = ScnBitmapProps_build(ihdr.width, ihdr.height, color);
                        }
                        break;
                    case 2: //Truecolor
                        if(ihdr.bit_depth != 8 && ihdr.bit_depth != 16){
                            SCN_PRINTF_ERROR("PNG load, bitdepth(%d) not allowed for colorMode(%d) by specification.\n", ihdr.bit_depth, ihdr.color_type);
                        } else {
                            fmt     = SPNG_FMT_RGB8;
                            color   = ENScnBitmapColor_RGB8;
                            props   = ScnBitmapProps_build(ihdr.width, ihdr.height, color);
                        }
                        break;
                    case 3: //Indexed-color
                        if(ihdr.bit_depth != 1 && ihdr.bit_depth != 2 && ihdr.bit_depth != 4 && ihdr.bit_depth != 8){
                            SCN_PRINTF_ERROR("PNG load, bitdepth(%d) not allowed for colorMode(%d) by specification.\n", ihdr.bit_depth, ihdr.color_type);
                        } else {
                            //Waiting for 'PTE' and 'tRNS' to detemrine bmpProps.
                            struct spng_plte plte; struct spng_trns trns;
                            if(0 != spng_get_plte(sctx, &plte)){
                                printf("ERROR, spng_get_plte failed.\n");
                            } else {
                                fmt     = (0 == spng_get_trns(sctx, &trns) ? SPNG_FMT_RGBA8 : SPNG_FMT_RGB8);
                                color   = (fmt == SPNG_FMT_RGBA8 ? ENScnBitmapColor_RGBA8 : ENScnBitmapColor_RGB8);
                                props   = ScnBitmapProps_build(ihdr.width, ihdr.height, color);
                            }
                        }
                        break;
                    case 4: //Grayscale with alpha
                        if(ihdr.bit_depth != 8 && ihdr.bit_depth != 16){
                            SCN_PRINTF_ERROR("PNG load, bitdepth(%d) not allowed for colorMode(%d) by specification.\n", ihdr.bit_depth, ihdr.color_type);
                        } else {
                            fmt     = SPNG_FMT_GA8;
                            color   = ENScnBitmapColor_GRAYALPHA8;
                            props   = ScnBitmapProps_build(ihdr.width, ihdr.height, color);
                        }
                        break;
                    case 6: //Truecolor with alpha
                        if(ihdr.bit_depth != 8 && ihdr.bit_depth != 16){
                            SCN_PRINTF_ERROR("PNG load, bitdepth(%d) not allowed for colorMode(%d) by specification.\n", ihdr.bit_depth, ihdr.color_type);
                        } else {
                            fmt     = SPNG_FMT_RGBA8;
                            color   = ENScnBitmapColor_RGBA8;
                            props   = ScnBitmapProps_build(ihdr.width, ihdr.height, color);
                        }
                        break;
                    default:
                        SCN_PRINTF_ERROR("PNG load, colorMode(%d) not in specification.\n", ihdr.color_type);
                        break;
                }
                //
                size_t bmpSz = 0; void* dataN = NULL;
                if(fmt == SPNG_FMT_RAW){
                    //undefined color
                } else if(0 != (rr = spng_decoded_image_size(sctx, fmt, &bmpSz))){
                    printf("ERROR, spng_decoded_image_size failed(%d).\n", rr);
                } else if(bmpSz != (props.bytesPerLine * props.size.height)){
                    printf("ERROR, spng_decoded_image_size missmatch(%d v %d).\n", (ScnUI32)bmpSz, (props.bytesPerLine * props.size.height));
                } else if(NULL == (dataN = ScnContext_malloc(ctx, props.bytesPerLine * props.size.height, "ScnPngLoader_loadFromPath::data"))){
                    printf("ERROR, ScnContext_malloc failed.\n");
                } else {
                    data = dataN;
                    dataSz = props.bytesPerLine * props.size.height;
                    if(0 != (rr = spng_decode_image(sctx, data, dataSz, fmt, 0))){
                        printf("ERROR, spng_decode_image failed(%d).\n", rr);
                    } else {
                        r = ScnTRUE;
                    }
                }
            }
            spng_ctx_free(sctx);
            close(file);
        }
    }
    if(dstData != NULL) *dstData = data;
    if(dstDataSz != NULL) *dstDataSz = dataSz;
    if(dstProps != NULL) *dstProps = props;
    return r;
}
