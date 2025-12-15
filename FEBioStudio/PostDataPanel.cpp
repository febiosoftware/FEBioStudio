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
#include "PostDataPanel.h"
#include <QBoxLayout>
#include <QTableView>
#include <QLabel>
#include <QHeaderView>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QFileDialog>
#include <QFormLayout>
#include <QCheckBox>
#include <QRadioButton>
#include <QAction>
#include <QMenu>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QStackedWidget>
#include <QSplitter>
#include <QGroupBox>
#include "MainWindow.h"
#include "Document.h"
#include <PostLib/FEPostModel.h>
#include <PostGL/GLModel.h>
#include <QtCore/QAbstractTableModel>
#include <PostLib/DataFilter.h>
#include "PropertyListView.h"
#include "PropertyListForm.h"
#include <PostLib/FEMeshData_T.h>
#include <PostLib/FEMathData.h>
#include "PostDocument.h"
#include "GLModelDocument.h"
#include <FEBioMonitor/FEBioMonitorDoc.h>
#include <PostLib/FEDataField.h>
#include <PostLib/FEDistanceMap.h>
#include <PostLib/FEAreaCoverage.h>
#include <MeshTools/FESelection.h>
#include "DlgAddEquation.h"
#include <FEBioLink/FEBioClass.h>
#include <FEBioLink/FEBioModule.h>
#include <FECore/fecore_enum.h>
#include "DlgStartThread.h"

class CCurvatureProps : public CPropertyList
{
public:
	Post::CurvatureField*	m_pf;

public:
	CCurvatureProps(Post::CurvatureField* pf) : m_pf(pf)
	{
		addProperty("Smoothness", CProperty::Int);
		addProperty("Max iterations", CProperty::Int);
		addProperty("Use Extended Quadric", CProperty::Bool);
	}

	QVariant GetPropertyValue(int i) override
	{
		switch (i)
		{
		case 0: return m_pf->m_nlevels; break;
		case 1: return m_pf->m_nmax; break;
		case 2: return (m_pf->m_bext != 0); break;
		}

		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		switch (i)
		{
		case 0: m_pf->m_nlevels = v.toInt(); break;
		case 1: m_pf->m_nmax = v.toInt(); break;
		case 2: m_pf->m_bext = (v.toBool() ? 1 : 0); break;
		}
	}
};

class CMathScalarDataProps : public CPropertyList
{
public:
	Post::FEScalarMathDataField*	m_pd;

	CMathScalarDataProps(Post::FEScalarMathDataField* pd) : m_pd(pd)
	{
		addProperty("Equation", CProperty::String);
	}

	QVariant GetPropertyValue(int i) override
	{
		return QString::fromStdString(m_pd->EquationString());
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		m_pd->SetEquationString((v.toString()).toStdString());
	}
};

class CMathElemDataProps : public CPropertyList
{
public:
	Post::FEMathElemDataField* m_pd;

	CMathElemDataProps(Post::FEMathElemDataField* pd) : m_pd(pd)
	{
		addProperty("Equation", CProperty::String);
	}

	QVariant GetPropertyValue(int i) override
	{
		return QString::fromStdString(m_pd->EquationString());
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		m_pd->SetEquationString((v.toString()).toStdString(), false);
	}
};

class CMathDataVec3Props : public CPropertyList
{
public:
	Post::FEMathVec3DataField*	m_pd;

	CMathDataVec3Props(Post::FEMathVec3DataField* pd) : m_pd(pd)
	{
		addProperty("x", CProperty::String);
		addProperty("y", CProperty::String);
		addProperty("z", CProperty::String);
	}

	QVariant GetPropertyValue(int i) override
	{
		switch (i)
		{
		case 0: return QString::fromStdString(m_pd->EquationString(0)); break;
		case 1: return QString::fromStdString(m_pd->EquationString(1)); break;
		case 2: return QString::fromStdString(m_pd->EquationString(2)); break;
		}

		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		switch (i)
		{
		case 0: m_pd->SetEquationString(0, (v.toString()).toStdString()); break;
		case 1: m_pd->SetEquationString(1, (v.toString()).toStdString()); break;
		case 2: m_pd->SetEquationString(2, (v.toString()).toStdString()); break;
		}
	}
};

class CStrainProps : public CPropertyList
{
private:
	Post::StrainDataField*	m_ps;

public:
	CStrainProps(Post::StrainDataField* ps, int maxStates) : m_ps(ps)
	{
		addProperty("reference state", CProperty::Int)->setIntRange(0, maxStates);
	}

	QVariant GetPropertyValue(int i) override
	{
		switch (i)
		{
		case 0: return m_ps->ReferenceState() + 1; break;
		}

		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		switch (i)
		{
		case 0: m_ps->SetReferenceState(v.toInt() - 1); break;
		}
	}
};

class DistanceMapThread : public CustomThread
{
public:
	DistanceMapThread(Post::FEDistanceMap* map) : m_map(map) {}

	void run() Q_DECL_OVERRIDE
	{
		m_map->Init();

		m_bstop = false;
		m_progress = 0.0;
		Post::FEPostModel& fem = *m_map->GetModel();
		for (int n = 0; n < fem.GetStates(); ++n)
		{
			m_map->ApplyState(n);
			if (m_bstop) break;

			m_progress = 100.0*(double)(n + 1) / (double)fem.GetStates();
		}
		emit resultReady(true);
	}

	bool hasProgress() override { return true; }

	double progress() override { return m_progress; }

	void stop() override { m_bstop = true; }

private:
	Post::FEDistanceMap* m_map;
	double m_progress = 0.0;
	bool m_bstop = false;
};

class CDistanceMapProps : public CPropertyList
{
public:
	CDistanceMapProps(Post::FEDistanceMap* map) : m_map(map)
	{
		addProperty("Assign to surface1", CProperty::Action, "");
		addProperty("Assign to surface2", CProperty::Action, "");
		addProperty("Signed distance", CProperty::Bool);
		addProperty("Flip Primary", CProperty::Bool);
		addProperty("Flip Secondary", CProperty::Bool);
		addProperty("Method", CProperty::Enum)->setEnumValues(QStringList() << "new" << "old");
		addProperty("", CProperty::Action, "Apply");
	}

