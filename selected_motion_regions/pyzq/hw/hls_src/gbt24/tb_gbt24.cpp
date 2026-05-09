#include "gbt24.h"
#include <iostream>
#include <cassert>

static axis24_t make_pixel(unsigned char r, unsigned char g, unsigned char b,
                           int user, int last) {
    axis24_t p;
    p.data = ((unsigned int)r << 16) | ((unsigned int)g << 8) | (unsigned int)b;
    p.keep = 0x7;
    p.strb = 0x7;
    p.user = user;
    p.last = last;
    p.id   = 0;
    p.dest = 0;
    return p;
}

int main() {
    hls::stream<axis24_t> in_stream;
    hls::stream<axis24_t> out_stream;

    const int W = 4;
    const int H = 2;

    // Frame:
    // Row 0
    in_stream.write(make_pixel(255,   0,   0, 1, 0)); // red
    in_stream.write(make_pixel(  0, 255,   0, 0, 0)); // green
    in_stream.write(make_pixel(  0,   0, 255, 0, 0)); // blue
    in_stream.write(make_pixel(255, 255, 255, 0, 1)); // white, end of line

    // Row 1
    in_stream.write(make_pixel(  0,   0,   0, 0, 0)); // black
    in_stream.write(make_pixel(128, 128, 128, 0, 0)); // gray
    in_stream.write(make_pixel(200, 100,  50, 0, 0)); // mixed
    in_stream.write(make_pixel( 10,  20,  30, 0, 1)); // end of line

    // mode = 2 => grayscale + threshold
    // threshold = 128
    gbt24(in_stream, out_stream, W, H, 128, 2, 0);

    for (int i = 0; i < W * H; i++) {
        axis24_t p = out_stream.read();

        unsigned int rgb = (unsigned int)p.data;
        unsigned char r = (rgb >> 16) & 0xFF;
        unsigned char g = (rgb >> 8)  & 0xFF;
        unsigned char b = (rgb >> 0)  & 0xFF;

        std::cout << "pix[" << i << "] = "
                  << (int)r << ", "
                  << (int)g << ", "
                  << (int)b
                  << " user=" << (int)p.user
                  << " last=" << (int)p.last
                  << std::endl;
    }

    std::cout << "Testbench finished." << std::endl;
    return 0;
}