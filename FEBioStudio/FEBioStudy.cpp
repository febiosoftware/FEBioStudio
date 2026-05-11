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
#include "stdafx.h"
#include "FEBioStudy.h"
#include "ModelDocument.h"
#include <QFileInfo>
#include <FEBio/FEBioExport4.h>

CStudy::CStudy(CModelDocument* doc, StudyType type) : m_doc(doc), m_type(type)
{

}

COptimizationStudy::COptimizationStudy(CModelDocument* doc)
	: CStudy(doc, OPTIMIZATION_STUDY)
{
	SetTypeString("Parameter optimization");
}

bool COptimizationStudy::WriteFiles(const QString& dir)
{
	QString name = QString::fromStdString(GetName());
	QString studyPath = name + ".opt";
	QString febPath = name + ".feb";
	m_febioFileName = febPath.toStdString();
	m_optionsFileName = studyPath.toStdString();

	QString logFileName = dir + "/" + name + ".log";

	m_outputFile = logFileName.toStdString();

	try {
		setCurrentTask("Saving FEBio input file ...");
		FEBioExport4* febExport = new FEBioExport4(GetDocument()->GetProject());
		febExport->SetMixedMeshFlag(false);
		//		febExport->SetProgressTracker(prg);
		bool ret = febExport->Write(m_febioFileName.c_str());
		if (ret == false)
		{
			setErrorString(febExport->GetErrorMessage());
			return false;
		}

		setCurrentTask("Saving optimization input file ...");
		if (GenerateFEBioOptimizationFile(m_optionsFileName, Options()) == false)
		{
			return errf("Failed creating the optimization input file.");
		}
	}
	catch (...)
	{
		return errf("Something went terribly wrong!");
	}
	return true;

}

void COptimizationStudy::Save(OArchive& ar)
{
	ar.WriteChunk(DataField::StudyName      , GetName());
	ar.WriteChunk(DataField::StudyInfo      , GetInfo());
	ar.WriteChunk(DataField::FEBFileName    , m_febioFileName);
	ar.WriteChunk(DataField::OptionsFileName, m_optionsFileName);
	ar.WriteChunk(DataField::OutputFileName , m_outputFile);
	ar.WriteChunk(DataField::OptMethod      , m_ops.m_method);
	ar.WriteChunk(DataField::ObjTol         , m_ops.m_obj_tol);
	ar.WriteChunk(DataField::FDiffScale     , m_ops.m_f_diff_scale);
	ar.WriteChunk(DataField::OutputLevel    , m_ops.m_outLevel);
	ar.WriteChunk(DataField::PrintLevel     , m_ops.m_printLevel);
	ar.WriteChunk(DataField::Objective      , m_ops.m_objective);
	ar.WriteChunk(DataField::ReportFlag     , m_ops.m_report);

	// parameters
	for (size_t i = 0; i < m_ops.m_params.size(); ++i)
	{
		ar.BeginChunk(DataField::Param);
		ar.WriteChunk(0, m_ops.m_params[i].m_name);
		ar.WriteChunk(1, m_ops.m_params[i].m_initVal);
		ar.WriteChunk(2, m_ops.m_params[i].m_minVal);
		ar.WriteChunk(3, m_ops.m_params[i].m_maxVal);
		ar.EndChunk();
	}

	// data-fit model
	ar.WriteChunk(DataField::ObjParam, m_ops.m_objParam);
	for (size_t i = 0; i < m_ops.m_data.size(); ++i)
	{
		ar.BeginChunk(DataField::ObjData);
		ar.WriteChunk(0, m_ops.m_data[i].m_time);
		ar.WriteChunk(1, m_ops.m_data[i].m_value);
		ar.EndChunk();
	}

	// target model
	for (size_t i = 0; i < m_ops.m_trgVar.size(); ++i)
	{
		ar.BeginChunk(DataField::TrgVar);
		ar.WriteChunk(0, m_ops.m_trgVar[i].m_name);
		ar.WriteChunk(1, m_ops.m_trgVar[i].m_val);
		ar.EndChunk();
	}

	// element-data model
	ar.WriteChunk(DataField::EDVar, m_ops.m_edVar);
	for (size_t i = 0; i < m_ops.m_edData.size(); ++i)
	{
		ar.BeginChunk(DataField::EDData);
		ar.WriteChunk(0, m_ops.m_edData[i].m_id);
		ar.WriteChunk(1, m_ops.m_edData[i].m_value);
		ar.EndChunk();
	}

	// node-data model
	ar.WriteChunk(DataField::NDVar, m_ops.m_ndVar);
	for (size_t i = 0; i < m_ops.m_ndData.size(); ++i)
	{
		ar.BeginChunk(DataField::NDData);
		ar.WriteChunk(0, m_ops.m_ndData[i].m_id);
		ar.WriteChunk(1, m_ops.m_ndData[i].m_value);
		ar.EndChunk();
	}
}

