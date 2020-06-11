/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "DocTemplate.h"
#include <QtCore/QDir>
#include <XML/XMLReader.h>
#include <MeshTools/FEProject.h>

vector<DocTemplate> TemplateManager::m_doc;
string TemplateManager::m_path = "$(PREVIEW_PATH)\\templates\\";

DocTemplate::DocTemplate()
{
}

DocTemplate::DocTemplate(const DocTemplate& doc)
{
	title = doc.title;
	description = doc.description;
	fileName = doc.fileName;
	module = doc.module;
}

void DocTemplate::operator = (const DocTemplate& doc)
{
	title = doc.title;
	description = doc.description;
	fileName = doc.fileName;
	module = doc.module;
}

string TemplateManager::TemplatePath() { return m_path; }

int TemplateManager::Templates() { return (int) m_doc.size(); }

const DocTemplate& TemplateManager::GetTemplate(int i) { return m_doc[i]; }

void TemplateManager::Init()
{
	// hard code all templates for now
	DocTemplate doc;
	doc.title = "Structural Mechanics";
	doc.description = "Quasi-static or dynamical structural mechanics analysis.";
	doc.module = MODULE_MECH;
	AddTemplate(doc);

	doc.title = "Biphasic Analysis";
	doc.description = "Transient or quasi-static biphasic analysis";
	doc.module = MODULE_MECH | MODULE_BIPHASIC;
	AddTemplate(doc);

	doc.title = "Multiphasic Analysis";
	doc.description = "Transient or quasi-static analysis with solutes";
	doc.module = MODULE_MECH | MODULE_BIPHASIC | MODULE_MULTIPHASIC | MODULE_SOLUTES | MODULE_REACTIONS;
	AddTemplate(doc);

	doc.title = "Heat Transfer Analysis";
	doc.description = "Transient or steady-state heat conduction analysis.";
	doc.module = MODULE_HEAT;
	AddTemplate(doc);

	doc.title = "Fluid Mechanics";
	doc.description = "Fluid dynamics analysis";
	doc.module = MODULE_FLUID;
	AddTemplate(doc);

	doc.title = "Fluid-Structure Interaction";
	doc.description = "FSI analysis where a fluid interacts with a mechanical domain";
	doc.module = MODULE_MECH | MODULE_FLUID | MODULE_FLUID_FSI;
	AddTemplate(doc);

	doc.title = "Reaction-Diffusion Analysis";
	doc.description = "Transient reaction-diffusion analysis";
	doc.module = MODULE_REACTIONS | MODULE_SOLUTES | MODULE_REACTION_DIFFUSION;
	AddTemplate(doc);

	doc.title = "All physics";
	doc.description = "Enable all physics modules.";
	doc.module = MODULE_ALL;
	AddTemplate(doc);

/*	// load all doc templates
	QStringList filters;
	filters << "*.prvtmp";
	QDir dir(m_path.c_str());
	QStringList files = dir.entryList(filters);
	for (int i=0; i<files.count(); ++i)
	{
		string fileName = m_path + files.at(i).toStdString();
		const char* szfile = fileName.c_str();
		LoadTemplate(szfile);
	}
*/
}

void TemplateManager::AddTemplate(DocTemplate& tmp)
{
	m_doc.push_back(tmp);
}

bool TemplateManager::LoadTemplate(const char* sztmp)
{
	FILE* fp = fopen(sztmp, "rt");
	if (fp == 0) return false;

	XMLReader xml;
	if (xml.Attach(fp) == 0) return false;

	XMLTag tag;
	if (xml.FindTag("PreView_Template", tag) == false) {fclose(fp); return false; }

	DocTemplate doc;

	char buf[512] = {0};
	++tag;
	do
	{
		if      (tag == "title"      ) { tag.value(buf); doc.title = buf; }
		else if (tag == "description") { tag.value(buf); doc.description = buf; } 
		else if (tag == "file"       ) { tag.value(buf); doc.fileName = buf; }
		else
		{
			fclose(fp);
			return false;
		}
		++tag;
	}
	while (!tag.isend());

	fclose(fp);

	AddTemplate(doc);

	return true;
}
