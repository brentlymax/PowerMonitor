/**
 * PowerMonitor for sampling CPU power state.
 * Copyright (c) (2013) Intel Corporation All Rights Reserved.
 * Based on Intel Power Gadget API.
 * Author: Brently Maxwell
 *
 * This file contains the PowerMonitor class. Objects of this
 * class have helper functions to sample CPU power consumption.
 */

#include "PowerMonitor.h"

using namespace std;

// Correspond to the funcID values returned by GetMsrFunc.
const int MSR_FUNC_FREQ = 0;			// Frequency of sample in MHz
const int MSR_FUNC_POWER = 1;			// Average Power in Watts, Cumulative Energy in Joules, and Cumulative Energy in milliWatts per hour
const int MSR_FUNC_TEMP = 2;			// Temperature in Degrees Celsius
const int MSR_FUNC_MAX_POWER = 3;		// Package Power Limit in Watts.

/**
 * Constructor for PowerMonitor object.
 */
PowerMonitor::PowerMonitor()
{
	// Search for Power Gadget's library.
	#if _M_X64
		PCWSTR dllName = L"\\EnergyLib64.dll";
	#else
		PCWSTR dllName = L"\\EnergyLib32.dll";
	#endif
	#pragma warning(disable : 4996)
	PCWSTR powerGadgetDir = _wgetenv(L"IPG_Dir");

	if (powerGadgetDir) {
		energyLib = LoadLibrary((std::wstring(powerGadgetDir) + dllName).c_str());
	}
	if (energyLib) {
		IntelEnergyLibInitialize = (IntelEnergyLibInitialize_t)GetProcAddress(energyLib, "IntelEnergyLibInitialize");
		GetNumMsrs = (GetNumMsrs_t)GetProcAddress(energyLib, "GetNumMsrs");
		GetMsrName = (GetMsrName_t)GetProcAddress(energyLib, "GetMsrName");
		GetMsrFunc = (GetMsrFunc_t)GetProcAddress(energyLib, "GetMsrFunc");
		GetPowerData = (GetPowerData_t)GetProcAddress(energyLib, "GetPowerData");
		ReadSample = (ReadSample_t)GetProcAddress(energyLib, "ReadSample");
		auto GetMaxTemperature = (GetMaxTemperature_t)GetProcAddress(energyLib, "GetMaxTemperature");
		if (IntelEnergyLibInitialize && ReadSample) {
			if (IntelEnergyLibInitialize()) {
				if (GetMaxTemperature) {
					GetMaxTemperature(0, &maxTemperature_);
				}
				ReadSample();
			}
			else {
				ClearEnergyLibFunctionPointers();
			}
		}
	}
}

/**
 * Samples CPU power state. The reference parameters are the power values sampled.
 * This version has a parameter used to update a watts variable out of scope.
 * The variable represents the power values sampled.
 */
void PowerMonitor::SamplePowerState(double& watts)
{
	if (!IntelEnergyLibInitialize || !GetNumMsrs || !GetMsrName || !GetMsrFunc || !GetPowerData || !ReadSample) {
		return;
	}

	ReadSample();

	int funcID;
	wchar_t MSRName[1024];
	GetMsrFunc(MSR_FUNC_POWER, &funcID);
	GetMsrName(MSR_FUNC_POWER, MSRName);

	int nData;
	double data[3] = {};
	GetPowerData(0, MSR_FUNC_POWER, data, &nData);

	// Round to nearest .0001 to remove excess precision.
	data[0] = round(data[0] * 10000) / 10000;
	data[2] = round(data[2] * 10000) / 10000;
	watts = data[0];

	// For testing.
	//wprintf(L"%s Power (W) = %3.2f\n", MSRName, data[0]);
	//wprintf(L"%s Energy(J) = %3.2f\n", MSRName, data[1]);
	//wprintf(L"%s Energy(mWh)=%3.2f\n", MSRName, data[2]);
}

/**
 * Samples variables related to CPU power state.
 */
void PowerMonitor::SamplePowerState()
{
	if (!IntelEnergyLibInitialize || !GetNumMsrs || !GetMsrName || !GetMsrFunc || !GetPowerData || !ReadSample) {
		return;
	}

	int numMSRs = 0;
	GetNumMsrs(&numMSRs);
	ReadSample();

	for (int i = 0; i < numMSRs; ++i) {
		int funcID;
		wchar_t MSRName[1024];
		GetMsrFunc(i, &funcID);
		GetMsrName(i, MSRName);

		int nData;
		double data[3] = {};
		GetPowerData(0, i, data, &nData);

		if (funcID == MSR_FUNC_FREQ) {
			wprintf(L"%s = %4.0f MHz\n", MSRName, data[0]);
		}
		else if (funcID == MSR_FUNC_POWER) {
			// Round to nearest .0001 to remove excess precision.
			data[0] = round(data[0] * 10000) / 10000;
			data[2] = round(data[2] * 10000) / 10000;
			wprintf(L"%s Power (W) = %3.2f\n", MSRName, data[0]);
			wprintf(L"%s Energy(J) = %3.2f\n", MSRName, data[1]);
			wprintf(L"%s Energy(mWh)=%3.2f\n", MSRName, data[2]);
		}
		else if (funcID == MSR_FUNC_TEMP) {
			// The 3.02 version of Intel Power Gadget seems to report the temperature in F instead of C.
			wprintf(L"%s Temp (C) = %3.0f (max is %3.0f)\n", MSRName, data[0], (double)maxTemperature_);
		}
		else if (funcID == MSR_FUNC_MAX_POWER) {
			//wprintf(L"%s Max Power (W) = %3.0f\n", MSRName, data[0]);
		}
		else {
			//wprintf(L"Unused funcID %d\n", funcID);
		}
	}
}

/**
 * Clear the library and mark it as unavailable (used if initialization fails).
 */
void PowerMonitor::ClearEnergyLibFunctionPointers()
{
	IntelEnergyLibInitialize = nullptr;
	GetNumMsrs = nullptr;
	GetMsrName = nullptr;
	GetMsrFunc = nullptr;
	GetPowerData = nullptr;
	ReadSample = nullptr;
}
