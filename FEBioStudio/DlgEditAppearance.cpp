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
#include "DlgEditAppearance.h"
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QFormLayout>
#include "CColorButton.h"
#include <GLWLib/convert.h>
#include "DragBox.h"
#include "MainWindow.h"
#include <QComboBox>
#include <QPainter>
#include <RTLib/RayTracer.h>
#include <GLLib/GLContext.h>

class EditMaterialScene : public GLScene
{
public:
	EditMaterialScene();
	void Render(GLRenderEngine& re, GLContext& rc);

	BOX GetBoundingBox() override { return BOX(-1, -1, -1, 1, 1, 1); }

public:
	GLMaterial mat;
	bool useEnvMap = false;
};

struct PresetMaterial
{
	GLColor ambient;
	GLColor diffuse;
	GLColor specular;
	float shininess;
	float reflection;
	const char* szname;
};

PresetMaterial MatList[] = {
	{GLColor( 90,  90,  90), GLColor(180, 180, 190), GLColor(255, 255, 255), 0.80f, 0.55f, "aluminium"},
	{GLColor( 30,   0,   0), GLColor(150,  15,  20), GLColor(180,  60,  60), 0.60f, 0.10f, "blood"},
	{GLColor( 80,  75,  65), GLColor(220, 210, 190), GLColor( 60,  60,  55), 0.10f, 0.00f, "bone"},
	{GLColor( 30,  15,  10), GLColor(120,  60,  40), GLColor(160,  80,  50), 0.35f, 0.00f, "bone marrow"},
	{GLColor( 60,  50,  55), GLColor(200, 170, 175), GLColor(120, 110, 115), 0.20f, 0.00f, "brain tissue"},
	{GLColor( 84,  56,   8), GLColor(199, 145,  28), GLColor(253, 232, 207), 0.22f, 0.40f, "brass"},
	{GLColor( 54,  33,  13), GLColor(181, 110,  46), GLColor( 99,  69,  43), 0.20f, 0.40f, "bronze"},
	{GLColor( 50,  60,  70), GLColor(180, 200, 210), GLColor(100, 110, 120), 0.15f, 0.05f, "cartilage"},
	{GLColor( 64,  64,  64), GLColor(102, 102, 102), GLColor(197, 197, 197), 0.60f, 0.70f, "chrome"},
	{GLColor( 48,  18,   5), GLColor(179,  69,  20), GLColor( 66,  36,  23), 0.10f, 0.50f, "copper"},
	{GLColor(  5,  43,   5), GLColor( 20, 156,  20), GLColor(161, 186, 161), 0.60f, 0.30f, "emerald (gem)"},
	{GLColor( 30,  40,  20), GLColor(120, 140,  60), GLColor(100, 120, 80) , 0.35f, 0.00f, "gallbladder"},
	{GLColor( 64,  51,  18), GLColor(191, 155,  59), GLColor(161, 143,  94), 0.40f, 0.50f, "gold"},
	{GLColor( 40,  10,  10), GLColor(170,  40,  40), GLColor(200,  80,  80), 0.35f, 0.00f, "heart"},
	{GLColor( 60,  45,  40), GLColor(210, 170, 150), GLColor(170, 150, 140), 0.30f, 0.00f, "intestines"},
	{GLColor( 50,  20,  20), GLColor(160,  70,  70), GLColor(190, 100,  90), 0.25f, 0.00f, "kidneys"},
	{GLColor( 45,  20,  15), GLColor(150,  60,  45), GLColor(180,  80,  70), 0.30f, 0.00f, "liver"},
	{GLColor( 70,  60,  60), GLColor(200, 160, 160), GLColor(150, 130, 130), 0.25f, 0.00f, "lungs"},
	{GLColor( 40,  10,  10), GLColor(180,  45,  45), GLColor(100,  50,  50), 0.25f, 0.00f, "muscle "},
	{GLColor(  0,   0,   0), GLColor(128,   0,   0), GLColor(179, 153, 153), 0.25f, 0.10f, "plastic (red)"},
	{GLColor(  0,   0,   0), GLColor(  0, 128,   0), GLColor(153, 179, 153), 0.25f, 0.10f, "plastic (green)"},
	{GLColor(  0,   0,   0), GLColor(  0,   0, 128), GLColor(153, 153, 179), 0.25f, 0.10f, "plastic (blue)"},
	{GLColor(  5,   5,   5), GLColor(  3,   3,   3), GLColor(102, 102, 102), 0.08f, 0.00f, "rubber  (black)"},
	{GLColor( 13,   0,   0), GLColor(128, 102, 102), GLColor(179,  10,  10), 0.08f, 0.00f, "rubber  (red)"},
	{GLColor( 70,  55,  45), GLColor(210, 180, 160), GLColor(150, 130, 120), 0.25f, 0.00f, "pancreas"},
	{GLColor( 48,  48,  48), GLColor(130, 130, 130), GLColor(130, 130, 130), 0.40f, 0.50f, "silver"},
	{GLColor( 40,  10,  10), GLColor(120,  20,  30), GLColor(150,  60,  70), 0.30f, 0.00f, "spleen"},
	{GLColor( 60,  45,  45), GLColor(210, 160, 160), GLColor(160, 140, 140), 0.25f, 0.00f, "stomach"},
	{GLColor( 60,  45,  35), GLColor(210, 160, 120), GLColor(150, 120, 100), 0.35f, 0.00f, "tendon "},
	{GLColor( 50,  50,  55), GLColor(160, 160, 170), GLColor(190, 200, 220), 0.45f, 0.50f, "titanium " },
	{GLColor(180, 180, 170), GLColor(240, 240, 230), GLColor(255, 255, 250), 0.65f, 0.70f, "tooth enamel"},
	{GLColor( 70,  50,  50), GLColor(220, 170, 170), GLColor(150, 120, 120), 0.30f, 0.00f, "urinary bladder"},
};

