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
#include "DlgEditMaterial.h"
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QColorDialog>
#include <QFormLayout>
#include "CColorButton.h"
#include <GLWLib/convert.h>
#include "DragBox.h"
#include <RHILib/rhiSceneView.h>
#include "MainWindow.h"
#include <GLLib/glx.h>
#include <GLLib/GLMeshBuilder.h>
#include <QComboBox>

struct PresetMaterial
{
	GLColor ambient;
	GLColor diffuse;
	GLColor specular;
	float shininess;
	const char* szname;
};

PresetMaterial MatList[] = {
	{GLColor( 30,   0,   0), GLColor(150,  15,  20), GLColor(180,  60,  60), 0.60, "blood"},
	{GLColor( 80,  75,  65), GLColor(220, 210, 190), GLColor( 60,  60,  55), 0.10, "bone"},
	{GLColor( 30,  15,  10), GLColor(120,  60,  40), GLColor(160,  80,  50), 0.35, "bone marrow"},
	{GLColor( 60,  50,  55), GLColor(200, 170, 175), GLColor(120, 110, 115), 0.20, "brain tissue"},
	{GLColor( 84,  56,   8), GLColor(199, 145,  28), GLColor(253, 232, 207), 0.22, "brass"},
	{GLColor( 54,  33,  13), GLColor(181, 110,  46), GLColor( 99,  69,  43), 0.20, "bronze"},
	{GLColor( 50,  60,  70), GLColor(180, 200, 210), GLColor(100, 110, 120), 0.15, "cartilage"},
	{GLColor( 64,  64,  64), GLColor(102, 102, 102), GLColor(197, 197, 197), 0.60, "chrome"},
	{GLColor( 48,  18,   5), GLColor(179,  69,  20), GLColor( 66,  36,  23), 0.10, "copper"},
	{GLColor(  5,  43,   5), GLColor( 20, 156,  20), GLColor(161, 186, 161), 0.60, "emerald (gem)"},
	{GLColor( 30,  40,  20), GLColor(120, 140,  60), GLColor(100, 120, 80) , 0.35, "gallbladder"},
	{GLColor( 64,  51,  18), GLColor(191, 155,  59), GLColor(161, 143,  94), 0.40, "gold"},
	{GLColor( 40,  10,  10), GLColor(170,  40,  40), GLColor(200,  80,  80), 0.35, "heart"},
	{GLColor( 60,  45,  40), GLColor(210, 170, 150), GLColor(170, 150, 140), 0.30, "intestines"},
	{GLColor( 50,  20,  20), GLColor(160,  70,  70), GLColor(190, 100,  90), 0.25, "kidneys"},
	{GLColor( 45,  20,  15), GLColor(150,  60,  45), GLColor(180,  80,  70), 0.30, "liver"},
	{GLColor( 70,  60,  60), GLColor(200, 160, 160), GLColor(150, 130, 130), 0.25, "lungs"},
	{GLColor( 40,  10,  10), GLColor(180,  45,  45), GLColor(100,  50,  50), 0.25, "muscle "},
	{GLColor(  0,   0,   0), GLColor(128,   0,   0), GLColor(179, 153, 153), 0.25, "plastic (red)"},
	{GLColor(  0,   0,   0), GLColor(  0, 128,   0), GLColor(153, 179, 153), 0.25, "plastic (green)"},
	{GLColor(  0,   0,   0), GLColor(  0,   0, 128), GLColor(153, 153, 179), 0.25, "plastic (blue)"},
	{GLColor(  5,   5,   5), GLColor(  3,   3,   3), GLColor(102, 102, 102), 0.08, "rubber  (black)"},
	{GLColor( 13,   0,   0), GLColor(128, 102, 102), GLColor(179,  10,  10), 0.08, "rubber  (red)"},
	{GLColor( 70,  55,  45), GLColor(210, 180, 160), GLColor(150, 130, 120), 0.25, "pancreas"},
	{GLColor( 48,  48,  48), GLColor(130, 130, 130), GLColor(130, 130, 130), 0.40, "silver"},
	{GLColor( 40,  10,  10), GLColor(120,  20,  30), GLColor(150,  60,  70), 0.30, "spleen"},
	{GLColor( 60,  45,  45), GLColor(210, 160, 160), GLColor(160, 140, 140), 0.25, "stomach"},
	{GLColor( 60,  45,  35), GLColor(210, 160, 120), GLColor(150, 120, 100), 0.35, "tendon "},
	{GLColor( 50,  50,  55), GLColor(160, 160, 170), GLColor(190, 200, 220), 0.45, "titanium " },
	{GLColor(180, 180, 170), GLColor(240, 240, 230), GLColor(255, 255, 250), 0.65, "tooth enamel"},
	{GLColor( 70,  50,  50), GLColor(220, 170, 170), GLColor(150, 120, 120), 0.30, "urinary bladder"},
};

