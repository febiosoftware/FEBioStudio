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
#include "FEBioDiagnostic.h"
#include "ModelDocument.h"
#include <FEBio/FEBioExport3.h>
#include <FEBioXML/XMLWriter.h>

FEBioTangentDiagnostic::FEBioTangentDiagnostic()
{
	m_matIndex = -1;
	m_scenario = 0;
	m_strain = 0.0;
}

bool FEBioTangentDiagnostic::WriteDiagnosticFile(CModelDocument* doc, const std::string& fileName)
{
	FEBioExport3 febioExport(doc->GetProject());

	XMLWriter& xml = febioExport.GetXMLWriter();

	if (xml.open(fileName.c_str()) == false) return false;

	XMLElement root("febio_diagnostic");
	root.add_attribute("type", "tangent test");
	xml.add_branch(root);

	// print options
	XMLElement ops("Control");
	xml.add_branch(ops);
	{
		xml.add_leaf("time_steps", 10);
		xml.add_leaf("step_size", 0.1);
	}
	xml.close_branch();

	// Material
	XMLElement mat("Material");
	xml.add_branch(mat);
	if (doc)
	{
		FSModel* fem = doc->GetFSModel();
		if ((m_matIndex >= 0) && (m_matIndex < fem->Materials()))
		{
			GMaterial* gmat = fem->GetMaterial(m_matIndex);
			FSMaterial* pmat = gmat->GetMaterialProperties();

			XMLElement m("material");
			m.add_attribute("id", 1);
			m.add_attribute("name", gmat->GetName());
			febioExport.WriteMaterial(pmat, m);
		}
	}
	xml.close_branch();

	// Scenario
	XMLElement scn("Scenario");
	if (m_scenario == 0) scn.add_attribute("type", "uni-axial");
	if (m_scenario == 1) scn.add_attribute("type", "simple shear");
	xml.add_branch(scn);
	{
		xml.add_leaf("strain", m_strain);
	}
	xml.close_branch();

	// closing tag of root branch
	xml.close_branch();

	xml.close();

	return true;
}
