/// From https://schneide.blog/2022/03/28/metal-in-c-with-sdl2/

#include <Metal/Metal.hpp>

#import <QuartzCore/CAMetalLayer.h>
#import <QuartzCore/QuartzCore.hpp>

void assign_device(void* layer, MTL::Device* device)
{
  CAMetalLayer* metalLayer = (CAMetalLayer*) layer;
  metalLayer.device = (__bridge id<MTLDevice>)(device);
}

CA::MetalDrawable* next_drawable(void* layer)
{
  CAMetalLayer* metalLayer = (CAMetalLayer*) layer;
  id <CAMetalDrawable> metalDrawable = [metalLayer nextDrawable];
  CA::MetalDrawable* pMetalCppDrawable = ( __bridge CA::MetalDrawable*) metalDrawable;
  return pMetalCppDrawable;
}
