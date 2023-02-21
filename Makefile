CXX := clang++
CFLAGS := -g -O2 $(shell pkg-config --cflags sdl2) -framework Metal -framework Foundation -framework MetalKit -Imetal-cpp
CXXFLAGS := --std=c++17 $(CFLAGS)
LDFLAGS := $(shell pkg-config --libs sdl2) -lm

all: mandelbrot

clean:
	rm -rf mandelbrot mandelbrot.dSYM mandelbrot_kernel.h

mandelbrot: mandelbrot.cc metal_bindings.mm metal_bindings.h mandelbrot_kernel.h
	$(CXX) $(CXXFLAGS) -o $@ mandelbrot.cc metal_bindings.mm $(LDFLAGS)

# Compile metal to a binary blob, just like SDL does
mandelbrot_kernel.h: mandelbrot.metal
	xcrun -sdk macosx metal mandelbrot.metal -o mandelbrot.metallib
	xxd -i mandelbrot.metallib | perl -w -p -e 's/\Aunsigned /const unsigned /;' > $@
	rm mandelbrot.metallib
