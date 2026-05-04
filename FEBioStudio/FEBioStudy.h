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

class CStudy : public FSThreadedTask
{
public:
	CStudy(CModelDocument* doc, StudyType type);

	CModelDocument* GetDocument() { return m_doc; }

	virtual bool Run() = 0;

	std::string GetFEBioFileName() const { return m_febioFileName; }
	std::string GetOptionsFileName() const { return m_optionsFileName; }

	virtual std::string GetOutputFileName() const { return std::string(); }

	StudyType GetType() const { return m_type; }

	virtual FSObject* GetStudyData() { return nullptr; }

protected:
	std::string m_febioFileName;
	std::string m_optionsFileName;

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
		LogFileName,
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
	};

public:
	COptimizationStudy(CModelDocument* doc);

	void SetOptions(FEBioOpt ops) { m_ops = ops; }
	FEBioOpt& Options() { return m_ops; }

	bool Run() override;

	std::string GetOutputFileName() const override { return m_logFileName; }

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	FEBioOpt m_ops;
	std::string m_logFileName;
};

class CFEBioStudy : public CStudy
{
	// Don't change the order of these fields as they are used for serialization!
	enum DataField {
		StudyName,
		StudyInfo,
		StudyData,
	};

public:
	CFEBioStudy(CModelDocument* doc, FSCoreStudy* study = nullptr);
	bool Run() override;
	std::string GetOutputFileName() const override { return m_reportFile; }

private:
	void SetStudy(FSCoreStudy* study);

	FSObject* GetStudyData() override { return m_study; }

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

private:
	FSCoreStudy* m_study = nullptr;
	std::string m_reportFile;
};
