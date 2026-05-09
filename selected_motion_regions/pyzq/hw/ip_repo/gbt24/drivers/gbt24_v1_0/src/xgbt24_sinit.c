// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2023.2 (64-bit)
// Tool Version Limit: 2023.10
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#ifdef SDT
#include "xparameters.h"
#endif
#include "xgbt24.h"

extern XGbt24_Config XGbt24_ConfigTable[];

#ifdef SDT
XGbt24_Config *XGbt24_LookupConfig(UINTPTR BaseAddress) {
	XGbt24_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XGbt24_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XGbt24_ConfigTable[Index].Ctrl_BaseAddress == BaseAddress) {
			ConfigPtr = &XGbt24_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XGbt24_Initialize(XGbt24 *InstancePtr, UINTPTR BaseAddress) {
	XGbt24_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XGbt24_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XGbt24_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XGbt24_Config *XGbt24_LookupConfig(u16 DeviceId) {
	XGbt24_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XGBT24_NUM_INSTANCES; Index++) {
		if (XGbt24_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XGbt24_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XGbt24_Initialize(XGbt24 *InstancePtr, u16 DeviceId) {
	XGbt24_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XGbt24_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XGbt24_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