	QVariant GetPropertyValue(int i) override
	{
		switch (i)
		{
		case 0: {
			int n = m_map->GetSurfaceSize(0);
			return QString("(%1 Faces)").arg(n);
		}
		break;
		case 1: {
			int n = m_map->GetSurfaceSize(1);
			return QString("(%1 Faces)").arg(n);
		}
		break;
		case 2: return m_map->m_bsigned; break;
		case 3: return m_map->m_flipPrimary; break;
		case 4: return m_map->m_flipSecondary; break;
		case 5: return m_map->m_nopt;
		}
		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		switch (i)
		{
		case 0: {
			m_map->InitSurface(0);
			SetModified(true);
		}
		break;
		case 1: {
			m_map->InitSurface(1);
			SetModified(true);
		}
		break;
		case 2: m_map->m_bsigned = v.toBool(); break;
		case 3: m_map->m_flipPrimary = v.toBool(); break;
		case 4: m_map->m_flipSecondary = v.toBool(); break;
		case 5: m_map->m_nopt = v.toInt(); break;
		case 6:
		{
			CDlgStartThread dlg(nullptr, new DistanceMapThread(m_map));
			dlg.exec();
		}
		break;
		}
	}

private:
	Post::FEDistanceMap*	m_map;
};

class CAreaCoverageProps : public CPropertyList
{
public:
	CAreaCoverageProps(Post::FEAreaCoverage* map) : m_map(map)
	{
		addProperty("Assign to surface1", CProperty::Action, "");
		addProperty("Assign to surface2", CProperty::Action, "");
		addProperty("Allow back intersections", CProperty::Bool);
		addProperty("Angle threshold", CProperty::Float);
		addProperty("Back search radius", CProperty::Float);
		addProperty("", CProperty::Action, "Apply");
	}

	QVariant GetPropertyValue(int i) override
	{
		switch (i)
		{
		case 0:
		{
			int n = m_map->GetSurfaceSize(0);
			return QString("(%1 Faces)").arg(n);
		}
		break;
		case 1:
		{
			int n = m_map->GetSurfaceSize(1);
			return QString("(%1 Faces)").arg(n);
		}
		break;
		case 2: return m_map->AllowBackIntersection(); break;
		case 3: return m_map->GetAngleThreshold(); break;
		case 4: return m_map->GetBackSearchRadius(); break;
		break;
		}

		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		switch (i)
		{
		case 0:
			{
				m_map->InitSurface(0);
				SetModified(true);
			}
			break;
		case 1:
			{
				m_map->InitSurface(1);
				SetModified(true);
			}
			break;
		case 2: m_map->AllowBackIntersection(v.toBool()); break;
		case 3: m_map->SetAngleThreshold(v.toDouble()); break;
		case 4: m_map->SetBackSearchRadius(v.toDouble()); break;
		case 5:
			{
				m_map->Apply();
			}
			break;
		}
	}

private:
	Post::FEAreaCoverage*	m_map;
};


class CDataModel : public QAbstractTableModel
{
public:
	CDataModel(QWidget* pw) : QAbstractTableModel(pw), m_fem(0) {}

	void SetFEModel(Post::FEPostModel* pfem)
	{
		beginResetModel();
		m_fem = pfem;
		endResetModel();
	}

	Post::FEPostModel* GetFSModel() { return m_fem; }

	int rowCount(const QModelIndex& index) const
	{
		if (m_fem == 0) return 0;
		Post::FEDataManager& dm = *m_fem->GetDataManager();
		return dm.DataFields();
	}

	int columnCount(const QModelIndex& index) const { return 4; }

	QVariant headerData(int section, Qt::Orientation orient, int role) const
	{
		if ((orient == Qt::Horizontal) && (role == Qt::DisplayRole))
		{
			switch (section)
			{
			case 0: return QVariant(QString("Data field")); break;
			case 1: return QVariant(QString("Type")); break;
			case 2: return QVariant(QString("Class")); break;
			case 3: return QVariant(QString("Format")); break;
			}
		}
		return QAbstractTableModel::headerData(section, orient, role);
	}

	QVariant data(const QModelIndex& index, int role) const
	{
		const char* szclass[] = { "NODE", "FACE", "ELEM" };
		if (m_fem == 0) return QVariant();

		if (!index.isValid()) return QVariant();
		if (role == Qt::DisplayRole)
		{
			int nrow = index.row();
			int ncol = index.column();
			Post::FEDataManager& dm = *m_fem->GetDataManager();
			Post::ModelDataField* pd = *dm.DataField(nrow);

			if (ncol == 0) return QString::fromStdString(pd->GetName());
			else if (ncol == 1) return QString(pd->TypeStr());
			else if (ncol == 2)
			{
				switch (pd->DataClass())
				{
				case NODE_DATA: return QString("NODE"); break;
				case FACE_DATA: return QString("FACE"); break;
				case ELEM_DATA: return QString("ELEM"); break;
				case OBJECT_DATA: return QString("Global"); break;
				default:
					assert(false);
					return QString("(unknown)");
				}
			}
			else if (ncol == 3)
			{
				switch (pd->Format())
				{
				case DATA_NODE  : return QString("NODE"); break;
				case DATA_ITEM  : return QString("ITEM"); break;
				case DATA_MULT  : return QString("MIXED"); break;
				case DATA_REGION: return QString("REGION"); break;
				default:
					assert(false);
					return QString("(unknown)");
				}
			}
		}
		return QVariant();
	}

public:
	Post::FEPostModel*	m_fem;
};

class Ui::CPostDataPanel
{
public:
	CDataModel*	data;
	QTableView*	list;
	::CPropertyListForm*	m_prop;
	QLineEdit*	name;

