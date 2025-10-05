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
#include "DlgTransform.h"
#include <CUILib/InputWidgets.h>
#include <QGroupBox>
#include <QFormLayout>
#include <QLabel>
#include <QBoxLayout>
#include <QDialogButtonBox>

class Ui::CDlgTransform
{
public:
	CFloatInput *posX, *posY, *posZ;
	CFloatInput *posRX, *posRY, *posRZ;

	CFloatInput *rotX, *rotY, *rotZ;
	CFloatInput *rotRX, *rotRY, *rotRZ;

	CFloatInput *sclX, *sclY, *sclZ;
	CFloatInput *sclRX, *sclRY, *sclRZ;

	void setPosition(const vec3d& pos, const vec3d& relPos)
	{
		posX->setValue(pos.x);
		posY->setValue(pos.y);
		posZ->setValue(pos.z);

		posRX->setValue(relPos.x);
		posRY->setValue(relPos.y);
		posRZ->setValue(relPos.z);
	}

	void setRotation(const vec3d& rot, const vec3d& relRot)
	{
		rotX->setValue(rot.x);
		rotY->setValue(rot.y);
		rotZ->setValue(rot.z);

		rotRX->setValue(relRot.x);
		rotRY->setValue(relRot.y);
		rotRZ->setValue(relRot.z);
	}

	void setScale(const vec3d& scl, const vec3d& relScl)
	{
		sclX->setValue(scl.x);
		sclY->setValue(scl.y);
		sclZ->setValue(scl.z);

		sclRX->setValue(relScl.x);
		sclRY->setValue(relScl.y);
		sclRZ->setValue(relScl.z);
	}

	vec3d getPosition() { return vec3d(posX->value(), posY->value(), posZ->value()); }
	vec3d getRelativePosition() { return vec3d(posRX->value(), posRY->value(), posRZ->value()); }

	vec3d getRotation() { return vec3d(rotX->value(), rotY->value(), rotZ->value()); }
	vec3d getRelativeRotation() { return vec3d(rotRX->value(), rotRY->value(), rotRZ->value()); }

	vec3d getScale() { return vec3d(sclX->value(), sclY->value(), sclZ->value()); }
	vec3d getRelativeScale() { return vec3d(sclRX->value(), sclRY->value(), sclRZ->value()); }

public:
	void setupUi(QWidget* parent)
	{
		posX = new CFloatInput; posY = new CFloatInput; posZ = new CFloatInput;
		posRX = new CFloatInput; posRY = new CFloatInput; posRZ = new CFloatInput;

		rotX = new CFloatInput; rotY = new CFloatInput; rotZ = new CFloatInput;
		rotRX = new CFloatInput; rotRY = new CFloatInput; rotRZ = new CFloatInput;

		sclX = new CFloatInput; sclY = new CFloatInput; sclZ = new CFloatInput;
		sclRX = new CFloatInput; sclRY = new CFloatInput; sclRZ = new CFloatInput;

		QFormLayout* posForm = new QFormLayout;
		posForm->addRow("", new QLabel("Absolute:"));
		posForm->addRow("X:", posX);
		posForm->addRow("Y:", posY);
		posForm->addRow("Z:", posZ);

		QFormLayout* relPosForm = new QFormLayout;
		relPosForm->addRow("", new QLabel("Relative:"));
		relPosForm->addRow("X:", posRX);
		relPosForm->addRow("Y:", posRY);
		relPosForm->addRow("Z:", posRZ);

		QFormLayout* rotForm = new QFormLayout;
		rotForm->addRow("", new QLabel("Absolute:"));
		rotForm->addRow("X:", rotX);
		rotForm->addRow("Y:", rotY);
		rotForm->addRow("Z:", rotZ);

		QFormLayout* relRotForm = new QFormLayout;
		relRotForm->addRow("", new QLabel("Relative:"));
		relRotForm->addRow("X:", rotRX);
		relRotForm->addRow("Y:", rotRY);
		relRotForm->addRow("Z:", rotRZ);

		QFormLayout* sclForm = new QFormLayout;
		sclForm->addRow("", new QLabel("Absolute:"));
		sclForm->addRow("X:", sclX);
		sclForm->addRow("Y:", sclY);
		sclForm->addRow("Z:", sclZ);

		QFormLayout* relSclForm = new QFormLayout;
		relSclForm->addRow("", new QLabel("Relative:"));
		relSclForm->addRow("X:", sclRX);
		relSclForm->addRow("Y:", sclRY);
		relSclForm->addRow("Z:", sclRZ);

		QHBoxLayout* posLayout = new QHBoxLayout;
		posLayout->addLayout(posForm);
		posLayout->addLayout(relPosForm);

		QHBoxLayout* rotLayout = new QHBoxLayout;
		rotLayout->addLayout(rotForm);
		rotLayout->addLayout(relRotForm);

		QHBoxLayout* sclLayout = new QHBoxLayout;
		sclLayout->addLayout(sclForm);
		sclLayout->addLayout(relSclForm);

		QGroupBox* posGroup = new QGroupBox("Translate");
		posGroup->setLayout(posLayout);

		QGroupBox* rotGroup = new QGroupBox("Rotate");
		rotGroup->setLayout(rotLayout);

		QGroupBox* sclGroup = new QGroupBox("Scale");
		sclGroup->setLayout(sclLayout);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addWidget(posGroup);
		mainLayout->addWidget(rotGroup);
		mainLayout->addWidget(sclGroup);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		mainLayout->addWidget(bb);

		parent->setLayout(mainLayout);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgTransform::CDlgTransform(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgTransform)
{
	ui->setupUi(this);
}

void CDlgTransform::Init()
{
	ui->setPosition(m_pos, m_relPos);
	ui->setRotation(m_rot, m_relRot);
	ui->setScale(m_scl, m_relScl);
}

void CDlgTransform::accept()
{
	m_pos = ui->getPosition();
	m_relPos = ui->getRelativePosition();

	m_rot = ui->getRotation();
	m_relRot = ui->getRelativeRotation();

	m_scl = ui->getScale();
	m_relScl = ui->getRelativeScale();

	QDialog::accept();
}
