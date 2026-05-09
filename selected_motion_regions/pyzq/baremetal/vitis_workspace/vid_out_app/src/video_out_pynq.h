#ifndef VIDEO_OUT_PYNQ_H
#define VIDEO_OUT_PYNQ_H

#include "xgpio.h"
#include "xvtc.h"
#include "xvidc.h"
#include "xstatus.h"

#define GPIO_RESET_CHANNEL 1
#define GPIO_DEBUG_CHANNEL 1

#define GPIO_RESET_ON  0
#define GPIO_RESET_OFF 1

int system_init(XVtc *VtcInstPtr,
                XGpio *ResetGpioInstPtr,
                XGpio *RxDebugGpioInstPtr,
                XGpio *TxDebugGpioInstPtr,
                XGpio *MotionGpioInstPtr);

void configure_vtc_gen(XVtc *InstancePtr, const XVidC_VideoStream *Strm);

#endif