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
#include "ModelPropsPanel.h"
#include "PropertyListView.h"
#include "PropertyListForm.h"
#include "ToolBox.h"
#include "SelectionBox.h"
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QBoxLayout>
#include <QMessageBox>
#include <QFormLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QDialogButtonBox>
#include "ModelDocument.h"
#include "MainWindow.h"
#include "ObjectProps.h"
#include <GeomLib/GPrimitive.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include <FEMLib/FELoadController.h>
#include <QGridLayout>
#include <QComboBox>
#include <QCheckBox>
#include "CColorButton.h"
#include "MeshInfoPanel.h"
#include <GLWLib/convert.h>
#include <GeomLib/GGroup.h>
#include <CUILib/ImageViewer.h>
#include <CUILib/HistogramViewer.h>
#include <ImageLib/ImageModel.h>
#include <PostGL/GLPlot.h>
#include <GeomLib/GModel.h>
#include <MeshLib/FEElementData.h>
#include "Commands.h"
#include "MaterialPropsView.h"
#include "FEClassPropsView.h"
#include "PlotWidget.h"
#include "DynamicStackedWidget.h"
#include "ImageFilterWidget.h"
#include "DlgPickNamedSelection.h"
#include "FiberODFWidget.h"
#include <ImageLib/FiberODFAnalysis.h>

//=============================================================================
CObjectPropsPanel::CObjectPropsPanel(QWidget* parent) : QWidget(parent)
{
	QGridLayout* l = new QGridLayout;

	l->addWidget(new QLabel("Name:"), 0, 0, Qt::AlignRight);
	l->addWidget(m_name = new QLineEdit, 0, 1);
	m_name->setObjectName("name");

	l->addWidget(m_col = new CColorButton, 0, 2);
	m_col->setObjectName("col");

	l->addWidget(new QLabel("Type:"), 1, 0, Qt::AlignRight);
	l->addWidget(m_type = new QLabel, 1, 1);

	l->addWidget(m_statusLabel = new QLabel("Active:"), 2, 0, Qt::AlignRight);
	l->addWidget(m_status = new QCheckBox, 2, 1);
	m_status->setObjectName("status");

	setLayout(l);

	QMetaObject::connectSlotsByName(this);
}

void CObjectPropsPanel::setName(const QString& name)
{
	m_name->setText(name);
}

void CObjectPropsPanel::setType(const QString& name)
{
	m_type->setText(name);
}

void CObjectPropsPanel::setColor(const QColor& col)
{
	m_col->setColor(col);
}

void CObjectPropsPanel::showColor(bool b)
{
	m_col->setVisible(b);
}

void CObjectPropsPanel::showStatus(bool b)
{
	m_status->setVisible(b);
	m_statusLabel->setVisible(b);
}

void CObjectPropsPanel::setNameReadOnly(bool b)
{
	m_name->setReadOnly(b);
}

void CObjectPropsPanel::setStatus(bool b)
{
	m_status->setChecked(b);
}

void CObjectPropsPanel::on_name_textEdited(const QString& t)
{
	emit nameChanged(t);
}

void CObjectPropsPanel::on_col_colorChanged(QColor c)
{
	emit colorChanged(c);
}

void CObjectPropsPanel::on_status_clicked(bool b)
{
	emit statusChanged(b);
}

//=============================================================================
CBCObjectPropsPanel::CBCObjectPropsPanel(QWidget* parent) : QWidget(parent)
{
	QGridLayout* l = new QGridLayout;

	l->addWidget(new QLabel("Name:"), 0, 0);
	l->addWidget(m_name = new QLineEdit, 0, 1);
	m_name->setObjectName("name");

	l->addWidget(new QLabel("Type:"), 1, 0);
	l->addWidget(m_type = new QLabel, 1, 1);

	l->addWidget(new QLabel("Step:"), 2, 0);
	l->addWidget(m_list = new QComboBox, 2, 1);
	m_list->setObjectName("list");

	l->addWidget(new QLabel("Active:"), 3, 0, Qt::AlignRight);
	l->addWidget(m_state = new QCheckBox, 3, 1);
	m_state->setObjectName("state");

	setLayout(l);

	QMetaObject::connectSlotsByName(this);
}

void CBCObjectPropsPanel::setStepValues(const vector<pair<QString, int> >& l)
{
	m_list->clear();
	for (size_t i=0; i<l.size(); ++i)
	{
		const pair<QString,int>& item = l[i];
		m_list->addItem(item.first, item.second);
	}
}

void CBCObjectPropsPanel::setStepID(int n)
{
	int nitem = m_list->findData(n);
	m_list->setCurrentIndex(nitem);
}

int CBCObjectPropsPanel::currentStepID()
{
	return m_list->currentData().toInt();
}

void CBCObjectPropsPanel::setName(const QString& name)
{
	m_name->setText(name);
}

void CBCObjectPropsPanel::setType(const QString& name)
{
	m_type->setText(name);
}

void CBCObjectPropsPanel::on_name_textEdited(const QString& t)
{
	emit nameChanged(t);
}

void CBCObjectPropsPanel::on_list_currentIndexChanged(int n)
{
	emit stepChanged(n);
}

void CBCObjectPropsPanel::showActiveState(bool b)
{
	m_state->setVisible(b);
}

void CBCObjectPropsPanel::setActiveState(bool b)
{
	m_state->setChecked(b);
}

void CBCObjectPropsPanel::on_state_toggled(bool b)
{
	emit stateChanged(b);
}

//=============================================================================
CGItemInfoPanel::CGItemInfoPanel(QWidget* parent, bool showActiveOption) : QWidget(parent)
{
	QGridLayout* l = new QGridLayout;

	l->addWidget(new QLabel("Name:"), 0, 0, Qt::AlignRight);
	l->addWidget(m_name = new QLineEdit, 0, 1);
	m_name->setObjectName("name");

	l->addWidget(new QLabel("Type:"), 1, 0, Qt::AlignRight);
	l->addWidget(m_type = new QLabel, 1, 1);

	l->addWidget(new QLabel("ID:"), 2, 0, Qt::AlignRight);
	l->addWidget(m_id = new QLabel, 2, 1);

	if (showActiveOption)
	{
		l->addWidget(new QLabel("Active:"), 3, 0, Qt::AlignRight);
		l->addWidget(m_active = new QCheckBox(""), 3, 1);
	}
	else m_active = nullptr;

	setLayout(l);

	if (m_active) QObject::connect(m_active, &QCheckBox::toggled, this, &CGItemInfoPanel::on_active_changed);
	QObject::connect(m_name, &QLineEdit::textEdited, this, &CGItemInfoPanel::on_name_textEdited);
}

