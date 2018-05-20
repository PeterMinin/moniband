#pragma once

#include <windef.h>

class Graph
{
public:
	Graph();
	virtual ~Graph();

	void SetSize(int width, int height);
	void Draw(HDC hdc, RECT rc);
};

