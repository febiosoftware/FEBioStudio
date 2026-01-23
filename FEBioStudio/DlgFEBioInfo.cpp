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
#include <QJsonParseError>
#include <QLineEdit>
#include <QHeaderView>
#include <FECore/FECoreKernel.h>
#include <FEBioLib/febio.h>
#include <map>
#include <FEBioLink/FEBioClass.h>
#include <FEBioLink/FEBioModule.h>
#include <FECore/FEModule.h>
#include <FEBioLib/version.h>
#include <QJsonObject>
#include <QJsonArray>
#include "PropertyList.h"
#include <FECore/FEModelParam.h>

using namespace std;

static FEBioModel febioModel;

class CDlgFEBioInfoUI
{
public:
	QTreeWidget* pw;
	QTreeWidget* params;
	QComboBox* pc;
	QComboBox* modules;
	QLineEdit* search;
	QPushButton* exportBtn;

public:
	void setup(QDialog* dlg)
	{
		QSplitter* split = new QSplitter;
		split->setOrientation(Qt::Vertical);

		QWidget* pane1 = new QWidget;

		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(0, 0, 0, 0);

		QLabel* febVersion = new QLabel;
		febVersion->setAlignment(Qt::AlignLeft);
		const char* szversion = febio::getVersionString();
		febVersion->setText(QString("FEBio version : %1").arg(szversion));
		l->addWidget(febVersion);

		QFrame* f = new QFrame;
		f->setFrameStyle(QFrame::HLine);
		l->addWidget(f);

		QHBoxLayout* h = new QHBoxLayout;
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(new QLabel("Super class ID:"));
		h->addWidget(pc = new QComboBox);
		h->addWidget(new QLabel("Modules:"));
		h->addWidget(modules = new QComboBox);
		h->addWidget(new QLabel("Filter:"));
		h->addWidget(search = new QLineEdit);
		h->addWidget(exportBtn = new QPushButton("Export..."));
//		h->addStretch();

		l->addLayout(h);

		l->addWidget(pw = new QTreeWidget, 2);
		pw->setColumnCount(6);
		pw->setHeaderLabels(QStringList() << "type string" << "super class ID" << "class name" << "base class" << "module" << "source");

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
		QObject::connect(exportBtn, SIGNAL(clicked()), dlg, SLOT(onExport()));
		dlg->setLayout(l);
	}
};

