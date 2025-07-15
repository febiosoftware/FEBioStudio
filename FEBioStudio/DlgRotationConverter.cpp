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
#include "DlgRotationConverter.h"
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QPushButton>
#include <FSCore/math3d.h>

class CRotationWidget : public QWidget
{
public:
	CRotationWidget(QWidget* parent = nullptr) : QWidget(parent) {}

	virtual quatd GetRotation() const = 0;

	virtual void SetRotation(quatd q) = 0;

	virtual void setEditable(bool b) { Q_UNUSED(b); } // Default implementation does nothing, can be overridden by derived classes
};

class CEulerAngleWidget : public CRotationWidget
{
public:
	CEulerAngleWidget(QWidget* parent = nullptr) : CRotationWidget(parent)
	{
		QFormLayout* layout = new QFormLayout;
		layout->addRow("X (deg):", x = new QLineEdit); x->setValidator(new QDoubleValidator(-360.0, 360.0, 2, this));
		layout->addRow("Y (deg):", y = new QLineEdit); y->setValidator(new QDoubleValidator(-360.0, 360.0, 2, this));
		layout->addRow("Z (deg):", z = new QLineEdit); z->setValidator(new QDoubleValidator(-360.0, 360.0, 2, this));
		x->setText("0.0");
		y->setText("0.0");
		z->setText("0.0");
		setLayout(layout);
	}

	quatd GetRotation() const override
	{
		quatd q;
		double xd = x->text().toDouble();
		double yd = y->text().toDouble();
		double zd = z->text().toDouble();
		q.SetEuler(xd * DEG2RAD, yd * DEG2RAD, zd * DEG2RAD);
		return q;
	}

	void SetRotation(quatd q) override
	{
		double dx, dy, dz;
		q.GetEuler(dx, dy, dz);
		x->setText(QString::number(dx * RAD2DEG, 'f', 2));
		y->setText(QString::number(dy * RAD2DEG, 'f', 2));
		z->setText(QString::number(dz * RAD2DEG, 'f', 2));
	}

	void setEditable(bool b) override
	{
		x->setReadOnly(!b);
		y->setReadOnly(!b);
		z->setReadOnly(!b);
	}

private:
	QLineEdit* x = nullptr;
	QLineEdit* y = nullptr;
	QLineEdit* z = nullptr;
};

class CAxisAngleWidget : public CRotationWidget
{
public:
	CAxisAngleWidget(QWidget* parent = nullptr) : CRotationWidget(parent)
	{
		QFormLayout* layout = new QFormLayout;
		layout->addRow("X:", x = new QLineEdit); x->setValidator(new QDoubleValidator(-1.0, 1.0, 2, this));
		layout->addRow("Y:", y = new QLineEdit); y->setValidator(new QDoubleValidator(-1.0, 1.0, 2, this));
		layout->addRow("Z:", z = new QLineEdit); z->setValidator(new QDoubleValidator(-1.0, 1.0, 2, this));
		layout->addRow("angle (deg):", w = new QLineEdit); w->setValidator(new QDoubleValidator(-360.0, 360.0, 2, this));
		setLayout(layout);
	}

	quatd GetRotation() const override
	{
		vec3d axis;
		axis.x = x->text().toDouble();
		axis.y = y->text().toDouble();
		axis.z = z->text().toDouble();
		axis.Normalize();
		double angle = w->text().toDouble() * DEG2RAD;
		return quatd(angle, axis);
	}

	void SetRotation(quatd q) override
	{
		vec3d axis = q.GetVector();
		double angle = q.GetAngle() * RAD2DEG;

		x->setText(QString::number(axis.x, 'g', 7));
		y->setText(QString::number(axis.y, 'g', 7));
		z->setText(QString::number(axis.z, 'g', 7));
		w->setText(QString::number(angle, 'f', 2));
	}

	void setEditable(bool b) override
	{
		x->setReadOnly(!b);
		y->setReadOnly(!b);
		z->setReadOnly(!b);
		w->setReadOnly(!b);
	}

private:
	QLineEdit* x = nullptr;
	QLineEdit* y = nullptr;
	QLineEdit* z = nullptr;
	QLineEdit* w = nullptr;
};

