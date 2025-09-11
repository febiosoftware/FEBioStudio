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
#include "DlgDistributionVisualizer.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QLabel>
#include "MainWindow.h"
#include <FEBioLink/FEBioClass.h>
#include <FEBioLink/FEBioModule.h>
#include <FECore/fecore_enum.h>
#include <FECore/FEModel.h>
#include <FECore/FECoreKernel.h>
#include <FEBioMech/FEFiberDensityDistribution.h>
#include "PropertyList.h"
#include "PropertyListView.h"
#include <CUILib/GLSceneView.h>
#include <GLLib/GLScene.h>
#include <GLLib/GLMesh.h>
#include <GLLib/GLCamera.h>
#include <GLLib/GLContext.h>
#include <GLLib/glx.h>
#include <GLWLib/GLTriad.h>

GLMesh CreateSphere()
{
	GLMesh m;

	const int ntheta = 200;
	const int nphi = 50;
	const double dtheta = 2 * PI / ntheta;
	const double dphi = PI / nphi;

	// add top node
	m.AddNode(vec3f(0.f, 0.f, 1.f));

	// add middle nodes
	for (int j = 1; j < nphi; ++j)
	{
		double phi = j * dphi;
		for (int i = 0; i < ntheta; ++i)
		{
			double theta = i * dtheta;
			double x = sin(phi) * cos(theta);
			double y = sin(phi) * sin(theta);
			double z = cos(phi);
			m.AddNode(vec3f(x, y, z));
		}
	}

	// add bottom node
	m.AddNode(vec3f(0.f, 0.f, -1.f));

	// add top surface
	for (int i = 0; i < ntheta; ++i)
	{
		int n0 = 0;
		int n1 = i + 1;
		int n2 = (i + 1)%ntheta + 1;
		m.AddFace(n0, n1, n2);
	}

	// add middle surface
	for (int j = 0; j < nphi - 2; ++j)
	{
		for (int i = 0; i < ntheta; ++i)
		{
			int n0 = j*ntheta + i + 1;
			int n1 = j*ntheta + (i + 1)%ntheta + 1;
			int n2 = n0 + ntheta;
			int n3 = n1 + ntheta;
			m.AddFace(n0, n2, n1);
			m.AddFace(n1, n2, n3);
		}
	}

	// add bottom surface
	int N = m.Nodes();
	for (int i = 0; i < ntheta; ++i)
	{
		int n0 = N-1;
		int n2 = n0 - ntheta + i;
		int n1 = n0 - ntheta + (i + 1) % ntheta;
		m.AddFace(n0, n1, n2);
	}

	m.Update();

	return m;
}

class CGLDistroScene : public GLScene
{
public:
	CGLDistroScene() 
	{
		m_sphere = CreateSphere();
		m_renderSphere = m_sphere;

		m_ptriad = new GLTriad(10, 10, 75, 75);
		m_ptriad->align(GLW_ALIGN_LEFT | GLW_ALIGN_TOP);
	}
	~CGLDistroScene() {}
	
	// Render the 3D scene
	void Render(GLRenderEngine& engine, GLContext& rc) override 
	{
		if (m_recache) {
			engine.deleteCachedMesh(&m_renderSphere); m_recache = false;
		}

		engine.setMaterial(GLMaterial::PLASTIC, GLColor::FromRGBf(0.7f, 0.5f, 0.2f));
		engine.renderGMesh(m_renderSphere);

		BOX box = m_renderSphere.GetBoundingBox();
		if (box.Radius() < 1e-9) box = GetBoundingBox();

		glx::renderBox(engine, box, GLColor::White(), false);
	}

	// Render on the 2D canvas
	void RenderCanvas(QPainter& painter, GLContext& rc) override 
	{
		m_ptriad->setOrientation(rc.m_cam->GetOrientation());
		m_ptriad->draw(&painter);
	}

	// get the bounding box of the entire scene
	BOX GetBoundingBox() override { 
		BOX b = m_renderSphere.GetBoundingBox();
		if (b.IsValid() == false) b = BOX(-1, -1, -1, 1, 1, 1);
		return b;
	}

	// get the bounding box of the current selection
	BOX GetSelectionBox() override { return BOX(-1, -1, -1, 1, 1, 1); }

	GLMesh& GetSphere() { return m_sphere; }
	GLMesh& GetRenderSphere() { return m_renderSphere; }

