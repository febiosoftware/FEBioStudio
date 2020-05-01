#pragma once
#include "FEFileReader.h"

namespace Post {

class FEU3DImport : public FEFileReader
{
public:
	FEU3DImport(FEPostModel* fem);

	bool Load(const char* szfile) override;
};

}
