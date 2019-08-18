#pragma once
#include <string>

namespace Post {

class FEDataField;

//-----------------------------------------------------------------------------
// Forward declaration of FEModel class
class FEModel;

//-----------------------------------------------------------------------------
// Scale data by facor
bool DataScale(FEModel& fem, int nfield, double scale);

//-----------------------------------------------------------------------------
// Apply a smoothing operation on data
bool DataSmooth(FEModel& fem, int nfield, double theta, int niters);

//-----------------------------------------------------------------------------
// Apply a smoothing operation on data
bool DataArithmetic(FEModel& fem, int nfield, int nop, int noperand);

//-----------------------------------------------------------------------------
// Calculate the gradient of a scale field
bool DataGradient(FEModel& fem, int vecField, int sclField);

//-----------------------------------------------------------------------------
// Extract a component from a data field
FEDataField* DataComponent(FEModel& fem, FEDataField* dataField, int ncomp, const std::string& sname);

}
