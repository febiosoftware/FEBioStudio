#include "stdafx.h"
#include "FEKinematFileReader.h"
#include <PostLib/FEKinemat.h>
#include <PostLib/FELSDYNAimport.h>
#include <PostLib/GLModel.h>
#include "PostDocument.h"
using namespace Post;

FEKinematFileReader::FEKinematFileReader(CPostDocument* doc) : m_doc(doc)
{
	m_min = m_max = m_step = 0;
	m_kine = new FEKinemat;
}

FEKinematFileReader::~FEKinematFileReader()
{
	delete m_kine;
	m_kine = nullptr;
}

void FEKinematFileReader::SetModelFile(const std::string& modelFile)
{
	m_modelFile = modelFile;
}

void FEKinematFileReader::SetKineFile(const std::string& kineFile)
{
	m_kineFile = kineFile;
}

void FEKinematFileReader::SetRange(int rngMin, int rngMax, int rngStep)
{
	m_min = rngMin;
	m_max = rngMax;
	m_step = rngStep;
}

std::string FEKinematFileReader::GetModelFile() const { return m_modelFile; }
std::string FEKinematFileReader::GetKineFile() const { return m_kineFile; }
int FEKinematFileReader::GetMin() const { return m_min; }
int FEKinematFileReader::GetMax() const { return m_max; }
int FEKinematFileReader::GetStep() const { return m_step; }

float FEKinematFileReader::GetFileProgress() const
{
	if (m_step <= 0) return 0.f;
	if (m_kine->IsKineValid() == false) return 0.f;
	int nstates = m_kine->States();
	if (nstates == 0) return 1.f;
	Post::FEPostModel* fem = m_doc->GetFSModel();
	if (fem == nullptr) return 0.f;
	int nsteps = fem->GetStates();
	return (float)nsteps / (float)nstates;
}

bool FEKinematFileReader::Load(const char* szfile)
{
	if (m_doc == nullptr) return false;

	// read the model
	Post::FEPostModel* fem = m_doc->GetFSModel();
	Post::FELSDYNAimport reader(fem);
	reader.read_displacements(true);
	bool bret = reader.Load(m_modelFile.c_str());
	if (bret == false) return false;

	// apply kine
	FEKinemat& kine = *m_kine;
	kine.SetRange(m_min, m_max, m_step);
	if (kine.Apply(fem, m_kineFile.c_str()) == false) return false;

	// update post document
	m_doc->Initialize();

	// a new model is created when the doc is initialized
	CGLModel* glm = m_doc->GetGLModel();

	// update displacements on all states
	if (glm->GetDisplacementMap() == nullptr)
	{
		glm->AddDisplacementMap("Displacement");
	}
	int nstates = glm->GetFSModel()->GetStates();
	for (int i = 0; i < nstates; ++i) glm->UpdateDisplacements(i, true);

	return true;
}
