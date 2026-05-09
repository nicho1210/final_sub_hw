#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>

#define FRAME_WIDTH   1280
#define FRAME_HEIGHT  720

#define DS_SHIFT      2
#define MOT_W         (FRAME_WIDTH  >> DS_SHIFT)   // 320
#define MOT_H         (FRAME_HEIGHT >> DS_SHIFT)   // 180
#define MOT_SIZE      (MOT_W * MOT_H)

#define X_SPLIT1      (FRAME_WIDTH / 3)            // 426
#define X_SPLIT2      ((FRAME_WIDTH * 2) / 3)      // 853
#define Y_SPLIT1      (FRAME_HEIGHT / 3)           // 240
#define Y_SPLIT2      ((FRAME_HEIGHT * 2) / 3)     // 480

typedef ap_axiu<24,1,1,1> axis_pixel_t;

void video_gray_live(
    hls::stream<axis_pixel_t>& in_stream,
    hls::stream<axis_pixel_t>& out_stream,
    volatile ap_uint<32> *motion_count_out
) {
#pragma HLS INTERFACE axis port=in_stream
#pragma HLS INTERFACE axis port=out_stream
#pragma HLS INTERFACE ap_none port=motion_count_out
#pragma HLS INTERFACE ap_ctrl_none port=return

    static ap_uint<1> prev_frame[MOT_SIZE];
#pragma HLS BIND_STORAGE variable=prev_frame type=ram_2p impl=lutram

    static ap_uint<11> x = 0;
    static ap_uint<10> y = 0;
    static ap_uint<16> addr = 0;
    static ap_uint<1> first_frame = 1;

    static ap_uint<16> motion_count_accum = 0;
    static ap_uint<16> motion_count_latched = 0;

    static ap_uint<9> region_mask_accum = 0;
    static ap_uint<9> region_mask_latched = 0;

    const ap_uint<8> THRESH = 96;

    while (1) {
#pragma HLS PIPELINE II=1
        axis_pixel_t p = in_stream.read();

        if (p.user[0]) {
            x = 0;
            y = 0;
            addr = 0;
            motion_count_accum = 0;
            region_mask_accum = 0;
        }

        ap_uint<8> g = p.data.range(15,8);
        ap_uint<1> curr_bin = (g >= THRESH) ? 1 : 0;

        // 保持已知穩定的 threshold 畫面輸出
        ap_uint<8> out_pix = curr_bin ? ap_uint<8>(255) : ap_uint<8>(0);

        // 每個 4x4 block 左上角才做一次 previous-frame 比較
        if ((x.range(1,0) == 0) && (y.range(1,0) == 0)) {
            ap_uint<1> prev_bin = prev_frame[addr];
            ap_uint<1> motion = first_frame ? ap_uint<1>(0) : (curr_bin ^ prev_bin);

            if (motion) {
                ap_uint<4> region_idx = 0;

                // 不用乘法，直接用比較決定區塊 0..8
                if (y < Y_SPLIT1) {
                    if (x < X_SPLIT1) {
                        region_idx = 0;   // 區1
                    } else if (x < X_SPLIT2) {
                        region_idx = 1;   // 區2
                    } else {
                        region_idx = 2;   // 區3
                    }
                } else if (y < Y_SPLIT2) {
                    if (x < X_SPLIT1) {
                        region_idx = 3;   // 區4
                    } else if (x < X_SPLIT2) {
                        region_idx = 4;   // 區5
                    } else {
                        region_idx = 5;   // 區6
                    }
                } else {
                    if (x < X_SPLIT1) {
                        region_idx = 6;   // 區7
                    } else if (x < X_SPLIT2) {
                        region_idx = 7;   // 區8
                    } else {
                        region_idx = 8;   // 區9
                    }
                }

                motion_count_accum++;
                region_mask_accum |= (ap_uint<9>(1) << region_idx);
            }

            prev_frame[addr] = curr_bin;

            if (addr == MOT_SIZE - 1)
                addr = 0;
            else
                addr++;
        }

        p.data.range(23,16) = out_pix;
        p.data.range(15,8)  = out_pix;
        p.data.range(7,0)   = out_pix;

        out_stream.write(p);

        if (x == FRAME_WIDTH - 1) {
            x = 0;
            if (y == FRAME_HEIGHT - 1) {
                y = 0;

                if (first_frame) {
                    motion_count_latched = 0;
                    region_mask_latched = 0;
                    first_frame = 0;
                } else {
                    motion_count_latched = motion_count_accum;
                    region_mask_latched = region_mask_accum;
                }
            } else {
                y++;
            }
        } else {
            x++;
        }

        // [15:0] = count, [24:16] = region mask
        *motion_count_out =
            ((ap_uint<32>)region_mask_latched << 16) |
            (ap_uint<32>)motion_count_latched;
    }
}
