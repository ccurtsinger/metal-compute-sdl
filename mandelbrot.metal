#include <metal_stdlib>

using namespace metal;

struct viewport {
  float x;
  float y;
  float scale;
};

// Mandelbrot kernel adapted from Apple's metal-cpp example code (https://developer.apple.com/metal/sample-code/)
kernel void mandelbrot_set(texture2d< half, access::write > tex [[texture(0)]],
                              uint2 index [[thread_position_in_grid]],
                              uint2 gridSize [[threads_per_grid]],
                              device viewport* view [[buffer(0)]]) {
    float2 kMandelbrotOrigin = {view->x, view->y};
    float2 kMandelbrotScale = {view->scale * gridSize.x / gridSize.y, view->scale};

    // Scale
    float x0 = kMandelbrotScale.x * ((float)index.x / gridSize.x) + kMandelbrotOrigin.x - kMandelbrotScale.x / 2;
    float y0 = kMandelbrotScale.y * ((float)index.y / gridSize.y) + kMandelbrotOrigin.y - kMandelbrotScale.y / 2;

    // Implement Mandelbrot set
    float x = 0.0;
    float y = 0.0;
    uint iteration = 0;
    uint max_iteration = 500;
    float xtmp = 0.0;
    while(x * x + y * y <= 4 && iteration < max_iteration)
    {
        xtmp = x * x - y * y + x0;
        y = 2 * x * y + y0;
        x = xtmp;
        iteration += 1;
    }

    // Convert iteration result to colors
    half color = (0.5 + 0.5 * cos(3.0 + iteration * 0.15));
    tex.write(half4(color, color, color, 1.0), index, 0);
}