	Post::ModelDataField*	m_activeField;

public:
	enum FilterType { 
		FILTER_SCALE,
		FILTER_SMOOTH,
		FILTER_ARITHMETIC,
		FILTER_MATHFNC,
		FILTER_GRADIENT,
		FILTER_COMPONENT,
		FILTER_FRACT_ISO,
		FILTER_CONVERT,
		FILTER_EIGEN,
		FILTER_TIME_RATE,
		FILTER_NORM_PROJ,
	};

public:
	void setupUi(::CPostDataPanel* parent)
	{
		m_activeField = 0;

		QVBoxLayout* pg = new QVBoxLayout(parent);
		pg->setContentsMargins(0,0,0,0);

		const int BW = 60;
		const int BH = 23;

		QAction* addActionStd = new QAction("Standard ...", parent); addActionStd->setObjectName("AddStandard");
		QAction* addActionFile = new QAction("From file ...", parent); addActionFile->setObjectName("AddFromFile");
		QAction* addEquation = new QAction("Equation ...", parent); addEquation->setObjectName("AddEquation");
		QAction* addFilter = new QAction("Filter ...", parent); addFilter->setObjectName("AddFilter");

		QPushButton* pbAdd = new QPushButton(); //pbAdd->setFixedSize(BW, BH);
		pbAdd->setText("Add");

		QMenu* menu = new QMenu(parent);
		menu->addAction(addActionStd);
		menu->addAction(addActionFile);
		menu->addAction(addEquation);
		menu->addAction(addFilter);
		pbAdd->setMenu(menu);

		QPushButton* pbCpy = new QPushButton("Copy"); pbCpy->setObjectName("CopyButton"); //pbCpy->setFixedSize(BW, BH); 
		QPushButton* pbDel = new QPushButton("Delete"); pbDel->setObjectName("DeleteButton"); //pbDel->setFixedSize(BW, BH); 
		QPushButton* pbExp = new QPushButton("Export..."); pbExp->setObjectName("ExportButton"); //pbExp->setFixedSize(BW, BH); 

		QHBoxLayout* ph = new QHBoxLayout;
		ph->setSpacing(0);
		ph->addWidget(pbAdd);
		ph->addWidget(pbCpy);
		ph->addWidget(pbDel);
		ph->addWidget(pbExp);
		ph->addStretch();

		pg->addLayout(ph);

		QSplitter* psplitter = new QSplitter;
		psplitter->setOrientation(Qt::Vertical);
		pg->addWidget(psplitter);


		list = new QTableView;
		list->setObjectName(QStringLiteral("dataList"));
		list->setSelectionBehavior(QAbstractItemView::SelectRows);
		list->setSelectionMode(QAbstractItemView::SingleSelection);
		list->horizontalHeader()->setStretchLastSection(true);
		list->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
		//		list->verticalHeader()->setDefaultSectionSize(24);
		list->verticalHeader()->hide();

		data = new CDataModel(list);
		list->setModel(data);

		psplitter->addWidget(list);

		m_prop = new ::CPropertyListForm;
		m_prop->setObjectName("props");

		QWidget* w = new QWidget;
		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(0,0,0,0);
		w->setLayout(l);

		QHBoxLayout* nameLayout = new QHBoxLayout;
		nameLayout->addWidget(new QLabel("name:"));
		nameLayout->addWidget(name = new QLineEdit); name->setObjectName("fieldName");
		l->addLayout(nameLayout);
		QGroupBox* propBox = new QGroupBox("Properties");
		QVBoxLayout* propBoxLayout = new QVBoxLayout;
		propBoxLayout->addWidget(m_prop);
		propBox->setLayout(propBoxLayout);
		l->addWidget(propBox);

		psplitter->addWidget(w);

		QMetaObject::connectSlotsByName(parent);
	}
};

