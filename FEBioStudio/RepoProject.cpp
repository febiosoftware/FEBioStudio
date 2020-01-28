#include "RepoProject.h"

CRepoProject::CRepoProject(int columns, char** data)
{
	id = std::stoi(data[0]);
	name = QString(data[1]);
	description = QString(data[2]);
	owner = QString(data[3]);
}
