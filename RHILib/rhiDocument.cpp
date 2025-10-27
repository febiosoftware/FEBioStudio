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
#include "rhiDocument.h"
#include <MeshIO/STLimport.h>
#include <MeshIO/PLYImport.h>
#include <MeshIO/VTUImport.h>
#include <FEMLib/FSProject.h>
#include <FEMLib/FSModel.h>
#include <GeomLib/GModel.h>
#include <GeomLib/GObject.h>
#include <QFileInfo>
#include "../FEBioStudio/PropertyList.h"
#include "../FEBioStudio/MainWindow.h"

rhiDocument::rhiDocument(CMainWindow* wnd) : CGLSceneDocument(wnd)
{
	m_scene = new rhiScene();
}

bool rhiDocument::ImportFile(const QString& fileName)
{
	QFileInfo fi(fileName);

	QString ext = fi.suffix();

	// read STL from file.
	std::string sfile = fileName.toStdString();
	FSProject dummy;

	if (ext == "stl")
	{
		STLimport stl(dummy);
		if (stl.Load(sfile.c_str()) == false) return false;
	}
	else if (ext == "ply")
	{
		PLYImport ply(dummy);
		if (ply.Load(sfile.c_str()) == false) return false;
	}
	else if (ext == "vtp")
	{
		VTPimport vtp(dummy);
		if (vtp.Load(sfile.c_str()) == false) return false;
	}
	else return false;

	GModel& gm = dummy.GetFSModel().GetModel();
	if (gm.Objects() > 0)
	{
		GObject * po = gm.Object(0);
		GLMesh* pm = po->GetRenderMesh();

		SetDocTitle(fi.baseName().toStdString());

		// add a copy since scene will take ownership
		GLMesh* copy = new GLMesh(*pm);

		GetRhiScene()->AddMesh(copy);

		BOX box = pm->GetBoundingBox();
		GetMainWindow()->AddLogEntry("Success reading file '" + fi.fileName() + "'.\n");
		GetMainWindow()->AddLogEntry(QString("Mesh info: %1 nodes, %2 elements\nbounding box: [%3,%4] x [%5,%6] x [%7,%8]\n")
			.arg(pm->Nodes()).arg(pm->Faces())
			.arg(box.x0).arg(box.x1)
			.arg(box.y0).arg(box.y1)
			.arg(box.z0).arg(box.z1));

		return true;
	}

	return false;
}

class CRhiDocProps : public CPropertyList
{
public:
	CRhiDocProps(rhiDocument* doc) : m_doc(doc)
	{
		addProperty("object color", CProperty::Color);
		addProperty("object shininess", CProperty::Float)->setFloatRange(0, 1);
		addProperty("object reflectivity", CProperty::Float)->setFloatRange(0, 1);
		addProperty("object opacity", CProperty::Float)->setFloatRange(0, 1);
		addProperty("background color", CProperty::Color);
		addProperty("light position", CProperty::Vec3);
		addProperty("specular color", CProperty::Color);
		addProperty("use texture", CProperty::Enum)->setEnumValues(QStringList() << "(none)" << "jet" << "parula" << "gray");
		addProperty("render mesh lines", CProperty::Bool);
		addProperty("mesh color", CProperty::Color);
		addProperty("render mesh nodes", CProperty::Bool);
		addProperty("nodes color", CProperty::Color);
		addProperty("use stipple", CProperty::Bool);
		addProperty("do clipping", CProperty::Bool);
		addProperty("clip X", CProperty::Float)->setFloatRange(-1, 1);
		addProperty("clip Y", CProperty::Float)->setFloatRange(-1, 1);
		addProperty("clip Z", CProperty::Float)->setFloatRange(-1, 1);
		addProperty("clip W", CProperty::Float);
		addProperty("render overlay", CProperty::Bool);
		addProperty("show grid", CProperty::Bool);
	}

	QVariant GetPropertyValue(int i)
	{
		rhiScene* s = m_doc->GetRhiScene();
		switch (i)
		{
		case 0: return color; break;
		case 1: return shininess; break;
		case 2: return reflectivity; break;
		case 3: return opacity; break;
		case 4: return toQColor(s->bgcol); break;
		case 5: return Vec3fToString(s->light); break;
		case 6: return toQColor(s->specColor); break;
		case 7: return s->texture; break;
		case 8: return s->renderMesh; break;
		case 9: return toQColor(s->meshColor); break;
		case 10: return s->renderNodes; break;
		case 11: return toQColor(s->nodeColor); break;
		case 12: return s->useStipple; break;
		case 13: return s->doClipping; break;
		case 14: return s->clipPlane[0]; break;
		case 15: return s->clipPlane[1]; break;
		case 16: return s->clipPlane[2]; break;
		case 17: return s->clipPlane[3]; break;
		case 18: return m_doc->m_renderOverlay; break;
		case 19: return s->showGrid; break;
		}

		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		rhiScene* s = m_doc->GetRhiScene();
		switch (i)
		{
		case 0: s->SetObjectColor(toGLColor(color = v.value<QColor>())); break;
		case 1: s->SetObjectShininess(shininess = v.toFloat()); break;
		case 2: s->SetObjectReflectivity(reflectivity = v.toFloat()); break;
		case 3: s->SetObjectOpacity(opacity = v.toFloat()); break;
		case 4: s->bgcol = toGLColor(v.value<QColor>()); break;
		case 5: s->light = StringToVec3f(v.toString()); break;
		case 6: s->specColor = toGLColor(v.value<QColor>()); break;
		case 7: s->texture = v.toInt(); break;
		case 8: s->renderMesh = v.toBool(); break;
		case 9: s->meshColor = toGLColor(v.value<QColor>()); break;
		case 10: s->renderNodes = v.toBool(); break;
		case 11: s->nodeColor = toGLColor(v.value<QColor>()); break;
		case 12: s->useStipple = v.toBool(); break;
		case 13: s->doClipping = v.toBool(); break;
		case 14: s->clipPlane[0] = v.toFloat(); break;
		case 15: s->clipPlane[1] = v.toFloat(); break;
		case 16: s->clipPlane[2] = v.toFloat(); break;
		case 17: s->clipPlane[3] = v.toFloat(); break;
		case 18: m_doc->m_renderOverlay = v.toBool(); break;
		case 19: s->showGrid = v.toBool(); break;
		}
	}

private:
	QColor color = QColor::fromRgb(200, 180, 160);
	float shininess = 0.8f;
	float reflectivity = 0.8f;
	float opacity = 1.0f;
	rhiDocument* m_doc;
};

CPropertyList* rhiDocument::GetDocProperties()
{
	return new CRhiDocProps(this);
}
