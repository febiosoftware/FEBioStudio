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
#include "MeshButtonBox.h"
#include <QGridLayout>
#include <QButtonGroup>
#include <vector>
#include <QPushButton>
#include <assert.h>
//using namespace std;

using std::vector;

class Ui::CMeshButtonBox
{
public:
	vector<ClassDescriptor*>	m_CD;
	QGridLayout*		mainLayout;
	QButtonGroup*		buttonGroup;

public:
	void setup(QWidget* w, int classType, unsigned int nflag)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(0,0,0,0);
		l->setSpacing(0);
		mainLayout = new QGridLayout;
		mainLayout->setContentsMargins(0,0,0,0);
		mainLayout->setSpacing(0);
		buttonGroup = new QButtonGroup(w);

		Class_Iterator it;
		for (it = ClassKernel::FirstCD(); it != ClassKernel::LastCD(); ++it)
		{
			ClassDescriptor* pcd = *it;
			unsigned int iflag = pcd->Flag();
			if ((pcd->GetType() == classType) && (pcd->Flag() & nflag))
			{
				AddButton(pcd->GetName(), pcd);
			}
		}

		l->addLayout(mainLayout);
		l->addStretch();
		w->setLayout(l);
		w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

		QObject::connect(buttonGroup, SIGNAL(idClicked(int)), w, SLOT(onButtonClicked(int)));
	}

	void AddButton(const QString& txt, ClassDescriptor* pcd)
	{
		int ncount = (int)m_CD.size();
		int y = ncount % 2;
		int x = ncount / 2;

		QPushButton* pb = new QPushButton(txt);
		pb->setCheckable(true);
		mainLayout->addWidget(pb, x, y);
		buttonGroup->addButton(pb, ncount);
		m_CD.push_back(pcd);
	}

	void SetFlag(unsigned int flag)
	{
		int N = m_CD.size();
		for (int i=0; i<N; ++i)
		{
			QAbstractButton* b = buttonGroup->button(i);
			ClassDescriptor* cd = m_CD[i];
			if (cd->Flag() & flag) b->setEnabled(true);
			else b->setEnabled(false);
		}
	}
};

//=============================================================================
// CMeshButtonBox
//=============================================================================

CMeshButtonBox::CMeshButtonBox(int classType, unsigned int nflag, QWidget* parent) : QWidget(parent), ui(new Ui::CMeshButtonBox)
{
	ui->setup(this, classType, nflag);
}

ClassDescriptor* CMeshButtonBox::GetClassDescriptor(int n) { return ui->m_CD[n]; }

void CMeshButtonBox::onButtonClicked(int n)
{
	emit buttonSelected(n);
}

void CMeshButtonBox::setFlag(unsigned int flag)
{
	ui->SetFlag(flag);
}