void CDlgFEBioInfo::onExport()
{
	// get all the FEBio modules
	QJsonArray moduleArray;
	std::vector<FEBio::FEBioModule> moduleList = FEBio::GetAllModules();
	for (auto m : moduleList)
	{
		QJsonObject modObj;

		// the module description is actually a JSON object
		QJsonParseError error;
		QString desc = QString(m.m_szdesc);
		QJsonDocument doc = QJsonDocument::fromJson(desc.toUtf8(), &error);

		if (error.error == QJsonParseError::NoError) {
			modObj = doc.object();
			modObj["name"] = m.m_szname;
		}
		else
		{
			modObj["name"] = m.m_szname;
			modObj["info"] = m.m_szdesc;
		}
		moduleArray.append(modObj);
	}

#ifdef FEBIO_EXPERIMENTAL
	bool includeExperimentals = true;
#else
	bool includeExperimentals = false;
#endif

	// loop over all items visible in the tree
	QJsonArray featureArray;
	for (int i = 0; i < ui->pw->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem* item = ui->pw->topLevelItem(i);
		QJsonObject feature;
		QJsonArray paramArray;
		// get the parameters for this item
		int index = item->data(0, Qt::UserRole).toInt();
		FECoreKernel& febio = FECoreKernel::GetInstance();
		FECoreBase* pcb = nullptr;

		const FECoreFactory* fac = febio.GetFactoryClass(index);
		if (fac && ((fac->GetSpecID() != FECORE_EXPERIMENTAL) || includeExperimentals))
		{
			try {
				pcb = fac->Create(nullptr);
			}
			catch (...)
			{
				pcb = nullptr;
			}
		}
		if (pcb)
		{
			FEParameterList& pl = pcb->GetParameterList();
			int N = pl.Parameters();
			auto it = pl.first();
			for (int j = 0; j < N; ++j, ++it)
			{
				FEParam& pi = *it;

				// skip hidden parameters
				bool isHidden = (pi.GetFlags() & FE_PARAM_HIDDEN);
				if (isHidden) continue;

				QJsonObject paramObj;
				QString paramVal("");
				if (pi.dim() == 1)
				{
					switch (pi.type())
					{
					case FE_PARAM_INT   : paramVal = QString::number(pi.value<int>()); break;
					case FE_PARAM_DOUBLE: paramVal = QString::number(pi.value<double>()); break;
					case FE_PARAM_BOOL  : paramVal = QString(pi.value<bool>() ? "true" : "false"); break;
					case FE_PARAM_VEC3D : paramVal = Vec3dToString(pi.value<vec3d>()); break;
					case FE_PARAM_DOUBLE_MAPPED:
					{
						FEParamDouble& v = pi.value<FEParamDouble>();
						if (v.isConst()) paramVal = QString::number(v.constValue());
					}
					break;
					case FE_PARAM_VEC3D_MAPPED:
					{
						FEParamVec3& v = pi.value<FEParamVec3>();
						if (v.isConst()) paramVal = Vec3dToString(v.constValue());
					}
					break;
					}
				}
				paramObj["name"] = pi.name();
				paramObj["description"] = pi.longName();
				paramObj["default"] = paramVal;
				paramObj["units"] = (pi.units() ? pi.units() : "");
				paramArray.append(paramObj);
			}
			N = pcb->PropertyClasses();
			for (int i = 0; i < N; ++i)
			{
				FEProperty* prop = pcb->PropertyClass(i);
				if (prop)
				{
					const char* szname = prop->GetName();
					if (szname)
					{
						QJsonObject paramObj;
						paramObj["name"] = szname;
						paramObj["description"] = "";
						paramObj["default"] = "";
						paramObj["units"] = "";
						paramArray.append(paramObj);
					}
				}
			}

			feature["type_string"] = item->text(0);
			feature["super_class_id"] = item->text(1);
			feature["class_name"] = item->text(2);
			feature["base_class"] = item->text(3);
			feature["module"] = item->text(4);
			feature["source"] = item->text(5);
			feature["parameters"] = paramArray;
			delete pcb;
		}

		featureArray.append(feature);
	}

	QJsonObject root;
	root["features"] = featureArray;
	root["modules"] = moduleArray;

	// request output file name
	QString fileName = QFileDialog::getSaveFileName(this, "Export FEBio features", "", "JSON Files (*.json);;All Files (*)");
	if (fileName.isEmpty()) return;

	// store the json array to file
	QFile jsonFile(fileName);
	if (jsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QJsonDocument jsonDoc(root);
		jsonFile.write(jsonDoc.toJson());
		jsonFile.close();
	}
	else
	{
		QMessageBox::warning(this, "Export FEBio features", QString("Could not open file %1 for writing").arg(fileName));
	}
}

void CDlgFEBioInfo::onTreeChanged()
{
	ui->params->clear();

	QTreeWidgetItem* it = ui->pw->currentItem();
	if (it == nullptr) return;
	FECoreKernel& febio = FECoreKernel::GetInstance();

	int sid = it->data(1, Qt::UserRole).toInt();
	string stype = it->text(0).toStdString();

	int index = it->data(0, Qt::UserRole).toInt();
	if ((index < 0) || (index >= febio.FactoryClasses())) return;

#ifdef FEBIO_EXPERIMENTAL
	bool includeExperimentals = true;
#else
	bool includeExperimentals = false;
#endif


	FECoreBase* pcb = nullptr;
	const FECoreFactory* fac = febio.GetFactoryClass(index);
	if (fac && ((fac->GetSpecID() != FECORE_EXPERIMENTAL) || includeExperimentals))
	{
		try {
			pcb = fac->Create(&febioModel);
		}
		catch (...)
		{
			pcb = nullptr;
		}
	}

	if (pcb)
	{
		FEParameterList& pl = pcb->GetParameterList();
		int N = pl.Parameters();
		auto it = pl.first();
		for (int i=0; i<N; ++i, ++it)
		{
			FEParam& pi = *it;

			int dim = pi.dim();

			QTreeWidgetItem* twi = new QTreeWidgetItem(ui->params);
			twi->setText(0, pi.name());

			QString stype = "(unknown)";
			switch (pi.type())
			{
			case FE_PARAM_INVALID          : stype = "invalid"; break;
			case FE_PARAM_INT: {
				if (dim > 1) stype = QString("int[%1]").arg(dim);
				else stype = "int";
			}
			break;
			case FE_PARAM_BOOL             : stype = "bool"; break;
			case FE_PARAM_DOUBLE           : {
				if (dim > 1) stype = QString("double[%1]").arg(dim);
				else stype = "double";
			}
			break;
			case FE_PARAM_VEC2D            : stype = "vec2d"; break;
			case FE_PARAM_VEC3D            : stype = "vec3d"; break;
			case FE_PARAM_MAT3D            : stype = "mat3d"; break;
			case FE_PARAM_MAT3DS           : stype = "mat3ds"; break;
			case FE_PARAM_STRING           : stype = "string"; break;
			case FE_PARAM_DATA_ARRAY       : stype = "data_array"; break;
			case FE_PARAM_TENS3DRS         : stype = "tens3drs"; break;
			case FE_PARAM_STD_STRING       : stype = "std::string"; break;
			case FE_PARAM_STD_VECTOR_INT   : stype = "std::vector<int>"; break;
			case FE_PARAM_STD_VECTOR_DOUBLE: stype = "std::vector<double>"; break;
			case FE_PARAM_STD_VECTOR_VEC2D : stype = "std::vector<vec2d>"; break;
			case FE_PARAM_STD_VECTOR_STRING: stype = "std::vector<string>"; break;
			case FE_PARAM_DOUBLE_MAPPED    : stype = "FEParamDouble"; break;
			case FE_PARAM_VEC3D_MAPPED     : stype = "FEParamVec3d"; break;
			case FE_PARAM_MAT3D_MAPPED     : stype = "FEParamMat3d"; break;
			case FE_PARAM_MAT3DS_MAPPED    : stype = "FEParamMat3ds"; break;
			case FE_PARAM_MATERIALPOINT    : stype = "FEMaterialPoint"; break;
			}
				
			twi->setText(1, stype);
		}

		int Props = pcb->PropertyClasses();
		for (int i = 0; i < Props; ++i)
		{
			FEProperty* prop = pcb->PropertyClass(i);
			const char* szclass = prop->GetClassName();
			if (szclass == nullptr) szclass = "(unknown)";
			QTreeWidgetItem* twi = new QTreeWidgetItem(ui->params);
			twi->setText(0, prop->GetName());
			twi->setText(1, szclass);
		}

		delete pcb;
	}
}

