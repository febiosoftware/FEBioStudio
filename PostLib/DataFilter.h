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

#pragma once
#include <string>
#include <MathLib/math3d.h>

namespace Post {

class FEDataField;

//-----------------------------------------------------------------------------
// Forward declaration of FEPostModel class
class FEPostModel;

//-----------------------------------------------------------------------------
// Scale data by facor
bool DataScale(FEPostModel& fem, int nfield, double scale);
bool DataScaleVec3(FEPostModel& fem, int nfield, vec3d scale);

//-----------------------------------------------------------------------------
// Apply a smoothing operation on data
bool DataSmooth(FEPostModel& fem, int nfield, double theta, int niters);

//-----------------------------------------------------------------------------
// Apply a smoothing operation on data
bool DataArithmetic(FEPostModel& fem, int nfield, int nop, int noperand);

//-----------------------------------------------------------------------------
// Calculate the gradient of a scale field
bool DataGradient(FEPostModel& fem, int vecField, int sclField);

//-----------------------------------------------------------------------------
// Calculate the fractional anisotropy of a tensor field
bool DataFractionalAnsisotropy(FEPostModel& fem, int scalarField, int tensorField);

//-----------------------------------------------------------------------------
// Extract a component from a data field
FEDataField* DataComponent(FEPostModel& fem, FEDataField* dataField, int ncomp, const std::string& sname);

//-----------------------------------------------------------------------------
// convert between formats
FEDataField* DataConvert(FEPostModel& fem, FEDataField* dataField, int newFormat, const std::string& name);

//-----------------------------------------------------------------------------
FEDataField* DataEigenTensor(FEPostModel& fem, FEDataField* dataField, const std::string& name);
}
