#include "xil_printf.h"
#include "xparameters.h"
#include "video_out_pynq.h"
#include "video_resolutions.h"
#include "sleep.h"

#ifndef XPAR_AXI_GPIO_0_BASEADDR
#error "XPAR_AXI_GPIO_0_BASEADDR not found in xparameters.h"
#endif

#ifndef XPAR_AXI_GPIO_1_BASEADDR
#error "XPAR_AXI_GPIO_1_BASEADDR not found in xparameters.h"
#endif

#ifndef XPAR_AXI_GPIO_2_BASEADDR
#error "XPAR_AXI_GPIO_2_BASEADDR not found in xparameters.h"
#endif

#ifndef XPAR_AXI_GPIO_3_BASEADDR
#error "XPAR_AXI_GPIO_3_BASEADDR not found in xparameters.h"
#endif

#ifndef XPAR_V_TC_0_BASEADDR
#error "XPAR_V_TC_0_BASEADDR not found in xparameters.h"
#endif

#ifndef XPAR_XCLK_WIZ_0_BASEADDR
#ifdef XPAR_CLK_WIZ_0_BASEADDR
#define XPAR_XCLK_WIZ_0_BASEADDR XPAR_CLK_WIZ_0_BASEADDR
#else
#error "Cannot find Clock Wizard base address macro in xparameters.h"
#endif
#endif

#define MOTION_GPIO_CHANNEL  1
#define MOTION_THRESHOLD     200

static void print_stream_info(const XVidC_VideoStream *Strm)
{
    xil_printf("Video stream info:\r\n");
    xil_printf("  HActive     = %d\r\n", Strm->Timing.HActive);
    xil_printf("  HSyncWidth  = %d\r\n", Strm->Timing.HSyncWidth);
    xil_printf("  HFrontPorch = %d\r\n", Strm->Timing.HFrontPorch);
    xil_printf("  HBackPorch  = %d\r\n", Strm->Timing.HBackPorch);
    xil_printf("  VActive     = %d\r\n", Strm->Timing.VActive);
    xil_printf("  VSyncWidth  = %d\r\n", Strm->Timing.F0PVSyncWidth);
    xil_printf("  VFrontPorch = %d\r\n", Strm->Timing.F0PVFrontPorch);
    xil_printf("  VBackPorch  = %d\r\n", Strm->Timing.F0PVBackPorch);
    xil_printf("  Interlaced  = %d\r\n", Strm->IsInterlaced);
}

static void print_motion_regions(u32 mask)
{
    int first = 1;
    xil_printf("regions=");

    for (int i = 0; i < 9; i++) {
        if (mask & (1U << i)) {
            if (!first) {
                xil_printf(",");
            }
            xil_printf("%d", i + 1);
            first = 0;
        }
    }

    if (first) {
        xil_printf("none");
    }
}

