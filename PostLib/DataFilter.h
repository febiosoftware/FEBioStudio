#pragma once
#include <string>

namespace Post {

class FEDataField;

//-----------------------------------------------------------------------------
// Forward declaration of FEPostModel class
class FEPostModel;

//-----------------------------------------------------------------------------
// Scale data by facor
bool DataScale(FEPostModel& fem, int nfield, double scale);

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

}
