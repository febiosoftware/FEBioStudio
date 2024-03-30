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
#include "DlgFind.h"
#include <QBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QMessageBox>
#include <QComboBox>
#include <QStackedWidget>
#include "PropertyList.h" // for StringToVec3d

bool string_to_int_list(QString listString, std::vector<int>& list);

class Ui::CDlgFind
{
public:
	QRadioButton* pselNodes;
	QRadioButton* pselEdges;
	QRadioButton* pselFaces;
	QRadioButton* pselElems;
	QComboBox* method;
	QLineEdit* pids;
	QLineEdit* pcoord;
	QLineEdit* pmin;
	QLineEdit* pmax;
	QCheckBox* pclear;
	QStackedWidget* stack;

public:
	void setupUi(QDialog* parent)
	{
		QVBoxLayout* pvb = new QVBoxLayout;

		QVBoxLayout* pvg = new QVBoxLayout;
		pselNodes = new QRadioButton("Select nodes");
		pselEdges = new QRadioButton("Select edges");
		pselFaces = new QRadioButton("Select faces");
		pselElems = new QRadioButton("Select elements");
		pselNodes->setChecked(true);
		pvg->addWidget(pselNodes);
		pvg->addWidget(pselEdges);
		pvg->addWidget(pselFaces);
		pvg->addWidget(pselElems);
		pvb->addLayout(pvg);

		QHBoxLayout* hb = new QHBoxLayout;
		hb->addWidget(new QLabel("method:"));
		method = new QComboBox(); method->addItems(QStringList() << "ID" << "coordinates" << "coordinates range");
		hb->addWidget(method);
		method->setSizePolicy(QSizePolicy::Expanding, method->sizePolicy().verticalPolicy());
		pvb->addLayout(hb);

		stack = new QStackedWidget;
		
		QWidget* wid = new QWidget;
		QGridLayout* pgrid = new QGridLayout;
		QLabel* label = new QLabel("IDs:");
		pids = new QLineEdit; label->setBuddy(pids);
		pgrid->addWidget(label, 0, 0); pgrid->addWidget(pids, 0, 1);
		label = new QLabel("e.g. 1, 2:5, 4:20:5");
		pgrid->addWidget(label, 1,1);
		wid->setLayout(pgrid);
		stack->addWidget(wid);

		QWidget* wcoord = new QWidget;
		pgrid = new QGridLayout;
		label = new QLabel("x,y,z:");
		pcoord = new QLineEdit; label->setBuddy(pcoord);
		pgrid->addWidget(label, 0, 0); pgrid->addWidget(pcoord, 0, 1);
		label = new QLabel("e.g. 1.0, 2.0, 3.0");
		pgrid->addWidget(label, 1, 1);
		wcoord->setLayout(pgrid);
		stack->addWidget(wcoord);

		QWidget* wrange = new QWidget;
		QFormLayout* form = new QFormLayout;
		form->addRow("x0, y0, z0", pmin = new QLineEdit);
		form->addRow("x1, y1, z1", pmax = new QLineEdit);
		wrange->setLayout(form);
		stack->addWidget(wrange);

		pvb->addWidget(stack);

		pclear = new QCheckBox("Clear current selection");
		pvb->addWidget(pclear);
		pclear->setChecked(true);

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pvb->addWidget(buttonBox);
		parent->setLayout(pvb);

		pids->setFocus();

		QObject::connect(method, SIGNAL(currentIndexChanged(int)), stack, SLOT(setCurrentIndex(int)));
		QObject::connect(buttonBox, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgFind::CDlgFind(QWidget* parent, int nsel) : QDialog(parent), ui(new Ui::CDlgFind)
{
	ui->setupUi(this);

	m_method = 0;

	if (nsel == 0) ui->pselNodes->setChecked(true);
	if (nsel == 1) ui->pselEdges->setChecked(true);
	if (nsel == 2) ui->pselFaces->setChecked(true);
	if (nsel == 3) ui->pselElems->setChecked(true);
}

void CDlgFind::accept()
{
	if (ui->pselNodes->isChecked()) m_bsel[0] = true; else m_bsel[0] = false;
	if (ui->pselEdges->isChecked()) m_bsel[1] = true; else m_bsel[1] = false;
	if (ui->pselFaces->isChecked()) m_bsel[2] = true; else m_bsel[2] = false;
	if (ui->pselElems->isChecked()) m_bsel[3] = true; else m_bsel[3] = false;

	m_method = ui->method->currentIndex();

	m_bclear = false;
	if (ui->pclear->isChecked()) m_bclear = true;

	if (m_method == 0)
	{
		// std::string s = ui->pitem->text().toStdString();
		// char sz[256] = {0};
		// strcpy(sz, s.c_str());
		if (string_to_int_list(ui->pids->text(), m_item) == false)
		{
			QMessageBox::critical(this, "Find", "Invalid item list");
			return;
		}
	}
	else if (m_method == 1)
	{
		m_coord = StringToVec3d(ui->pcoord->text());
	}
	else
	{
		m_min = StringToVec3d(ui->pmin->text());
		m_max = StringToVec3d(ui->pmax->text());
	}

	QDialog::accept();
}

// converts a string to a list of numbers. 
// Note: no white space allowed in the string.
// Note: the numbers are assumed to be positive
bool string_to_int_list(QString listString, std::vector<int>& list)
{
	// remove all spaces
    listString.replace(" ", "");

	// make sure string isn't empty
    if (listString.isEmpty()) return false;

	list.clear();
    std::string stdString = listString.toStdString();

	const char* ch = stdString.c_str();
	int n0 = -1, n1 = -1, nn = -1;
	do
	{
		if      (n0 < 0) n0 = (int) atoi(ch);
		else if (n1 < 0) n1 = (int) atoi(ch);
		else if (nn < 0) nn = (int) atoi(ch);

		while (isdigit(*ch)) ++ch;
		switch (*ch)
		{
		case ':': ++ch; break;
		case ',': ++ch;
		case '\0':
			{
				if (n0 >= 0)
				{
					if (n1 == -1) n1 = n0;
					if (nn == -1) nn = 1;

					if ((n0 <= n1) && (nn>0))
						for (int n=n0; n<= n1; n += nn) list.push_back(n);
					else if ((n1 <= n0) && (nn<0))
						for (int n=n0; n >= n1; n += nn) list.push_back(n);
				}

				n0 = -1;
				n1 = -1;
				nn = -1;
			}
			break;
		default:
			return false;
		}
	}
	while (*ch);

	return true;
}
