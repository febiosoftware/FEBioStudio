/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#pragma once
#include <FSCore/FSThreadedTask.h>
#include <FEMLib/FSCoreStudy.h>
#include "FEBioOpt.h"

enum StudyType
{
	INVALID_STUDY,
	OPTIMIZATION_STUDY,
	FEBIO_STUDY,
};

class CModelDocument;
class QString;

class CStudy : public FSThreadedTask
{
public:
	CStudy(CModelDocument* doc, StudyType type);

	CModelDocument* GetDocument() { return m_doc; }

	// write input files for the study.
	// the dir parameter specifies the directory where the files should be written.
	virtual bool WriteFiles(const QString& dir) = 0;

	virtual std::string GetStudyType() const = 0;

	std::string GetFEBioFileName() const { return m_febioFileName; }
	std::string GetOptionsFileName() const { return m_optionsFileName; }
	std::string GetOutputFileName() const { return m_outputFile; }

	void SetFEBioFileName(const std::string& fileName) { m_febioFileName = fileName; }
	void SetOptionsFileName(const std::string& fileName) { m_optionsFileName = fileName; }
	void SetOutputFileName(const std::string& fileName) { m_outputFile = fileName; }

	StudyType GetType() const { return m_type; }

	virtual FSObject* GetStudyData() { return nullptr; }

protected:
	std::string m_febioFileName;
	std::string m_optionsFileName;
	std::string m_outputFile;

private:
	CModelDocument* m_doc;
	StudyType m_type;
};

class COptimizationStudy : public CStudy
{
	// Don't change the order of these fields as they are used for serialization!
	enum DataField {
		StudyName,
		StudyInfo,
		OutputFileName,
		OptMethod,
		ObjTol,
		FDiffScale,
		OutputLevel,
		PrintLevel,
		Objective,
		Param,
		ObjParam,
		ObjData,
		TrgVar,
		EDVar,
		EDData,
		NDVar,
		NDData,
		ReportFlag,
		FEBFileName,
		OptionsFileName,
	};

public:
	COptimizationStudy(CModelDocument* doc);

	void SetOptions(FEBioOpt ops) { m_ops = ops; }
	FEBioOpt& Options() { return m_ops; }

	bool WriteFiles(const QString& dir) override;

	std::string GetStudyType() const override { return "optimize"; };

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	FEBioOpt m_ops;
};

class CFEBioStudy : public CStudy
{
	// Don't change the order of these fields as they are used for serialization!
	enum DataField {
		StudyName,
		StudyInfo,
		StudyData,
		FEBFileName,
		OptionsFileName,
		OutputFileName,
	};

public:
	CFEBioStudy(CModelDocument* doc, FSCoreStudy* study = nullptr);

	bool WriteFiles(const QString& dir) override;

	std::string GetStudyType() const override;

private:
	void SetStudy(FSCoreStudy* study);

	FSObject* GetStudyData() override { return m_study; }

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	FSCoreStudy* m_study = nullptr;
};
