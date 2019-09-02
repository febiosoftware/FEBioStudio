#pragma once
#include <string>
#include <vector>
class CDocument;

enum ERROR_TYPE
{
	CRITICAL,
	WARNING
};

typedef std::pair<ERROR_TYPE, std::string> MODEL_ERROR;

void checkModel(CDocument* doc, std::vector<MODEL_ERROR>& errorList);
