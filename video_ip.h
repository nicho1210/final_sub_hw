#ifndef VIDEO_IP_H
#define VIDEO_IP_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>

// Fixed design resolution used by the final PYNQ HDMI build
#define FRAME_WIDTH   1280
#define FRAME_HEIGHT  720

// Motion detector downsamples by 4x4 blocks
#define DS_SHIFT      2
#define MOT_W         (FRAME_WIDTH  >> DS_SHIFT)   // 320
#define MOT_H         (FRAME_HEIGHT >> DS_SHIFT)   // 180
#define MOT_SIZE      (MOT_W * MOT_H)

// 3x3 region split points
#define X_SPLIT1      (FRAME_WIDTH / 3)            // 426
#define X_SPLIT2      ((FRAME_WIDTH * 2) / 3)      // 853
#define Y_SPLIT1      (FRAME_HEIGHT / 3)           // 240
#define Y_SPLIT2      ((FRAME_HEIGHT * 2) / 3)     // 480

// AXI4-Stream pixel: RGB888 + sideband fields
typedef ap_axiu<24,1,1,1> axis_pixel_t;

// Final top-level IP
//
// in_stream        : AXI4-Stream video input
// out_stream       : AXI4-Stream thresholded video output
// motion_count_out : packed 32-bit motion information
//                    [15:0]  = motion_count
//                    [24:16] = region_mask (regions 1..9 -> bits 0..8)
//                    [31:25] = reserved
void video_gray_live(
    hls::stream<axis_pixel_t>& in_stream,
    hls::stream<axis_pixel_t>& out_stream,
    volatile ap_uint<32> *motion_count_out
);

#endif // VIDEO_IP_H
