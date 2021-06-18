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
#include "DlgFEBioInfo.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplitter>
#include <FECore/FECoreKernel.h>
#include <FEBioLib/febio.h>
#include <map>
using namespace std;

static bool initFEBio = false;

//static FEBioModel febioModel;

class CDlgFEBioInfoUI
{
public:
	QTreeWidget* pw;
	QTreeWidget* params;
	QComboBox* pc;
	QComboBox* modules;
	FECoreBase* m_pcb;
	QPushButton* pb;

public:
	void setup(QDialog* dlg)
	{
		m_pcb = nullptr;

		QSplitter* split = new QSplitter;
		split->setOrientation(Qt::Vertical);

		QWidget* pane1 = new QWidget;

		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(0, 0, 0, 0);

		QHBoxLayout* h = new QHBoxLayout;
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(new QLabel("Filter:"));
		h->addWidget(pc = new QComboBox);
		h->addWidget(new QLabel("Modules:"));
		h->addWidget(modules = new QComboBox);
		h->addStretch();
		h->addWidget(pb = new QPushButton("Load Plugin ..."));

		l->addLayout(h);

		l->addWidget(pw = new QTreeWidget, 2);
		pw->setColumnCount(5);
		pw->setHeaderLabels(QStringList() << "type string" << "class ID" << "class name" << "module" << "source");

		pane1->setLayout(l);

		QWidget* pane2 = new QWidget;
		l = new QVBoxLayout;
		l->setContentsMargins(0, 0, 0, 0);
		l->addWidget(params = new QTreeWidget, 1);
		params->setColumnCount(2);
		params->setHeaderLabels(QStringList() << "parameter" << "type");
		pane2->setLayout(l);

		split->addWidget(pane1);
		split->addWidget(pane2);

		l = new QVBoxLayout;
		l->addWidget(split);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);
		l->addWidget(bb);

		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(pc, SIGNAL(currentIndexChanged(int)), dlg, SLOT(onFilterChanged()));
		QObject::connect(modules, SIGNAL(currentIndexChanged(int)), dlg, SLOT(onModulesChanged()));
		QObject::connect(pw, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), dlg, SLOT(onTreeChanged()));
		QObject::connect(pb, SIGNAL(clicked()), dlg, SLOT(onLoadPlugin()));

		dlg->setLayout(l);
	}
};

std::map<SUPER_CLASS_ID, const char*> idmap;
void initMap()
{
	idmap.clear();

	idmap[FEINVALID_ID                 ] = "FEINVALID_ID";
	idmap[FEOBJECT_ID                  ] = "FEOBJECT_ID";
	idmap[FETASK_ID                    ] = "FETASK_ID";
	idmap[FESOLVER_ID                  ] = "FESOLVER_ID";
	idmap[FEMATERIAL_ID                ] = "FEMATERIAL_ID";
	idmap[FEBODYLOAD_ID                ] = "FEBODYLOAD_ID";
	idmap[FESURFACELOAD_ID             ] = "FESURFACELOAD_ID";
	idmap[FEEDGELOAD_ID                ] = "FEEDGELOAD_ID";
	idmap[FENODALLOAD_ID               ] = "FENODALLOAD_ID";
	idmap[FENLCONSTRAINT_ID            ] = "FENLCONSTRAINT_ID";
	idmap[FEPLOTDATA_ID                ] = "FEPLOTDATA_ID";
	idmap[FEANALYSIS_ID                ] = "FEANALYSIS_ID";
	idmap[FESURFACEPAIRINTERACTION_ID  ] = "FESURFACEPAIRINTERACTION_ID";
	idmap[FENODELOGDATA_ID             ] = "FENODELOGDATA_ID";
	idmap[FEFACELOGDATA_ID             ] = "FEFACELOGDATA_ID";
	idmap[FEELEMLOGDATA_ID             ] = "FEELEMLOGDATA_ID";
	idmap[FEOBJLOGDATA_ID              ] = "FEOBJLOGDATA_ID";
	idmap[FEBC_ID                      ] = "FEBC_ID";
	idmap[FEGLOBALDATA_ID              ] = "FEGLOBALDATA_ID";
	idmap[FERIGIDOBJECT_ID             ] = "FERIGIDOBJECT_ID";
	idmap[FENLCLOGDATA_ID              ] = "FENLCLOGDATA_ID";
	idmap[FECALLBACK_ID                ] = "FECALLBACK_ID";
	idmap[FEDOMAIN_ID                  ] = "FEDOMAIN_ID";
	idmap[FEIC_ID                      ] = "FEIC_ID";
	idmap[FEDATAGENERATOR_ID           ] = "FEDATAGENERATOR_ID";
	idmap[FELOADCONTROLLER_ID          ] = "FELOADCONTROLLER_ID";
	idmap[FEMODEL_ID                   ] = "FEMODEL_ID";
	idmap[FEMODELDATA_ID               ] = "FEMODELDATA_ID";
	idmap[FESCALARGENERATOR_ID         ] = "FESCALARGENERATOR_ID";
	idmap[FEVECTORGENERATOR_ID         ] = "FEVECTORGENERATOR_ID";
	idmap[FEMAT3DGENERATOR_ID          ] = "FEMAT3DGENERATOR_ID";
	idmap[FEMAT3DSGENERATOR_ID         ] = "FEMAT3DSGENERATOR_ID";
	idmap[FEFUNCTION1D_ID              ] = "FEFUNCTION1D_ID";
	idmap[FELINEARSOLVER_ID            ] = "FELINEARSOLVER_ID";
	idmap[FEMESHADAPTOR_ID             ] = "FEMESHADAPTOR_ID";
	idmap[FEMESHADAPTORCRITERION_ID    ] = "FEMESHADAPTORCRITERION_ID";
	idmap[FERIGIDBC_ID                 ] = "FERIGIDBC_ID";
	idmap[FENEWTONSTRATEGY_ID          ] = "FENEWTONSTRATEGY_ID";
	idmap[FEITEMLIST_ID                ] = "FEITEMLIST_ID";
	idmap[FETIMECONTROLLER_ID          ] = "FETIMECONTROLLER_ID";
	idmap[FEEIGENSOLVER_ID             ] = "FEEIGENSOLVER_ID";
    idmap[FESURFACEPAIRINTERACTIONNL_ID] = "FESURFACEPAIRINTERACTIONNL_ID";
	idmap[FEDATARECORD_ID              ] = "FEDATARECORD_ID";
}

