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
#include "DlgFind.h"
#include <QBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QMessageBox>

static bool string_to_int_list(char* sz, std::vector<int>& list);

class Ui::CDlgFind
{
public:
	QRadioButton* pselNodes;
	QRadioButton* pselEdges;
	QRadioButton* pselFaces;
	QRadioButton* pselElems;
	QLineEdit* pitem;
	QCheckBox* pclear;

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

		QGridLayout* pgrid = new QGridLayout;
		QLabel* label = new QLabel("Item:");
		pitem = new QLineEdit; label->setBuddy(pitem);
		pgrid->addWidget(label, 0, 0); pgrid->addWidget(pitem, 0, 1);
		label = new QLabel("e.g. 1, 2:5, 4:20:5");
		pgrid->addWidget(label, 1,1);
		pvb->addLayout(pgrid);

		pclear = new QCheckBox("Clear current selection");
		pvb->addWidget(pclear);
		pclear->setChecked(true);

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pvb->addWidget(buttonBox);
		parent->setLayout(pvb);

		pitem->setFocus();

		QObject::connect(buttonBox, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgFind::CDlgFind(QWidget* parent, int nsel) : QDialog(parent), ui(new Ui::CDlgFind)
{
	ui->setupUi(this);

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

	m_bclear = false;
	if (ui->pclear->isChecked()) m_bclear = true;

	std::string s = ui->pitem->text().toStdString();
	char sz[256] = {0};
	strcpy(sz, s.c_str());
	if (string_to_int_list(sz, m_item) == false)
	{
		QMessageBox::critical(this, "Find", "Invalid item list");
		return;
	}

	QDialog::accept();
}

// converts a string to a list of numbers. 
// Note: no white space allowed in the string.
// Note: the numbers are converted to zero-base
bool string_to_int_list(char* sz, std::vector<int>& list)
{
	// remove all white-space
	char* ch = sz;
	int l = 0;
	while (*ch)
	{
		if (isspace(*ch) == 0)
		{
			sz[l] = *ch;
			l++;
		}
		++ch;
	}
	sz[l] = 0;

	// make sure there
	if (strlen(sz) == 0) return false;

	list.clear();
	ch = sz;
	int n0 = -1, n1 = -1, nn = -1;
	do
	{
		if      (n0 < 0) n0 = (int) atoi(ch) - 1;
		else if (n1 < 0) n1 = (int) atoi(ch) - 1;
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