int system_init(XVtc *VtcInstPtr,
                XGpio *ResetGpioInstPtr,
                XGpio *RxDebugGpioInstPtr,
                XGpio *TxDebugGpioInstPtr,
                XGpio *MotionGpioInstPtr)
{
    int Status;
    XVtc_Config  *VtcConfig;
    XGpio_Config *Gpio0Config;
    XGpio_Config *Gpio1Config;
    XGpio_Config *Gpio2Config;
    XGpio_Config *Gpio3Config;

    xil_printf("[INIT] Looking up VTC config by baseaddr...\r\n");
    VtcConfig = XVtc_LookupConfig(XPAR_V_TC_0_BASEADDR);
    if (VtcConfig == NULL) {
        xil_printf("[ERR ] VTC LookupConfig failed\r\n");
        return XST_FAILURE;
    }
    xil_printf("[ OK ] VTC config found, base = 0x%08x\r\n", VtcConfig->BaseAddress);

    xil_printf("[INIT] Initializing VTC...\r\n");
    Status = XVtc_CfgInitialize(VtcInstPtr, VtcConfig, VtcConfig->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("[ERR ] VTC initialization failed, status = %d\r\n", Status);
        return XST_FAILURE;
    }
    xil_printf("[ OK ] VTC initialized\r\n");

    xil_printf("[INIT] Looking up AXI GPIO 0 config by baseaddr...\r\n");
    Gpio0Config = XGpio_LookupConfig(XPAR_AXI_GPIO_0_BASEADDR);
    if (Gpio0Config == NULL) {
        xil_printf("[ERR ] GPIO0 LookupConfig failed\r\n");
        return XST_FAILURE;
    }
    xil_printf("[ OK ] GPIO0 config found, base = 0x%08x\r\n", Gpio0Config->BaseAddress);

    xil_printf("[INIT] Initializing AXI GPIO 0 (reset GPIO)...\r\n");
    Status = XGpio_CfgInitialize(ResetGpioInstPtr, Gpio0Config, Gpio0Config->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("[ERR ] GPIO0 initialization failed, status = %d\r\n", Status);
        return XST_FAILURE;
    }
    xil_printf("[ OK ] GPIO0 initialized\r\n");

    xil_printf("[INIT] Looking up AXI GPIO 1 config by baseaddr...\r\n");
    Gpio1Config = XGpio_LookupConfig(XPAR_AXI_GPIO_1_BASEADDR);
    if (Gpio1Config == NULL) {
        xil_printf("[ERR ] GPIO1 LookupConfig failed\r\n");
        return XST_FAILURE;
    }
    xil_printf("[ OK ] GPIO1 config found, base = 0x%08x\r\n", Gpio1Config->BaseAddress);

    xil_printf("[INIT] Initializing AXI GPIO 1 (HDMI RX debug)...\r\n");
    Status = XGpio_CfgInitialize(RxDebugGpioInstPtr, Gpio1Config, Gpio1Config->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("[ERR ] GPIO1 initialization failed, status = %d\r\n", Status);
        return XST_FAILURE;
    }
    xil_printf("[ OK ] GPIO1 initialized (rx_lock)\r\n");

    xil_printf("[INIT] Looking up AXI GPIO 2 config by baseaddr...\r\n");
    Gpio2Config = XGpio_LookupConfig(XPAR_AXI_GPIO_2_BASEADDR);
    if (Gpio2Config == NULL) {
        xil_printf("[ERR ] GPIO2 LookupConfig failed\r\n");
        return XST_FAILURE;
    }
    xil_printf("[ OK ] GPIO2 config found, base = 0x%08x\r\n", Gpio2Config->BaseAddress);

    xil_printf("[INIT] Initializing AXI GPIO 2 (HDMI TX debug)...\r\n");
    Status = XGpio_CfgInitialize(TxDebugGpioInstPtr, Gpio2Config, Gpio2Config->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("[ERR ] GPIO2 initialization failed, status = %d\r\n", Status);
        return XST_FAILURE;
    }
    xil_printf("[ OK ] GPIO2 initialized (tx_lock)\r\n");

    xil_printf("[INIT] Looking up AXI GPIO 3 config by baseaddr...\r\n");
    Gpio3Config = XGpio_LookupConfig(XPAR_AXI_GPIO_3_BASEADDR);
    if (Gpio3Config == NULL) {
        xil_printf("[ERR ] GPIO3 LookupConfig failed\r\n");
        return XST_FAILURE;
    }
    xil_printf("[ OK ] GPIO3 config found, base = 0x%08x\r\n", Gpio3Config->BaseAddress);

    xil_printf("[INIT] Initializing AXI GPIO 3 (motion info)...\r\n");
    Status = XGpio_CfgInitialize(MotionGpioInstPtr, Gpio3Config, Gpio3Config->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("[ERR ] GPIO3 initialization failed, status = %d\r\n", Status);
        return XST_FAILURE;
    }
    xil_printf("[ OK ] GPIO3 initialized (motion_info)\r\n");

    xil_printf("[INIT] Configuring GPIO0 channel %d as output...\r\n", GPIO_RESET_CHANNEL);
    XGpio_SetDataDirection(ResetGpioInstPtr, GPIO_RESET_CHANNEL, 0x0);

    xil_printf("[INIT] Configuring GPIO1 channel %d as input...\r\n", GPIO_DEBUG_CHANNEL);
    XGpio_SetDataDirection(RxDebugGpioInstPtr, GPIO_DEBUG_CHANNEL, 0x1);

    xil_printf("[INIT] Configuring GPIO2 channel %d as input...\r\n", GPIO_DEBUG_CHANNEL);
    XGpio_SetDataDirection(TxDebugGpioInstPtr, GPIO_DEBUG_CHANNEL, 0x1);

    xil_printf("[INIT] Configuring GPIO3 channel %d as input...\r\n", MOTION_GPIO_CHANNEL);
    XGpio_SetDataDirection(MotionGpioInstPtr, MOTION_GPIO_CHANNEL, 0xFFFFFFFF);

    xil_printf("[ OK ] GPIO directions set\r\n");

    return XST_SUCCESS;
}

