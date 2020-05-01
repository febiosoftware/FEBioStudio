#pragma once
#include <string>
#include <vector>
class FEProject;

enum ERROR_TYPE
{
	CRITICAL,
	WARNING
};

typedef std::pair<ERROR_TYPE, std::string> MODEL_ERROR;

void checkModel(FEProject& prj, std::vector<MODEL_ERROR>& errorList);