void CGItemInfoPanel::setName(const QString& name)
{
	m_name->setText(name);
}

void CGItemInfoPanel::setType(const QString& name)
{
	m_type->setText(name);
}

void CGItemInfoPanel::setID(int nid)
{
	m_id->setText(QString::number(nid));
}

void CGItemInfoPanel::setActive(bool b)
{
	if (m_active) m_active->setChecked(b);
}

void CGItemInfoPanel::on_active_changed()
{
	if (m_active)
	{
		emit activeChanged(m_active->isChecked());
	}
}

void CGItemInfoPanel::on_name_textEdited(const QString& t)
{
	emit nameChanged(t);
}

//=============================================================================
CMeshDataInfoPanel::CMeshDataInfoPanel(QWidget* parent) : QWidget(parent)
{
	QGridLayout* l = new QGridLayout;

	l->addWidget(new QLabel("Name:"), 0, 0, Qt::AlignRight);
	l->addWidget(m_name = new QLineEdit, 0, 1);
	m_name->setObjectName("name");

	l->addWidget(new QLabel("Type:"), 1, 0, Qt::AlignRight);
	l->addWidget(m_type = new QLabel, 1, 1);

	l->addWidget(new QLabel("Data type:"), 2, 0, Qt::AlignRight);
	l->addWidget(m_dataType = new QLabel, 2, 1);

	l->addWidget(new QLabel("Data format:"), 3, 0, Qt::AlignRight);
	l->addWidget(m_dataFmt = new QLabel, 3, 1);

	setLayout(l);

	QMetaObject::connectSlotsByName(this);
}

void CMeshDataInfoPanel::setName(const QString& name)
{
	m_name->setText(name);
}

void CMeshDataInfoPanel::setType(int data)
{
	switch (data)
	{
	case NODE_DATA: m_type->setText("Node data"   ); break;
	case FACE_DATA: m_type->setText("Surface data"); break;
	case ELEM_DATA: m_type->setText("Element data"); break;
	case PART_DATA: m_type->setText("Part data"   ); break;
	default:
		m_type->setText("(unknown)");
	}
}

void CMeshDataInfoPanel::setDataType(int ndatatype)
{
	switch (ndatatype)
	{
	case DATA_SCALAR: m_dataType->setText("scalar"); break;
	case DATA_VEC3 : m_dataType->setText("vec3"); break;
	case DATA_MAT3 : m_dataType->setText("mat3"); break;
	default:
		m_dataType->setText("(unknown)");
	}
}

void CMeshDataInfoPanel::setDataFormat(int ndataformat)
{
	switch (ndataformat)
	{
	case DATA_ITEM: m_dataFmt->setText("item"); break;
	case DATA_NODE: m_dataFmt->setText("node"); break;
	case DATA_MULT: m_dataFmt->setText("mult"); break;
	default:
		m_dataFmt->setText("(unknown)");
	}
}

void CMeshDataInfoPanel::on_name_textEdited(const QString& t)
{
	emit nameChanged(t);
}

//=============================================================================
class Ui::CModelPropsPanel
{
	enum {
		OBJECT_PANEL,
		BCOBJECT_PANEL,
		GPARTINFO_PANEL,
		GITEMINFO_PANEL,
		MESHDATA_PANEL,
		MESHINFO_PANEL,
		PARTINFO_PANEL,
		PROPS_PANEL,
		SELECTION1_PANEL,
		SELECTION2_PANEL,
		IMAGE_PANEL,
        FIBERODF_PANEL
	};

	enum {
		PROPS_VIEW,
		PROPS_FORM,
//		PROPS_MAT,
		PROPS_FECLASS,
		PROPS_PLOT,
		PROPS_MATH,
		PROPS_MATH_INTERVAL
	};

public:
	QStackedWidget*	stack;
	DynamicStackedWidget*	propStack;
	CItemListSelectionBox* sel1;
	CItemListSelectionBox* sel2;
	::CPropertyListView* props;
	::CPropertyListForm* form;
//	CMaterialPropsView*	mat;
	FEClassEdit*		fec;
	CCurveEditWidget* plt;
	CMathEditWidget* math;
	CMathEditWidget* math2;
    ::CFiberODFWidget* fiberODF;

	CToolBox* tool;
	CObjectPropsPanel*	obj;
	CBCObjectPropsPanel*	bcobj;
	CGItemInfoPanel*		gitemInfo;
	CGItemInfoPanel*		gpartInfo;
	CMeshDataInfoPanel*		data;
	CMeshInfoPanel*	mesh;
	CPartInfoPanel* part;
	QTabWidget* imageTab;

    ::CPropertyListView* imageProps;
    CImageFilterWidget* imageFilters;
	CHistogramViewer*	histoView;

	bool		m_showImageTools;

public:
	void setupUi(::CMainWindow* wnd, QWidget* parent)
	{
		m_showImageTools = false;

		props = new ::CPropertyListView; props->setObjectName("props");
		form  = new ::CPropertyListForm; form->setObjectName("form");
//		mat   = new CMaterialPropsView; mat->setObjectName("mat");
		fec   = new FEClassEdit(wnd); fec->setObjectName("fec");
		plt   = new CCurveEditWidget; plt->setObjectName("plt");
		math  = new CMathEditWidget; math->setObjectName("math");
		math->SetOrdinate("t");

		math2 = new CMathEditWidget; math2->setObjectName("math2");
		math2->SetOrdinate("t");
		math2->showRangeOptions(true);

		obj = new CObjectPropsPanel;
		obj->setObjectName("object");

		bcobj = new CBCObjectPropsPanel;
		bcobj->setObjectName("bcobject");

		gpartInfo = new CGItemInfoPanel(nullptr, true);
		gpartInfo->setObjectName("partInfo");

		gitemInfo = new CGItemInfoPanel;
		gitemInfo->setObjectName("itemInfo");

		data = new CMeshDataInfoPanel;
		data->setObjectName("data");

		propStack = new DynamicStackedWidget;
		propStack->addWidget(props);
		propStack->addWidget(form);
//		propStack->addWidget(mat);
		propStack->addWidget(fec);
		propStack->addWidget(plt);
		propStack->addWidget(math);
		propStack->addWidget(math2);
        propStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

		sel1 = new CItemListSelectionBox;
		sel1->setObjectName("select1");

		sel2 = new CItemListSelectionBox;
		sel2->setObjectName("select2");

		mesh = new CMeshInfoPanel;
		part = new CPartInfoPanel;

		imageTab = new QTabWidget;
        imageTab->addTab(imageProps = new ::CPropertyListView, "Properties");
        imageTab->addTab(imageFilters = new CImageFilterWidget(wnd), "Filters");
		imageTab->addTab(histoView = new CHistogramViewer, "Histogram");

        fiberODF = new ::CFiberODFWidget(wnd);
        fiberODF->setObjectName("fiberODF");

		// compose toolbox
		tool = new CToolBox;
		tool->addTool("Info", obj);
		tool->addTool("Info", bcobj);
		tool->addTool("Info", gpartInfo);
		tool->addTool("Info", gitemInfo);
		tool->addTool("Info", data);
		tool->addTool("Mesh Info", mesh);
		tool->addTool("Mesh Info", part);
		tool->addTool("Properties", propStack);
		tool->addTool("Selection", sel1);
		tool->addTool("Selection", sel2);
		tool->addTool("3D Image", imageTab);
        tool->addTool("Fiber ODF Analysis", fiberODF);

		// hide all panels initially
//		tool->getToolItem(OBJECT_PANEL)->setVisible(false);
		tool->getToolItem(BCOBJECT_PANEL)->setVisible(false);
		tool->getToolItem(MESHINFO_PANEL)->setVisible(false);
		tool->getToolItem(PARTINFO_PANEL)->setVisible(false);
		tool->getToolItem(MESHDATA_PANEL)->setVisible(false);
//		tool->getToolItem(PROPS_PANEL)->setVisible(false);
		tool->getToolItem(SELECTION1_PANEL)->setVisible(false);
		tool->getToolItem(SELECTION2_PANEL)->setVisible(false);
        tool->getToolItem(IMAGE_PANEL)->setVisible(false);
        tool->getToolItem(FIBERODF_PANEL)->setVisible(false);

		stack = new QStackedWidget;
		QLabel* label = new QLabel("");
		label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		stack->addWidget(label);
		stack->addWidget(tool);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->setContentsMargins(0,0,0,0);
		mainLayout->addWidget(stack);
		parent->setLayout(mainLayout);

		QMetaObject::connectSlotsByName(parent);
	}