void COptimizationStudy::Load(IArchive& ar)
{
	std::string s;
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case DataField::StudyName: ar.read(s); SetName(s); break;
		case DataField::StudyInfo: ar.read(s); SetInfo(s); break;
		case DataField::FEBFileName: ar.read(m_febioFileName); break;
		case DataField::OptionsFileName: ar.read(m_optionsFileName); break;
		case DataField::OutputFileName: ar.read(m_outputFile); break;
		case DataField::OptMethod: ar.read(m_ops.m_method); break;
		case DataField::ObjTol: ar.read(m_ops.m_obj_tol); break;
		case DataField::FDiffScale: ar.read(m_ops.m_f_diff_scale); break;
		case DataField::OutputLevel: ar.read(m_ops.m_outLevel); break;
		case DataField::PrintLevel: ar.read(m_ops.m_printLevel); break;
		case DataField::Objective: ar.read(m_ops.m_objective); break;
		case DataField::ReportFlag: ar.read(m_ops.m_report); break;
		// parameters
		case DataField::Param:
		{
			FEBioOpt::Param p("");
			while (ar.OpenChunk() == IArchive::IO_OK)
			{
				int nid = ar.GetChunkID();
				switch (nid)
				{
				case 0: ar.read(s); p.m_name = s; break;
				case 1: ar.read(p.m_initVal); break;
				case 2: ar.read(p.m_minVal); break;
				case 3: ar.read(p.m_maxVal); break;
				}
				ar.CloseChunk();
			}
			m_ops.AddParameter(p);
			break;
		}
		// data-fit model
		case DataField::ObjParam: ar.read(m_ops.m_objParam); break;
		case DataField::ObjData:
		{
			FEBioOpt::Data d;
			while (ar.OpenChunk() == IArchive::IO_OK)
			{
				int nid = ar.GetChunkID();
				switch (nid)
				{
				case 0: ar.read(d.m_time); break;
				case 1: ar.read(d.m_value); break;
				}
				ar.CloseChunk();
			}
			m_ops.AddData(d.m_time, d.m_value);
			break;
		}
		// target model
		case DataField::TrgVar:
		{
			FEBioOpt::TargetVar v;
			while (ar.OpenChunk() == IArchive::IO_OK)
			{
				int nid = ar.GetChunkID();
				switch (nid)
				{
				case 0: ar.read(s); v.m_name = s; break;
				case 1: ar.read(v.m_val); break;
				}
				ar.CloseChunk();
			}
			m_ops.m_trgVar.push_back(v);
			break;
		}
		// element-data model
		case DataField::EDVar: ar.read(m_ops.m_edVar); break;
		case DataField::EDData:
		{
			FEBioOpt::IDValue v;
			while (ar.OpenChunk() == IArchive::IO_OK)
			{
				int nid = ar.GetChunkID();
				switch (nid)
				{
				case 0: ar.read(v.m_id); break;
				case 1: ar.read(v.m_value); break;
				}
				ar.CloseChunk();
			}
			m_ops.m_edData.push_back(v);
			break;
		}
		// node-data model
		case DataField::NDVar: ar.read(m_ops.m_ndVar); break;
		case DataField::NDData:
		{
			FEBioOpt::IDValue v;
			while (ar.OpenChunk() == IArchive::IO_OK)
			{
				int nid = ar.GetChunkID();
				switch (nid)
				{
				case 0: ar.read(v.m_id); break;
				case 1: ar.read(v.m_value); break;
				}
				ar.CloseChunk();
			}
			m_ops.m_ndData.push_back(v);
			break;
		}
		}
		ar.CloseChunk();
	}
}

//=================================================================================================
CFEBioStudy::CFEBioStudy(CModelDocument* doc, FSCoreStudy* study) : CStudy(doc, FEBIO_STUDY)
{
	SetStudy(study);
}

std::string CFEBioStudy::GetStudyType() const 
{ 
	return (m_study ? m_study->GetTypeString() : "");
}

void CFEBioStudy::SetStudy(FSCoreStudy* study)
{
	if (m_study) delete m_study;
	m_study = study;
	if (m_study) SetTypeString(m_study->GetTypeString());
}

