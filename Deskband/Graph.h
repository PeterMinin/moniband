#pragma once

#include <windef.h>
#include <vector>
#include <deque>
#include <chrono>


class Graph
{
public:
	virtual ~Graph() {}

	void Draw(HDC hdc, RECT rc);
	void StartUpdateTimer(HWND hwnd, UINT_PTR timerId);
	void StopUpdateTimer();

protected:
	typedef std::vector<float> Measurement;

	virtual Measurement Measure() = 0;

private:
	typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
	
	std::deque<Measurement> values;
	std::deque<TimePoint> times;
	UINT timerPeriod = 1000;
	HWND timerHwnd = nullptr;
	UINT_PTR timerId = 0;
};

