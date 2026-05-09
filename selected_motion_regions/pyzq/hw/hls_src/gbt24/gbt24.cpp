#include "gbt24.h"

static ap_uint<8> rgb_to_gray(ap_uint<8> r, ap_uint<8> g, ap_uint<8> b) {
    // Integer approximation:
    // gray = 0.299R + 0.587G + 0.114B
    return (ap_uint<8>)((77 * r + 150 * g + 29 * b) >> 8);
}

void gbt24(
    hls::stream<axis24_t>& s_axis_video,
    hls::stream<axis24_t>& m_axis_video,
    int width,
    int height,
    int threshold,
    int mode,
    int enable_blur
) {
#pragma HLS INTERFACE axis port=s_axis_video
#pragma HLS INTERFACE axis port=m_axis_video

#pragma HLS INTERFACE s_axilite port=width       bundle=CTRL
#pragma HLS INTERFACE s_axilite port=height      bundle=CTRL
#pragma HLS INTERFACE s_axilite port=threshold   bundle=CTRL
#pragma HLS INTERFACE s_axilite port=mode        bundle=CTRL
#pragma HLS INTERFACE s_axilite port=enable_blur bundle=CTRL
#pragma HLS INTERFACE s_axilite port=return      bundle=CTRL

    // Phase 1: enable_blur is reserved, not used yet
    (void)enable_blur;

Row_Loop:
    for (int y = 0; y < height; y++) {
    Col_Loop:
        for (int x = 0; x < width; x++) {
#pragma HLS PIPELINE II=1

            axis24_t in_pix  = s_axis_video.read();
            axis24_t out_pix = in_pix;

            ap_uint<8> r = in_pix.data.range(23, 16);
            ap_uint<8> g = in_pix.data.range(15, 8);
            ap_uint<8> b = in_pix.data.range(7, 0);

            ap_uint<8> gray = rgb_to_gray(r, g, b);
            ap_uint<8> outv;

            if (mode == 0) {
                // bypass
                out_pix.data = in_pix.data;
            } else if (mode == 1) {
                // grayscale
                outv = gray;
                ap_uint<24> rgb_out = ((ap_uint<24>)outv << 16) |
                                      ((ap_uint<24>)outv << 8)  |
                                      (ap_uint<24>)outv;
                out_pix.data = rgb_out;
            } else {
                // mode == 2 : grayscale + threshold
                outv = (gray >= threshold) ? (ap_uint<8>)255 : (ap_uint<8>)0;
                ap_uint<24> rgb_out = ((ap_uint<24>)outv << 16) |
                                      ((ap_uint<24>)outv << 8)  |
                                      (ap_uint<24>)outv;
                out_pix.data = rgb_out;
            }

            // Preserve AXI4-Stream sideband signals
            out_pix.keep = in_pix.keep;
            out_pix.strb = in_pix.strb;
            out_pix.user = in_pix.user;
            out_pix.last = in_pix.last;
            out_pix.id   = in_pix.id;
            out_pix.dest = in_pix.dest;

            m_axis_video.write(out_pix);
        }
    }
}