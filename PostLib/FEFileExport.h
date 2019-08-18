#pragma once
#include "FEModel.h"

namespace Post {

//-----------------------------------------------------------------------------
// Base class for file exporters
class FEFileExport
{
public:
	FEFileExport(void);
	virtual ~FEFileExport(void);

	virtual bool Save(FEModel& fem, const char* szfile) = 0;
};

}