void CDlgFEBioInfo::onFilterChanged()
{
	Update();
}

void CDlgFEBioInfo::onModulesChanged()
{
	Update();
}

void CDlgFEBioInfo::onTreeChanged()
{
	delete ui->m_pcb;
	ui->params->clear();

	QTreeWidgetItem* it = ui->pw->currentItem();
	if (it == nullptr)
	{
		ui->m_pcb = nullptr;
	}
	else
	{
		FECoreKernel& febio = FECoreKernel::GetInstance();

		int sid = it->data(1, Qt::UserRole).toInt();
		string stype = it->text(0).toStdString();

		FECoreFactory* fac = febio.FindFactoryClass(sid, stype.c_str());
		if (fac)
		{
			try {
				ui->m_pcb = fac->Create(nullptr);
			}
			catch (...)
			{
				ui->m_pcb = nullptr;
			}
		}

		if (ui->m_pcb)
		{
			FEParameterList& pl = ui->m_pcb->GetParameterList();
			int N = pl.Parameters();
			auto it = pl.first();
			for (int i=0; i<N; ++i, ++it)
			{
				FEParam& pi = *it;

				QTreeWidgetItem* twi = new QTreeWidgetItem(ui->params);
				twi->setText(0, pi.name());

				const char* sztype = "(unknown)";
				switch (pi.type())
				{
				case FE_PARAM_INVALID          : sztype = "invalid"; break;
				case FE_PARAM_INT              : sztype = "int"; break;
				case FE_PARAM_BOOL             : sztype = "bool"; break;
				case FE_PARAM_DOUBLE           : sztype = "double"; break;
				case FE_PARAM_VEC2D            : sztype = "vec2d"; break;
				case FE_PARAM_VEC3D            : sztype = "vec3d"; break;
				case FE_PARAM_MAT3D            : sztype = "mat3d"; break;
				case FE_PARAM_MAT3DS           : sztype = "mat3ds"; break;
				case FE_PARAM_IMAGE_3D         : sztype = "image3d"; break;
				case FE_PARAM_STRING           : sztype = "string"; break;
				case FE_PARAM_DATA_ARRAY       : sztype = "data_array"; break;
				case FE_PARAM_TENS3DRS         : sztype = "tens3drs"; break;
				case FE_PARAM_STD_STRING       : sztype = "std::string"; break;
				case FE_PARAM_STD_VECTOR_INT   : sztype = "std::vector<int>"; break;
				case FE_PARAM_STD_VECTOR_DOUBLE: sztype = "std::vector<double>"; break;
				case FE_PARAM_STD_VECTOR_VEC2D : sztype = "std::vector<vec2d>"; break;
				case FE_PARAM_STD_VECTOR_STRING: sztype = "std::vector<string>"; break;
				case FE_PARAM_DOUBLE_MAPPED    : sztype = "FEParamDouble"; break;
				case FE_PARAM_VEC3D_MAPPED     : sztype = "FEParamVec3d"; break;
				case FE_PARAM_MAT3D_MAPPED     : sztype = "FEParamMat3d"; break;
				case FE_PARAM_MAT3DS_MAPPED    : sztype = "FEParamMat3ds"; break;
				case FE_PARAM_MATERIALPOINT    : sztype = "FEMaterialPoint"; break;
				}
				
				twi->setText(1, sztype);
			}

			int Props = ui->m_pcb->PropertyClasses();
			for (int i = 0; i < Props; ++i)
			{
				FEProperty* prop = ui->m_pcb->PropertyClass(i);
				QTreeWidgetItem* twi = new QTreeWidgetItem(ui->params);
				twi->setText(0, prop->GetName());
				twi->setText(1, "(Property)");
			}
		}
	}
}

