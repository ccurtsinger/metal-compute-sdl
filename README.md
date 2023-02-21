# Metal Compute to SDL
This is a small working example of how to use a Metal compute kernel to generate a texture and dispaly it using SDL. The texture is stored exclusively on the GPU.

This demo was based on a few different examples:

1. [Metal in C++ with SDL2](https://schneide.blog/2022/03/28/metal-in-c-with-sdl2/) shows how to bind a Metal device to an SDL view
2. [Learn Metal with C++](https://developer.apple.com/metal/LearnMetalCPP.zip) includes an example compute kernel and the corresponding C++ code to launch the kernel. The kernel in this example was adapted from Apple's code.
3. [larsgeb/m1-gpu-cpp](https://github.com/larsgeb/m1-gpu-cpp) includes additional compute-only examples of metal-cpp
