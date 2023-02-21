#include <SDL.h>
#include <assert.h>

#include "mandelbrot_kernel.h"
#include "metal_bindings.h"

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

using NS::StringEncoding::UTF8StringEncoding;

struct viewport {
  float x;
  float y;
  float scale;
};

int main(int argc, char** argv) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  // Create a window
  SDL_Window* window = SDL_CreateWindow(
      "Mandelbrot Set", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH,
      WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
  if (window == NULL) {
    fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  // Get drawable dimensions
  int screen_width = 0;
  int screen_height = 0;
  SDL_Metal_GetDrawableSize(window, &screen_width, &screen_height);
  if (screen_width == 0 || screen_height == 0) {
    fprintf(stderr, "Unable to get drawable dimensions: %s\n", SDL_GetError());
    SDL_Quit();
    exit(EXIT_FAILURE);
  }

  // Create a Metal view
  auto metal_view = SDL_Metal_CreateView(window);
  assert(metal_view != nullptr);

  // Create Metal device
  auto device = MTL::CreateSystemDefaultDevice();
  assert(device != nullptr);

  // Create a command queue for the metal device
  auto queue = device->newCommandQueue();
  assert(queue != nullptr);

  // Package the kernel binary blob
  auto lib_data = dispatch_data_create(mandelbrot_metallib, mandelbrot_metallib_len,
                                       dispatch_get_global_queue(0, 0), {});
  
  // Build a metal library from the binary blob
  NS::Error* error = nullptr;
  auto library = device->newLibrary(lib_data, &error);
  assert(library != nullptr);

  // Get the mandelbrot function
  auto mandelbrot_fn =
      library->newFunction(NS::String::string("mandelbrot_set", NS::UTF8StringEncoding));
  assert(mandelbrot_fn != nullptr);

  // Create a pipeline state object
  auto pso = device->newComputePipelineState(mandelbrot_fn, &error);
  assert(pso != nullptr);

  // Clean up library and function objects
  mandelbrot_fn->release();
  library->release();

  // Assign the metal device to the metal view
  assign_device(SDL_Metal_GetLayer(metal_view), device);

  // Create a buffer to track the viewport position and scale
  auto position_buffer =
      device->newBuffer(sizeof(viewport), MTL::ResourceStorageModeManaged);
  assert(position_buffer != nullptr);

  // Viewport and rate of change
  viewport view = {-1.0, 0.0, 2.5};
  viewport delta = {0.0, 0.0, 0.0};

  // Event loop
  bool running = true;
  while (running) {
    // Update the position buffer
    viewport* ptr = reinterpret_cast<viewport*>(position_buffer->contents());
    *ptr = view;
    position_buffer->didModifyRange(NS::Range::Make(0, sizeof(viewport)));

    // Get the next drawable surface from the metal view
    auto surface = next_drawable(SDL_Metal_GetLayer(metal_view));
    assert(surface != nullptr);

    // Prepare to run the pipeline
    auto buffer = queue->commandBuffer();
    assert(buffer != nullptr);
    auto encoder = buffer->computeCommandEncoder();
    assert(encoder != nullptr);
    encoder->setComputePipelineState(pso);

    // Set arguments for the kernel
    encoder->setTexture(surface->texture(), 0);
    encoder->setBuffer(position_buffer, 0, 0);

    // Set kernel dimensions
    auto grid_size = MTL::Size(screen_width, screen_height, 1);
    auto thread_group_size = MTL::Size(pso->maxTotalThreadsPerThreadgroup(), 1, 1);
    encoder->dispatchThreads(grid_size, thread_group_size);

    // Finished setting up kernel invocation
    encoder->endEncoding();

    // Ask Metal to display the rendered surface
    buffer->presentDrawable(surface);

    // Start the kernel
    buffer->commit();

    // Process SDL events while the kernel executes
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      } else if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
          SDL_Metal_GetDrawableSize(window, &screen_width, &screen_height);
        }
      }
    }

    // Check key states
    const uint8_t* keys = SDL_GetKeyboardState(NULL);

    // Move up and down
    if (keys[SDL_SCANCODE_UP]) {
      delta.y = -1;
    } else if (keys[SDL_SCANCODE_DOWN]) {
      delta.y = 1;
    } else {
      delta.y *= 0.9;
    }

    // Move left and right
    if (keys[SDL_SCANCODE_LEFT]) {
      delta.x = -1;
    } else if (keys[SDL_SCANCODE_RIGHT]) {
      delta.x = 1;
    } else {
      delta.x *= 0.9;
    }

    // Zoom in and out
    if (keys[SDL_SCANCODE_EQUALS]) {
      delta.scale = -view.scale / 50;
    } else if (keys[SDL_SCANCODE_MINUS]) {
      delta.scale = view.scale / 50;
    } else {
      delta.scale *= 0.9;
    }

    // Update scale
    view.scale += delta.scale;
    //if (view.scale < 0.001) view.scale = 0.001;

    // Update position
    view.x += delta.x * 0.005 * view.scale;
    view.y += delta.y * 0.005 * view.scale;

    // Finish on the GPU before moving to the next frame
    buffer->waitUntilCompleted();
  }

  // Clean up Metal resources
  position_buffer->release();
  pso->release();
  queue->release();
  device->release();

  // Clean up SDL resources
  SDL_Metal_DestroyView(metal_view);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