	void showObjectInfo(bool b, bool showColor = false, bool editName = true, QColor col = QColor(0,0,0), bool showActive = false, bool isActive = false) 
	{ 
		obj->showColor(showColor);
		obj->showStatus(showActive);
		if (showActive) obj->setStatus(isActive);
		if (showColor) obj->setColor(col);
		tool->getToolItem(OBJECT_PANEL)->setVisible(b); 
		obj->setNameReadOnly(!editName);
	}

	void showBCObjectInfo(bool b, bool showActiveState = false, bool isActive = false)
	{
		tool->getToolItem(BCOBJECT_PANEL)->setVisible(b);
		if (showActiveState)
			bcobj->setActiveState(isActive);
	}

	void showGItemInfo(bool b, const QString& name = "", const QString& type = "", int nid = -1)
	{
		if (b)
		{
			gitemInfo->setName(name);
			gitemInfo->setType(type);
			gitemInfo->setID(nid);
		}
		tool->getToolItem(GITEMINFO_PANEL)->setVisible(b);
	}

	void showGPartInfo(bool b, const QString& name = "", const QString& type = "", int nid = -1, bool isActive = true)
	{
		if (b)
		{
			gpartInfo->setName(name);
			gpartInfo->setType(type);
			gpartInfo->setID(nid);
			gpartInfo->setActive(isActive);
		}
		tool->getToolItem(GPARTINFO_PANEL)->setVisible(b);
	}

	void showMeshDataInfo(bool b, FEMeshData* meshdata = nullptr)
	{
		if (b && meshdata)
		{
			data->setName(QString::fromStdString(meshdata->GetName()));
			data->setType(meshdata->GetDataClass());
			data->setDataType(meshdata->GetDataType());
			data->setDataFormat(meshdata->GetDataFormat());
		}
		else
		{
			data->setName("");
			data->setType(-1);
			data->setDataType(-1);
			data->setDataFormat(-1);
		}

		tool->getToolItem(MESHDATA_PANEL)->setVisible(b);
	}

	void showPropsPanel(bool b) { tool->getToolItem(PROPS_PANEL)->setVisible(b); }
	void showSelectionPanel1(bool b) { tool->getToolItem(SELECTION1_PANEL)->setVisible(b); }
	void showSelectionPanel2(bool b) { tool->getToolItem(SELECTION2_PANEL)->setVisible(b); }

	void setSelection1Title(const QString& t) { tool->getToolItem(SELECTION1_PANEL)->setTitle(t); }
	void setSelection2Title(const QString& t) { tool->getToolItem(SELECTION2_PANEL)->setTitle(t); }

	void setName(const QString& txt) { obj->setName(txt); }
	void setType(const QString& txt) { obj->setType(txt); }

	void setBCName(const QString& txt) { bcobj->setName(txt); }
	void setBCType(const QString& txt) { bcobj->setType(txt); }

	void setPropertyList(CPropertyList* pl)
	{
		propStack->setCurrentIndex(PROPS_VIEW);
		props->Update(pl);
		form->setPropertyList(0);
//		mat->SetMaterial(nullptr);
		fec->SetFEClass(nullptr, nullptr);
	}

	void setPropertyForm(CPropertyList* pl)
	{
		propStack->setCurrentIndex(PROPS_FORM);
		props->Update(0);
		form->setPropertyList(pl);
//		mat->SetMaterial(nullptr);
		fec->SetFEClass(nullptr, nullptr);
	}

/*	void setMaterialData(GMaterial* pm)
	{
		propStack->setCurrentIndex(PROPS_MAT);
		props->Update(0);
		form->setPropertyList(0);
		mat->SetMaterial(pm);
	}
*/
	void setFEClassData(FSCoreBase* pc, FSModel* fem)
	{
		propStack->setCurrentIndex(PROPS_FECLASS);
		props->Update(0);
		form->setPropertyList(0);
//		mat->SetMaterial(nullptr);
		fec->SetFEClass(pc, fem);
	}

	void showPlotWidget(FSLoadController* plc)
	{
		props->Update(0);
		form->setPropertyList(0);
		fec->SetFEClass(nullptr, nullptr);
		propStack->setCurrentIndex(PROPS_PLOT);
		
		plt->Clear();

		if (plc == nullptr) return;

		LoadCurve* lc = plc->CreateLoadCurve();
		if (lc)
		{
			lc->SetExtendMode(plc->GetParam("extend")->GetIntValue());
			lc->SetInterpolator(plc->GetParam("interpolate")->GetIntValue());
			plt->SetLoadCurve(lc);
		}
	}

	void showMathWidget(FSLoadController* plc)
	{
		props->Update(0);
		form->setPropertyList(0);
		fec->SetFEClass(nullptr, nullptr);
		propStack->setCurrentIndex(PROPS_MATH);
		plt->Clear();
		plt->SetLoadCurve(nullptr);

		if (plc == nullptr) return;

		Param* p = plc->GetParam("math"); assert(p);
		std::string s = p->GetStringValue();
		math->SetMath(QString::fromStdString(s));
	}