int main(void)
{
    int Status;
    XVidC_VideoStream VidStream;
    XVtc VtcInst;
    XGpio ResetGpioInst;
    XGpio RxDebugGpioInst;
    XGpio TxDebugGpioInst;
    XGpio MotionGpioInst;

    u32 heartbeat = 0;
    u32 rx_lock = 0;
    u32 tx_lock = 0;
    u32 prev_rx_lock = 0;

    u32 motion_info = 0;
    u32 motion_count = 0;
    u32 motion_mask = 0;
    u32 motion_detected = 0;

    xil_printf("\r\n");
    xil_printf("====================================\r\n");
    xil_printf(" HDMI passthrough application start \r\n");
    xil_printf(" Target output mode: 1280x720p60    \r\n");
    xil_printf("====================================\r\n");

    Status = system_init(&VtcInst,
                         &ResetGpioInst,
                         &RxDebugGpioInst,
                         &TxDebugGpioInst,
                         &MotionGpioInst);
    if (Status != XST_SUCCESS) {
        xil_printf("[ERR ] system_init failed\r\n");
        return XST_FAILURE;
    }

    xil_printf("[INIT] Programming clock wizard to 74.25 MHz...\r\n");
    Status = ClkWiz_Set_Output_Clock(
        XPAR_XCLK_WIZ_0_BASEADDR,
        CLKWIZ_CLKOUT0_74_25_MHz
    );
    if (Status != XST_SUCCESS) {
        xil_printf("[ERR ] Clock wizard failed to lock, status = %d\r\n", Status);
        return XST_FAILURE;
    }
    xil_printf("[ OK ] Clock wizard configured for 74.25 MHz\r\n");

    xil_printf("[INIT] Asserting video reset (aresetn = 0)...\r\n");
    XGpio_DiscreteWrite(&ResetGpioInst, GPIO_RESET_CHANNEL, GPIO_RESET_ON);
    xil_printf("[DBG ] GPIO0 channel %d write = %d\r\n", GPIO_RESET_CHANNEL, GPIO_RESET_ON);
    usleep(10000);

    xil_printf("[INIT] Preparing XVidC stream structure...\r\n");
    XVidC_SetVideoStream(&VidStream,
                         XVIDC_VM_1280x720_60_P,
                         XVIDC_CSF_RGB,
                         XVIDC_BPC_8,
                         XVIDC_PPC_1);
    xil_printf("[ OK ] XVidC stream prepared\r\n");
    print_stream_info(&VidStream);

    xil_printf("[INIT] Configuring VTC generator...\r\n");
    configure_vtc_gen(&VtcInst, &VidStream);
    xil_printf("[ OK ] VTC generator timing loaded\r\n");

    xil_printf("[INIT] Enabling VTC generator...\r\n");
    XVtc_EnableGenerator(&VtcInst);
    xil_printf("[ OK ] VTC generator enabled\r\n");

    xil_printf("[INIT] Releasing video reset (aresetn = 1)...\r\n");
    XGpio_DiscreteWrite(&ResetGpioInst, GPIO_RESET_CHANNEL, GPIO_RESET_OFF);
    xil_printf("[DBG ] GPIO0 channel %d write = %d\r\n", GPIO_RESET_CHANNEL, GPIO_RESET_OFF);

    usleep(300000);

    rx_lock = XGpio_DiscreteRead(&RxDebugGpioInst, GPIO_DEBUG_CHANNEL) & 0x1;
    tx_lock = XGpio_DiscreteRead(&TxDebugGpioInst, GPIO_DEBUG_CHANNEL) & 0x1;

    motion_info = XGpio_DiscreteRead(&MotionGpioInst, MOTION_GPIO_CHANNEL);
    motion_count = motion_info & 0xFFFF;
    motion_mask  = (motion_info >> 16) & 0x01FF;
    motion_detected = (motion_mask != 0U) ? 1U : 0U;

    prev_rx_lock = rx_lock;

    xil_printf("[DBG ] rx_lock=%lu, tx_lock=%lu, motion_count=%lu, motion=%lu, ",
               (unsigned long)rx_lock,
               (unsigned long)tx_lock,
               (unsigned long)motion_count,
               (unsigned long)motion_detected);
    print_motion_regions(motion_mask);
    xil_printf("\r\n");

    xil_printf("[DONE] Passthrough pipeline configured.\r\n");
    xil_printf("[INFO] Connect laptop HDMI -> board HDMI IN\r\n");
    xil_printf("[INFO] Connect board HDMI OUT -> monitor\r\n");
    xil_printf("[INFO] Set laptop output to 1280x720 @ 60 Hz for first test.\r\n");
    xil_printf("[INFO] Motion threshold = %d blocks\r\n", MOTION_THRESHOLD);

    while (1) {
        sleep(2);
        heartbeat++;

        rx_lock = XGpio_DiscreteRead(&RxDebugGpioInst, GPIO_DEBUG_CHANNEL) & 0x1;
        tx_lock = XGpio_DiscreteRead(&TxDebugGpioInst, GPIO_DEBUG_CHANNEL) & 0x1;

        motion_info = XGpio_DiscreteRead(&MotionGpioInst, MOTION_GPIO_CHANNEL);
        motion_count = motion_info & 0xFFFF;
        motion_mask  = (motion_info >> 16) & 0x01FF;
        motion_detected = (motion_mask != 0U) ? 1U : 0U;

        xil_printf("[HB  ] app alive=%lu, rx_lock=%lu, tx_lock=%lu, motion_count=%lu, motion=%lu, ",
                   (unsigned long)heartbeat,
                   (unsigned long)rx_lock,
                   (unsigned long)tx_lock,
                   (unsigned long)motion_count,
                   (unsigned long)motion_detected);
        print_motion_regions(motion_mask);
        xil_printf("\r\n");

        if ((prev_rx_lock == 0) && (rx_lock == 1) && (tx_lock == 0)) {
            xil_printf("[REC ] RX lock rose 0->1 while TX=0, restarting VTC and pulsing reset...\r\n");

            XGpio_DiscreteWrite(&ResetGpioInst, GPIO_RESET_CHANNEL, GPIO_RESET_ON);
            xil_printf("[DBG ] GPIO0 channel %d write = %d\r\n",
                       GPIO_RESET_CHANNEL, GPIO_RESET_ON);
            usleep(10000);

            xil_printf("[REC ] Resetting and reconfiguring VTC...\r\n");
            XVtc_DisableGenerator(&VtcInst);
            XVtc_Reset(&VtcInst);
            usleep(1000);

            configure_vtc_gen(&VtcInst, &VidStream);
            XVtc_EnableGenerator(&VtcInst);
            xil_printf("[REC ] VTC restarted\r\n");
            usleep(10000);

            XGpio_DiscreteWrite(&ResetGpioInst, GPIO_RESET_CHANNEL, GPIO_RESET_OFF);
            xil_printf("[DBG ] GPIO0 channel %d write = %d\r\n",
                       GPIO_RESET_CHANNEL, GPIO_RESET_OFF);
            usleep(100000);

            rx_lock = XGpio_DiscreteRead(&RxDebugGpioInst, GPIO_DEBUG_CHANNEL) & 0x1;
            tx_lock = XGpio_DiscreteRead(&TxDebugGpioInst, GPIO_DEBUG_CHANNEL) & 0x1;

            motion_info = XGpio_DiscreteRead(&MotionGpioInst, MOTION_GPIO_CHANNEL);
            motion_count = motion_info & 0xFFFF;
            motion_mask  = (motion_info >> 16) & 0x01FF;
            motion_detected = (motion_mask != 0U) ? 1U : 0U;

            xil_printf("[REC ] after VTC restart + reset pulse: rx_lock=%lu, tx_lock=%lu, motion_count=%lu, motion=%lu, ",
                       (unsigned long)rx_lock,
                       (unsigned long)tx_lock,
                       (unsigned long)motion_count,
                       (unsigned long)motion_detected);
            print_motion_regions(motion_mask);
            xil_printf("\r\n");
        }

        prev_rx_lock = rx_lock;
    }

    return XST_SUCCESS;
}

