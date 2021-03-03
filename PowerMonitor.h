/**
 * PowerMonitor for sampling CPU power state.
 * Copyright (c) (2013) Intel Corporation All Rights Reserved.
 * Based on Intel Power Gadget API.
 * Author: Brently Maxwell
 *
 * This is the header file for the PowerMonitor class.
 */

#pragma once
#include <iostream>
#include <string>
#include <stdio.h>
#include <tchar.h>
#include <math.h>

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>

using namespace std;

typedef int(*IntelEnergyLibInitialize_t)();
typedef int(*GetNumMsrs_t)(int* nMsr);
typedef int(*GetMsrName_t)(int iMsr, wchar_t* szName);
typedef int(*GetMsrFunc_t)(int iMsr, int* pFuncID);
typedef int(*GetPowerData_t)(int iNode, int iMsr, double* pResult, int* nResult);
typedef int(*ReadSample_t)();
typedef int(*GetMaxTemperature_t)(int iNode, int* degreeC);

class PowerMonitor
{
public:
	PowerMonitor();
	void SamplePowerState(double& watts);
	void SamplePowerState();
	bool IsInitialized() const { return energyLib != nullptr; }

private:
	void ClearEnergyLibFunctionPointers();
	HMODULE energyLib = nullptr;
	IntelEnergyLibInitialize_t IntelEnergyLibInitialize = nullptr;
	GetNumMsrs_t GetNumMsrs = nullptr;
	GetMsrName_t GetMsrName = nullptr;
	GetMsrFunc_t GetMsrFunc = nullptr;
	GetPowerData_t GetPowerData = nullptr;
	ReadSample_t ReadSample = nullptr;
	int maxTemperature_ = 0;
	PowerMonitor& operator=(const PowerMonitor&) = delete;
	PowerMonitor(const PowerMonitor&) = delete;
};
