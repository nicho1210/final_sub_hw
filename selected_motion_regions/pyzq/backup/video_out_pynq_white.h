#ifndef _VID_OUT_PYNQ_H_
#define _VID_OUT_PYNQ_H_

#include "xdebug.h"
#include "xvtc.h"
#include "xvidc.h"
#include "xgpio.h"
#include "xaxivdma.h"
#include "xil_cache.h"
#include "xparameters.h"
#include "sleep.h"

#define GPIO_RESET_ON       0
#define GPIO_RESET_OFF      1
#define GPIO_RESET_CHANNEL  1

#define FRAME_W      1280
#define FRAME_H      720
#define FRAME_STRIDE (FRAME_W * 3)
#define FRAME_BYTES  (FRAME_STRIDE * FRAME_H)

int system_init(XVtc *VtcInstPtr,
                XGpio *XGpioInstPtr,
                XAxiVdma *VdmaInstPtr);

void configure_vtc_gen(XVtc *InstancePtr, const XVidC_VideoStream *Strm);

#endif /* _VID_OUT_PYNQ_H_ */