	void Update() override
	{
		m_recache = true;
	}

private:
	GLMesh m_sphere;
	GLMesh m_renderSphere;
	GLTriad* m_ptriad;

	bool m_recache = true;
};

class CDistroGLWidget : public CGLManagedSceneView
{
public:
	CDistroGLWidget(QWidget* parent) : CGLManagedSceneView(new CGLDistroScene(), parent)
	{
		setMinimumSize(400, 400);
	}

	CGLDistroScene* GetScene()
	{
		return dynamic_cast<CGLDistroScene*>(GetActiveScene());
	}
};

class FEBioPropsList : public CPropertyList
{
public:
	FEBioPropsList(FECoreBase* pc) : m_pc(pc)
	{
		FEParameterList& PL = pc->GetParameterList();
		FEParamIterator it = PL.first();
		int NP = PL.Parameters();
		m_param.resize(NP);
		for (int i = 0; i < NP; ++i, ++it)
		{
			FEParam& p = *it;
			m_param[i] = &p;
			switch (p.type())
			{
			case FE_PARAM_INT   : addProperty(p.name(), CProperty::Int  ); break;
			case FE_PARAM_DOUBLE: 
			case FE_PARAM_DOUBLE_MAPPED: 
				addProperty(p.name(), CProperty::Float); break;
			case FE_PARAM_BOOL  : addProperty(p.name(), CProperty::Bool ); break;
			case FE_PARAM_VEC3D: addProperty(p.name(), CProperty::Vec3); break;
			case FE_PARAM_VEC3D_MAPPED: addProperty(p.name(), CProperty::Vec3); break;
			case FE_PARAM_MAT3DS: addProperty(p.name(), CProperty::Mat3s); break;
			case FE_PARAM_MAT3DS_MAPPED: addProperty(p.name(), CProperty::Mat3s); break;
			default:
				addProperty(p.name(), CProperty::String); break;
			}
		}
	}

	QVariant GetPropertyValue(int i) override
	{
		FEParam* p = m_param[i];
		if (p && (p->type() == FE_PARAM_DOUBLE))
		{
			double val = p->value<double>();
			return val;
		}
		else if (p && (p->type() == FE_PARAM_DOUBLE_MAPPED))
		{
			double val = p->value<FEParamDouble>().constValue();
			return val;
		}
		else if (p && (p->type() == FE_PARAM_INT))
		{
			int val = p->value<int>();
			return val;
		}
		else if (p && (p->type() == FE_PARAM_BOOL))
		{
			bool val = p->value<bool>();
			return val;
		}
		else if (p && p->type() == FE_PARAM_VEC3D)
		{
			vec3d val = p->value<vec3d>();
			return Vec3dToString(val);
		}
		else if (p && p->type() == FE_PARAM_VEC3D_MAPPED)
		{
			vec3d val = p->value<FEParamVec3>().constValue();
			return Vec3dToString(val);
		}
		else if (p && p->type() == FE_PARAM_MAT3DS)
		{
			mat3ds val = p->value<mat3ds>();
			return Mat3dsToString(val);
		}
		else if (p && p->type() == FE_PARAM_MAT3DS_MAPPED)
		{
			mat3ds val = p->value<FEParamMat3ds>().constValue();
			return Mat3dsToString(val);
		}

		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		FEParam* p = m_param[i];
		if (p && (p->type() == FE_PARAM_DOUBLE))
		{
			double val = v.toDouble();
			p->value<double>() = val;
		}
		else if (p && (p->type() == FE_PARAM_DOUBLE_MAPPED))
		{
			double val = v.toDouble();
			p->value<FEParamDouble>().constValue() = val;
		}
		else if (p && (p->type() == FE_PARAM_INT))
		{
			int val = v.toInt();
			p->value<int>() = val;
		}
		else if (p && (p->type() == FE_PARAM_BOOL))
		{
			bool val = v.toBool();
			p->value<bool>() = val;
		}
		else if (p && (p->type() == FE_PARAM_VEC3D))
		{
			vec3d val = StringToVec3d(v.toString());
			p->value<vec3d>() = val;
		}
		else if (p && (p->type() == FE_PARAM_VEC3D_MAPPED))
		{
			vec3d val = StringToVec3d(v.toString());
			p->value<FEParamVec3>().constValue() = val;
		}
		else if (p && (p->type() == FE_PARAM_MAT3DS))
		{
			mat3ds val = StringToMat3ds(v.toString());
			p->value<mat3ds>() = val;
		}
		else if (p && (p->type() == FE_PARAM_MAT3DS_MAPPED))
		{
			mat3ds val = StringToMat3ds(v.toString());
			p->value<FEParamMat3ds>().constValue() = val;
		}
	}

private:
	FECoreBase* m_pc;
	std::vector<FEParam*> m_param;
};

