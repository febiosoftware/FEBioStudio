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
#include <QDir>
#include <FEBioRun/FEBioRun.h>
#include <FEBio/FEBioExport4.h>

CFEBioStudy::CFEBioStudy(CModelDocument* doc, StudyType type) : m_doc(doc), m_type(type)
{

}

COptimizationStudy::COptimizationStudy(CModelDocument* doc)
	: CFEBioStudy(doc, STUDY_OPTIMIZATION)
{
	SetTypeString("Parameter optimization");
}

bool COptimizationStudy::Run()
{
	CModelDocument* doc = GetDocument();
	std::string filepath = doc->GetDocFilePath();
	QFileInfo fi(QString::fromStdString(filepath));
	QString dir = fi.absolutePath();

	// set the working directory to this folder
	QDir::setCurrent(dir);

	QString name = QString::fromStdString(GetName());
	QString studyPath = name + ".opt";
	QString febPath = name + ".feb";
	string febfile = febPath.toStdString();
	string optfile = studyPath.toStdString();

	m_logFileName = dir + "/" + name + ".log";

	try {
		setCurrentTask("Saving FEBio input file ...");
		FEBioExport4* febExport = new FEBioExport4(doc->GetProject());
		febExport->SetMixedMeshFlag(false);
//		febExport->SetProgressTracker(prg);
		bool ret = febExport->Write(febfile.c_str());
		if (ret == false)
		{
			setErrorString(febExport->GetErrorMessage());
			return false;
		}

		setCurrentTask("Saving optimization input file ...");
		if (GenerateFEBioOptimizationFile(optfile, Options()) == false)
		{
			return errf("Failed creating the optimization input file.");
		}

		setCurrentTask("Running optimization ...");
		string cmd = "-i " + febfile + " -s " + optfile;
		int returnCode = FEBio::runModel(cmd, nullptr, nullptr, nullptr);
		if (returnCode != 0) return errf("Study failed! FEBio error terminated.");
	}
	catch (...)
	{
		return errf("Something went terribly wrong!");
	}
	return true;
}

void COptimizationStudy::Save(OArchive& ar)
{
	ar.WriteChunk(DataField::StudyName   , GetName());
	ar.WriteChunk(DataField::StudyInfo   , GetInfo());
	ar.WriteChunk(DataField::LogFileName , m_logFileName.toStdString());
	ar.WriteChunk(DataField::OptMethod   , m_ops.m_method);
	ar.WriteChunk(DataField::ObjTol      , m_ops.m_obj_tol);
	ar.WriteChunk(DataField::FDiffScale  , m_ops.m_f_diff_scale);
	ar.WriteChunk(DataField::OutputLevel , m_ops.m_outLevel);
	ar.WriteChunk(DataField::PrintLevel  , m_ops.m_printLevel);
	ar.WriteChunk(DataField::Objective   , m_ops.m_objective);

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
		case DataField::LogFileName: { ar.read(s); m_logFileName = QString::fromStdString(s); break; }
		case DataField::OptMethod: ar.read(m_ops.m_method); break;
		case DataField::ObjTol: ar.read(m_ops.m_obj_tol); break;
		case DataField::FDiffScale: ar.read(m_ops.m_f_diff_scale); break;
		case DataField::OutputLevel: ar.read(m_ops.m_outLevel); break;
		case DataField::PrintLevel: ar.read(m_ops.m_printLevel); break;
		case DataField::Objective: ar.read(m_ops.m_objective); break;
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