EditMaterialScene::EditMaterialScene()
{
	GetCamera().SetTargetDistance(3);
	GetCamera().Update(true);
}

void EditMaterialScene::Render(GLRenderEngine& re, GLContext& rc)
{
	if (useEnvMap) ActivateEnvironmentMap(re);
	re.setLightPosition(0, vec3f(1, 1, 1));
	PositionCameraInScene(re);

	re.setMaterial(mat);

	RayTracer& rt = dynamic_cast<RayTracer&>(re);
	rt.addSphere(vec3d(0,0,0), 1.0);

	GLScene::Render(re, rc);
	DeactivateEnvironmentMap(re);
}

class CSceneWidget : public QFrame
{
public:
	CSceneWidget(QWidget* parent = nullptr) : QFrame(parent) {}

	void resizeEvent(QResizeEvent* event) override
	{
		QRect rect = contentsRect();
		int W = rect.width();
		int H = rect.height();

		img = QImage(W, H, QImage::Format_ARGB32);
		img.fill(Qt::red);

		requestUpdate();
		QFrame::resizeEvent(event);
	}

	void paintEvent(QPaintEvent* ev)
	{
		QFrame::paintEvent(ev);

		QRect rt = contentsRect();
		QPainter p(this);
		p.drawImage(rt.x(), rt.y(), img);
		p.end();
	}

	void requestUpdate()
	{
		updateImage();
		update();
	}

	void updateImage()
	{
		int W = img.width();
		int H = img.height();

		rt.setWidth(W);
		rt.setHeight(H);
		rt.useMultiThread(false);

#ifndef NDEBUG
		rt.setSampleCount(1);
#else
		rt.setSampleCount(2);
#endif

#ifdef NDEBUG
		rt.setOutput(false);
#endif

		rt.setRenderShadows(false);

		GLContext rc;
		rc.m_cam = &scene.GetCamera();

		rt.start();
		scene.Render(rt, rc);
		rt.finish();

		RayTraceSurface& trg = rt.surface();

		for (size_t j = 0; j < H; ++j)
			for (size_t i = 0; i < W; ++i)
			{
				GLColor c = trg.colorValue(i, j);
				QRgb rgb = qRgba(c.r, c.g, c.b, c.a);
				img.setPixel((int)i, (int)j, rgb);
			}
	}

public:
	EditMaterialScene scene;

private:
	QImage img;
	RayTracer rt;
	CRGBAImage envMap;
};

