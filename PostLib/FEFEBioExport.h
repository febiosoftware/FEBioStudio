#pragma once
#include "FEFileExport.h"

namespace Post {
//-----------------------------------------------------------------------------
// Export model to a FEBio file
class FEFEBioExport : public FEFileExport
{
public:
	FEFEBioExport(void) {}
	~FEFEBioExport(void) {}

	bool Save(FEPostModel& fem, const char* szfile);
};
}
