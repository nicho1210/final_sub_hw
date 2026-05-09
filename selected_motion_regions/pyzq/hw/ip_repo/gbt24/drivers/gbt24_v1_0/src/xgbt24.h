// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.2 (64-bit)
// Tool Version Limit: 2023.10
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XGBT24_H
#define XGBT24_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xgbt24_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
#ifdef SDT
    char *Name;
#else
    u16 DeviceId;
#endif
    u64 Ctrl_BaseAddress;
} XGbt24_Config;
#endif

typedef struct {
    u64 Ctrl_BaseAddress;
    u32 IsReady;
} XGbt24;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XGbt24_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XGbt24_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XGbt24_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XGbt24_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifdef SDT
int XGbt24_Initialize(XGbt24 *InstancePtr, UINTPTR BaseAddress);
XGbt24_Config* XGbt24_LookupConfig(UINTPTR BaseAddress);
#else
int XGbt24_Initialize(XGbt24 *InstancePtr, u16 DeviceId);
XGbt24_Config* XGbt24_LookupConfig(u16 DeviceId);
#endif
int XGbt24_CfgInitialize(XGbt24 *InstancePtr, XGbt24_Config *ConfigPtr);
#else
int XGbt24_Initialize(XGbt24 *InstancePtr, const char* InstanceName);
int XGbt24_Release(XGbt24 *InstancePtr);
#endif

void XGbt24_Start(XGbt24 *InstancePtr);
u32 XGbt24_IsDone(XGbt24 *InstancePtr);
u32 XGbt24_IsIdle(XGbt24 *InstancePtr);
u32 XGbt24_IsReady(XGbt24 *InstancePtr);
void XGbt24_EnableAutoRestart(XGbt24 *InstancePtr);
void XGbt24_DisableAutoRestart(XGbt24 *InstancePtr);

void XGbt24_Set_width(XGbt24 *InstancePtr, u32 Data);
u32 XGbt24_Get_width(XGbt24 *InstancePtr);
void XGbt24_Set_height(XGbt24 *InstancePtr, u32 Data);
u32 XGbt24_Get_height(XGbt24 *InstancePtr);
void XGbt24_Set_threshold(XGbt24 *InstancePtr, u32 Data);
u32 XGbt24_Get_threshold(XGbt24 *InstancePtr);
void XGbt24_Set_mode(XGbt24 *InstancePtr, u32 Data);
u32 XGbt24_Get_mode(XGbt24 *InstancePtr);
void XGbt24_Set_enable_blur(XGbt24 *InstancePtr, u32 Data);
u32 XGbt24_Get_enable_blur(XGbt24 *InstancePtr);

void XGbt24_InterruptGlobalEnable(XGbt24 *InstancePtr);
void XGbt24_InterruptGlobalDisable(XGbt24 *InstancePtr);
void XGbt24_InterruptEnable(XGbt24 *InstancePtr, u32 Mask);
void XGbt24_InterruptDisable(XGbt24 *InstancePtr, u32 Mask);
void XGbt24_InterruptClear(XGbt24 *InstancePtr, u32 Mask);
u32 XGbt24_InterruptGetEnabled(XGbt24 *InstancePtr);
u32 XGbt24_InterruptGetStatus(XGbt24 *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
