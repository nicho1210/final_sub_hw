#include "xil_printf.h"
#include "xparameters.h"
#include "video_out_pynq.h"
#include "video_resolutions.h"
#include "xaxivdma_hw.h"

#include <string.h>

#if defined(XPAR_DDR_MEM_BASEADDR)
#define DDR_BASE_ADDR XPAR_DDR_MEM_BASEADDR
#elif defined(XPAR_PS7_DDR_0_S_AXI_BASEADDR)
#define DDR_BASE_ADDR XPAR_PS7_DDR_0_S_AXI_BASEADDR
#else
#error "Cannot find DDR base address macro in xparameters.h"
#endif

#define FRAME_BUFFER_BASEADDR (DDR_BASE_ADDR + 0x10000000U)

#ifndef XPAR_AXI_VDMA_0_DEVICE_ID
#define XPAR_AXI_VDMA_0_DEVICE_ID 0
#endif

#ifndef XPAR_AXI_GPIO_0_DEVICE_ID
#define XPAR_AXI_GPIO_0_DEVICE_ID 0
#endif

#ifndef XPAR_V_TC_0_DEVICE_ID
#define XPAR_V_TC_0_DEVICE_ID 0
#endif

static void dump_vdma_status(XAxiVdma *VdmaInstPtr)
{
    u32 mm2s_cr;
    u32 mm2s_sr;
    u32 parkptr;

    mm2s_cr = XAxiVdma_ReadReg(VdmaInstPtr->BaseAddr,
                               XAXIVDMA_TX_OFFSET + XAXIVDMA_CR_OFFSET);

    mm2s_sr = XAxiVdma_ReadReg(VdmaInstPtr->BaseAddr,
                               XAXIVDMA_TX_OFFSET + XAXIVDMA_SR_OFFSET);

    parkptr = XAxiVdma_ReadReg(VdmaInstPtr->BaseAddr,
                               XAXIVDMA_PARKPTR_OFFSET);

    xil_printf("VDMA MM2S CR      = 0x%08lx\r\n", (unsigned long)mm2s_cr);
    xil_printf("VDMA MM2S SR      = 0x%08lx\r\n", (unsigned long)mm2s_sr);
    xil_printf("VDMA PARKPTR      = 0x%08lx\r\n", (unsigned long)parkptr);

    xil_printf("VDMA halted       = %d\r\n", (mm2s_sr & 0x00000001U) ? 1 : 0);
    xil_printf("VDMA idle         = %d\r\n", (mm2s_sr & 0x00000002U) ? 1 : 0);
    xil_printf("VDMA internal err = %d\r\n", (mm2s_sr & 0x00000010U) ? 1 : 0);
    xil_printf("VDMA slave err    = %d\r\n", (mm2s_sr & 0x00000020U) ? 1 : 0);
    xil_printf("VDMA decode err   = %d\r\n", (mm2s_sr & 0x00000040U) ? 1 : 0);
}

static int configure_vdma_read(XAxiVdma *VdmaInstPtr, u32 FrameAddr)
{
    int Status;
    int Timeout;
    XAxiVdma_DmaSetup ReadCfg;
    UINTPTR FrameStoreStartAddr[XAXIVDMA_MAX_FRAMESTORE];

    memset(&ReadCfg, 0, sizeof(ReadCfg));
    memset(FrameStoreStartAddr, 0, sizeof(FrameStoreStartAddr));

    XAxiVdma_Reset(VdmaInstPtr, XAXIVDMA_READ);
    Timeout = 100000;
    while (Timeout > 0 && XAxiVdma_ResetNotDone(VdmaInstPtr, XAXIVDMA_READ)) {
        Timeout--;
    }

    if (Timeout == 0) {
        xil_printf("VDMA read reset timeout\r\n");
        return XST_FAILURE;
    }

    xil_printf("After VDMA reset:\r\n");
    dump_vdma_status(VdmaInstPtr);

    Status = XAxiVdma_SetFrmStore(VdmaInstPtr, 1, XAXIVDMA_READ);
    if (Status != XST_SUCCESS) {
        xil_printf("XAxiVdma_SetFrmStore failed\r\n");
        return XST_FAILURE;
    }

    ReadCfg.VertSizeInput       = FRAME_H;
    ReadCfg.HoriSizeInput       = FRAME_STRIDE;
    ReadCfg.Stride              = FRAME_STRIDE;
    ReadCfg.FrameDelay          = 0;
    ReadCfg.EnableCircularBuf   = 0;   /* disable circular mode */
    ReadCfg.EnableSync          = 0;
    ReadCfg.PointNum            = 0;
    ReadCfg.EnableFrameCounter  = 0;
    ReadCfg.FixedFrameStoreAddr = 0;   /* use frame store 0 */

    Status = XAxiVdma_DmaConfig(VdmaInstPtr, XAXIVDMA_READ, &ReadCfg);
    if (Status != XST_SUCCESS) {
        xil_printf("XAxiVdma_DmaConfig failed\r\n");
        return XST_FAILURE;
    }

    FrameStoreStartAddr[0] = (UINTPTR)FrameAddr;

    Status = XAxiVdma_DmaSetBufferAddr(VdmaInstPtr, XAXIVDMA_READ, FrameStoreStartAddr);
    if (Status != XST_SUCCESS) {
        xil_printf("XAxiVdma_DmaSetBufferAddr failed\r\n");
        return XST_FAILURE;
    }

    /* IMPORTANT: do NOT call XAxiVdma_StartParking() here */

    Status = XAxiVdma_DmaStart(VdmaInstPtr, XAXIVDMA_READ);
    if (Status != XST_SUCCESS) {
        xil_printf("XAxiVdma_DmaStart failed\r\n");
        return XST_FAILURE;
    }

    xil_printf("After VDMA start:\r\n");
    dump_vdma_status(VdmaInstPtr);

    return XST_SUCCESS;
}

