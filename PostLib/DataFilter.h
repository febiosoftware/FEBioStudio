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
#include <FSCore/math3d.h>

namespace Post {

class ModelDataField;

// Forward declaration of FEPostModel class
class FEPostModel;

// Scale data by facor
bool DataScale(FEPostModel& fem, int nfield, double scale);
bool DataScaleVec3(FEPostModel& fem, int nfield, vec3d scale);

// Apply a smoothing operation on data
bool DataSmooth(FEPostModel& fem, int nfield, double theta, int niters);

// Apply an arithmic operation on data
bool DataArithmetic(FEPostModel& fem, int nfield, int nop, int noperand);

// Apply an math operation on data
bool DataMath(FEPostModel& fem, int nfield, int nop);

// Calculate the gradient of a scale field
// (set config to 0 for material, to 1 for spatial gradient)
bool DataGradient(FEPostModel& fem, int vecField, int sclField, int config = 1);

// Calculate the fractional anisotropy of a tensor field
bool DataFractionalAnsisotropy(FEPostModel& fem, int scalarField, int tensorField);

// Extract a component from a data field
ModelDataField* DataComponent(FEPostModel& fem, ModelDataField* dataField, int ncomp, const std::string& sname);

// convert between formats
ModelDataField* DataConvert(FEPostModel& fem, ModelDataField* dataField, int newClass, int newFormat, const std::string& name);

// calculate eigen tensor
ModelDataField* DataEigenTensor(FEPostModel& fem, ModelDataField* dataField, const std::string& name);

// calculate the time rate of a quantity
ModelDataField* DataTimeRate(FEPostModel& fem, ModelDataField* dataField, const std::string& name);

// project a data field onto the surface and onto the surface normal
// For a scalar field, this just maps the field to the surface.
// For a vector field, this calculates the scalar field N*V. 
// For a tensor field, this calculates the scalar field N*(S*N)
ModelDataField* SurfaceNormalProjection(FEPostModel& fem, ModelDataField* dataField, const std::string& name);
}
