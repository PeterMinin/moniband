#pragma once

#include <pdh.h>

class CpuMonitor
{
public:
	CpuMonitor();

	float GetTotalUsage();

private:
	PDH_HQUERY cpuQuery;
	PDH_HCOUNTER cpuTotal;
};

