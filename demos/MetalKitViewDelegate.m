//
//  MetalKitViewDelegate.m
//  ixtli-render-demos-mac
//
//  Created by Marcos Ortega on 28/7/25.
//

#import "MetalKitViewDelegate.h"

#include "ixrender/ScnRender.h"
#include "ixrender/api/ScnApiMetal.h"
#include "ixrender/type/ScnBitmap.h"
#include "../src/utils/ScnMemMap.h"
#include "../src/utils/SncPngLoader.h"

// Custom memory allocation for this demos and tests,
// for detecting memory-leaks using ScnMemMap.
void* ScnMemMap_custom_malloc(const ScnUI32 newSz, const char* dbgHintStr);
void* ScnMemMap_custom_realloc(void* ptr, const ScnUI32 newSz, const char* dbgHintStr);
void ScnMemMap_custom_free(void* ptr);
STScnMemMap* gMemmap = NULL;

NS_ASSUME_NONNULL_BEGIN

#define IXTLI_RENDER_DEMO_RENDER_SLOTS_AMMOUNT  3
#define IXTLI_FRAMES_COUNT_AND_RELEASE_RENDER   360

@implementation MetalKitViewDelegate {
    ScnContextRef       ctx;
    ScnRenderRef        render;
    ScnFramebuffRef     framebuff;
    ScnNode2dRef        node;
    ScnModel2dRef       model;
    ScnTextureRef       tex;
    STScnVertex2DTexPtr verts;
    ScnUI32             vertsSz;
    //tex
    struct {
        STScnBitmapProps props;
        //data
        struct {
            ScnBYTE*    ptr;
            ScnUI32     use;
        } data;
    } bmp;
    //memory leak detection
    STScnMemMap         memmap;
    ScnUI32             framesCount;
@protected
    MTKView             *metalKitView;
}

