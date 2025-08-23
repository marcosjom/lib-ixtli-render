//
//  MTKViewDelegateHelloTexture.m
//  ixtli-render-demos-mac
//
//  Created by Marcos Ortega on 28/7/25.
//

#import "MTKViewDelegateHelloTexture.h"

#include "ixrender/ScnRender.h"
#include "ixrender/api/ScnApiMetal.h"
#include "ixrender/type/ScnBitmap.h"
#include "../src/utils/ScnMemMap.h"
#include "../src/utils/ScnTextureMaker.h"

// Custom memory allocation for this demos and tests,
// for detecting memory-leaks using ScnMemMap.
void* ScnMemMap_custom_malloc(const ScnUI32 newSz, const char* dbgHintStr);
void* ScnMemMap_custom_realloc(void* ptr, const ScnUI32 newSz, const char* dbgHintStr);
void ScnMemMap_custom_free(void* ptr);
STScnMemMap* gMemmap = NULL;

NS_ASSUME_NONNULL_BEGIN

#define IXTLI_RENDER_DEMO_RENDER_SLOTS_AMMOUNT  3
#define IXTLI_FRAMES_COUNT_AND_RELEASE_RENDER   360

@implementation MTKViewDelegateHelloTexture {
    ScnContextRef       ctx;
    ScnRenderRef        render;
    ScnFramebuffRef     framebuff;
    ScnModel2dRef       model;
    ScnTextureRef       tex;
    STScnVertex2DTexPtr verts;
    ScnUI32             vertsSz;
    ScnUI32             framesCount;
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
@protected
    MTKView             *metalKitView;
}

- (void)initIxtli {
    //init context
    {
        STScnContextItf ctxItf;
        ScnMemset(&ctxItf, 0, sizeof(ctxItf));
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
            NSLog(@"ScnContext_alloc failed.\n");
            return;
        }
    }
    //init render
    {
        //get API (Metal for Apple)
        STScnApiItf itf;
        if(!ScnApiMetal_getApiItf(&itf)){
            NSLog(@"ScnApiMetal_getApiItf failed.\n");
            return;
        }
        //allocate render
        render = ScnRender_alloc(ctx);
        if(ScnRender_isNull(render)){
            NSLog(@"ScnRender_alloc failed.\n");
            return;
        }
        //prepare render
        STScnRenderCfg cfg = ScnRender_getDefaultCfg();
        if(!ScnRender_prepare(render, &itf, NULL, &cfg)){
            NSLog(@"ScnRender_prepare failed.\n");
            ScnRender_releaseAndNull(&render);
            return;
        }
        //open device
        if(!ScnRender_openDevice(render, NULL, IXTLI_RENDER_DEMO_RENDER_SLOTS_AMMOUNT)){
            NSLog(@"ScnRender_openDevice failed.\n");
            ScnRender_releaseAndNull(&render);
            return;
        }
    }
    //init framebuffer
    {
        //allocate framebuffer
        framebuff = ScnRender_allocFramebuff(render);
        if(ScnFramebuff_isNull(framebuff)){
            NSLog(@"ScnRender_allocFramebuff failed.\n");
            return;
        }
        //bind to view
        if(!ScnFramebuff_bindToOSView(framebuff, (__bridge void*)metalKitView)){
            NSLog(@"ScnFramebuff_bindToOSView failed.\n");
            ScnFramebuff_releaseAndNull(&framebuff);
            return;
        }
    }
    //init texture
    {
        ScnBitmapRef bmp = ScnTextureMaker_make(ctx, 256, 256, ENScnBitmapColor_RGBA8, 32);
        if(ScnBitmap_isNull(bmp)){
            NSLog(@"ScnTextureMaker_make failed.\n");
            return;
        }
        void* bmpData = NULL;
        const STScnBitmapProps bmpProps = ScnBitmap_getProps(bmp, &bmpData);
        STScnGpuTextureCfg texCfg = STScnGpuTextureCfg_Zero;
        texCfg.width    = bmpProps.size.width;
        texCfg.height   = bmpProps.size.height;
        texCfg.color    = bmpProps.color;
        tex = ScnRender_allocTexture(render, ENScnResourceMode_Static, &texCfg, &bmpProps, bmpData);
        if(ScnTexture_isNull(tex)){
             NSLog(@"ScnRender_allocTexture failed.\n");
            ScnBitmap_releaseAndNull(&bmp);
            return;
        }
        ScnBitmap_releaseAndNull(&bmp);
    }
    //init scene
    {
        model = ScnRender_allocModel(render);
        if(ScnModel2d_isNull(model)){
            NSLog(@"ScnRender_allocModel failed.\n");
            return;
        }
        //allocate vertices
        vertsSz = 4;
        verts   = ScnModel2d_addDrawTex(model, ENScnRenderShape_TriangStrip, vertsSz, tex);
        if(verts.ptr == NULL){
            NSLog(@"ScnModel2d_addDraw failed.\n");
            return;
        }
        //update vertices data
        [self updateModelVertsSquare:metalKitView.drawableSize];
        //start
        framesCount = 0;
        NSLog(@"Ixtli demo initialized");
    }
}