	void showMathIntervalWidget(FSLoadController* plc)
	{
		props->Update(0);
		form->setPropertyList(0);
		fec->SetFEClass(nullptr, nullptr);
		propStack->setCurrentIndex(PROPS_MATH_INTERVAL);
		plt->Clear();
		plt->SetLoadCurve(nullptr);

		if (plc == nullptr) return;

		Param* p = plc->GetParam("math"); assert(p);
		std::string s = p->GetStringValue();
		math2->SetMath(QString::fromStdString(s));
		math2->setLeftExtend(plc->GetParam("left_extend")->GetIntValue());
		math2->setRightExtend(plc->GetParam("right_extend")->GetIntValue());
		vector<double> v = plc->GetParam("interval")->GetArrayDoubleValue();
		math2->setMinMaxRange(v[0], v[1]);
	}

    void showFiberODFWidget(bool b, CFiberODFAnalysis* analysis = nullptr)
    {

        // props->Update(nullptr);
		// form->setPropertyList(nullptr);

        fiberODF->setAnalysis(analysis);

        tool->getToolItem(FIBERODF_PANEL)->setVisible(b);
    }

	void showImagePanel(bool b, CImageModel* img = nullptr, CPropertyList* props = nullptr)
	{
		if (b && (m_showImageTools==false))
		{
			m_showImageTools = true;

            imageProps->Update(props);
            imageFilters->SetImageModel(img);
			histoView->SetImageModel(img);
		}
		else if ((b == false) && m_showImageTools)
		{
			m_showImageTools = false;
		}
		tool->getToolItem(IMAGE_PANEL)->setVisible(b);
	}

	void showProperties(bool b)
	{
		if (b == false)
		{
			stack->setCurrentIndex(0);
			setPropertyList(0);
		}
		else
		{
			stack->setCurrentIndex(1);
		}
	}

	CItemListSelectionBox* selectionPanel(int n)
	{
		return (n==0?sel1 : sel2);
	}

	void setStepList(vector<pair<QString, int> >& l)
	{
		bcobj->setStepValues(l);
	}

	void setCurrentStepID(int n)
	{
		bcobj->setStepID(n);
	}

	int current_bcobject_value()
	{
		return bcobj->currentStepID();
	}

	void showMeshInfoPanel(bool b)
	{
		tool->getToolItem(MESHINFO_PANEL)->setVisible(b);
	}

	void showPartInfoPanel(bool b)
	{
		tool->getToolItem(PARTINFO_PANEL)->setVisible(b);
	}

	void setObject(GObject* po)
	{
		mesh->setInfo(po);
	}

	void setPart(GPart* pg)
	{
		part->setInfo(pg);
	}
};

//=============================================================================
CModelPropsPanel::CModelPropsPanel(CMainWindow* wnd, QWidget* parent) : QWidget(parent), m_wnd(wnd), ui(new Ui::CModelPropsPanel)
{
	m_currentObject = 0;
	m_isUpdating = false;
	ui->setupUi(wnd, this);
}

void CModelPropsPanel::Update()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
	if (doc == nullptr) return;

	// rebuild the step list
	FSModel* fem = doc->GetFSModel();
	int N = fem->Steps();
	vector<pair<QString,int> > steps(N);
	for (int i=0; i<N; ++i)
	{
		FSStep* step = fem->GetStep(i);
		steps[i].first = QString::fromStdString(step->GetName());
		steps[i].second = step->GetID();
	}

	m_isUpdating = true;
	ui->setStepList(steps);
	m_isUpdating = false;
}

void CModelPropsPanel::Refresh()
{
	if (m_currentObject)
	{
		m_currentObject->UpdateData(false);
		ui->props->Refresh();
	}
}

void CModelPropsPanel::AssignCurrentSelection()
{
	addSelection(0);
}

