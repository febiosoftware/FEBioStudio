#include "FEFileReader.h"
using namespace Post;

FEFileReader::FEFileReader(Post::FEPostModel* model) : m_fem(model)
{
}

FEFileReader::~FEFileReader()
{
}

void FEFileReader::SetPostModel(FEPostModel* fem)
{
	m_fem = fem;
}

bool FEFileReader::Load(FEPostModel* fem, const char* szfile)
{
	SetPostModel(fem);
	return Load(szfile);
}
