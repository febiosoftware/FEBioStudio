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
#include "DlgRAWImport.h"
#include <QLineEdit>
#include <QBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QValidator>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <ImageLib/3DImage.h>

class Ui::CDlgRAWImport
{
public:
	QComboBox* im;
	QLineEdit*	nx;
	QLineEdit*	ny;
	QLineEdit*	nz;

	QLineEdit*	x0;
	QLineEdit*	y0;
	QLineEdit*	z0;

	QLineEdit*	w;
	QLineEdit*	h;
	QLineEdit*	d;
    QCheckBox*  swap;

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* lo = new QVBoxLayout;
        // enum { UINT_8, INT_8, UINT_16, INT_16, UINT_RGB8, INT_RGB8, UINT_RGB16, INT_RGB16, REAL_32, REAL_64 };

		im = new QComboBox; im->addItems(QStringList() << "8-bit unsigned" << "8-bit signed" << "16-bit unsigned" 
            << "16-bit signed" << "8-bit unsigned RGB" << "8-bit signed RGB" << "16-bit unsigned RGB" 
            << "16-bit signed RGB" << "32-bit real" << "64-bit real");

		nx = new QLineEdit; nx->setValidator(new QIntValidator(1, 4096));
		ny = new QLineEdit; ny->setValidator(new QIntValidator(1, 4096));
		nz = new QLineEdit; nz->setValidator(new QIntValidator(1, 4096));

		x0 = new QLineEdit; x0->setValidator(new QDoubleValidator); x0->setText(QString::number(0.0));
		y0 = new QLineEdit; y0->setValidator(new QDoubleValidator); y0->setText(QString::number(0.0));
		z0 = new QLineEdit; z0->setValidator(new QDoubleValidator); z0->setText(QString::number(0.0));

		w = new QLineEdit; w->setValidator(new QDoubleValidator); w->setText(QString::number(1.0));
		h = new QLineEdit; h->setValidator(new QDoubleValidator); h->setText(QString::number(1.0));
		d = new QLineEdit; d->setValidator(new QDoubleValidator); d->setText(QString::number(1.0));

        swap = new QCheckBox;
        swap->setChecked(false);

		int row = 0;
		QGridLayout* grid = new QGridLayout;
		grid->addWidget(new QLabel("Image type:"  ), row, 0, Qt::AlignRight); grid->addWidget(im, row, 1, 1, 2); row++;
		grid->addWidget(new QLabel("<b>Image dimensions:</b>"), row, 0); row++;
		grid->addWidget(new QLabel("width:" ), row, 0, Qt::AlignRight); grid->addWidget(nx, row, 1); grid->addWidget(new QLabel("pixels"), row, 2); row++;
		grid->addWidget(new QLabel("height:"), row, 0, Qt::AlignRight); grid->addWidget(ny, row, 1); grid->addWidget(new QLabel("pixels"), row, 2); row++;
		grid->addWidget(new QLabel("slices:"), row, 0, Qt::AlignRight); grid->addWidget(nz, row, 1); row++;
		grid->addWidget(new QLabel("<b>Physical dimensions:</b>"), row, 0); row++;
		grid->addWidget(new QLabel("x0:"), row, 0, Qt::AlignRight); grid->addWidget(x0, row, 1); row++;
		grid->addWidget(new QLabel("y0:"), row, 0, Qt::AlignRight); grid->addWidget(y0, row, 1); row++;
		grid->addWidget(new QLabel("z0:"), row, 0, Qt::AlignRight); grid->addWidget(z0, row, 1); row++;
		grid->addWidget(new QLabel("width:" ), row, 0, Qt::AlignRight); grid->addWidget(w, row, 1); row++;
		grid->addWidget(new QLabel("height:"), row, 0, Qt::AlignRight); grid->addWidget(h, row, 1); row++;
		grid->addWidget(new QLabel("depth:" ), row, 0, Qt::AlignRight); grid->addWidget(d, row, 1); row++;
        grid->addWidget(new QLabel("<b>Misc:</b>"), row, 0); row++;
        grid->addWidget(new QLabel("Swap endianness:" ), row, 0, Qt::AlignRight); grid->addWidget(swap, row, 1); row++;

		lo->addLayout(grid);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		lo->addWidget(bb);

		parent->setLayout(lo);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgRAWImport::CDlgRAWImport(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgRAWImport)
{
	ui->setupUi(this);
}

void CDlgRAWImport::accept()
{
	m_nx = ui->nx->text().toInt();	
	m_ny = ui->ny->text().toInt();
	m_nz = ui->nz->text().toInt();

	m_x0 = ui->x0->text().toDouble();
	m_y0 = ui->y0->text().toDouble();
	m_z0 = ui->z0->text().toDouble();

	m_w = ui->w->text().toDouble();
	m_h = ui->h->text().toDouble();
	m_d = ui->d->text().toDouble();

    m_type = ui->im->currentIndex();

    m_swapEndianness = ui->swap->isChecked();

	QDialog::accept();
}