class CRotationVectorWidget : public CRotationWidget
{
public:
	CRotationVectorWidget(QWidget* parent = nullptr) : CRotationWidget(parent)
	{
		QFormLayout* layout = new QFormLayout;
		layout->addRow("X (deg):", x = new QLineEdit); x->setValidator(new QDoubleValidator(-360.0, 360.0, 2, this));
		layout->addRow("Y (deg):", y = new QLineEdit); y->setValidator(new QDoubleValidator(-360.0, 360.0, 2, this));
		layout->addRow("Z (deg):", z = new QLineEdit); z->setValidator(new QDoubleValidator(-360.0, 360.0, 2, this));
		x->setText("0.0");
		y->setText("0.0");
		z->setText("0.0");
		setLayout(layout);
	}

	quatd GetRotation() const override
	{
		vec3d v;
		v.x = x->text().toDouble()*DEG2RAD;
		v.y = y->text().toDouble()*DEG2RAD;
		v.z = z->text().toDouble()*DEG2RAD;
		quatd q(v);
		return q;
	}

	void SetRotation(quatd q) override
	{
		vec3d v = q.GetRotationVector();
		x->setText(QString::number(v.x * RAD2DEG));
		y->setText(QString::number(v.y * RAD2DEG));
		z->setText(QString::number(v.z * RAD2DEG));
	}

	void setEditable(bool b) override
	{
		x->setReadOnly(!b);
		y->setReadOnly(!b);
		z->setReadOnly(!b);
	}

private:
	QLineEdit* x = nullptr;
	QLineEdit* y = nullptr;
	QLineEdit* z = nullptr;
};

class CQuaternionWidget : public CRotationWidget
{
public:
	CQuaternionWidget(QWidget* parent = nullptr) : CRotationWidget(parent)
	{
		QFormLayout* layout = new QFormLayout;
		layout->addRow("x:", x = new QLineEdit); x->setValidator(new QDoubleValidator(-1.0, 1.0, 7, this));
		layout->addRow("y:", y = new QLineEdit); y->setValidator(new QDoubleValidator(-1.0, 1.0, 7, this));
		layout->addRow("z:", z = new QLineEdit); z->setValidator(new QDoubleValidator(-1.0, 1.0, 7, this));
		layout->addRow("w:", w = new QLineEdit); w->setValidator(new QDoubleValidator(-1.0, 1.0, 7, this));
		setLayout(layout);
	}

	quatd GetRotation() const override
	{
		double qx = x->text().toDouble();
		double qy = y->text().toDouble();
		double qz = z->text().toDouble();
		double qw = w->text().toDouble();
		quatd q(qx, qy, qz, qw);
		q.MakeUnit();
		return q;
	}

	void SetRotation(quatd q) override
	{
		q.MakeUnit();
		x->setText(QString::number(q.x));
		y->setText(QString::number(q.y));
		z->setText(QString::number(q.z));
		w->setText(QString::number(q.w));
	}

	void setEditable(bool b) override
	{
		x->setReadOnly(!b);
		y->setReadOnly(!b);
		z->setReadOnly(!b);
		w->setReadOnly(!b);
	}

private:
	QLineEdit* x = nullptr;
	QLineEdit* y = nullptr;
	QLineEdit* z = nullptr;
	QLineEdit* w = nullptr;
};

class CRotationMatrixWidget : public CRotationWidget
{
private:
	QTableWidget* table = nullptr;

public:
	CRotationMatrixWidget(QWidget* parent = nullptr) : CRotationWidget(parent)
	{
		QVBoxLayout* layout = new QVBoxLayout;

		table = new QTableWidget(3, 3, this);
		table->setHorizontalHeaderLabels(QStringList() << "X" << "Y" << "Z");
		table->setVerticalHeaderLabels(QStringList() << "X" << "Y" << "Z");
		table->setItem(0, 0, new QTableWidgetItem("1.0"));
		table->setItem(0, 1, new QTableWidgetItem("0.0"));
		table->setItem(0, 2, new QTableWidgetItem("0.0"));
		table->setItem(1, 0, new QTableWidgetItem("0.0"));
		table->setItem(1, 1, new QTableWidgetItem("1.0"));
		table->setItem(1, 2, new QTableWidgetItem("0.0"));
		table->setItem(2, 0, new QTableWidgetItem("0.0"));
		table->setItem(2, 1, new QTableWidgetItem("0.0"));
		table->setItem(2, 2, new QTableWidgetItem("1.0"));
		layout->addWidget(table);
		setLayout(layout);
	}

