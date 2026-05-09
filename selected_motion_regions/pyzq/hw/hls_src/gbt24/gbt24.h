#ifndef GBT24_H
#define GBT24_H

#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <ap_int.h>

typedef ap_axiu<24,1,1,1> axis24_t;

void gbt24(
    hls::stream<axis24_t>& s_axis_video,
    hls::stream<axis24_t>& m_axis_video,
    int width,
    int height,
    int threshold,
    int mode,
    int enable_blur
);

#endif