- (void)destroyIxtli {
     NSLog(@"Cleanup after %d frames.\n", framesCount);
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

- (void)updateModelVertsSquare:(CGSize)sz
{
    //build triangle with coord (0,0) as his center
    const float smallSz = (sz.width < sz.height ? sz.width : sz.height);
    const float smallSzHalf = smallSz / 4.f;
    STScnVertex2DTex *v, *vs[4];
    //
    vs[0] = v = &verts.ptr[0];
    v->x = -smallSzHalf;
    v->y = -smallSzHalf;
    v->tex.x = 0.f;
    v->tex.y = 0.f;
    v->color.r = 255;
    v->color.g = 255;
    v->color.b = 255;
    v->color.a = 255;
    //
    vs[1] = v = &verts.ptr[1];
    v->x = smallSzHalf;
    v->y = -smallSzHalf;
    v->tex.x = 1.f;
    v->tex.y = 0.f;
    v->color.r = 255;
    v->color.g = 255;
    v->color.b = 255;
    v->color.a = 255;
    //
    vs[2] = v = &verts.ptr[2];
    v->x = -smallSzHalf;
    v->y = smallSzHalf;
    v->tex.x = 0.f;
    v->tex.y = 1.f;
    v->color.r = 255;
    v->color.g = 255;
    v->color.b = 255;
    v->color.a = 255;
    //
    vs[3] = v = &verts.ptr[3];
    v->x = smallSzHalf;
    v->y = smallSzHalf;
    v->tex.x = 1.f;
    v->tex.y = 1.f;
    v->color.r = 255;
    v->color.g = 255;
    v->color.b = 255;
    v->color.a = 255;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view
{
    ctx             = (ScnContextRef)ScnContextRef_Zero;
    render          = (ScnRenderRef)ScnObjRef_Zero;
    framebuff       = (ScnFramebuffRef)ScnObjRef_Zero;
    model           = (ScnModel2dRef)ScnObjRef_Zero;
    verts           = (STScnVertex2DTexPtr)STScnVertex2DTexPtr_Zero;
    ScnMemset(&tex, 0, sizeof(tex));
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
    //NSLog(@"MTKViewDelegateHelloTexture, mtkView:drawableSizeWillChange(%f, %f).\n", size.width, size.height);
    //sync model vertices
    if(!ScnModel2d_isNull(model)){
        [self updateModelVertsSquare:size];
        ScnModel2d_v1FlagForSync(model, verts, vertsSz);
    }
    //sync framebuffer
    if(!ScnFramebuff_isNull(framebuff)){
        const STScnSize2DU uSize = (STScnSize2DU){ size.width, size.height };
        STScnGpuFramebuffProps props = ScnFramebuff_getProps(framebuff);
        props.viewport  = (STScnRectU) { 0u, 0u, size.width, size.height };
        props.ortho     = (STScnAABBox3d){ { 0.f, size.width }, { 0.f, size.height }, { 0.f, 1.f } };
        if(!ScnFramebuff_syncSize(framebuff, uSize)){
             NSLog(@"ScnFramebuff_syncSize(%.1f %.1f) failed.\n", size.width, size.height);
        } else if(!ScnFramebuff_setProps(framebuff, &props)){
             NSLog(@"ScnFramebuff_syncSizeAndViewport(%.1f %.1f) failed.\n", size.width, size.height);
        } else {
             NSLog(@"ScnFramebuff_syncSizeAndViewport(%.1f %.1f) ok.\n", size.width, size.height);
        }
    }
}

/// Notifies the app when the system is ready draw a frame into a view.
- (void)drawInMTKView:(nonnull MTKView *)view
{
    //render
    SCN_ASSERT(ScnRender_isNull(render) || view.device == (__bridge id<MTLDevice>)ScnRender_getApiDevice(render))
    if(ScnRender_isNull(render) || ScnFramebuff_isNull(framebuff) || ScnModel2d_isNull(model) || !ScnRender_hasOpenDevice(render)){
        return;
    }
    //render job
    {
        //get render slot
        ScnRenderJobRef job = ScnRender_allocRenderJob(render);
        if(ScnRenderJob_isNull(job)){
            return;
        }
        //push framebuffer
        if(!ScnRenderJob_framebuffPush(job, framebuff)){
            NSLog(@"ScnRender_jobFramebuffPush failed.\n");
            ScnRenderJob_releaseAndNull(&job);
            return;
        }
        //add model (centered to view)
        {
            STScnGpuFramebuffProps fbProps = ScnFramebuff_getProps(framebuff);
            STScnNode2dProps mldProps = STScnNode2dProps_Identity;
            mldProps.tform.tx = fbProps.viewport.width / 2.f;
            mldProps.tform.ty = fbProps.viewport.height / 2.f;
            if(!ScnRenderJob_model2dAddWithNodeProps(job, model, mldProps)){
                NSLog(@"ScnRenderJob_model2dAddWithNodeProps failed.\n");
                ScnRenderJob_releaseAndNull(&job);
                return;
            }
        }
        //pop framebuffer
        if(!ScnRenderJob_framebuffPop(job)){
            NSLog(@"ScnRenderJob_framebuffPop failed.\n");
            ScnRenderJob_releaseAndNull(&job);
            return;
        }
        //enqueue job
        if(!ScnRender_enqueue(render, job)){
            NSLog(@"ScnRender_enqueue failed.\n");
        }
        //release job
        ScnRenderJob_releaseAndNull(&job);
    }
    //
    if(framesCount < IXTLI_FRAMES_COUNT_AND_RELEASE_RENDER){
        ++framesCount;
        if((framesCount % 120) == 0){
            NSLog(@"%d frames.\n", framesCount);
        }
    }
}

@end

NS_ASSUME_NONNULL_END

//custom memory allocation (for memory leaks detection)

void* ScnMemMap_custom_malloc(const ScnUI32 newSz, const char* dbgHintStr){
    void* r = malloc(newSz);
    if(gMemmap == NULL){
         NSLog(@"ScnMemMap_custom_malloc::gCurInstance is NULL.\n");
    } else {
        ScnMemMap_ptrAdd(gMemmap, r, newSz, dbgHintStr);
    }
    return r;
}

void* ScnMemMap_custom_realloc(void* ptr, const ScnUI32 newSz, const char* dbgHintStr){
    //"If there is not enough memory, the old memory block is not freed and null pointer is returned."
    void* r = realloc(ptr, newSz);
    if(gMemmap == NULL){
         NSLog(@"ScnMemMap_custom_realloc::gCurInstance is NULL.\n");
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
         NSLog(@"ScnMemMap_custom_free::gCurInstance is NULL.\n");
    } else {
        ScnMemMap_ptrRemove(gMemmap, ptr);
    }
    free(ptr);
}
