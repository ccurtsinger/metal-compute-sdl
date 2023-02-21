
namespace MTL {
  class Device;
};

namespace CA {
  class MetalDrawable;
};

void assign_device(void* layer, MTL::Device* device);

CA::MetalDrawable* next_drawable(void* layer);