EditMaterialScene::EditMaterialScene()
{
	GetCamera().SetTargetDistance(3);
	GetCamera().Update(true);
}

void EditMaterialScene::Render(GLRenderEngine& re, GLContext& rc)
{
	if (!mesh) BuildMesh();

	if (mesh)
	{
		re.setLightPosition(0, vec3f(1, 1, 1));
		re.setProjection(45, 0.01, 5);
		PositionCameraInScene(re);
		re.setMaterial(mat);
		re.renderGMesh(*mesh.get());
	}
}

void EditMaterialScene::BuildMesh()
{
	GLMeshBuilder mb;
	mb.beginShape();
	glx::drawSphere(mb, 1, res, res);
	mb.endShape();
	mesh.reset(mb.takeMesh());
}

class UIDlgEditMaterial
{
public:
	CColorButton* ambient;
	CColorButton* diffuse;
	CColorButton* specular;
	CDragBox* specExp;
	rhiSceneView* view;
	QComboBox* preset;

	EditMaterialScene scene;
	
public:
	void setup(CDlgEditMaterial* dlg)
	{
		QHBoxLayout* h = new QHBoxLayout;

		view = new rhiSceneView(CMainWindow::GetInstance());
		QWidget* w = QWidget::createWindowContainer(view);
		w->setMinimumSize(QSize(300, 300));
		h->addWidget(w);

		view->SetScene(&scene);

		QVBoxLayout* l = new QVBoxLayout;

		QFormLayout* form = new QFormLayout;

		form->addRow("Presets:", preset = new QComboBox);
		form->addRow("Ambient", ambient = new CColorButton);
		form->addRow("Diffuse", diffuse = new CColorButton);
		form->addRow("Specular", specular = new CColorButton);
		form->addRow("Shininess", specExp = new CDragBox);
		specExp->setRange(0, 1); specExp->setSingleStep(0.01);

		preset->addItem("(user)");
		int n = sizeof(MatList) / sizeof(PresetMaterial);
		for (int i = 0; i < n; ++i)
			preset->addItem(MatList[i].szname);

		l->addLayout(form);


		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		h->addLayout(l);
		dlg->setLayout(h);

		QObject::connect(bb, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
		QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

		QObject::connect(preset, &QComboBox::currentIndexChanged, dlg, &CDlgEditMaterial::selectPreset);
		QObject::connect(ambient, &CColorButton::colorChanged, dlg, &CDlgEditMaterial::updateMaterial);
		QObject::connect(diffuse, &CColorButton::colorChanged, dlg, &CDlgEditMaterial::updateMaterial);
		QObject::connect(specular, &CColorButton::colorChanged, dlg, &CDlgEditMaterial::updateMaterial);
		QObject::connect(specExp, &CDragBox::valueChanged, dlg, &CDlgEditMaterial::updateMaterial);
	}
};

CDlgEditMaterial::CDlgEditMaterial(QWidget* parent) : QDialog(parent), ui(new UIDlgEditMaterial)
{
	setWindowTitle("Edit material");
	ui->setup(this);
}

CDlgEditMaterial::~CDlgEditMaterial()
{

}

void CDlgEditMaterial::SetMaterial(const GLMaterial& mat)
{
	ui->scene.mat = mat;
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
}

GLMaterial CDlgEditMaterial::GetMaterial() const
{
	return ui->scene.mat;
}

void CDlgEditMaterial::accept()
{
	updateMaterial();
	QDialog::accept();
}

void CDlgEditMaterial::updateMaterial()
{
	ui->preset->setCurrentIndex(0);

	ui->scene.mat.ambient = toGLColor(ui->ambient->color());
	ui->scene.mat.diffuse = toGLColor(ui->diffuse->color());
	ui->scene.mat.specular = toGLColor(ui->specular->color());
	ui->scene.mat.shininess = ui->specExp->value();
	ui->view->requestUpdate();
}

void CDlgEditMaterial::selectPreset()
{
	int n = ui->preset->currentIndex() - 1;
	if (n >= 0)
	{
		PresetMaterial mat = MatList[n];
		ui->scene.mat.ambient = mat.ambient;
		ui->scene.mat.diffuse = mat.diffuse;
		ui->scene.mat.specular = mat.specular;
		ui->scene.mat.shininess = mat.shininess;
		SetMaterial(ui->scene.mat);
		ui->view->requestUpdate();
	}
}
