#include "stdafx.h"
#include "MeshButtonBox.h"
#include <QGridLayout>
#include <QButtonGroup>
#include <vector>
#include <QPushButton>
#include <assert.h>
using namespace std;

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
		l->setMargin(0);
		l->setSpacing(0);
		mainLayout = new QGridLayout;
		mainLayout->setMargin(0);
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

		QObject::connect(buttonGroup, SIGNAL(buttonClicked(int)), w, SLOT(onButtonClicked(int)));
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
