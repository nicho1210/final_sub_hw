#include "video_ip.h"

#include <iostream>
#include <thread>
#include <cstdint>

static axis_pixel_t make_pixel(ap_uint<8> v, bool sof, bool eol)
{
    axis_pixel_t p;
    p.data.range(23, 16) = v;
    p.data.range(15, 8)  = v;
    p.data.range(7, 0)   = v;
    p.user = sof ? 1 : 0;
    p.last = eol ? 1 : 0;
    p.keep = 0x7;
    p.strb = 0x7;
    p.id   = 0;
    p.dest = 0;
    return p;
}

static void feed_frame(
    hls::stream<axis_pixel_t> &in_stream,
    int hot_x,
    int hot_y,
    ap_uint<8> hot_value,
    ap_uint<8> bg_value)
{
    for (int y = 0; y < FRAME_HEIGHT; ++y) {
        for (int x = 0; x < FRAME_WIDTH; ++x) {
            bool sof = (x == 0 && y == 0);
            bool eol = (x == FRAME_WIDTH - 1);
            ap_uint<8> v = (x == hot_x && y == hot_y) ? hot_value : bg_value;
            in_stream.write(make_pixel(v, sof, eol));
        }
    }
}

static int drain_and_check_frame(
    hls::stream<axis_pixel_t> &out_stream,
    int hot_x,
    int hot_y,
    ap_uint<8> hot_expect,
    ap_uint<8> bg_expect)
{
    int errors = 0;

    for (int y = 0; y < FRAME_HEIGHT; ++y) {
        for (int x = 0; x < FRAME_WIDTH; ++x) {
            axis_pixel_t p = out_stream.read();

            ap_uint<8> expect = (x == hot_x && y == hot_y) ? hot_expect : bg_expect;
            ap_uint<8> r = p.data.range(23, 16);
            ap_uint<8> g = p.data.range(15, 8);
            ap_uint<8> b = p.data.range(7, 0);

            if (r != expect || g != expect || b != expect) {
                if (errors < 8) {
                    std::cout << "Pixel mismatch at (" << x << "," << y << ") "
                              << "got=" << (unsigned)r
                              << " expect=" << (unsigned)expect << std::endl;
                }
                ++errors;
            }

            bool sof_expect = (x == 0 && y == 0);
            bool eol_expect = (x == FRAME_WIDTH - 1);

            if ((unsigned)p.user != (unsigned)(sof_expect ? 1 : 0)) {
                if (errors < 8) {
                    std::cout << "TUSER mismatch at (" << x << "," << y << ")" << std::endl;
                }
                ++errors;
            }

            if ((unsigned)p.last != (unsigned)(eol_expect ? 1 : 0)) {
                if (errors < 8) {
                    std::cout << "TLAST mismatch at (" << x << "," << y << ")" << std::endl;
                }
                ++errors;
            }
        }
    }

    return errors;
}

int main()
{
    hls::stream<axis_pixel_t> in_stream("in_stream");
    hls::stream<axis_pixel_t> out_stream("out_stream");
    ap_uint<32> motion_info_out = 0;

    const int hot_x = 640;   // divisible by 4, region 5
    const int hot_y = 360;   // divisible by 4, region 5

    int errors = 0;

    // -----------------------------
    // Frame 1: all black
    // -----------------------------
    feed_frame(in_stream, -1, -1, 255, 0);

    video_gray_live(in_stream, out_stream, &motion_info_out);

    errors += drain_and_check_frame(out_stream, -1, -1, 255, 0);

    unsigned count_f1 = (unsigned)(motion_info_out & 0xFFFF);
    unsigned mask_f1  = (unsigned)((motion_info_out >> 16) & 0x01FF);

    if (count_f1 != 0 || mask_f1 != 0) {
        std::cout << "Frame 1 motion info mismatch: "
                  << "count=" << count_f1
                  << " mask=0x" << std::hex << mask_f1 << std::dec << std::endl;
        ++errors;
    }

    // -----------------------------
    // Frame 2: one bright sampled block in region 5
    // -----------------------------
    feed_frame(in_stream, hot_x, hot_y, 255, 0);

    video_gray_live(in_stream, out_stream, &motion_info_out);

    errors += drain_and_check_frame(out_stream, hot_x, hot_y, 255, 0);

    unsigned count_f2 = (unsigned)(motion_info_out & 0xFFFF);
    unsigned mask_f2  = (unsigned)((motion_info_out >> 16) & 0x01FF);

    if (count_f2 != 1) {
        std::cout << "Frame 2 motion_count mismatch: got=" << count_f2
                  << " expect=1" << std::endl;
        ++errors;
    }

    if (mask_f2 != (1u << 4)) {
        std::cout << "Frame 2 region_mask mismatch: got=0x"
                  << std::hex << mask_f2
                  << " expect=0x" << (1u << 4) << std::dec << std::endl;
        ++errors;
    }

    if (errors == 0) {
        std::cout << "ALL TESTS PASSED" << std::endl;
        return 0;
    } else {
        std::cout << "TEST FAILED with " << errors << " error(s)" << std::endl;
        return errors;
    }
}