- (void)initIxtli {
    STScnContextItf ctxItf;
    memset(&ctxItf, 0, sizeof(ctxItf));
    //define context interface
    {
        //custom memory allocation (for memory leaks dtection)
        {
            //mem
            ctxItf.mem.malloc   = ScnMemMap_custom_malloc;
            ctxItf.mem.realloc  = ScnMemMap_custom_realloc;
            ctxItf.mem.free     = ScnMemMap_custom_free;
        }
        //use default for others
        ScnContextItf_fillMissingMembers(&ctxItf);
    }
    ctx = ScnContext_alloc(&ctxItf);
    if(ScnContext_isNull(ctx)){
        printf("ERROR, ScnContext_alloc failed.\n");
    } else {
        // Do any additional setup after loading the view.
        STScnApiItf itf;
        if(!ScnApiMetal_getApiItf(&itf)){
            printf("ERROR, ScnApiMetal_getApiItf failed.\n");
        } else {
            render = ScnRender_alloc(ctx);
            if(ScnRender_isNull(render)){
                printf("ERROR, ScnRender_alloc failed.\n");
            } else if(!ScnRender_prepare(render, &itf, NULL)){
                printf("ERROR, ScnRender_prepare failed.\n");
            } else if(!ScnRender_openDevice(render, NULL, IXTLI_RENDER_DEMO_RENDER_SLOTS_AMMOUNT)){
                printf("ERROR, ScnRender_openDevice failed.\n");
            } else {
                printf("Render initialized.\n");
                id<MTLDevice> apiDev = (__bridge id<MTLDevice>)ScnRender_getApiDevice(render);
                if(apiDev == nil){
                    printf("ERROR, ScnRender_getApiDevice failed.\n");
                } else {
                    metalKitView.device = apiDev;
                    framebuff = ScnRender_allocFramebuff(render);
                    if(ScnFramebuff_isNull(framebuff)){
                        printf("ERROR, ScnRender_allocFramebuff failed.\n");
                    } else if(!ScnFramebuff_bindToOSView(framebuff, (__bridge void*)metalKitView)){
                        printf("ERROR, ScnFramebuff_bindToOSView failed.\n");
                    } else {
                        //load texture
                        const char* texPath = [[NSString stringWithFormat:@"%@/cat256.png", [[NSBundle mainBundle] resourcePath]] UTF8String];
                        if(!ScnPngLoader_loadFromPath(ctx, texPath, &bmp.props, (void**)&bmp.data.ptr, &bmp.data.use)){
                            printf("ERROR, ScnPngLoader_loadFromPath failed for: '%s'.\n", texPath);
                        } else
                        /*const char* texPath = "runtime-drawing";
                        bmp.props       = ScnBitmapProps_build(256, 256, ENScnBitmapColor_RGBA8);
                        bmp.data.use    = bmp.props.bytesPerLine * bmp.props.size.height;
                        bmp.data.ptr    = (ScnBYTE*)ScnContext_malloc(ctx, bmp.data.use, "bmp.data.ptr ");
                        {
                            ScnUI32* p32 = (ScnUI32*)bmp.data.ptr;
                            const ScnUI32* p32AfterEnd = p32 + (bmp.data.use / 4);
                            while(p32 < p32AfterEnd){
                                *p32 = (0xA0 << 24) | (0x10 << 16) | (0x25 << 8) | 0xFF;
                                ++p32;
                            }
                        }*/
                        {
                            printf("PNG loaded: '%s'.\n", texPath);
                            STScnGpuTextureCfg texCfg = STScnGpuTextureCfg_Zero;
                            texCfg.width = bmp.props.size.width;
                            texCfg.height = bmp.props.size.height;
                            texCfg.color = bmp.props.color;
                            tex = ScnRender_allocTexture(render, &texCfg, &bmp.props, bmp.data.ptr);
                            if(ScnTexture_isNull(tex)){
                                printf("ERROR, ScnRender_allocTexture failed for: '%s'.\n", texPath);
                            } else {
                                node = ScnNode2d_alloc(ctx);
                                model = ScnRender_allocModel(render);
                                if(ScnNode2d_isNull(node) || ScnModel2d_isNull(model)){
                                    printf("ERROR, ScnNode2d_alloc or ScnRender_allocModel failed.\n");
                                } else {
                                    vertsSz = 3;
                                    verts   = ScnModel2d_addDrawTex(model, ENScnRenderShape_TriangStrip, vertsSz, tex);
                                    [self updateModelVerts:metalKitView.drawableSize];
                                    framesCount = 0;
                                    printf("Ixtli and demo initialized.\n");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

- (void)destroyIxtli {
    printf("Cleanup after %d frames.\n", framesCount);
    vertsSz = 0;
    verts = (STScnVertex2DTexPtr)STScnVertex2DTexPtr_Zero;
    //tex
    {
        //data
        if(bmp.data.ptr != NULL){
            ScnContext_mfree(ctx, bmp.data.ptr);
            bmp.data.ptr = NULL;
        }
        bmp.data.use = 0;
    }
    ScnTexture_releaseAndNull(&tex);
    ScnModel2d_releaseAndNull(&model);
    ScnNode2d_releaseAndNull(&node);
    ScnFramebuff_releaseAndNull(&framebuff);
    ScnRender_releaseAndNull(&render);
    ScnContext_releaseAndNull(&ctx);
    //custom memory manager
    {
        ScnMemMap_printFinalReport(&memmap);
    }
}
    
- (void)startOrStop {
    if(ScnContext_isNull(ctx)){
        [self initIxtli];
    } else {
        [self destroyIxtli];
    }
}

- (void)updateModelVerts:(CGSize)sz
{
    //build triangle with coord (0,0) as his center
    const float smallSz = (sz.width < sz.height ? sz.width : sz.height);
    const float smallSzHalf = smallSz / 2.f;
    STScnVertex2DTex *v, *vs[3]; float rad;
    //
    rad = DEG_2_RAD(90.f);
    vs[0] = v = &verts.ptr[0]; v->x = smallSzHalf * cosf(rad); v->y = smallSzHalf * sinf(rad);  v->color.r = 255; v->color.g = 55;  v->color.b = 155; v->color.a = 255;
    v->tex.x = 0.f; v->tex.y = 0.f;
    //
    rad = DEG_2_RAD(90.f + (360.f / 3.f));
    vs[1] = v = &verts.ptr[1]; v->x = smallSzHalf * cosf(rad); v->y = smallSzHalf * sinf(rad);  v->color.r = 55;  v->color.g = 155; v->color.b = 255; v->color.a = 255;
    v->tex.x = 1.f; v->tex.y = 0.f;
    //
    rad = DEG_2_RAD(90.f + (360.f * 2.f / 3.f));
    vs[2] = v = &verts.ptr[2]; v->x = smallSzHalf * cosf(rad); v->y = smallSzHalf * sinf(rad);  v->color.r = 155; v->color.g = 255; v->color.b = 55;  v->color.a = 255;
    v->tex.x = 1.f; v->tex.y = 1.f;
    //move to the center of the viewport
    ScnNode2d_setTranslateXY(node, sz.width / 2.f, sz.height / 2.f);
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view
{
    ctx             = (ScnContextRef)ScnContextRef_Zero;
    render          = (ScnRenderRef)ScnObjRef_Zero;
    framebuff       = (ScnFramebuffRef)ScnObjRef_Zero;
    node            = (ScnNode2dRef)ScnObjRef_Zero;
    model           = (ScnModel2dRef)ScnObjRef_Zero;
    verts           = (STScnVertex2DTexPtr)STScnVertex2DTexPtr_Zero;
    memset(&tex, 0, sizeof(tex));
    vertsSz         = 0;
    metalKitView    = nil;

    self = [super init];
    if (nil == self) { return nil; }

    metalKitView = view;

    //Init memory custom manager
    ScnMemMap_init(&memmap);
    gMemmap = &memmap;
    
    [self initIxtli];
    
    return self;
}

/// Notifies the app when the system adjusts the size of its viewable area.
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    //NSLog(@"MetalKitViewDelegate, mtkView:drawableSizeWillChange(%f, %f).\n", size.width, size.height);
    //sync model vertices
    if(!ScnModel2d_isNull(model)){
        [self updateModelVerts:size];
        ScnModel2d_v1FlagForSync(model, verts, vertsSz);
    }
    //sync framebuffer
    if(!ScnFramebuff_isNull(framebuff)){
        const STScnSize2DU uSize = (STScnSize2DU){ size.width, size.height };
        STScnGpuFramebuffProps props = ScnFramebuff_getProps(framebuff);
        props.viewport  = (STScnRectU) { 0u, 0u, size.width, size.height };
        props.ortho     = (STScnAABBox3d){ { 0.f, size.width }, { 0.f, size.height }, { 0.f, 1.f } };
        if(!ScnFramebuff_syncSize(framebuff, uSize)){
            printf("ScnFramebuff_syncSize(%.1f %.1f) failed.\n", size.width, size.height);
        } else if(!ScnFramebuff_setProps(framebuff, &props)){
            printf("ScnFramebuff_syncSizeAndViewport(%.1f %.1f) failed.\n", size.width, size.height);
        } else {
            printf("ScnFramebuff_syncSizeAndViewport(%.1f %.1f) ok.\n", size.width, size.height);
        }
    }
}

/// Notifies the app when the system is ready draw a frame into a view.
- (void)drawInMTKView:(nonnull MTKView *)view
{
    //animation
    if(!ScnNode2d_isNull(node)){
        const float rotDeg = ScnNode2d_getRotDeg(node);
        ScnNode2d_setRotDeg(node, rotDeg + 1.f);
    }
    //render
    SCN_ASSERT(ScnRender_isNull(render) || view.device == (__bridge id<MTLDevice>)ScnRender_getApiDevice(render))
    if(!ScnRender_isNull(render) && ScnRender_hasOpenDevice(render) && !ScnFramebuff_isNull(framebuff)){
        if(!ScnRender_jobStart(render)){
            printf("ScnRender_jobStart failed.\n");
        } else {
            if(!ScnRender_jobFramebuffPush(render, framebuff)){
                printf("ScnRender_jobFramebuffPush failed.\n");
            } else {
                if(!ScnModel2d_isNull(model) && !ScnNode2d_isNull(node) && !ScnRender_jobModel2dAddWithNode(render, model, node)){
                    printf("ScnRender_jobModel2dAddWithNode failed.\n");
                }
                if(!ScnRender_jobFramebuffPop(render)){
                    printf("ScnRender_jobFramebuffPop failed.\n");
                }
            }
            if(!ScnRender_jobEnd(render)){
                printf("ScnRender_jobEnd failed.\n");
            }
        }
    }
    //
    if(framesCount < IXTLI_FRAMES_COUNT_AND_RELEASE_RENDER){
        ++framesCount;
        if((framesCount % 120) == 0){
            printf("%d frames.\n", framesCount);
        }
    }
}

@end

NS_ASSUME_NONNULL_END

//custom memory allocation (for memory leaks detection)

void* ScnMemMap_custom_malloc(const ScnUI32 newSz, const char* dbgHintStr){
    void* r = malloc(newSz);
    if(gMemmap == NULL){
        SCN_PRINTF_ERROR("ScnMemMap_custom_malloc::gCurInstance is NULL.\n");
    } else {
        ScnMemMap_ptrAdd(gMemmap, r, newSz, dbgHintStr);
    }
    return r;
}

void* ScnMemMap_custom_realloc(void* ptr, const ScnUI32 newSz, const char* dbgHintStr){
    //"If there is not enough memory, the old memory block is not freed and null pointer is returned."
    void* r = realloc(ptr, newSz);
    if(gMemmap == NULL){
        SCN_PRINTF_ERROR("ScnMemMap_custom_realloc::gCurInstance is NULL.\n");
    } else {
        if(r != NULL){
            if(ptr != NULL){
                ScnMemMap_ptrRemove(gMemmap, ptr);
            }
            ScnMemMap_ptrAdd(gMemmap, r, newSz, dbgHintStr);
        }
    }
    return r;
}

void ScnMemMap_custom_free(void* ptr){
    if(gMemmap == NULL){
        SCN_PRINTF_ERROR("ScnMemMap_custom_free::gCurInstance is NULL.\n");
    } else {
        ScnMemMap_ptrRemove(gMemmap, ptr);
    }
    free(ptr);
}
