#pragma once
#include <stdio.h>
#include <string>
#include <MeshIO/FileReader.h>

#ifdef WIN32
typedef __int64 off_type;
#endif

#ifdef LINUX // same for Linux and Mac OS X
typedef off_t off_type;
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
typedef off_t off_type;
#endif

//-----------------------------------------------------------------------------
// forward declaration of model class
namespace Post {

class FEPostModel;

//-----------------------------------------------------------------------------
class FEFileReader : public FileReader
{
public:
	FEFileReader(FEPostModel* fem = nullptr);
	virtual ~FEFileReader();

	void SetPostModel(FEPostModel* fem);

	using FileReader::Load;
	bool Load(FEPostModel* fem, const char* szfile);

protected:
	FEPostModel*	m_fem;
};

}