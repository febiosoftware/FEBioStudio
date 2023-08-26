#pragma once
#include <FSCore/FileReader.h>

class CPostDocument;
class FEKinemat;

class FEKinematFileReader : public FileReader
{
public:
	FEKinematFileReader(CPostDocument* doc);
	~FEKinematFileReader();
	bool Load(const char* szfile) override;

	void SetModelFile(const std::string& modelFile);
	void SetKineFile(const std::string& kineFile);
	void SetRange(int rngMin, int rngMax, int rngStep);

	float GetFileProgress() const override;

public:
	std::string GetModelFile() const;
	std::string GetKineFile() const;
	int GetMin() const;
	int GetMax() const;
	int GetStep() const;

private:
	std::string	m_modelFile;
	std::string	m_kineFile;
	int	m_min, m_max, m_step;

private:
	FEKinemat*		m_kine;
	CPostDocument* m_doc;
};
