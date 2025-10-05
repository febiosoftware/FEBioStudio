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
        int             m_allocatorID;
	};

	std::vector<FEBioModule> GetAllModules();

	const char* GetModuleName(int moduleId);
	int GetModuleId(const std::string& moduleName);
    int GetModuleAllocatorID(int moduleId);

	void SetActiveModule(int moduleID);
	int GetActiveModule();
	const char* GetActiveModuleName();

	int SetActiveModule(const char* szmoduleName);

	void InitFSModel(FSModel& fem);

	void BlockCreateEvents(bool b);
}
