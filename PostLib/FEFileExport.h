#pragma once

namespace Post {

	class FEPostModel;

//-----------------------------------------------------------------------------
// Base class for file exporters
class FEFileExport
{
public:
	FEFileExport(void);
	virtual ~FEFileExport(void);

	virtual bool Save(FEPostModel& fem, const char* szfile) = 0;
};

}