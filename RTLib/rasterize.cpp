/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "rasterize.h"
#include <cmath>

// Returns a list of pixel coordinates along the line
std::vector<rt::Pixel> rt::rasterizeLine(int x0, int y0, int x1, int y1, bool& swapped)
{
	std::vector<Pixel> points;

	bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

	swapped = false;

	// If the line is steep, swap x and y
	if (steep) {
		std::swap(x0, y0);
		std::swap(x1, y1);
	}

	// Ensure left-to-right drawing
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
		swapped = !swapped;
	}

	int dx = x1 - x0;
	int dy = std::abs(y1 - y0);
	int error = dx / 2;
	int ystep = (y0 < y1) ? 1 : -1;
	int y = y0;

	for (int x = x0; x <= x1; ++x) {
		if (steep)
			points.push_back({ y, x,1 });
		else
			points.push_back({ x, y,1 });

		error -= dy;
		if (error < 0) {
			y += ystep;
			error += dx;
		}
	}

	return points;
}


// Helper: fractional part
inline float fpart(float x) { return x - std::floor(x); }
inline float rfpart(float x) { return 1.0f - fpart(x); }

// Returns a list of pixels with brightness values
std::vector<rt::Pixel> rt::rasterizeLineAA(float x0, float y0, float x1, float y1, bool& swapped)
{
	std::vector<rt::Pixel> pixels;
	bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

	// Swap axes if line is steep
	if (steep) {
		std::swap(x0, y0);
		std::swap(x1, y1);
	}

	// Ensure left-to-right drawing
	swapped = false;
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
		swapped = true;
	}

	float dx = x1 - x0;
	float dy = y1 - y0;
	float gradient = (dx == 0.0f) ? 1.0f : dy / dx;

	// handle first endpoint
	float xend = std::round(x0);
	float yend = y0 + gradient * (xend - x0);
	float xgap = rfpart(x0 + 0.5f);
	int xpxl1 = (int)xend;
	int ypxl1 = (int)std::floor(yend);

	if (steep) {
		pixels.push_back({ ypxl1,     xpxl1, rfpart(yend) * xgap });
		pixels.push_back({ ypxl1 + 1, xpxl1, fpart(yend) * xgap });
	}
	else {
		pixels.push_back({ xpxl1, ypxl1,     rfpart(yend) * xgap });
		pixels.push_back({ xpxl1, ypxl1 + 1, fpart(yend) * xgap });
	}

	float intery = yend + gradient;

	// handle second endpoint
	xend = std::round(x1);
	yend = y1 + gradient * (xend - x1);
	xgap = fpart(x1 + 0.5f);
	int xpxl2 = (int)xend;
	int ypxl2 = (int)std::floor(yend);

	// main loop
	for (int x = xpxl1 + 1; x < xpxl2; ++x) {
		if (steep) {
			pixels.push_back({ (int)std::floor(intery),     x, rfpart(intery) });
			pixels.push_back({ (int)std::floor(intery) + 1, x, fpart(intery) });
		}
		else {
			pixels.push_back({ x, (int)std::floor(intery),     rfpart(intery) });
			pixels.push_back({ x, (int)std::floor(intery) + 1, fpart(intery) });
		}
		intery += gradient;
	}

	// second endpoint
	if (steep) {
		pixels.push_back({ ypxl2,     xpxl2, rfpart(yend) * xgap });
		pixels.push_back({ ypxl2 + 1, xpxl2, fpart(yend) * xgap });
	}
	else {
		pixels.push_back({ xpxl2, ypxl2,     rfpart(yend) * xgap });
		pixels.push_back({ xpxl2, ypxl2 + 1, fpart(yend) * xgap });
	}

	return pixels;
}

std::vector<rt::Pixel> rt::rasterizePoint(int x0, int y0, float pointSize)
{
	std::vector<rt::Pixel> pixels;
	if (pointSize <= 1.f)
	{
		pixels.push_back({ x0, y0, 1.f });
	}
	else
	{
		int r = (int)(pointSize / 2.f);
		int r2 = r * r;
		for (int y = -r; y <= r; ++y)
			for (int x = -r; x <= r; ++x)
			{
				int distSqr = x * x + y * y;
				if (distSqr <= r2 )
				{
					float brightness = 1.f;
					pixels.push_back({ x0 + x, y0 + y, brightness });
				}
				else
				{
					float dist = sqrtf((float)distSqr);
					float dr = dist - (float)r;
					if (dr < 1.f)
					{
						float brightness = 1.f - dr;
						pixels.push_back({ x0 + x, y0 + y, brightness });
					}
				}
			}
	}

	return pixels;
}
