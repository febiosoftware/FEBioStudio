#pragma once
#include <vector>
#include <string>

class FSModel;

namespace FEBio {

	struct FEBioModule
	{
		const char* m_szname;
		const char* m_szdesc;
		int				m_id;
	};

	std::vector<FEBioModule> GetAllModules();

	const char* GetModuleName(int moduleId);
	int GetModuleId(const std::string& moduleName);

	void SetActiveModule(int moduleID);
	int GetActiveModule();

	void InitFSModel(FSModel& fem);
}