void CModelPropsPanel::SetObjectProps(FSObject* po, CPropertyList* props, int flags)
{
	if ((po == 0) && (props == 0))
	{
		ui->showProperties(false);
		m_currentObject = 0;
	}
	else
	{
		ui->showProperties(true);
		ui->showImagePanel(false);
        ui->showFiberODFWidget(false);
		// CImageSource* imgSrc = dynamic_cast<CImageSource*>(po);
		// if (imgSrc)
		// {
		// 	CImageModel* img = imgSrc->GetImageModel();
		// 	if (img)
		// 	{
		// 		ui->showPropsPanel(false);
		// 		ui->showImagePanel(true, img);
		// 		props = nullptr;
		// 	}
		// }

        CImageModel* img = dynamic_cast<CImageModel*>(po);
        if(img)
        {
            ui->showPropsPanel(false);
            ui->showImagePanel(true, img, props);
            props = nullptr;
        }

		m_currentObject = po;
		SetSelection(0, 0);
		SetSelection(1, 0);

		ui->showBCObjectInfo(false);
		ui->showGItemInfo(false);
		ui->showGPartInfo(false);
		ui->showMeshDataInfo(false);

		if (dynamic_cast<GObject*>(m_currentObject))
			ui->showMeshInfoPanel(true);
		else
			ui->showMeshInfoPanel(false);

		if (dynamic_cast<GPart*>(m_currentObject))
		{
			ui->showPartInfoPanel(true);
		}
		else
			ui->showPartInfoPanel(false);

		if (dynamic_cast<FSMaterial*>(m_currentObject))
		{
            // don't show the object info pane
            ui->showObjectInfo(false);
		}
		else
		{
			// set the object's name
			if (m_currentObject)
			{
				QString name = QString::fromStdString(m_currentObject->GetName());
				ui->setName(name);

				std::string stype = CGLDocument::GetTypeString(m_currentObject);
				QString type(stype.c_str());
				ui->setType(type);

				bool nameEditable = !(flags & 0x08);

				// show the color if it's a material or an object
				// TODO: maybe encode that in the flag?
				if (dynamic_cast<GObject*>(po))
				{
					GObject* go = dynamic_cast<GObject*>(po);
					ui->showObjectInfo(true, true, nameEditable, toQColor(go->GetColor()));
					ui->showMeshInfoPanel(true);
					ui->setObject(go);
				}
				else if (dynamic_cast<GMaterial*>(po))
				{
                    GMaterial* mo = dynamic_cast<GMaterial*>(po);
                    ui->showObjectInfo(true, true, nameEditable, toQColor(mo->Diffuse()));
				}
				else if (dynamic_cast<GDiscreteElementSet*>(po))
				{
					GDiscreteElementSet* pd = dynamic_cast<GDiscreteElementSet*>(po);
					bool isActive = pd->IsActive();
					ui->showObjectInfo(true, true, nameEditable, toQColor(pd->GetColor()), true, isActive);
				}
				else if (dynamic_cast<FSStepComponent*>(po))
				{
					FSStepComponent* pc = dynamic_cast<FSStepComponent*>(po);

					ui->showObjectInfo(false, false, nameEditable);

					ui->setBCName(name);
					ui->setBCType(type);
					ui->setCurrentStepID(pc->GetStep());
					ui->showBCObjectInfo(true, true, pc->IsActive());
				}
				else if (dynamic_cast<Post::CGLObject*>(po))
				{
					Post::CGLObject* plot = dynamic_cast<Post::CGLObject*>(po);
					ui->showObjectInfo(true, false, nameEditable, QColor(0, 0, 0), true, plot->IsActive());
				}
				else if (dynamic_cast<GPart*>(po))
				{
					GPart* pg = dynamic_cast<GPart*>(po);
					QString typeStr("Part");
					ui->setPart(pg);
					ui->showObjectInfo(false);
					bool isActive = pg->IsActive();
					ui->showGPartInfo(true, QString::fromStdString(pg->GetName()), typeStr, pg->GetID(), isActive);
				}
				else if (dynamic_cast<GItem*>(po))
				{
					GItem* git = dynamic_cast<GItem*>(po);
					QString typeStr("unknown");
					if (dynamic_cast<GPart*>(git)) {
						typeStr = "Part"; ui->setPart(dynamic_cast<GPart*>(git)); }
					if (dynamic_cast<GFace*>(git)) typeStr = "Surface";
					if (dynamic_cast<GEdge*>(git)) typeStr = "Edge";
					if (dynamic_cast<GNode*>(git)) typeStr = "Node";

					ui->showObjectInfo(false);
					ui->showGItemInfo(true, QString::fromStdString(git->GetName()), typeStr, git->GetID());
				}
				else if (dynamic_cast<FEMeshData*>(po))
				{
					FEMeshData* pd = dynamic_cast<FEMeshData*>(po);
					ui->showMeshDataInfo(true, pd);
					ui->showObjectInfo(false);
				}
				else if (dynamic_cast<CImageAnalysis*>(po))
				{
					CImageAnalysis* ima = dynamic_cast<CImageAnalysis*>(po);
					ui->showObjectInfo(true, false, nameEditable, QColor(), true, ima->IsActive());
				}
				else ui->showObjectInfo(true, false, nameEditable);
			}
			else ui->showObjectInfo(false);
		}

		// show the property list
		if (dynamic_cast<GMaterial*>(po))
		{
            std::string stype = CGLDocument::GetTypeString(m_currentObject);

            GMaterial* mo = dynamic_cast<GMaterial*>(po);
//			ui->setMaterialData(mo);
            ui->setFEClassData(mo->GetMaterialProperties(), mo->GetModel());
            ui->showPropsPanel(true);
		}
		else if (dynamic_cast<FSLoadController*>(po))
		{
			FSLoadController* plc = dynamic_cast<FSLoadController*>(po);
			if (plc && plc->IsType("loadcurve"))
				ui->showPlotWidget(plc);
			else if (plc && plc->IsType("math"))
				ui->showMathWidget(plc);
			else if (plc && plc->IsType("math-interval"))
				ui->showMathIntervalWidget(plc);
			else
			{
				ui->setFEClassData(plc, plc->GetFSModel());
				ui->showPropsPanel(true);
			}
		}
		else if (dynamic_cast<FSModelComponent*>(po))
		{
			FSModelComponent* pc = dynamic_cast<FSModelComponent*>(po);
			ui->setFEClassData(pc, pc->GetFSModel());
			ui->showPropsPanel(true);
		}
		else if (dynamic_cast<GDiscreteSpringSet*>(po))
		{
			GDiscreteSpringSet* pds = dynamic_cast<GDiscreteSpringSet*>(po);
			ui->setFEClassData(pds->GetMaterial(), pds->GetModel()->GetFSModel());
			ui->showPropsPanel(true);
		}
        else if (dynamic_cast<CFiberODFAnalysis*>(po))
        {
            ui->showFiberODFWidget(true, dynamic_cast<CFiberODFAnalysis*>(po));
            ui->setPropertyList(props);
            ui->showPropsPanel(true);
        }
		else if (props)
		{
			if (flags & 1)
				ui->setPropertyForm(props);
			else
				ui->setPropertyList(props);

			ui->showPropsPanel(true);
		}
		else ui->showPropsPanel(false);

		ui->showSelectionPanel1(true); ui->setSelection1Title("Selection");
		ui->showSelectionPanel2(false);

		FSPairedInterface* pi = dynamic_cast<FSPairedInterface*>(m_currentObject);
		if (pi)
		{
			ui->setSelection1Title("Primary");
			ui->setSelection2Title("Secondary");
			ui->showSelectionPanel2(true);
			SetSelection(0, pi->GetPrimarySurface(), true);
			SetSelection(1, pi->GetSecondarySurface(), true);
			return;
		}

		IHasItemLists* hil = dynamic_cast<IHasItemLists*>(m_currentObject);
		if (hil && (hil->GetMeshItemType() != 0))
		{
			ui->showSelectionPanel1(true);

            // GMaterials don't use named selections and can only be assinged to parts
			if (dynamic_cast<GMaterial*>(m_currentObject))
            {
				SetSelection(0, hil->GetItemList(0), false);
            }
			// TODO: This actually prevents the user from making an initial assignment, 
			//       so need to come up with a different solution here. 
/*            // It doesn't make sense to change the selection of FEMeshData objects
            else if(dynamic_cast<FEMeshData*>(m_currentObject))
            {
                CItemListSelectionBox* sel = ui->selectionPanel(0);
                sel->SetItemList(hil->GetItemList(0));
                sel->showNameType(false);
                sel->enableAddButton(false);
                sel->enableRemoveButton(false);
                sel->enableDeleteButton(false);
            }
*/
			else
            {
                SetSelection(0, hil->GetItemList(0), true);
            }
			return;
		}

		FEItemListBuilder* pl = dynamic_cast<FEItemListBuilder*>(m_currentObject);
		if (pl) { 
			SetSelection(0, pl, false); 
			return;
		}

		GDiscreteElementSet* ds = dynamic_cast<GDiscreteElementSet*>(m_currentObject);
		if (ds)
		{
			SetSelection(ds);
			return;
		}

		ui->showSelectionPanel1(false);
		ui->showSelectionPanel2(false);
	}
}

void CModelPropsPanel::SetSelection(int n, FEItemListBuilder* item)
{
	CItemListSelectionBox* sel = ui->selectionPanel(n);
	sel->SetItemList(item);
}