CDlgFEBioInfo::CDlgFEBioInfo(QWidget* parent) : QDialog(parent), ui(new CDlgFEBioInfoUI)
{
	setWindowTitle("FEBio Info");

	setMinimumSize(800, 600);

	ui->setup(this);

	if (initFEBio == false)
	{
		febio::InitLibrary();
		initMap();
	}

	ui->pc->addItem("(all)", -1);
	for (auto it : idmap)
	{
		ui->pc->addItem(it.second, it.first);
	}
	ui->pc->model()->sort(0);

	UpdateModules();
}

void CDlgFEBioInfo::UpdateModules()
{
	ui->modules->clear();
	ui->modules->addItem("(all)", -1);
	FECoreKernel& febio = FECoreKernel::GetInstance();
	int mods = febio.Modules();
	for (int i = 0; i < mods; ++i)
	{
		ui->modules->addItem(febio.GetModuleName(i), i);
	}
	ui->modules->model()->sort(0);
}

void CDlgFEBioInfo::Update()
{
	FECoreKernel& fecore = FECoreKernel::GetInstance();

	ui->pw->clear();

	int nfilter = ui->pc->currentIndex();
	if (nfilter != -1) nfilter = ui->pc->currentData().toInt();

	int nmod = ui->modules->currentIndex();
	if (nmod != -1) nmod = ui->modules->currentData().toInt();

	int mods = fecore.Modules();

	for (int i = 0; i < fecore.FactoryClasses(); ++i)
	{
		const FECoreFactory* fac = fecore.GetFactoryClass(i);
		const char* sztype = fac->GetTypeStr();
		const char* szclass = fac->GetClassName();
		unsigned int mod = fac->GetModuleID();
		SUPER_CLASS_ID sid = fac->GetSuperClassID();
		int allocId = fac->GetAllocatorID();

		if ((nfilter == -1) || (sid == nfilter))
		{
			// check the modules
			bool add = (nmod == -1 ? true : false);
			QString modules;
			if (mod > 0)
			{
				modules.append(fecore.GetModuleName(mod - 1));
				if (nmod == mod - 1) add = true;
				vector<int> moddeps = fecore.GetModuleDependencies(mod - 1);
				for (int j = 0; j < moddeps.size(); ++j)
				{
					modules.append(", ");
					modules.append(fecore.GetModuleName(moddeps[j] - 1));

					if (nmod == moddeps[j] - 1)
					{
						add = true;
					}
				}
			}

			if (add)
			{
				const char* szid = "(unknown)";
				if (idmap.find(sid) != idmap.end()) szid = idmap[sid];

				const char* szalloc = febio::GetPluginName(allocId);
				if (szalloc == nullptr) szalloc = "";

				QTreeWidgetItem* it = new QTreeWidgetItem(ui->pw);
				it->setText(0, QString(sztype));
				it->setText(1, QString(szid)); it->setData(1, Qt::UserRole, sid);
				it->setText(2, QString(szclass));
				it->setText(3, modules);
				it->setText(4, QString(szalloc));
			}
		}
	}

	ui->pw->model()->sort(0);
}

void CDlgFEBioInfo::onLoadPlugin()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Load Plugin", "", "FEBio Plugins (*.dll)");
	if (fileName.isEmpty() == false)
	{
		std::string sfile = fileName.toStdString();

		bool bsuccess = febio::ImportPlugin(sfile.c_str());
		if (bsuccess == false)
		{
			QMessageBox::critical(this, "Load Plugin", "The plugin failed to load.");
		}
		else
		{
			UpdateModules();
			Update();
		}
	}
}
