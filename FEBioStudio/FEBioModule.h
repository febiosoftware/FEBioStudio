#pragma once
#include <vector>

namespace FEBio {

	struct FEBioModule
	{
		const char* m_szname;
		const char* m_szdesc;
		int				m_id;
	};

	std::vector<FEBioModule> GetAllModules();

	const char* GetModuleName(int moduleId);

}