void configure_vtc_gen(XVtc *InstancePtr, const XVidC_VideoStream *Strm)
{
    XVtc_Timing XVtc_Timingconf;

    xil_printf("[DBG ] Resetting VTC...\r\n");
    XVtc_Reset(InstancePtr);
    XVtc_DisableGenerator(InstancePtr);
    XVtc_Disable(InstancePtr);

    XVtc_Timingconf.HActiveVideo  = Strm->Timing.HActive;
    XVtc_Timingconf.HBackPorch    = Strm->Timing.HBackPorch;
    XVtc_Timingconf.HFrontPorch   = Strm->Timing.HFrontPorch;
    XVtc_Timingconf.HSyncPolarity = Strm->Timing.HSyncPolarity;
    XVtc_Timingconf.HSyncWidth    = Strm->Timing.HSyncWidth;
    XVtc_Timingconf.Interlaced    = Strm->IsInterlaced;

    XVtc_Timingconf.V0BackPorch   = Strm->Timing.F0PVBackPorch;
    XVtc_Timingconf.V0FrontPorch  = Strm->Timing.F0PVFrontPorch;
    XVtc_Timingconf.V0SyncWidth   = Strm->Timing.F0PVSyncWidth;
    XVtc_Timingconf.V1BackPorch   = Strm->Timing.F1VBackPorch;
    XVtc_Timingconf.V1FrontPorch  = Strm->Timing.F1VFrontPorch;
    XVtc_Timingconf.V1SyncWidth   = Strm->Timing.F1VSyncWidth;
    XVtc_Timingconf.VActiveVideo  = Strm->Timing.VActive;
    XVtc_Timingconf.VSyncPolarity = Strm->Timing.VSyncPolarity;

    xil_printf("[DBG ] Applying VTC timing...\r\n");
    XVtc_SetGeneratorTiming(InstancePtr, &XVtc_Timingconf);
    XVtc_RegUpdate(InstancePtr);

    xil_printf("[DBG ] Enabling VTC core...\r\n");
    XVtc_Enable(InstancePtr);
}