class UIDlgDistributionVisualizer
{
public:
	FEModel fem;

	QComboBox* distro = nullptr;
	CPropertyList* props = nullptr;
	CPropertyListView* propsView = nullptr;
	CDistroGLWidget* glw = nullptr;

	vector<FEFiberDensityDistribution*> fdd;
	
public:
	UIDlgDistributionVisualizer() {}

	~UIDlgDistributionVisualizer()
	{
		for (auto p : fdd) delete p;
		fdd.clear();
	}

	void setup(CDlgDistributionVisualizer* dlg)
	{
		distro = new QComboBox;
		int mod = FEBio::GetModuleId("solid");
		int baseClassId = FEBio::GetBaseClassIndex("FEFiberDensityDistribution");
		std::vector<FEBio::FEBioClassInfo> classList = FEBio::FindAllClasses(mod, FEMATERIALPROP_ID, baseClassId, 0);
		for (auto& it : classList) distro->addItem(it.sztype);

		fdd.assign(classList.size(), nullptr);

		propsView = new CPropertyListView;

		QVBoxLayout* vl = new QVBoxLayout;
		vl->addWidget(distro);
		vl->addWidget(propsView);

		glw = new CDistroGLWidget(nullptr);

		QHBoxLayout* hl = new QHBoxLayout;
		hl->addLayout(vl);
		hl->addWidget(glw);

		QVBoxLayout* l = new QVBoxLayout;
		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);
		l->addLayout(hl);
		l->addWidget(bb);
		QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
		QObject::connect(distro, &QComboBox::currentIndexChanged, dlg, &CDlgDistributionVisualizer::onDistroChanged);
		QObject::connect(propsView, &CPropertyListView::dataChanged, dlg, &CDlgDistributionVisualizer::onUpdateVisual);

		dlg->setLayout(l);
	}
};

CDlgDistributionVisualizer::CDlgDistributionVisualizer(CMainWindow* parent) : QDialog(parent), ui(new UIDlgDistributionVisualizer)
{
	ui->setup(this);
	setWindowTitle("Distribution Visualizer");
}

CDlgDistributionVisualizer::~CDlgDistributionVisualizer()
{
	delete ui;
}

void CDlgDistributionVisualizer::onDistroChanged(int n)
{
	string classType = ui->distro->currentText().toStdString();
	const char* sztype = classType.c_str();

	if ((n < 0) || (n >= ui->fdd.size())) return;

	FEFiberDensityDistribution* fdd = ui->fdd[n];
	if (fdd == nullptr)
	{
		fdd = fecore_new_ex<FEFiberDensityDistribution>(sztype, "solid", &ui->fem);
		ui->fdd[n] = fdd;
	}

	delete ui->props;
	ui->props = new FEBioPropsList(fdd);

	ui->propsView->Update(ui->props);

	onUpdateVisual();
}

void CDlgDistributionVisualizer::onUpdateVisual()
{
	CGLDistroScene* scene = ui->glw->GetScene();
	GLMesh& m0 = scene->GetSphere();
	GLMesh& m1 = scene->GetRenderSphere();

	int n = ui->distro->currentIndex();
	if ((n < 0) || (n >= ui->fdd.size())) return;

	FEFiberDensityDistribution* fdd = ui->fdd[n];
	if (fdd)
	{
		FEMaterialPoint mp;
		for (int i = 0; i < m0.Nodes(); ++i)
		{
			vec3d ri = to_vec3d(m0.Node(i).r);

			double R = fdd->FiberDensity(mp, ri);
			m1.Node(i).r = to_vec3f(ri * R);
		}
		m1.UpdateBoundingBox();
		m1.UpdateNormals();
		scene->Update();
		scene->ZoomExtents(false);
		ui->glw->update();
	}
}