void CModelPropsPanel::SetSelection(int n, FEItemListBuilder* item, bool showNameType)
{
	CItemListSelectionBox* sel = ui->selectionPanel(n);
	sel->SetItemList(item);
	sel->showNameType(showNameType);
}

void CModelPropsPanel::SetSelection(GDiscreteElementSet* set)
{
	// clear the name
	::CSelectionBox* sel = ui->selectionPanel(0);
	sel->showNameType(false);
	sel->enableAddButton(false);
	sel->enableRemoveButton(false);
	sel->enableDeleteButton(false);
	sel->setCollapsed(true);

	// set the type
	sel->setType("Discrete Elements");

	// set the items
	sel->clearData();
	int N = set->size();
	for (int i = 0; i<N; ++i)
	{
		GDiscreteElement& de = set->element(i);
		sel->addData(QString::fromStdString(de.GetName()), i);
	}
}


void CModelPropsPanel::on_select1_addButtonClicked() { addSelection(0); }
void CModelPropsPanel::on_select2_addButtonClicked() { addSelection(1); }

void CModelPropsPanel::addSelection(int n)
{
	// get the document
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());
	if (pdoc == nullptr) return;

	// get the current selection
	FESelection* ps = pdoc->GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return;

	GModel& mdl = *pdoc->GetGModel();

	assert(m_currentObject);
	if (m_currentObject == 0) return;

	IHasItemLists* pil = dynamic_cast<IHasItemLists*>(m_currentObject);
	if (pil)
	{
		// create the item list from the selection
		FEItemListBuilder* pg = ps->CreateItemList();
		if (pg == nullptr)
		{
			QMessageBox::critical(this, "FEBio Studio", "You cannot assign an empty selection.");
			return;
		}

		// get the current item list
		FEItemListBuilder* pl = pil->GetItemList(n);

		// see whether the current list exists or not
		if (pl == nullptr)
		{
			// see if we can assign it
			int itemType = pil->GetMeshItemType();
			if (pg->Supports(itemType) == false)
			{
				QMessageBox::critical(this, "FEBio Studio", "You cannot apply the current selection to this model component.");
				delete pg;
				return;
			}

			// for part data, we need to convert to an FSPartSet
			FEPartData* pd = dynamic_cast<FEPartData*>(m_currentObject);
			if (pd)
			{
				// make sure it's a part list
				GPartList* partList = dynamic_cast<GPartList*>(pg);
				if (partList == nullptr)
				{
					QMessageBox::critical(this, "FEBio Studio", "You cannot apply the current selection to this model component.");
					delete pg;
					return;
				}

				// extract the part set
				FSPartSet* partSet = partList->BuildPartSet();
				if (partSet == nullptr)
				{
					QMessageBox::critical(this, "FEBio Studio", "You cannot apply the current selection to this model component.");
					delete pg;
					return;
				}

				// make sure its on the same object
				if (pd->GetMesh() != partSet->GetMesh())
				{
					QMessageBox::critical(this, "FEBio Studio", "You cannot apply the current selection to this model component.");
					delete pg;
					return;
				}

				// don't forget to add it to the object
				GObject* po = pd->GetMesh()->GetGObject();
				po->AddFEPartSet(partSet);

				// ok, we're good
				delete pg;
				pg = partSet;
			}

			// for model components and mesh data, we need to give this new list a name and add it to the model
			FSModelComponent* pmc = dynamic_cast<FSModelComponent*>(m_currentObject);
			FEMeshData* pmd = dynamic_cast<FEMeshData*>(m_currentObject);
			if (pmc || pmd)
			{
				string s = m_currentObject->GetName();
				if (pil->ItemLists() == 2)
				{
					if (n == 0) s += "Primary";
					else s += "Secondary";
				}
				pg->SetName(s);
				mdl.AddNamedSelection(pg);
			}

			pdoc->DoCommand(new CCmdSetItemList(pil, pg, n));
			SetSelection(n, pil->GetItemList(n));

			emit modelChanged();
		}
		else
		{
			// merge with the current list
			if (pg->Type() != pl->Type())
			{
				QMessageBox::critical(this, "FEBio Studio", "The selection is not of the correct type.");
			}
			else
			{
				// for groups, make sure that they are on the same mesh
				FSGroup* pg_prv = dynamic_cast<FSGroup*>(pl);
				FSGroup* pg_new = dynamic_cast<FSGroup*>(pg);
				if (pg_prv && pg_new && (pg_prv->GetMesh() != pg_new->GetMesh()))
				{
					QMessageBox::critical(this, "FEBio Studio", "You cannot assign the current selection.\nThe model component was already assigned to a different mesh.");
				}
				else
				{
					if (pl->GetReferenceCount() > 1)
					{
						const char* szmsg = "This selection is used by multiple model components.\nChanging the selection may affect other components.\nDo you wish to continue?";
						if (QMessageBox::question(this, "FEBio Studio", szmsg, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
						{
							delete pg;
							return;
						}
					}
					vector<int> l = pg->CopyItems();
					pdoc->DoCommand(new CCmdAddToItemListBuilder(pl, l));
				}
			}
			SetSelection(n, pl);
			pil->SetItemList(pl, n);
			delete pg;
		}
		emit selectionChanged();
		return;
	}

	FEItemListBuilder* pl = dynamic_cast<FEItemListBuilder*>(m_currentObject);
	if (pl)
	{
		// create the item list builder
		FEItemListBuilder* pg = ps->CreateItemList();

		// merge with the current list
		if (pg->Type() != pl->Type())
		{
			QMessageBox::critical(this, "FEBio Studio", "The selection is not of the correct type.");
		}
		else
		{
			// for groups, make sure that they are on the same mesh
			FSGroup* pg_prv = dynamic_cast<FSGroup*>(pl);
			FSGroup* pg_new = dynamic_cast<FSGroup*>(pg);
			if (pg_prv && pg_new && (pg_prv->GetMesh() != pg_new->GetMesh()))
			{
				QMessageBox::critical(this, "FEBio Studio", "You cannot assign the current selection.\nThe model component was already assigned to a different mesh.");
			}
			else
			{
				vector<int> l = pg->CopyItems();
				pdoc->DoCommand(new CCmdAddToItemListBuilder(pl, l));
			}
		}
		SetSelection(n, pl);

		// don't forget to clean up
		delete pg;

		emit selectionChanged();
		return;
	}
}

void CModelPropsPanel::on_select1_subButtonClicked() { subSelection(0); }
void CModelPropsPanel::on_select2_subButtonClicked() { subSelection(1); }

void CModelPropsPanel::subSelection(int n)
{
	// get the document
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());

	// get the current selection
	FESelection* ps = pdoc->GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return;

	// get the item list
	FEItemListBuilder* pl = nullptr;

	IHasItemLists* pmc = dynamic_cast<IHasItemLists*>(m_currentObject);
	if (pmc) pl = pmc->GetItemList(n);

	FEItemListBuilder* pil = dynamic_cast<FEItemListBuilder*>(m_currentObject);
	if (pil) pl = pil;

	if (pl)
	{
		if (pl->GetReferenceCount() > 1)
		{
			const char* szmsg = "This selection is used by multiple model components.\nChanging the selection may affect other components.\nDo you wish to continue?";
			if (QMessageBox::question(this, "FEBio Studio", szmsg, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
			{
				return;
			}
		}

		// create the item list builder
		FEItemListBuilder* pg = ps->CreateItemList();

		// subtract from the current list
		if (pg->Type() == pl->Type())
		{
			vector<int> l = pg->CopyItems();
			pdoc->DoCommand(new CCmdRemoveFromItemListBuilder(pl, l));
		}

		if (pmc) pmc->SetItemList(pl, n);

		SetSelection(n, pl);
		delete pg;
		emit selectionChanged();
		return;
	}
}

void CModelPropsPanel::on_select1_delButtonClicked() { delSelection(0); }
void CModelPropsPanel::on_select2_delButtonClicked() { delSelection(1); }

void CModelPropsPanel::delSelection(int n)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());

	FEItemListBuilder* pl = nullptr;

	IHasItemLists* pmc = dynamic_cast<IHasItemLists*>(m_currentObject);
	if (pmc)
	{
		pl = pmc->GetItemList(n);
		if (pl && (pl->GetReferenceCount() > 1))
		{
			const char* szmsg = "This selection is used by multiple model components.\nChanging the selection may affect other components.\nDo you wish to continue?";
			if (QMessageBox::question(this, "FEBio Studio", szmsg, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes)
			{
				return;
			}
		}
	}
	else
	{
		FEItemListBuilder* pil = dynamic_cast<FEItemListBuilder*>(m_currentObject);
		if (pil) pl = pil;
	}

	if (pl)
	{

		CSelectionBox* sel = ui->selectionPanel(n);
		vector<int> items;
		sel->getSelectedItems(items);

		pdoc->DoCommand(new CCmdRemoveFromItemListBuilder(pl, items));

		if (pmc) pmc->SetItemList(pl, n);

		SetSelection(n, pl);
		emit selectionChanged();
	}
}

void CModelPropsPanel::on_select1_selButtonClicked() { selSelection(0); }
void CModelPropsPanel::on_select2_selButtonClicked() { selSelection(1); }

void CModelPropsPanel::on_select1_nameChanged(const QString& t)
{
	FEItemListBuilder* pl = 0;

	IHasItemLists* pmc = dynamic_cast<IHasItemLists*>(m_currentObject);
	if (pmc) pl = pmc->GetItemList(0);
	if (pl == 0) return;

	string sname = t.toStdString();
	pl->SetName(sname);
}

void CModelPropsPanel::on_select1_clearButtonClicked() { clearSelection(0); }
void CModelPropsPanel::on_select2_clearButtonClicked() { clearSelection(1); }

void CModelPropsPanel::on_select1_pickClicked() { PickSelection(0); }
void CModelPropsPanel::on_select2_pickClicked() { PickSelection(1); }

void CModelPropsPanel::PickSelection(int n)
{
	CModelDocument* pdoc = m_wnd->GetModelDocument();
	if (pdoc == nullptr) return;

	IHasItemLists* hil = dynamic_cast<IHasItemLists*>(m_currentObject);

	// find the required mesh type
	int meshType = -1;
	if (hil) meshType = hil->GetMeshItemType();
	else return;

	GModel& gm = *pdoc->GetGModel();

	// build the candidate list
	QStringList names;
	if (meshType & FE_NODE_FLAG)
	{
		auto l = gm.AllNamedSelections(GO_NODE);
		for (auto i : l) names.push_back(QString::fromStdString(i->GetName()));

		l = gm.AllNamedSelections(FE_NODESET);
		for (auto i : l) names.push_back(QString::fromStdString(i->GetName()));
	}
	if ((meshType & FE_FACE_FLAG) || (meshType & FE_NODE_FLAG))
	{
		auto l = gm.AllNamedSelections(GO_FACE);
		for (auto i : l) names.push_back(QString::fromStdString(i->GetName()));

		l = gm.AllNamedSelections(FE_SURFACE);
		for (auto i : l) names.push_back(QString::fromStdString(i->GetName()));
	}
	if (meshType & FE_PART_FLAG)
	{
		auto l = gm.AllNamedSelections(DOMAIN_PART);
		for (auto i : l) names.push_back(QString::fromStdString(i->GetName()));
	}

	// get the current selection
	FEItemListBuilder* pl = nullptr;
	if (hil) pl = hil->GetItemList(n);

	CDlgPickNamedSelection dlg(this);
	dlg.setNameList(names);
	if (pl) dlg.setSelection(QString::fromStdString(pl->GetName()));
	if (dlg.exec())
	{
		QString qs = dlg.getSelection();
		if (qs.isEmpty() == false)
		{
			std::string s = qs.toStdString();
			if ((pl == nullptr) || (s != pl->GetName()))
			{
				pl = gm.FindNamedSelection(s);
				if (hil) hil->SetItemList(pl, n);
				SetSelection(n, pl);
			}
		}
	}
}

void CModelPropsPanel::clearSelection(int n)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());

	FEItemListBuilder* pl = 0;

	if (dynamic_cast<IHasItemLists*>(m_currentObject))
	{
		IHasItemLists* pmc = dynamic_cast<IHasItemLists*>(m_currentObject);
		pl = pmc->GetItemList(n);
		if (pl)
		{
			pdoc->DoCommand(new CCmdRemoveItemListBuilder(pmc, n));
			SetSelection(n, nullptr);
			emit selectionChanged();
		}
	}
}