void WriteParam(XMLWriter& xml, Param& p)
{
	switch (p.GetParamType())
	{
	case Param_BOOL  : xml.add_leaf(p.GetShortName(), p.GetBoolValue  ()); break;
	case Param_INT   : xml.add_leaf(p.GetShortName(), p.GetIntValue   ()); break;
	case Param_FLOAT : xml.add_leaf(p.GetShortName(), p.GetFloatValue ()); break;
	case Param_STRING: xml.add_leaf(p.GetShortName(), p.GetStringValue()); break;
	case Param_ARRAY_DOUBLE:
	{
		std::vector<double> v = p.GetArrayDoubleValue();
		xml.add_leaf(p.GetShortName(), v.data(), v.size());
		break;
	}
	default:
		assert(false);
	}
}

void WriteModelComponent(XMLWriter& xml, FSModelComponent* pc)
{
	for (int i = 0; i < pc->Parameters(); ++i)
	{
		Param& p = pc->GetParam(i);
		WriteParam(xml, p);
	}

	for (int i = 0; i < pc->Properties(); ++i)
	{
		FSProperty& prop = pc->GetProperty(i);
		if (prop.Size() == 0) continue;

		std::string name = prop.GetName();

		for (size_t j = 0; j < prop.Size(); ++j)
		{
			FSModelComponent* pc = dynamic_cast<FSModelComponent*>(prop.GetComponent(j));
			if (pc == nullptr) continue;

			XMLElement el(name.c_str());
			if (!prop.IsFixed())
				el.add_attribute("type", pc->GetTypeString());

			xml.add_branch(el);
			WriteModelComponent(xml, pc);
			xml.close_branch();
		}
	}
}

bool WriteTaskControlFile(const std::string& filename, const std::string& taskName, FSCoreStudy* study)
{
	try {
		XMLWriter xml;
		xml.open(filename.c_str());

		XMLElement el("febio_study");
		el.add_attribute("type", taskName.c_str());
		xml.add_branch(el);
		WriteModelComponent(xml, study);
		xml.close_branch();
		xml.close();
	}
	catch (...)
	{
		return false;
	}
	return true;
}

bool CFEBioStudy::WriteFiles(const QString& dir)
{
	QString name = QString::fromStdString(GetName());
	QString studyPath = name + ".opt";
	QString febPath = name + ".feb";
	QString reportPath = name + ".febr";
	m_febioFileName = febPath.toStdString();
	m_optionsFileName = studyPath.toStdString();
	m_outputFile = reportPath.toStdString();

	try {
		setCurrentTask("Saving FEBio input file ...");
		FEBioExport4* febExport = new FEBioExport4(GetDocument()->GetProject());
		febExport->SetMixedMeshFlag(false);
		bool ret = febExport->Write(m_febioFileName.c_str());
		if (ret == false)
		{
			setErrorString(febExport->GetErrorMessage());
			return false;
		}

		std::string taskName = m_study->GetTypeString();

		setCurrentTask("Saving study control file ...");
		if (WriteTaskControlFile(m_optionsFileName, taskName, m_study) == false)
		{
			return errf("Failed creating the study control file.");
		}
	}
	catch (...)
	{
		return errf("Something went terribly wrong!");
	}

	return true;
}

void CFEBioStudy::Save(OArchive& ar)
{
	ar.WriteChunk(StudyName      , GetName());
	ar.WriteChunk(StudyInfo      , GetInfo());
	ar.WriteChunk(FEBFileName    , m_febioFileName);
	ar.WriteChunk(OptionsFileName, m_optionsFileName);
	ar.WriteChunk(OutputFileName , m_outputFile);

	if (m_study)
	{
		ar.BeginChunk(StudyData);
		m_study->Save(ar);
		ar.EndChunk();
	}
}

void CFEBioStudy::Load(IArchive& ar)
{
	std::string s;
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case StudyName: ar.read(s); SetName(s); break;
		case StudyInfo: ar.read(s); SetInfo(s); break;
		case FEBFileName: ar.read(m_febioFileName); break;
		case OptionsFileName: ar.read(m_optionsFileName); break;
		case OutputFileName: ar.read(m_outputFile); break;
		case StudyData:
		{
			FSCoreStudy* study = new FSCoreStudy(GetDocument()->GetFSModel());
			study->Load(ar);
			SetStudy(study);
			break;
		}
		}
		ar.CloseChunk();
	}
}