int system_init(XVtc *VtcInstPtr,
                XGpio *XGpioInstPtr,
                XAxiVdma *VdmaInstPtr)
{
    int Status;
    XVtc_Config *VtcConfig;
    XAxiVdma_Config *VdmaConfig;

    VtcConfig = XVtc_LookupConfig(XPAR_V_TC_0_DEVICE_ID);
    if (VtcConfig == NULL) {
        xil_printf("VTC LookupConfig failed\r\n");
        return XST_FAILURE;
    }

    Status = XVtc_CfgInitialize(VtcInstPtr, VtcConfig, VtcConfig->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("VTC initialization failed\r\n");
        return XST_FAILURE;
    }

    Status = XGpio_Initialize(XGpioInstPtr, XPAR_AXI_GPIO_0_DEVICE_ID);
    if (Status != XST_SUCCESS) {
        xil_printf("GPIO initialization failed\r\n");
        return XST_FAILURE;
    }

    VdmaConfig = XAxiVdma_LookupConfig(XPAR_AXI_VDMA_0_DEVICE_ID);
    if (VdmaConfig == NULL) {
        xil_printf("VDMA LookupConfig failed\r\n");
        return XST_FAILURE;
    }

    Status = XAxiVdma_CfgInitialize(VdmaInstPtr, VdmaConfig, VdmaConfig->BaseAddress);
    if (Status != XST_SUCCESS) {
        xil_printf("VDMA initialization failed\r\n");
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

int main()
{
    int Status;
    const XVidC_VideoTimingMode *VmPtr;
    XVidC_VideoStream VidStream;
    XVtc VtcInst;
    XGpio XGpioInst;
    XAxiVdma VdmaInst;
    u8 *fb;
    u32 i;

    xil_printf("Phase1: DDR -> VDMA -> HDMI\r\n");

    VmPtr = XVidC_GetVideoModeData(XVIDC_VM_1280x720_60_P);
    if (VmPtr == NULL) {
        xil_printf("XVidC_GetVideoModeData failed\r\n");
        return XST_FAILURE;
    }

    Status = system_init(&VtcInst, &XGpioInst, &VdmaInst);
    if (Status != XST_SUCCESS) {
        xil_printf("system_init failed\r\n");
        return XST_FAILURE;
    }

    ClkWiz_Set_Output_Clock(XPAR_XCLK_WIZ_0_BASEADDR, CLKWIZ_CLKOUT0_74_25_MHz);

    /* Hold video output block in reset first */
    XGpio_DiscreteWrite(&XGpioInst, GPIO_RESET_CHANNEL, GPIO_RESET_ON);
    usleep(200);

    /* Fill DDR frame buffer with pure white */
    fb = (u8 *)FRAME_BUFFER_BASEADDR;
    for (i = 0; i < FRAME_BYTES; i++) {
        fb[i] = 0xFF;
    }
    Xil_DCacheFlushRange((UINTPTR)FRAME_BUFFER_BASEADDR, FRAME_BYTES);

    xil_printf("Frame buffer filled white @ 0x%08lx, bytes=%d\r\n",
               (unsigned long)FRAME_BUFFER_BASEADDR, FRAME_BYTES);

    /* Configure VTC timing */
    XVidC_SetVideoStream(&VidStream,
                         XVIDC_VM_1280x720_60_P,
                         XVIDC_CSF_RGB,
                         XVIDC_BPC_8,
                         XVIDC_PPC_1);

    configure_vtc_gen(&VtcInst, &VidStream);
    XVtc_EnableGenerator(&VtcInst);

    /* Release video reset before starting VDMA */
    XGpio_DiscreteWrite(&XGpioInst, GPIO_RESET_CHANNEL, GPIO_RESET_OFF);
    usleep(1000);

    xil_printf("After releasing video reset:\r\n");
    dump_vdma_status(&VdmaInst);

    /* Configure and start VDMA read channel */
    Status = configure_vdma_read(&VdmaInst, FRAME_BUFFER_BASEADDR);
    if (Status != XST_SUCCESS) {
        xil_printf("configure_vdma_read failed\r\n");
        return XST_FAILURE;
    }

    usleep(1000);
    xil_printf("1ms after VDMA start:\r\n");
    dump_vdma_status(&VdmaInst);

    xil_printf("Pipeline started\r\n");

    while (1) {
        /* Keep running */
    }

    return 0;
}

void configure_vtc_gen(XVtc *InstancePtr, const XVidC_VideoStream *Strm)
{
    XVtc_Timing XVtc_Timingconf;

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

    XVtc_SetGeneratorTiming(InstancePtr, &XVtc_Timingconf);
    XVtc_RegUpdate(InstancePtr);

    XVtc_Enable(InstancePtr);
    XVtc_EnableGenerator(InstancePtr);
}