	quatd GetRotation() const override
	{
		mat3d R;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				R(i, j) = table->item(i, j)->text().toDouble();
			}
		}
		quatd q(R);
		return q;
	}

	void SetRotation(quatd q) override
	{
		mat3d R = q.RotationMatrix();
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				table->item(i, j)->setText(QString::number(R(i, j)));
			}
		}
	}

	void setEditable(bool b) override
	{
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				QTableWidgetItem* item = table->item(i, j);
				if (item) item->setFlags(b ? (item->flags() | Qt::ItemIsEditable) : (item->flags() & ~Qt::ItemIsEditable));
			}
		}
	}
};

class CRotationInputWidget : public QStackedWidget
{
public:
	CRotationInputWidget(QWidget* parent = nullptr) : QStackedWidget(parent)
	{
		addWidget(new CRotationVectorWidget);
		addWidget(new CEulerAngleWidget);
		addWidget(new CAxisAngleWidget);
		addWidget(new CQuaternionWidget);
		addWidget(new CRotationMatrixWidget);
	}

	void setEditable(bool b)
	{
		for (int i = 0; i < count(); ++i)
		{
			CRotationWidget* w = dynamic_cast<CRotationWidget*>(widget(i));
			if (w) w->setEditable(b);
		}
	}

	quatd GetRotation()
	{
		CRotationWidget* widget = dynamic_cast<CRotationWidget*>(currentWidget());
		quatd q;
		if (widget) q = widget->GetRotation();
		return q;
	}

	void SetRotation(quatd q)
	{
		CRotationWidget* widget = dynamic_cast<CRotationWidget*>(currentWidget());
		if (widget) widget->SetRotation(q);
	}
};

CDlgRotationConverter::CDlgRotationConverter(QWidget* parent) : QDialog(parent)
{
	setWindowTitle("Rotation Converter");

	QStringList methods = QStringList() << "Rotation vector" << "Euler angles" << "Axis - angle" << "Quaternion" << "Rotation Matrix";

	QFormLayout* input = new QFormLayout;
	QComboBox* inputMethod = new QComboBox;
	inputMethod->addItems(methods);
	input->addRow("Input method", inputMethod);

	CRotationInputWidget* inputWidget = new CRotationInputWidget;

	connect(inputMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), inputWidget, &QStackedWidget::setCurrentIndex);

	QFormLayout* output = new QFormLayout;
	QComboBox* outputMethod = new QComboBox;
	outputMethod->addItems(methods);
	output->addRow("Ouput method", outputMethod);

	CRotationInputWidget* outputWidget = new CRotationInputWidget;
	outputWidget->setEditable(false);

	connect(outputMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), outputWidget, &QStackedWidget::setCurrentIndex);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &CDlgRotationConverter::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &CDlgRotationConverter::reject);

	QPushButton* convertButton = new QPushButton("Convert", this);
	connect(convertButton, &QPushButton::clicked, this, [inputWidget, outputWidget]() {
			quatd inputRotation = inputWidget->GetRotation();
			outputWidget->SetRotation(inputRotation);
		});

	connect(outputMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [inputWidget, outputWidget]() {
		quatd inputRotation = inputWidget->GetRotation();
		outputWidget->SetRotation(inputRotation);
		});

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addLayout(input);
	layout->addWidget(inputWidget);
	layout->addWidget(convertButton);
	layout->addLayout(output);
	layout->addWidget(outputWidget);

	layout->addWidget(buttonBox);
	setLayout(layout);
}
