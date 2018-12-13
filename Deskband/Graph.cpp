#include "stdafx.h"
#include "Graph.h"

#include <cassert>


void Graph::Draw(HDC hdc, RECT rc)
{
	
}

void Graph::StartUpdateTimer(HWND hwnd, UINT_PTR timerId)
{
	assert(timerId != 0);
	this->timerHwnd = hwnd;
	this->timerId = timerId;
	SetTimer(hwnd, timerId, timerPeriod, NULL);
}

void Graph::StopUpdateTimer()
{
	KillTimer(timerHwnd, timerId);
}
