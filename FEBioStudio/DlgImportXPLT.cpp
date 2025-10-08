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
#include "DlgImportXPLT.h"
#include <QRadioButton>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QLabel>


// converts a string to a list of numbers. 
// Note: no white space allowed in the string.
// Note: the numbers are converted to zero-base
static bool string_to_int_list(char* sz, std::vector<int>& list)
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
		if (n0 < 0) n0 = (int)atoi(ch) - 1;
		else if (n1 < 0) n1 = (int)atoi(ch) - 1;
		else if (nn < 0) nn = (int)atoi(ch);

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
					for (int n = n0; n <= n1; n += nn) list.push_back(n);
				else if ((n1 <= n0) && (nn<0))
					for (int n = n0; n >= n1; n += nn) list.push_back(n);
			}

			n0 = -1;
			n1 = -1;
			nn = -1;
		}
		break;
		default:
			return false;
		}
	} while (*ch);

	return true;
}

class Ui::CDlgImportXPLT
{
public:
	QRadioButton* pb1;
	QRadioButton* pb2;
	QRadioButton* pb3;
	QRadioButton* pb4;
	QLineEdit* pitems;

public:
	void setupUi(QDialog* parent)
	{
		QVBoxLayout* pv = new QVBoxLayout;

		QVBoxLayout* pg = new QVBoxLayout;
		pg->addWidget(pb1 = new QRadioButton("Read all states"));
		pg->addWidget(pb2 = new QRadioButton("Read all converged states (status = 0)"));
		pg->addWidget(pb3 = new QRadioButton("Read last state only"));
		pg->addWidget(pb4 = new QRadioButton("Read states from list:"));
		pv->addLayout(pg);

		pb1->setChecked(true);

		pv->addWidget(pitems = new QLineEdit);
		pv->addWidget(new QLabel("(e.g.:1,2,3:6,10:100:5)"));

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		pv->addWidget(bb);
		parent->setLayout(pv);

		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(pitems, SIGNAL(textEdited(const QString&)), pb4, SLOT(click()));
	}
};

CDlgImportXPLT::CDlgImportXPLT(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgImportXPLT)
{
	ui->setupUi(this);
	setWindowTitle("Import XPLT");
}

void CDlgImportXPLT::accept()
{
	if (ui->pb1->isChecked()) m_nop = 0;
	if (ui->pb2->isChecked()) m_nop = 1;
	if (ui->pb3->isChecked()) m_nop = 2;
	if (ui->pb4->isChecked()) m_nop = 3;

	std::string s = ui->pitems->text().toStdString();
	char buf[256] = {0}; 
	strcpy(buf, s.c_str());
	string_to_int_list(buf, m_item);

	QDialog::accept();
}