class UIDlgEditAppearance
{
public:
	CColorButton* ambient;
	CColorButton* diffuse;
	CColorButton* specular;
	CDragBox* specExp;
	CDragBox* reflect;
	CSceneWidget*view;
	QComboBox* preset;


public:
	void setup(CDlgEditAppearance* dlg)
	{
		QHBoxLayout* h = new QHBoxLayout;

		view = new CSceneWidget();
		QWidget* w = view;
		w->setMinimumSize(QSize(300, 300));
		h->addWidget(w);


		QVBoxLayout* l = new QVBoxLayout;

		QFormLayout* form = new QFormLayout;

		form->addRow("Presets:", preset = new QComboBox);
		form->addRow("Ambient", ambient = new CColorButton);
		form->addRow("Diffuse", diffuse = new CColorButton);
		form->addRow("Specular", specular = new CColorButton);
		form->addRow("Shininess", specExp = new CDragBox);
		form->addRow("Reflection", reflect = new CDragBox);
		specExp->setRange(0, 1); specExp->setSingleStep(0.01);
		reflect->setRange(0, 1); reflect->setSingleStep(0.01);

		preset->addItem("(user)");
		int n = sizeof(MatList) / sizeof(PresetMaterial);
		for (int i = 0; i < n; ++i)
			preset->addItem(MatList[i].szname);

		l->addLayout(form);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		h->addLayout(l);
		dlg->setLayout(h);

		// Assign "What's This?" help to widgets
		ambient->setWhatsThis("The ambient color is a constant color that is added to the material's total color.");
		diffuse->setWhatsThis("The diffuse color is the main color of the material.");
		specular->setWhatsThis("The specular color is the color of the highlights. (Also see shininess)");
		specExp->setWhatsThis("The shininess specifies the size of the specular highlight.");
		reflect->setWhatsThis("Sets how reflective the material is. (Needs environment map enabled to have any effect.)");
		preset->setWhatsThis("Choose a preset material.");

		QObject::connect(bb, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
		QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

		QObject::connect(preset, &QComboBox::currentIndexChanged, dlg, &CDlgEditAppearance::selectPreset);
		QObject::connect(ambient, &CColorButton::colorChanged, dlg, &CDlgEditAppearance::updateAppearance);
		QObject::connect(diffuse, &CColorButton::colorChanged, dlg, &CDlgEditAppearance::updateAppearance);
		QObject::connect(specular, &CColorButton::colorChanged, dlg, &CDlgEditAppearance::updateAppearance);
		QObject::connect(specExp, &CDragBox::valueChanged, dlg, &CDlgEditAppearance::updateAppearance);
		QObject::connect(reflect, &CDragBox::valueChanged, dlg, &CDlgEditAppearance::updateAppearance);
	}
};

CDlgEditAppearance::CDlgEditAppearance(QWidget* parent) : QDialog(parent), ui(new UIDlgEditAppearance)
{
	setWindowTitle("Edit Appearance");
	setWindowFlags(windowFlags() | Qt::WindowContextHelpButtonHint); // Enable "What's This?" button

	ui->setup(this);

	CMainWindow* wnd = CMainWindow::GetInstance();
	if (wnd)
	{
		bool useEnvMap = wnd->IsEnvironmentMapEnabled();
		if (useEnvMap)
		{
			ui->view->scene.SetEnvironmentMap(wnd->GetEnvironmentMapImage());
			ui->view->scene.useEnvMap = true;
		}
		else
		{
			ui->reflect->setValue(0.0);
			ui->reflect->setDisabled(true);
		}
	}
}

CDlgEditAppearance::~CDlgEditAppearance()
{

}

void CDlgEditAppearance::SetMaterial(const GLMaterial& mat)
{
	ui->view->scene.mat = mat;
	ui->ambient->blockSignals(true);
	ui->ambient->setColor(toQColor(mat.ambient));
	ui->ambient->blockSignals(false);

	ui->diffuse->blockSignals(true);
	ui->diffuse->setColor(toQColor(mat.diffuse));
	ui->diffuse->blockSignals(false);

	ui->specular->blockSignals(true);
	ui->specular->setColor(toQColor(mat.specular));
	ui->specular->blockSignals(false);

	ui->specExp->blockSignals(true);
	ui->specExp->setValue(mat.shininess);
	ui->specExp->blockSignals(false);

	if (ui->reflect->isEnabled())
	{
		ui->reflect->blockSignals(true);
		ui->reflect->setValue(mat.reflection);
		ui->reflect->blockSignals(false);
	}
}

GLMaterial CDlgEditAppearance::GetMaterial() const
{
	return ui->view->scene.mat;
}

void CDlgEditAppearance::accept()
{
	updateAppearance();
	QDialog::accept();
}

void CDlgEditAppearance::updateAppearance()
{
	ui->preset->setCurrentIndex(0);

	GLMaterial& mat = ui->view->scene.mat;
	mat.ambient = toGLColor(ui->ambient->color());
	mat.diffuse = toGLColor(ui->diffuse->color());
	mat.specular = toGLColor(ui->specular->color());
	mat.shininess = ui->specExp->value();
	mat.reflection = ui->reflect->value();
	ui->view->requestUpdate();
}

void CDlgEditAppearance::selectPreset()
{
	int n = ui->preset->currentIndex() - 1;
	if (n >= 0)
	{
		PresetMaterial preset = MatList[n];
		GLMaterial& mat = ui->view->scene.mat;

		mat.ambient   = preset.ambient;
		mat.diffuse   = preset.diffuse;
		mat.specular  = preset.specular;
		mat.shininess = preset.shininess;

		if (ui->reflect->isEnabled())
			mat.reflection = preset.reflection;
		else
			mat.reflection = 0;

		SetMaterial(mat);
		ui->view->requestUpdate();
	}
}
