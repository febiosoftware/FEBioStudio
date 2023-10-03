/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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

#include "ImageViewSettings.h"

CImageViewSettings::CImageViewSettings()
{
    AddDoubleParam(0.1, "Alpha Scale")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(1.0, "Gamma correction")->SetFloatRange(0.0, 2.0);
	AddDoubleParam(0.0, "Min Intensity")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(1.0, "Max Intensity")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0.0, "Min alpha")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(1.0, "Max alpha")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0.0, "Hue")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0.0, "Saturation")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(1.0, "Luminance")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0.0, "clipx_min")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(1.0, "clipx_max")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0.0, "clipy_min")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(1.0, "clipy_max")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0.0, "clipz_min")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(1.0, "clipz_max")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0.000, "chue1")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0.333, "chue2")->SetFloatRange(0.0, 1.0);
	AddDoubleParam(0.667, "chue3")->SetFloatRange(0.0, 1.0);
}