void CModelPropsPanel::on_select2_nameChanged(const QString& t)
{
	FEItemListBuilder* pl = 0;

	// this is only used by paired interfaces
	FSPairedInterface* pi = dynamic_cast<FSPairedInterface*>(m_currentObject);
	if (pi) pl = pi->GetSecondarySurface();

	if (pl == 0) return;

	string sname = t.toStdString();
	pl->SetName(sname);
}

void CModelPropsPanel::selSelection(int n)
{
	CModelDocument* pdoc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());

	assert(m_currentObject);
	if (m_currentObject == 0) return;

	CSelectionBox* sel = ui->selectionPanel(n);
	
	// get the selection list
	vector<int> l;
	sel->getSelectedItems(l);
	if (l.empty())
	{
		QMessageBox::information(this, "FEBio Studio", "Nothing to select");
		return;
	}

	pdoc->SelectItems(m_currentObject, l, n);
	m_wnd->UpdateToolbar();
	m_wnd->Update();
}

void CModelPropsPanel::on_object_nameChanged(const QString& txt)
{
	if (m_currentObject)
	{
		std::string sname = txt.toStdString();
		m_currentObject->SetName(sname.c_str());

		emit nameChanged(txt);
	}
}

void CModelPropsPanel::on_bcobject_nameChanged(const QString& txt)
{
	if (m_currentObject)
	{
		std::string sname = txt.toStdString();
		m_currentObject->SetName(sname.c_str());

		emit nameChanged(txt);
	}
}

