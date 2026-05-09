// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.2 (64-bit)
// Tool Version Limit: 2023.10
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xgbt24.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XGbt24_CfgInitialize(XGbt24 *InstancePtr, XGbt24_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_BaseAddress = ConfigPtr->Ctrl_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XGbt24_Start(XGbt24 *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_AP_CTRL) & 0x80;
    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_AP_CTRL, Data | 0x01);
}

u32 XGbt24_IsDone(XGbt24 *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XGbt24_IsIdle(XGbt24 *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XGbt24_IsReady(XGbt24 *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XGbt24_EnableAutoRestart(XGbt24 *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_AP_CTRL, 0x80);
}

void XGbt24_DisableAutoRestart(XGbt24 *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_AP_CTRL, 0);
}

void XGbt24_Set_width(XGbt24 *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_WIDTH_DATA, Data);
}

u32 XGbt24_Get_width(XGbt24 *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_WIDTH_DATA);
    return Data;
}

void XGbt24_Set_height(XGbt24 *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_HEIGHT_DATA, Data);
}

u32 XGbt24_Get_height(XGbt24 *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_HEIGHT_DATA);
    return Data;
}

void XGbt24_Set_threshold(XGbt24 *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_THRESHOLD_DATA, Data);
}

u32 XGbt24_Get_threshold(XGbt24 *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_THRESHOLD_DATA);
    return Data;
}

void XGbt24_Set_mode(XGbt24 *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_MODE_DATA, Data);
}

u32 XGbt24_Get_mode(XGbt24 *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_MODE_DATA);
    return Data;
}

void XGbt24_Set_enable_blur(XGbt24 *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_ENABLE_BLUR_DATA, Data);
}

u32 XGbt24_Get_enable_blur(XGbt24 *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_ENABLE_BLUR_DATA);
    return Data;
}

void XGbt24_InterruptGlobalEnable(XGbt24 *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_GIE, 1);
}

void XGbt24_InterruptGlobalDisable(XGbt24 *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_GIE, 0);
}

void XGbt24_InterruptEnable(XGbt24 *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_IER);
    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_IER, Register | Mask);
}

void XGbt24_InterruptDisable(XGbt24 *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_IER);
    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_IER, Register & (~Mask));
}

void XGbt24_InterruptClear(XGbt24 *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XGbt24_WriteReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_ISR, Mask);
}

u32 XGbt24_InterruptGetEnabled(XGbt24 *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_IER);
}

u32 XGbt24_InterruptGetStatus(XGbt24 *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XGbt24_ReadReg(InstancePtr->Ctrl_BaseAddress, XGBT24_CTRL_ADDR_ISR);
}

