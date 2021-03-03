/**
 * PowerMonitor for sampling CPU power state.
 * Copyright (c) (2013) Intel Corporation All Rights Reserved.
 * Based on Intel Power Gadget API.
 * Author: Brently Maxwell
 *
 * Utilizes Intel Power Gadget API to monitor CPU power consumption.
 */

#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include "PowerMonitor.h"

using namespace std;

const int delayInterval = 1000;	// used as miliseconds for Sleep().

/**
 * Samples current CPU power state 'n' number of times and gives the average Watts at the end.
 */
void SamplePowerState(int n, PowerMonitor& pMonitor)
{
	double averageWatts = 0;
	for (int i = 0; i < n; i++) {
		double watts = 0;
		pMonitor.SamplePowerState(watts);
		averageWatts += watts;
		Sleep(delayInterval);
	}
	averageWatts /= n;
	cout << "Average power consumption in Watts: " << averageWatts << endl;
}

/**
 * Samples current CPU power state endlessly and records Watts of each sample with a time stamp.
 */
void RecordPowerState(PowerMonitor& pMonitor)
{
	string filename = "C:/Programming/Workspace/CodeBlocks/PowerMonitor/powerusage.csv";
	ofstream os;
	os.open(filename, ios_base::app);
	if (!os) {
		cout << "Error opening file." << endl;
		return;
	}
	// Loops endlessly, recording the current time and watts used by the CPU.
	while (true) {
		double watts = 0;
		pMonitor.SamplePowerState(watts);
		auto now = chrono::system_clock::now();
		auto now_c = chrono::system_clock::to_time_t(now);
		stringstream ss;
		ss << put_time(localtime(&now_c), "%X") << "," << watts;  // Alernatively can do "%Y-%m-%d %X" to include date.
		os << ss.str() << endl;
		Sleep(delayInterval);
	}
	os.close();
}

/**
 * Main function.
 */
int _tmain(int argc, _TCHAR* argv[])
{
	// Initialize power monitor.
	PowerMonitor pMonitor;
	if (!pMonitor.IsInitialized()){
		cout << "Error: Intel Power Gadget library not found. Make sure it is installed and the program is running in the correct architecture." << endl;
		return -1;
	}
	int choice = 0;
	cout << "Welcome to the CPU Power Monitor." << endl;
	cout << "Press '1' to choose the CPU Power Sampler." << endl;
	cout << "Press '2' to choose the CPU Power Recorder." << endl;
	cout << "Press anything else to quit." << endl;
	cin >> choice;

	// CPU Power Sampler - samples 'n' number of times and gives average Watts at the end.
	if (choice == 1) {
		int n = 0;
		cout << "Welcome to the CPU Power Sampler." << endl;
		cout << "Please choose a sample size." << endl;
		cin >> n;
		while (n < 1) {
			cout << "Error: sample size must be 1 or more." << endl;
			cout << "Please choose a sample size." << endl;
			cin >> n;
		}
		cout << "Number of samples is: " << n << endl;
		cout << "Sampling now..." << endl;
		SamplePowerState(n, pMonitor);
	// CPU Power Recorder - samples endlessly and records Watts of each sample with a time stamp.
	} else if (choice == 2) {
		cout << "Welcome to the CPU Power Recorder." << endl;
		cout << "Press Ctrl + C to stop recording." << endl;
		cout << "Recording now..." << endl;
		RecordPowerState(pMonitor);
	// User decides to quit.
	} else {
		cout << "You chose to quit. Goodbye." << endl;
	}
	return 0;
}