void CModelPropsPanel::on_itemInfo_nameChanged(const QString& txt)
{
	if (m_currentObject)
	{
		std::string sname = txt.toStdString();
		m_currentObject->SetName(sname.c_str());

		emit nameChanged(txt);
	}
}

void CModelPropsPanel::on_partInfo_nameChanged(const QString& txt)
{
	if (m_currentObject)
	{
		std::string sname = txt.toStdString();
		m_currentObject->SetName(sname.c_str());

		emit nameChanged(txt);
	}
}

void CModelPropsPanel::on_partInfo_activeChanged(bool b)
{
	GPart* part = dynamic_cast<GPart*>(m_currentObject);
	if (part && (part->IsActive() != b))
	{
		part->SetActive(b);
		emit dataChanged(false);
	}
}

void CModelPropsPanel::on_data_nameChanged(const QString& txt)
{
	if (m_currentObject)
	{
		std::string sname = txt.toStdString();
		m_currentObject->SetName(sname.c_str());

		emit nameChanged(txt);
	}
}

void CModelPropsPanel::on_object_colorChanged(const QColor& col)
{
	GObject* po = dynamic_cast<GObject*>(m_currentObject);
	if (po)
	{
		po->SetColor(toGLColor(col));
	}

	GMaterial* mo = dynamic_cast<GMaterial*>(m_currentObject);
	if (mo)
	{
		mo->AmbientDiffuse(toGLColor(col));
	}

	GDiscreteObject* pd = dynamic_cast<GDiscreteObject*>(m_currentObject);
	if (pd)
	{
		pd->SetColor(toGLColor(col));
	}

	m_wnd->RedrawGL();
}

void CModelPropsPanel::on_props_dataChanged(int n)
{
	Post::CGLObject* po = dynamic_cast<Post::CGLObject*>(m_currentObject);
	if (po) po->Update();
	m_wnd->RedrawGL();
}

void CModelPropsPanel::on_form_dataChanged(bool itemModified)
{
	m_wnd->RedrawGL();
	emit dataChanged(itemModified);
}

void CModelPropsPanel::on_bcobject_stepChanged(int n)
{
	if (m_isUpdating) return;

	FSStepComponent* pc = dynamic_cast<FSStepComponent*>(m_currentObject);
	if (pc == 0) return;

	int stepId = ui->current_bcobject_value();
	if ((stepId !=-1) && (pc->GetStep() != stepId))
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(m_wnd->GetDocument());

		FSModel* fem = doc->GetFSModel();

		fem->AssignComponentToStep(pc, fem->GetStep(n));

		// Changing the step of a BC requires the whole model tree to be rebuild
		emit dataChanged(true);
	}
}

void CModelPropsPanel::on_bcobject_stateChanged(bool isActive)
{
	if (m_isUpdating) return;

	FSStepComponent* pc = dynamic_cast<FSStepComponent*>(m_currentObject);
	if (pc == 0) return;

	pc->Activate(isActive);

	emit dataChanged(false);
}

void CModelPropsPanel::on_object_statusChanged(bool b)
{
	if (m_isUpdating) return;

	Post::CGLObject* po = dynamic_cast<Post::CGLObject*>(m_currentObject);
	if (po)
	{
		po->Activate(b);
		emit dataChanged(false);
	}

	GDiscreteObject* disc = dynamic_cast<GDiscreteObject*>(m_currentObject);
	if (disc)
	{
		disc->SetActive(b);
	}
	CImageAnalysis* ima = dynamic_cast<CImageAnalysis*>(m_currentObject);
	if (ima)
	{
		ima->Activate(b);
		emit dataChanged(false);
	}
}

void CModelPropsPanel::on_math_mathChanged(QString m)
{
	FSLoadController* plc = dynamic_cast<FSLoadController*>(m_currentObject);
	if (plc && plc->IsType("math"))
	{
		Param* p = plc->GetParam("math"); assert(p);
		p->SetStringValue(m.toStdString());
	}
}

void CModelPropsPanel::on_math2_mathChanged(QString m)
{
	FSLoadController* plc = dynamic_cast<FSLoadController*>(m_currentObject);
	if (plc && plc->IsType("math-interval"))
	{
		Param* p = plc->GetParam("math"); assert(p);
		p->SetStringValue(m.toStdString());
	}
}

void CModelPropsPanel::on_math2_leftExtendChanged(int n)
{
	FSLoadController* plc = dynamic_cast<FSLoadController*>(m_currentObject);
	if (plc && plc->IsType("math-interval"))
	{
		plc->SetParamInt("left_extend", n);
	}
}

void CModelPropsPanel::on_math2_rightExtendChanged(int n)
{
	FSLoadController* plc = dynamic_cast<FSLoadController*>(m_currentObject);
	if (plc && plc->IsType("math-interval"))
	{
		plc->SetParamInt("right_extend", n);
	}
}

void CModelPropsPanel::on_math2_minChanged(double vmin)
{
	FSLoadController* plc = dynamic_cast<FSLoadController*>(m_currentObject);
	if (plc && plc->IsType("math-interval"))
	{
		Param* p = plc->GetParam("interval");
		vector<double> v = p->GetArrayDoubleValue();
		v[0] = vmin;
		p->SetArrayDoubleValue(v);
	}
}

void CModelPropsPanel::on_math2_maxChanged(double vmax)
{
	FSLoadController* plc = dynamic_cast<FSLoadController*>(m_currentObject);
	if (plc && plc->IsType("math-interval"))
	{
		Param* p = plc->GetParam("interval");
		vector<double> v = p->GetArrayDoubleValue();
		v[1] = vmax;
		p->SetArrayDoubleValue(v);
	}
}

void CModelPropsPanel::on_plt_dataChanged()
{
	FSLoadController* plc = dynamic_cast<FSLoadController*>(m_currentObject);
	if (plc && plc->IsType("loadcurve"))
	{
		plc->UpdateData(true);
	}
}