class Ui::CDlgAddDataFile
{
public:
	QLineEdit*	pfile;
	QLineEdit*	pname;
	QComboBox*	pclass;
	QComboBox*	ptype;

public:
	void setupUi(QDialog* parent)
	{
		QPushButton* buttonBrowse = new QPushButton("Browse...");
		buttonBrowse->setFixedWidth(75);
		QHBoxLayout* phb = new QHBoxLayout;
		phb->addWidget(pfile = new QLineEdit);
		phb->addWidget(buttonBrowse);

		QVBoxLayout* pv = new QVBoxLayout;
		QFormLayout* pl = new QFormLayout;
		pl->addRow("File:", phb);
		pl->addRow("Name:", pname = new QLineEdit);
		pname->setText(QString("DataField"));

		pclass = new QComboBox;
		pclass->addItem("Node");
		pclass->addItem("Face");
		pclass->addItem("Elem");

		QHBoxLayout* h = new QHBoxLayout();
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(pclass);
		h->addStretch();
		pl->addRow("Class:", h);

		// NOTE: This assumes the same order as DATA_TYPE enum (see MeshLib\enums.h)
		ptype = new QComboBox;
		ptype->addItem("float (1 fl)");
		ptype->addItem("vec3 (3 fl)");
		ptype->addItem("mat3 (9 fl)");
		ptype->addItem("mat3s (6 fl)");

		h = new QHBoxLayout();
		h->setContentsMargins(0, 0, 0, 0);
		h->addWidget(ptype);
		h->addStretch();
		pl->addRow("Type:", h);

		pv->addLayout(pl);

		QDialogButtonBox* b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(b);

		parent->setLayout(pv);

		QObject::connect(b, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(b, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(buttonBrowse, SIGNAL(clicked()), parent, SLOT(onBrowse()));
	}
};

CDlgAddDataFile::CDlgAddDataFile(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddDataFile)
{
	setWindowTitle("Import Datafile");
	setMinimumSize(600, 300);
	ui->setupUi(this);
	m_nclass = 0;
	m_ntype = 0;
}

void CDlgAddDataFile::accept()
{
	m_file = ui->pfile->text().toStdString();
	m_name = ui->pname->text().toStdString();
	m_nclass = ui->pclass->currentIndex();
	m_ntype = ui->ptype->currentIndex();

	QDialog::accept();
}

void CDlgAddDataFile::onBrowse()
{
	QString file = QFileDialog::getOpenFileName(this, "Open File");
	if (file.isEmpty() == false)
	{
		ui->pfile->setText(file);
	}
}

//=================================================================================================
class Ui::CDlgFilter
{
public:
	QLabel*		src;
	QLineEdit*	name;

	QComboBox* pselect;

	// scale filter
	QLineEdit* pscale;

	// smooth filter
	QLineEdit* ptheta;
	QLineEdit* piters;

	// arithmetic page
	QComboBox* poperation;
	QComboBox* poperand;

	QComboBox*	comp;

	// math function page
	QComboBox* mathFunction;

	// convert page
	QComboBox*	convClass;
	QComboBox*	convFmt;

	// gradient page
	QComboBox* gradConfig = new QComboBox;

public:
	void setupUi(QDialog* parent)
	{
		// the source field
		QHBoxLayout* lsrc = new QHBoxLayout;
		lsrc->addWidget(new QLabel("Source:"));
		lsrc->addWidget(src = new QLabel);

		// new name field
		QHBoxLayout* lname = new QHBoxLayout;
		lname->addWidget(new QLabel("Name:"));
		lname->addWidget(name = new QLineEdit);

		// filter choice
		pselect = new QComboBox;
		pselect->addItem("Scale");
		pselect->addItem("Smooth");
		pselect->addItem("Arithmetic");
		pselect->addItem("Math Function");
		pselect->addItem("Gradient");
		pselect->addItem("Component");
		pselect->addItem("Fraction Anisotropy");
		pselect->addItem("Convert Format");
		pselect->addItem("Eigen vectors");
		pselect->addItem("Time Derivative");
		pselect->addItem("Normal projection");

		QLabel* label;
		label = new QLabel("Filter:");
		label->setBuddy(pselect);

		QHBoxLayout* ph = new QHBoxLayout;
		ph->addWidget(label);
		ph->addWidget(pselect);

		QVBoxLayout* pvl = new QVBoxLayout;
		pvl->addLayout(lsrc);
		pvl->addLayout(lname);
		pvl->addLayout(ph);

		// scale filter
		QWidget* scalePage = new QWidget;
		QFormLayout* pform = new QFormLayout;
		pform->addRow("scale:", pscale = new QLineEdit);
		scalePage->setLayout(pform);

		// smooth filter
		QWidget* smoothPage = new QWidget;
		pform = new QFormLayout;
		pform->addRow("theta:", ptheta = new QLineEdit); ptheta->setValidator(new QDoubleValidator(0.0, 1.0, 6));
		pform->addRow("iterations:", piters = new QLineEdit); piters->setValidator(new QIntValidator(1, 1000));
		smoothPage->setLayout(pform);

		// arithmetic filter
		QWidget* arithmPage = new QWidget;
		pform = new QFormLayout;
		pform->addRow("Operation:", poperation = new QComboBox);
		pform->addRow("Operand:", poperand = new QComboBox);
		arithmPage->setLayout(pform);

		poperation->addItem("add");
		poperation->addItem("subtract");
		poperation->addItem("multiply");
		poperation->addItem("divide");
		poperation->addItem("least-square difference");

		// math function filter
		QWidget* mathPage = new QWidget;
		pform = new QFormLayout;
		pform->addRow("Function:", mathFunction = new QComboBox);
		mathPage->setLayout(pform);

		mathFunction->addItem("negate");
		mathFunction->addItem("abs");
		mathFunction->addItem("ramp");

		// gradient page
		QWidget* gradPage = new QWidget;
		QVBoxLayout* gradLayout = new QVBoxLayout;
		QHBoxLayout* gradRow = new QHBoxLayout;
		gradConfig = new QComboBox;
		gradRow->addWidget(new QLabel("Configuration:"));
		gradRow->addWidget(gradConfig);
		gradRow->addStretch();
		gradLayout->addLayout(gradRow);
		gradPage->setLayout(gradLayout);
		gradConfig->addItems(QStringList() << "Material" << "Spatial");
		gradConfig->setCurrentIndex(1);

		// fractional anisotropy (doesn't need options)
		QWidget* faPage = new QLabel("");

		// array component
		QWidget* compPage = new QWidget;
		pform = new QFormLayout;
		pform->addRow("Component:", comp = new QComboBox);
		compPage->setLayout(pform);

		// format conversion
		QWidget* convPage = new QWidget;
		pform = new QFormLayout;
		pform->addRow("Class:", convClass = new QComboBox);
		pform->addRow("Format:", convFmt = new QComboBox);
		convPage->setLayout(pform);

		// eigenvectors
		QWidget* pcaPage = new QWidget;

		// time derivative
		QWidget* dtPage = new QWidget;

		// normal projection
		QWidget* prjPage = new QWidget;
	

		QStackedWidget* stack = new QStackedWidget;
		stack->addWidget(scalePage);
		stack->addWidget(smoothPage);
		stack->addWidget(arithmPage);
		stack->addWidget(mathPage);
		stack->addWidget(gradPage);
		stack->addWidget(compPage);
		stack->addWidget(faPage);
		stack->addWidget(convPage);
		stack->addWidget(pcaPage);
		stack->addWidget(dtPage);
		stack->addWidget(prjPage);

		pvl->addWidget(stack);

		QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pvl->addWidget(buttonBox);

		parent->setLayout(pvl);

		QObject::connect(pselect, SIGNAL(currentIndexChanged(int)), stack, SLOT(setCurrentIndex(int)));
		QObject::connect(buttonBox, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgFilter::CDlgFilter(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgFilter)
{
	ui->setupUi(this);

	ui->pscale->setText(QString::number(1.0));
	ui->ptheta->setText(QString::number(1.0));
	ui->piters->setText(QString::number(1));
}

void CDlgFilter::setDataOperands(const std::vector<QString>& opNames)
{
	for (int i = 0; i<(int)opNames.size(); ++i)
	{
		ui->poperand->addItem(opNames[i]);
	}
}

void CDlgFilter::setDataField(Post::ModelDataField* pdf)
{
	ui->src->setText(QString::fromStdString(pdf->GetName()));

	m_nsc = pdf->dataComponents(Post::TENSOR_SCALAR);
	for (int i = 0; i < 9; ++i) m_scale[i] = 1.0;

	ui->comp->clear();
	int n = pdf->components(Post::TENSOR_SCALAR);
	for (int i = 0; i<n; ++i)
	{
		std::string cname = pdf->componentName(i, Post::TENSOR_SCALAR);
		ui->comp->addItem(QString::fromStdString(cname));
	}

	DATA_FORMAT frm = pdf->Format();
	ui->convFmt->clear();
	ui->convFmt->addItem("ITEM", (int)DATA_ITEM);
	ui->convFmt->addItem("NODE", (int)DATA_NODE);
	ui->convFmt->addItem("MIXED", (int)DATA_MULT);

	DATA_CLASS cls = pdf->DataClass();
	ui->convClass->clear();

	if      (cls == FACE_DATA) ui->convClass->addItem("Face", (int)FACE_DATA);
	else if (cls == NODE_DATA) ui->convClass->addItem("Node", (int)NODE_DATA);
	else if (cls == ELEM_DATA)
	{
		ui->convClass->addItem("Elem", (int)ELEM_DATA);
		ui->convClass->addItem("Face", (int)FACE_DATA);
		ui->convClass->addItem("Node", (int)NODE_DATA);
	}
}

int CDlgFilter::getArrayComponent()
{
	return ui->comp->currentIndex();
}

void CDlgFilter::setDefaultName(const QString& name)
{
	ui->name->setText(name);
}

QString CDlgFilter::getNewName()
{
	return ui->name->text();
}

int CDlgFilter::getNewDataFormat()
{
	return ui->convFmt->currentData().toInt();
}

int CDlgFilter::getNewDataClass()
{
	return ui->convClass->currentData().toInt();
}

double CDlgFilter::GetScaleFactor() { return m_scale[0]; }
vec3d  CDlgFilter::GetVecScaleFactor() { return vec3d(m_scale[0], m_scale[1], m_scale[2]); }

int CDlgFilter::GetGradientConfiguration()
{
	return ui->gradConfig->currentIndex();
}

int processScale(std::string& s, double* a, int maxa)
{
	int m = 0;
	const char* sz = s.c_str();
	const char* c = sz;
	while (c && (*c))
	{
		if (*c++ == ',')
		{
			a[m++] = atof(sz);
			sz = c;
		}
		if (m >= maxa) return -1;
	}
	if (sz) a[m++] = atof(sz);
	if (m > maxa) return -1;

	return m;
}

void CDlgFilter::accept()
{
	m_nflt = ui->pselect->currentIndex();

	if (m_nflt == 0)
	{
		std::string s = ui->pscale->text().toStdString();
		int m = processScale(s, m_scale, 9);
		if (m <= 0)
		{
			QMessageBox::critical(this, "Data Filter", "Invalid scale factor");
			return;
		}
		if (m == 1)
		{
			for (int i = 1; i < 9; ++i) m_scale[i] = m_scale[0];
		}
		else if (m != m_nsc)
		{
			QMessageBox::critical(this, "Data Filter", "Invalid scale factor");
			return;
		}
	}

	m_theta = ui->ptheta->text().toDouble();
	m_iters = ui->piters->text().toInt();

	m_nop = ui->poperation->currentIndex();
	m_ndata = ui->poperand->currentIndex();

	m_fnc = ui->mathFunction->currentIndex();

	if ((m_nflt == 2) && (m_ndata < 0))
	{
		QMessageBox::critical(this, "Data Filter", "Invalid operand selection");
	}
	else QDialog::accept();
}

//=================================================================================================
CPostDataPanel::CPostDataPanel(CMainWindow* pwnd, QWidget* parent) : CWindowPanel(pwnd, parent), ui(new Ui::CPostDataPanel)
{
	ui->setupUi(this);
}

Post::CGLModel* CPostDataPanel::GetActiveModel()
{
	CGLModelDocument* gldoc = dynamic_cast<CGLModelDocument*>(GetMainWindow()->GetDocument());
	if (gldoc == nullptr) return nullptr;

	Post::CGLModel* glm = gldoc->GetGLModel();
	if (glm && (glm->GetFSModel() == nullptr)) glm = nullptr;

	return glm;
}

void CPostDataPanel::Update(bool breset)
{
	Post::CGLModel* glm = GetActiveModel();
	if (glm)
	{
		Post::FEPostModel* oldFem = ui->data->GetFSModel();
		Post::FEPostModel* newFem = glm->GetFSModel();

		if ((oldFem != newFem) || breset)
		{
			ui->m_prop->setPropertyList(nullptr);
			ui->data->SetFEModel(glm->GetFSModel());
		}
	}
	else
	{
		ui->m_prop->setPropertyList(nullptr);
		ui->data->SetFEModel(nullptr);
	}
}

void CPostDataPanel::on_AddStandard_triggered()
{
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;

	QStringList items;
	CPostDocument* postDoc = dynamic_cast<CPostDocument*>(GetMainWindow()->GetDocument());
	if (postDoc)
	{
		Post::InitStandardDataFields();
		int stdDataFields = Post::StandardDataFields();
		for (int i = 0; i < stdDataFields; ++i)
		{
			std::string dataFieldName = Post::GetStandarDataFieldName(i);
			items.push_back(QString::fromStdString(dataFieldName));
		}
	}
	FEBioMonitorDoc* febDoc = dynamic_cast<FEBioMonitorDoc*>(GetMainWindow()->GetDocument());
	if (febDoc)
	{
		if (febDoc->IsPaused() == false)
		{
			QMessageBox::information(this, "FEBio Monitor", "You can only add data fields when the job is paused.");
			return;
		}

		auto allPlotClasses = FEBio::FindAllClasses(FEBio::GetActiveModule(), FEPLOTDATA_ID);
		for (auto& entry : allPlotClasses)
		{
			items.push_back(entry.sztype);
		}
	}
	
	if (!items.empty())
	{
		items.sort();
		bool ok = false;
		QString item = QInputDialog::getItem(this, "Select new data field", "data:", items, 0, false, &ok);
		if (ok)
		{
			if (postDoc)
			{
				Post::FEPostModel* fem = glm->GetFSModel();
				vector<int> L;
				if (glm->GetSelectionType() == SELECT_FE_FACES)
				{
					glm->GetSelectionList(L, SELECT_FE_FACES);
				}

				if (Post::AddStandardDataField(*fem, item.toStdString(), L) == false)
				{
					QMessageBox::critical(this, "Add Data Field", "Failed adding data");
				}
			}
			if (febDoc)
			{
				if (febDoc->AddDataField(item.toStdString()) == false)
				{
					QMessageBox::critical(this, "Add Data Field", "Failed adding data");
				}
			}

			// update the data list
			GetMainWindow()->UpdatePostToolbar();
			Update(true);
		}
	}
}

void CPostDataPanel::on_AddFromFile_triggered()
{
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "No model data loaded");
		return;
	}

	CDlgAddDataFile dlg(this);
	if (dlg.exec())
	{
		Post::FEPostModel* fem = glm->GetFSModel();
		bool bret = false;
		switch (dlg.m_nclass)
		{
		case 0: bret = Post::AddNodeDataFromFile(*fem, dlg.m_file.c_str(), dlg.m_name.c_str(), dlg.m_ntype); break;
		case 1: bret = Post::AddFaceDataFromFile(*fem, dlg.m_file.c_str(), dlg.m_name.c_str(), dlg.m_ntype); break;
		case 2: bret = Post::AddElemDataFromFile(*fem, dlg.m_file.c_str(), dlg.m_name.c_str(), dlg.m_ntype); break;
		default:
			assert(false);
		}

		if (bret == false)
		{
			QMessageBox::critical(this, "Add Data From File", "Failed reading data from file.");
		}

		// update the data list
		GetMainWindow()->UpdatePostToolbar();
		Update(true);
	}
}

void CPostDataPanel::on_AddEquation_triggered()
{
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;

	CDlgAddEquation dlg(this);
	if (dlg.exec())
	{
		Post::FEPostModel& fem = *glm->GetFSModel();

		QString name = dlg.GetDataName();

		int type = dlg.GetDataType();
		int classType = dlg.GetClassType();

		switch (type)
		{
		case 0:
		{
			QString eq = dlg.GetScalarEquation();
			if (eq.isEmpty()) eq = "";
			if (name.isEmpty()) name = eq;
			if (name.isEmpty()) name = "(empty)";

			// create new math data field
			if (classType == NODE_DATA)
			{
				Post::FEMathNodeDataField* pd = new Post::FEMathNodeDataField(&fem);
				pd->SetEquationString(eq.toStdString());

				// add it to the model
				fem.AddDataField(pd, name.toStdString());
			}
			else if (classType == ELEM_DATA)
			{
				Post::FEMathElemDataField* pd = new Post::FEMathElemDataField(&fem);
				pd->SetEquationString(eq.toStdString());

				// add it to the model
				fem.AddDataField(pd, name.toStdString());
			}
			else QMessageBox::critical(this, "FEBio Studio", "The selected class is not support for scalar expressions.");
		}
		break;
		case 1:
		{
			if (name.isEmpty()) name = "(empty)";

			QStringList s = dlg.GetVectorEquations();

			QString x = s.at(0);
			QString y = s.at(1);
			QString z = s.at(2);

			// create new math data field
			if (classType == NODE_DATA)
			{
				Post::FEMathVec3DataField* pd = new Post::FEMathVec3DataField(&fem);
				pd->SetEquationStrings(x.toStdString(), y.toStdString(), z.toStdString());

				// add it to the model
				Post::FEPostModel& fem = *glm->GetFSModel();
				fem.AddDataField(pd, name.toStdString());
			}
			else QMessageBox::critical(this, "FEBio Studio", "The selected class is not support for vector expressions.");
		}
		break;
		case 2:
		{
			if (name.isEmpty()) name = "(empty)";
			QStringList s = dlg.GetMatrixEquations();

			if (classType == NODE_DATA)
			{
				// create new math data field
				Post::FEMathMat3DataField* pd = new Post::FEMathMat3DataField(&fem);
				for (int i = 0; i < 9; ++i) pd->SetEquationString(i, s.at(i).toStdString());

				// add it to the model
				Post::FEPostModel& fem = *glm->GetFSModel();
				fem.AddDataField(pd, name.toStdString());
			}
			else QMessageBox::critical(this, "FEBio Studio", "The selected class is not support for math expressions.");
		}
		};

		// update the data list
		GetMainWindow()->UpdatePostToolbar();
		Update(true);
	}
}

void CPostDataPanel::on_CopyButton_clicked()
{
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;

	QItemSelectionModel* select = ui->list->selectionModel();
	QModelIndexList selRow = select->selectedRows();
	if (selRow.count() == 1)
	{
		int nsel = selRow.at(0).row();
		Post::FEPostModel& fem = *glm->GetFSModel();
		Post::FEDataManager& dm = *fem.GetDataManager();
		Post::ModelDataField* pdf = *dm.DataField(nsel);
		if (pdf)
		{
			bool bret = false;
			QString name = QString::fromStdString(pdf->GetName());
			QString text = QInputDialog::getText(this, "Copy Data Field", "Name:", QLineEdit::Normal, QString("%1_copy").arg(name), &bret);
			if (bret)
			{
				std::string sname = text.toStdString();
				fem.CopyDataField(pdf, sname.c_str());
				Update(true);
			}
		}
	}
}

void CPostDataPanel::on_DeleteButton_clicked()
{
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;

	QItemSelectionModel* select = ui->list->selectionModel();
	QModelIndexList selRow = select->selectedRows();
	if (selRow.count() == 1)
	{
		int nsel = selRow.at(0).row();

		Post::FEPostModel& fem = *glm->GetFSModel();
		Post::FEDataManager& dm = *fem.GetDataManager();
		Post::ModelDataField* pdf = *dm.DataField(nsel);
		if (pdf)
		{
			QString name = QString::fromStdString(pdf->GetName());
			QString sz(QString("Are you sure you want to delete the \"%1\" data field?").arg(name));
			if (QMessageBox::question(this, "Delete Data Field", sz) == QMessageBox::Yes)
			{
				fem.DeleteDataField(pdf);
				Update(true);
			}
		}
	}
}

void CPostDataPanel::on_AddFilter_triggered()
{
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;

	CMainWindow* wnd = GetMainWindow();
	QItemSelectionModel* select = ui->list->selectionModel();
	QModelIndexList selRow = select->selectedRows();
	if (selRow.count() == 1)
	{
		int nsel = selRow.at(0).row();
		Post::FEPostModel& fem = *glm->GetFSModel();
		Post::FEDataManager& dm = *fem.GetDataManager();
		Post::ModelDataField* pdf = *dm.DataField(nsel);
		if (pdf)
		{
			// build a list of compatible data fields
			vector<QString> dataNames;
			vector<int> dataIds;
			for (int i = 0; i<dm.DataFields(); ++i)
			{
				Post::ModelDataField* pdi = *dm.DataField(i);
				QString name = QString::fromStdString(pdi->GetName());
				if ((pdi != pdf) &&
					(pdi->DataClass() == pdf->DataClass()) &&
					(pdi->Format() == pdf->Format()) &&
					((pdi->Type() == pdf->Type()) || (pdi->Type() == DATA_SCALAR)))
				{
					dataNames.push_back(name);
					dataIds.push_back(i);
				}
			}

			CDlgFilter dlg(this);
			dlg.setDataOperands(dataNames);
			dlg.setDataField(pdf);

			QString name = QString::fromStdString(pdf->GetName());
			QString newName = QString("%0_flt").arg(name);
			dlg.setDefaultName(newName);

			if (dlg.exec())
			{
				// get the name for the new field
				string sname = dlg.getNewName().toStdString();

				Post::ModelDataField* newData = 0;
				bool bret = true;
				int nfield = pdf->GetFieldID();
				switch (dlg.m_nflt)
				{
				case Ui::CPostDataPanel::FILTER_SCALE:
				{
					newData = fem.CreateCachedCopy(pdf, sname.c_str());
					if (pdf->Type() == DATA_VEC3)
						bret = DataScaleVec3(fem, newData->GetFieldID(), dlg.GetVecScaleFactor());
					else
						bret = DataScale(fem, newData->GetFieldID(), dlg.GetScaleFactor());
				}
				break;
				case Ui::CPostDataPanel::FILTER_SMOOTH:
				{
					newData = fem.CreateCachedCopy(pdf, sname.c_str());
					bret = DataSmooth(fem, newData->GetFieldID(), dlg.m_theta, dlg.m_iters);
				}
				break;
				case Ui::CPostDataPanel::FILTER_ARITHMETIC:
				{
					newData = fem.CreateCachedCopy(pdf, sname.c_str());
					Post::FEDataFieldPtr p = fem.GetDataManager()->DataField(dataIds[dlg.m_ndata]);
					bret = DataArithmetic(fem, newData->GetFieldID(), dlg.m_nop, (*p)->GetFieldID());
				}
				break;
				case Ui::CPostDataPanel::FILTER_MATHFNC:
				{
					newData = fem.CreateCachedCopy(pdf, sname.c_str());
					bret = DataMath(fem, newData->GetFieldID(), dlg.m_fnc);
				}
				break;
				case Ui::CPostDataPanel::FILTER_GRADIENT:
				{
					// create new vector field for storing the gradient
					newData = new Post::FEDataField_T<Post::FENodeData<vec3f  > >(&fem);
					newData->SetName(sname);
					fem.AddDataField(newData);

					int config = dlg.GetGradientConfiguration();

					// now, calculate gradient from scalar field
					bret = DataGradient(fem, newData->GetFieldID(), nfield, config);
				}
				break;
				case Ui::CPostDataPanel::FILTER_COMPONENT:
				{
					// create new field for storing the component
					newData = DataComponent(fem, pdf, dlg.getArrayComponent(), sname);
					bret = (newData != nullptr);
				}
				break;
				case Ui::CPostDataPanel::FILTER_FRACT_ISO:
				{
					newData = new Post::FEDataField_T<Post::FEElementData<float, DATA_ITEM> >(&fem);
					newData->SetName(sname);
					fem.AddDataField(newData);

					// calculate fractional anisotropy
					bret = DataFractionalAnsisotropy(fem, newData->GetFieldID(), nfield);
				}
				break;
				case Ui::CPostDataPanel::FILTER_CONVERT:
				{
					int newformat = dlg.getNewDataFormat();
					int newClass  = dlg.getNewDataClass();
					newData = DataConvert(fem, pdf, newClass, newformat, sname);
					bret = (newData != nullptr);
				}
				break;
				case Ui::CPostDataPanel::FILTER_EIGEN:
				{
					newData = DataEigenTensor(fem, pdf, sname);
					bret = (newData != nullptr);
				}
				break;
				case Ui::CPostDataPanel::FILTER_TIME_RATE:
				{
					newData = DataTimeRate(fem, pdf, sname);
					bret = (newData != nullptr);
				}
				break;
				case Ui::CPostDataPanel::FILTER_NORM_PROJ:
				{
					newData = SurfaceNormalProjection(fem, pdf, sname);
					bret = (newData != nullptr);
				}
				break;
				default:
					QMessageBox::critical(this, "Data Filter", "Don't know this filter.");
				}

				if (bret == false)
				{
					if (newData) fem.DeleteDataField(newData);
					QMessageBox::critical(this, "Data Filter", "Cannot apply this filter.");
				}

				wnd->UpdatePostToolbar();
				Update(true);
				glm->Update(true);
				wnd->RedrawGL();
			}
		}
	}
}


//=============================================================================
class Ui::CDlgExportData
{
public:
	QCheckBox* cb;
	QCheckBox* wc;
	QRadioButton* pb1;
	QRadioButton* pb2;
	QRadioButton* pb3;
	QLineEdit* pitems;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;

		cb = new QCheckBox("Selection only");
		l->addWidget(cb);

		wc = new QCheckBox("Write face/element connectivity");
		l->addWidget(wc);

		QVBoxLayout* pg = new QVBoxLayout;
		pg->addWidget(pb1 = new QRadioButton("Write all states"));
		pg->addWidget(pb2 = new QRadioButton("Write current state only"));
		pg->addWidget(pb3 = new QRadioButton("Write states from list:"));
		l->addLayout(pg);

		pb1->setChecked(true);

		l->addWidget(pitems = new QLineEdit);
		l->addWidget(new QLabel("(e.g.:1,2,3:6,10:100:5)"));
		
		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		l->addWidget(bb);
		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgExportData::CDlgExportData(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgExportData())
{
	ui->setup(this);
}

CDlgExportData::~CDlgExportData()
{
	delete ui;
}

bool CDlgExportData::selectionOnly() const
{
	return ui->cb->isChecked();
}

bool CDlgExportData::writeConnectivity() const
{
	return ui->wc->isChecked();
}

int CDlgExportData::stateOutputOption() const
{
	int nop = -1;
	if (ui->pb1->isChecked()) nop = 0;
	if (ui->pb2->isChecked()) nop = 1;
	if (ui->pb3->isChecked()) nop = 2;
	return nop;
}

QString CDlgExportData::stateList() const
{
	return ui->pitems->text();
}

// see DlgFind.cpp
bool string_to_int_list(QString listString, std::vector<int>& list);

//=============================================================================
void CPostDataPanel::on_ExportButton_clicked()
{
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;

	QItemSelectionModel* select = ui->list->selectionModel();
	QModelIndexList selRow = select->selectedRows();
	if (selRow.count() == 1)
	{
		int nsel = selRow.at(0).row();
		Post::FEPostModel& fem = *glm->GetFSModel();
		Post::FEDataManager& dm = *fem.GetDataManager();
		Post::ModelDataField* pdf = *dm.DataField(nsel);
		if (pdf)
		{
			QString file = QFileDialog::getSaveFileName(this, "Export Data");
			if (file.isEmpty() == false)
			{
				CDlgExportData dlg(this);
				if (dlg.exec())
				{
					bool selectionOnly = dlg.selectionOnly();
					bool writeConnect = dlg.writeConnectivity();

					int op = dlg.stateOutputOption();
					vector<int> states;
					switch (op)
					{
					case 0: for (int i = 0; i < fem.GetStates(); ++i) states.push_back(i); break;
					case 1: states.push_back(fem.CurrentTimeIndex()); break;
					case 2:
					{
						QString s = dlg.stateList();
						if (string_to_int_list(s, states) == false)
						{
							QMessageBox::critical(this, "Export Data", "List of export states is not valid.");
							return;
						}
						else
						{
							// make zero-based
							for (int i = 0; i < states.size(); ++i) states[i] -= 1;
						}
					}
					break;
					default:
						assert(false);
						return;
					}

					std::string sfile = file.toStdString();
					if (Post::ExportDataField(fem, *pdf, sfile.c_str(), selectionOnly, writeConnect, states) == false)
					{
						QMessageBox::critical(this, "Export Data", "Export Failed!");
					}
					else
					{
						QMessageBox::information(this, "Export Data", QString("Data successfully exported to:\n%1").arg(file));
					}
				}
			}
		}
	}
	else QMessageBox::warning(this, "Export Data", "Please select a data field first.");
}

void CPostDataPanel::on_dataList_clicked(const QModelIndex& index)
{
	Post::FEPostModel* fem = ui->data->m_fem;
	Post::FEDataManager& dm = *fem->GetDataManager();
	int n = index.row();

	int nstates = fem->GetStates();

	Post::ModelDataField* p = *dm.DataField(n);

	if ((dynamic_cast<Post::CurvatureField*>(p)))
	{
		Post::CurvatureField* pf = dynamic_cast<Post::CurvatureField*>(p);
		ui->m_prop->setPropertyList(new CCurvatureProps(pf));
	}
	else if (dynamic_cast<Post::FEScalarMathDataField*>(p))
	{
		Post::FEScalarMathDataField* pm = dynamic_cast<Post::FEScalarMathDataField*>(p);
		ui->m_prop->setPropertyList(new CMathScalarDataProps(pm));
	}
	else if (dynamic_cast<Post::FEMathVec3DataField*>(p))
	{
		Post::FEMathVec3DataField* pm = dynamic_cast<Post::FEMathVec3DataField*>(p);
		ui->m_prop->setPropertyList(new CMathDataVec3Props(pm));
	}
	else if (dynamic_cast<Post::StrainDataField*>(p))
	{
		Post::StrainDataField* ps = dynamic_cast<Post::StrainDataField*>(p);
		ui->m_prop->setPropertyList(new CStrainProps(ps, nstates));
	}
	else if (dynamic_cast<Post::FEDistanceMap*>(p))
	{
		Post::FEDistanceMap* ps = dynamic_cast<Post::FEDistanceMap*>(p);
		ui->m_prop->setPropertyList(new CDistanceMapProps(ps));
	}
	else if (dynamic_cast<Post::FEAreaCoverage*>(p))
	{
		Post::FEAreaCoverage* ps = dynamic_cast<Post::FEAreaCoverage*>(p);
		ui->m_prop->setPropertyList(new CAreaCoverageProps(ps));
	}
	else ui->m_prop->setPropertyList(nullptr);

	ui->m_activeField = p;

	ui->name->setText(QString::fromStdString(p->GetName()));
}

void CPostDataPanel::on_fieldName_editingFinished()
{
	QString t = ui->name->text();
	if (ui->m_activeField && (t.isEmpty() == false))
	{
		ui->m_activeField->SetName(t.toStdString());
		Update(true);
		CPostDocument* doc = dynamic_cast<CPostDocument*>(GetDocument());
        if(doc) doc->GetFSModel()->UpdateDependants();
	}
}

void CPostDataPanel::on_props_dataChanged(bool b)
{
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;
    Post::FEPostModel* fem = glm->GetFSModel();
    if (fem == nullptr) return;
	
    fem->ResetAllStates();
	glm->Update(true);
	GetMainWindow()->RedrawGL();
}