CDlgFEBioInfo::CDlgFEBioInfo(QWidget* parent) : QDialog(parent), ui(new CDlgFEBioInfoUI)
{
	setWindowTitle("FEBio Info");

	setMinimumSize(800, 600);

	ui->setup(this);

	std::map<unsigned int, const char*> idmap = FEBio::GetSuperClassMap();
	ui->pc->addItem("(all)", -1);
	for (auto it : idmap)
	{
		ui->pc->addItem(it.second, it.first);
	}
	ui->pc->model()->sort(0);

	UpdateModules();
	Update();
	ui->pw->header()->resizeSections(QHeaderView::ResizeToContents);

	QObject::connect(ui->pc, SIGNAL(currentIndexChanged(int)), this, SLOT(Update()));
	QObject::connect(ui->modules, SIGNAL(currentIndexChanged(int)), this, SLOT(Update()));
	QObject::connect(ui->pw, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(onTreeChanged()));
	QObject::connect(ui->search, SIGNAL(textChanged(const QString&)), this, SLOT(Update()));
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

	QString searchString = ui->search->text();
	bool search = (searchString.isEmpty() == false);

	bool addModuleDependencies = false;

#ifdef FEBIO_EXPERIMENTAL
	bool includeExperimentals = true;
#else
	bool includeExperimentals = false;
#endif

	for (int i = 0; i < fecore.FactoryClasses(); ++i)
	{
		const FECoreFactory* fac = fecore.GetFactoryClass(i);

		if (fac && ((fac->GetSpecID() != FECORE_EXPERIMENTAL) || includeExperimentals))
		{
			const char* sztype = fac->GetTypeStr();
			const char* szclass = fac->GetClassName();
			const char* szbase = fac->GetBaseClassName();
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

					if (addModuleDependencies)
					{
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
				}

				if (add)
				{
					const char* szid = FEBio::GetSuperClassString(sid);
					if (szid == nullptr) szid = "(unknown)";

					const char* szalloc = febio::GetPluginName(allocId);
					if (szalloc == nullptr) szalloc = "";

					QString typeStr(sztype);
					QString idStr(szid);
					QString classStr(szclass);
					QString baseStr(szbase);
					QString allocStr(szalloc);

					if ((search == false) ||
						typeStr.contains(searchString, Qt::CaseInsensitive) ||
						idStr.contains(searchString, Qt::CaseInsensitive) ||
						classStr.contains(searchString, Qt::CaseInsensitive) ||
						baseStr.contains(searchString, Qt::CaseInsensitive) ||
						allocStr.contains(searchString, Qt::CaseInsensitive))
					{
						QTreeWidgetItem* it = new QTreeWidgetItem(ui->pw);
						it->setText(0, typeStr); it->setData(0, Qt::UserRole, i);
						it->setText(1, idStr); it->setData(1, Qt::UserRole, sid);
						it->setText(2, classStr);
						it->setText(3, baseStr);
						it->setText(4, modules);
						it->setText(5, allocStr);
					}
				}
			}
		}
	}

	ui->pw->model()->sort(0);
}
