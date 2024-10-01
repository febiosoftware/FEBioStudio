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
#include "PostModelPanel.h"
#include <QBoxLayout>
#include <QTreeWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QMessageBox>
#include <QHeaderView>
#include <QCheckBox>
#include <QSplitter>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QInputDialog>
#include "MainWindow.h"
#include "Document.h"
#include "PropertyListView.h"
#include <PostLib/GLModel.h>
#include <PostLib/FEPostModel.h>
#include <PostLib/GLObject.h>
#include <PostGL/GLPlot.h>
#include <PostGL/GLPlaneCutPlot.h>
#include <PostGL/GLVectorPlot.h>
#include <PostGL/GLSlicePLot.h>
#include <PostGL/GLIsoSurfacePlot.h>
#include <PostGL/GLLinePlot.h>
#include <PostGL/GLPointPlot.h>
#include <PostGL/GLStreamLinePlot.h>
#include <PostGL/GLParticleFlowPlot.h>
#include <PostGL/GLVolumeFlowPlot.h>
#include <PostGL/GLTensorPlot.h>
#include <ImageLib/3DImage.h>
#include <PostLib/VolumeRenderer.h>
#include <PostLib/ImageSlicer.h>
#include <PostLib/FEKinemat.h>
#include <ImageLib/ImageModel.h>
#include <PostLib/GLImageRenderer.h>
#include <PostLib/MarchingCubes.h>
#include <MeshIO/STLExport.h>
#include <PostGL/GLMirrorPlane.h>
#include <PostGL/GLRuler.h>
#include <PostGL/GLPointProbe.h>
#include <PostGL/GLCurveProbe.h>
#include <PostGL/GLMusclePath.h>
#include <PostGL/GLPlotGroup.h>
#include "ObjectProps.h"
#include <CUILib/ImageViewer.h>
#include <CUILib/HistogramViewer.h>
#include "GLView.h"
#include "PostDocument.h"
#include "GLModelDocument.h"
#include "GraphWindow.h"
#include "Commands.h"
#include <QFileDialog>
#include "DlgImportData.h"

//-----------------------------------------------------------------------------
class CModelProps : public CPropertyList
{
public:
	CModelProps(Post::CGLModel* fem) : m_fem(fem) 
	{
		addProperty("Element subdivisions"       , CProperty::Int)->setIntRange(0, 10).setAutoValue(true);
		addProperty("Render undeformed outline", CProperty::Bool);
		addProperty("Undeformed outline color" , CProperty::Color);
		addProperty("Outline color"            , CProperty::Color);
		addProperty("Node color"               , CProperty::Color);
		addProperty("Selection color"          , CProperty::Color);
		addProperty("Render shells as solids"  , CProperty::Bool);
		addProperty("Shell reference surface"  , CProperty::Enum, "set the shell reference surface")->setEnumValues(QStringList() << "Mid surface" << "bottom surface" << "top surface");
		addProperty("Render beams as solids"   , CProperty::Bool);
		addProperty("Solid beam radius"        , CProperty::Float);
		addProperty("Smoothing angle"          , CProperty::Float);
		addProperty("Render internal surfaces" , CProperty::Bool);
	}

	QVariant GetPropertyValue(int i)
	{
		QVariant v;
		switch (i)
		{
		case 0: v = m_fem->m_nDivs; break;
		case 1: v = m_fem->m_bghost; break;
		case 2: v = toQColor(m_fem->GetGhostColor()); break;
		case 3: v = toQColor(m_fem->m_line_col); break;
		case 4: v = toQColor(m_fem->m_node_col); break;
		case 5: v = toQColor(m_fem->m_sel_col); break;
		case 6: v = m_fem->ShowShell2Solid(); break;
		case 7: v = m_fem->ShellReferenceSurface(); break;
		case 8: v = m_fem->ShowBeam2Solid(); break;
		case 9: v = m_fem->SolidBeamRadius(); break;
		case 10: v = m_fem->GetSmoothingAngle(); break;
		case 11: v = m_fem->RenderInnerSurfaces(); break;
		}
		return v;
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		switch (i)
		{
		case 0: m_fem->SetSubDivisions(v.toInt()); break;
		case 1: m_fem->m_bghost   = v.toBool(); break;
		case 2: m_fem->SetGhostColor(toGLColor(v.value<QColor>())); break;
		case 3: m_fem->m_line_col = toGLColor(v.value<QColor>()); break;
		case 4: m_fem->m_node_col = toGLColor(v.value<QColor>()); break;
		case 5: m_fem->m_sel_col = toGLColor(v.value<QColor>()); break;
		case 6: m_fem->ShowShell2Solid(v.toBool()); break;
		case 7: m_fem->ShellReferenceSurface(v.toInt()); break;
		case 8: m_fem->ShowBeam2Solid(v.toBool()); break;
		case 9: m_fem->SolidBeamRadius(v.toFloat()); break;
		case 10: m_fem->SetSmoothingAngle(v.toDouble());  break;
		case 11: m_fem->RenderInnerSurfaces(v.toBool()); break;
		}
	}

private:
	Post::CGLModel*	m_fem;
};

//-----------------------------------------------------------------------------
class CMeshProps : public CPropertyList
{
public:
	CMeshProps(Post::FEPostModel* fem) : m_fem(fem)
	{
		Post::FEPostMesh& mesh = *fem->GetFEMesh(0);
		addProperty("Nodes"         , CProperty::Int, "Number of nodes"         )->setFlags(CProperty::Visible);
		addProperty("Faces"         , CProperty::Int, "Number of faces"         )->setFlags(CProperty::Visible);
		addProperty("Solid Elements", CProperty::Int, "Number of solid elements")->setFlags(CProperty::Visible);
		addProperty("Shell Elements", CProperty::Int, "Number of shell elements")->setFlags(CProperty::Visible);
		addProperty("Beam Elements" , CProperty::Int, "Number of beam elements" )->setFlags(CProperty::Visible);
	}

	QVariant GetPropertyValue(int i)
	{
		QVariant v;
		if (m_fem)
		{
			Post::FEPostMesh& mesh = *m_fem->CurrentState()->GetFEMesh();
			switch (i)
			{
			case 0: v = mesh.Nodes(); break;
			case 1: v = mesh.Faces(); break;
			case 2: v = mesh.SolidElements(); break;
			case 3: v = mesh.ShellElements(); break;
			case 4: v = mesh.BeamElements(); break;
			}
		}
		return v;
	}

	void SetPropertyValue(int i, const QVariant& v) { }

private:
	Post::FEPostModel*	m_fem;
};

//-----------------------------------------------------------------------------
class CViewProps : public CPropertyList
{
public:
	CViewProps(CGLView& view) : m_view(view)
	{
		addProperty("X-angle", CProperty::Float);
		addProperty("Y-angle", CProperty::Float);
		addProperty("Z-angle", CProperty::Float);
		addProperty("X-target", CProperty::Float);
		addProperty("Y-target", CProperty::Float);
		addProperty("Z-target", CProperty::Float);
		addProperty("Target distance", CProperty::Float);
		addProperty("Show title", CProperty::Bool);
		addProperty("Show subtitle", CProperty::Bool);
	}

	QVariant GetPropertyValue(int i)
	{
		CGLCamera* cam = m_view.GetCamera();
		if (cam == nullptr) return QVariant();

		quatd q = cam->GetOrientation();
		float w = q.GetAngle()*180.f/PI;
		vec3d v = q.GetVector()*w;

		vec3d r = cam->GetPosition();
		float d = cam->GetTargetDistance();

		switch (i)
		{
		case 0: return v.x; break;
		case 1: return v.y; break;
		case 2: return v.z; break;
		case 3: return r.x; break;
		case 4: return r.y; break;
		case 5: return r.z; break;
		case 6: return d; break;
		case 7: return m_view.isTitleVisible(); break;
		case 8: return m_view.isSubtitleVisible(); break;
		}

		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& val)
	{
		CGLCamera* cam = m_view.GetCamera();
		if (cam == nullptr) return;

		quatd q = cam->GetOrientation();
		float w = q.GetAngle()*180.f/PI;
		vec3d v = q.GetVector()*w;

		vec3d r = cam->GetPosition();
		float d = cam->GetTargetDistance();

		switch (i)
		{
		case 0: v.x = val.toFloat(); break;
		case 1: v.y = val.toFloat(); break;
		case 2: v.z = val.toFloat(); break;
		case 3: r.x = val.toFloat(); break;
		case 4: r.y = val.toFloat(); break;
		case 5: r.z = val.toFloat(); break;
		case 6: d = val.toFloat(); break;
		case 7: m_view.showTitle(val.toBool()); break;
		case 8: m_view.showSubtitle(val.toBool()); break;
		}

		w = PI*v.Length()/180.f; v.Normalize();
		q = quatd(w, v);
		cam->SetOrientation(q);

		cam->SetTarget(r);
		cam->SetTargetDistance(d);

		cam->Update(true);
	}

private:
	CGLView&	m_view;
};

//-----------------------------------------------------------------------------
class CCameraTransformProps : public CPropertyList
{
public:
	CCameraTransformProps(GLCameraTransform& cam) : m_cam(cam)
	{
		addProperty("X-angle", CProperty::Float);
		addProperty("Y-angle", CProperty::Float);
		addProperty("Z-angle", CProperty::Float);
		addProperty("X-target", CProperty::Float);
		addProperty("Y-target", CProperty::Float);
		addProperty("Z-target", CProperty::Float);
		addProperty("Target distance", CProperty::Float);
	}

	QVariant GetPropertyValue(int i)
	{
		quatd q = m_cam.rot;
		float w = q.GetAngle()*180.f/PI;
		vec3d v = q.GetVector()*w;

		vec3d r = m_cam.pos;
		float d = m_cam.trg.z;

		switch (i)
		{
		case 0: return v.x; break;
		case 1: return v.y; break;
		case 2: return v.z; break;
		case 3: return r.x; break;
		case 4: return r.y; break;
		case 5: return r.z; break;
		case 6: return d; break;
		}

		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& val)
	{
		quatd q = m_cam.rot;
		float w = q.GetAngle()*180.f/PI;
		vec3d v = q.GetVector()*w;

		vec3d r = m_cam.pos;
		float d = m_cam.trg.z;

		switch (i)
		{
		case 0: v.x = val.toFloat(); break;
		case 1: v.y = val.toFloat(); break;
		case 2: v.z = val.toFloat(); break;
		case 3: r.x = val.toFloat(); break;
		case 4: r.y = val.toFloat(); break;
		case 5: r.z = val.toFloat(); break;
		case 6: d = val.toFloat(); break;
		}

		w = PI*v.Length()/180.f; v.Normalize();
		q = quatd(w, v);
		m_cam.rot = q;

		m_cam.pos = r;
		m_cam.trg.z = d;
	}

private:
	GLCameraTransform&	m_cam;
};

//-----------------------------------------------------------------------------
class CImageModelProps : public CPropertyList
{
public:
	CImageModelProps(CImageModel* img)
	{
		m_img = img;

		addProperty("show box", CProperty::Bool);
		addProperty("x-min", CProperty::Float);
		addProperty("y-min", CProperty::Float);
		addProperty("z-min", CProperty::Float);
		addProperty("x-max", CProperty::Float);
		addProperty("y-max", CProperty::Float);
		addProperty("z-max", CProperty::Float);

		addProperty("dimensions", CProperty::String)->setFlags(CProperty::Visible);
	}

	QVariant GetPropertyValue(int i)
	{
		BOX box = m_img->GetBoundingBox();
		switch (i)
		{
		case 0: return m_img->ShowBox(); break;
		case 1: return box.x0; break;
		case 2: return box.y0; break;
		case 3: return box.z0; break;
		case 4: return box.x1; break;
		case 5: return box.y1; break;
		case 6: return box.z1; break;
		case 7: 
		{
			C3DImage* im = m_img->Get3DImage();
			if (im)
			{
				return QString("%1,%2,%3").arg(im->Width()).arg(im->Height()).arg(im->Depth());
			}
			else return QString("0,0,0");
		}
		break;
		}
		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& val)
	{
		BOX box = m_img->GetBoundingBox();
		switch (i)
		{
		case 0: m_img->ShowBox(val.toBool()); break;
		case 1: box.x0 = val.toFloat(); break;
		case 2: box.y0 = val.toFloat(); break;
		case 3: box.z0 = val.toFloat(); break;
		case 4: box.x1 = val.toFloat(); break;
		case 5: box.y1 = val.toFloat(); break;
		case 6: box.z1 = val.toFloat(); break;
		case 7: break; // I don't think we would ever get here
		}
		m_img->SetBoundingBox(box);
	}

private:
	CImageModel*	m_img;
};

//-----------------------------------------------------------------------------
class CModelTreeItem : public QTreeWidgetItem
{
public:
	enum Flag {
		CANNOT_RENAME = 0x01,
		CANNOT_DELETE = 0x02,
		CANNOT_DISABLE = 0x04,
		IS_ENABLED = 0x08,
		ALL_FLAGS = 0x0F
	};

	struct Flags
	{
	public:
		Flags() { m_flags = 0; }
		Flags(Flag flag) { m_flags = flag; }
		Flags(const Flags& flags) { m_flags = flags.m_flags; }

		Flags operator | (const Flags& flags)
		{
			Flags newFlags;
			newFlags.m_flags = m_flags;
			newFlags.m_flags |= flags.m_flags;
			return newFlags;
		}

		Flags operator |= (const Flags& flags)
		{
			m_flags |= flags.m_flags;
			return *this;
		}

		bool HasFlag(Flag flag) const { return (m_flags & flag); }

	private:
		unsigned int m_flags;
	};

public:
	CModelTreeItem(FSObject* po, QTreeWidget* tree) : QTreeWidgetItem(tree), m_po(po), m_propList(nullptr) {}
	CModelTreeItem(FSObject* po, QTreeWidgetItem* item) : QTreeWidgetItem(item), m_po(po), m_propList(nullptr) {}
	~CModelTreeItem() { if (m_propList) delete m_propList; }

	FSObject* Object() { return m_po; }

	void SetObject(FSObject* po) { m_po = po; }

	void SetPropertyList(CPropertyList* propList)
	{
		m_propList = propList;
	}

	CPropertyList* GetPropertyList() { return m_propList; }

	void SetFlags(const Flags& flags) { m_flags = flags; }

	bool CanDelete() { return (m_flags.HasFlag(CANNOT_DELETE) == false); }
	bool CanRename() { return (m_flags.HasFlag(CANNOT_RENAME) == false); }
	bool CanDisable() { return (m_flags.HasFlag(CANNOT_DISABLE) == false); }

	bool IsEnabled() const { return (m_flags.HasFlag(IS_ENABLED)); }

private:
	FSObject* m_po;
	CPropertyList*	m_propList;
	Flags m_flags;
};

CModelTreeItem::Flags operator | (CModelTreeItem::Flag left, CModelTreeItem::Flag right)
{
	return CModelTreeItem::Flags(left) | CModelTreeItem::Flags(right);
}

//-----------------------------------------------------------------------------
class CPostModelTree : public QTreeWidget
{
public:
	CPostModelTree(::CPostModelPanel* view) : m_view(view) {}
	void contextMenuEvent(QContextMenuEvent* ev) override
	{
		m_view->ShowContextMenu(ev);
	}

private:
	CPostModelPanel*	m_view;
};

//-----------------------------------------------------------------------------
class Ui::CPostModelPanel
{
public:
	CPostModelTree*			m_tree;
	::CPropertyListView*	m_props;

	CImageViewer*	m_imgView;

	CHistogramViewer* m_histo;

	QTabWidget*	m_tab;


	QLineEdit* name;

	QPushButton* autoUpdate;
	QPushButton* apply;
	QPushButton* delButton;

public:
	void setupUi(::CPostModelPanel* parent)
	{
		QVBoxLayout* pg = new QVBoxLayout(parent);
		pg->setContentsMargins(0,0,0,0);

		QSplitter* psplitter = new QSplitter;
		psplitter->setOrientation(Qt::Vertical);
		pg->addWidget(psplitter);

		m_tree = new CPostModelTree(parent);
		m_tree->setUniformRowHeights(true);
		m_tree->setObjectName(QStringLiteral("postModel"));
		m_tree->setColumnCount(1);
		m_tree->setHeaderHidden(true);
		m_tree->setColumnCount(2);
//		m_tree->setStyleSheet("QTreeView::item { border: 1px solid #d9d9d9; border-left-color: transparent; border-top-color: transparent; border-bottom-color: transparent;}");
		m_tree->header()->setDefaultSectionSize(16);
		m_tree->header()->setMinimumSectionSize(16);
		m_tree->header()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
		m_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
		m_tree->header()->setStretchLastSection(false);

		QWidget* w = new QWidget;
		QVBoxLayout* pvl = new QVBoxLayout;
		
    pvl->setContentsMargins(0,0,0,0);
		
		QHBoxLayout* phl = new QHBoxLayout;
		phl->addWidget(new QLabel("name:"));
		phl->addWidget(name = new QLineEdit); name->setObjectName("nameEdit");
		delButton = new QPushButton("Delete"); delButton->setObjectName("deleteButton");
		phl->addWidget(delButton);

		/*		autoUpdate = new QPushButton; autoUpdate->setObjectName("autoUpdate"); autoUpdate->setToolTip("Auto-update");
				autoUpdate->setIcon(QIcon(":/icons/auto.png")); autoUpdate->setFixedWidth(20);
				autoUpdate->setCheckable(true);

				apply = new QPushButton; apply->setObjectName("applyButton"); autoUpdate->setToolTip("Update");
				apply->setIcon(QIcon(":/icons/apply.png"));

				QHBoxLayout* bl = new QHBoxLayout;
				bl->setSpacing(0);
				bl->addStretch();
				bl->addWidget(autoUpdate);
				bl->addWidget(apply);

				phl->addLayout(bl);
		*/

		pvl->addLayout(phl);

		m_tab = new QTabWidget;
		m_props = new ::CPropertyListView;
		m_props->setObjectName("props");
		m_tab->addTab(m_props, "Properties");
		//		tab->setTabPosition(QTabWidget::West);

		m_imgView = new CImageViewer;
		//		m_tab->addTab(m_imgView, "Image Viewer");

		m_histo = new CHistogramViewer;

		m_imgView->hide();
		m_histo->hide();

		pvl->addWidget(m_tab);
		w->setLayout(pvl);

		psplitter->addWidget(m_tree);
		psplitter->addWidget(w);

		QMetaObject::connectSlotsByName(parent);
	}

	CModelTreeItem* currentItem()
	{
		QTreeWidgetItem* current = m_tree->currentItem();
		CModelTreeItem* item = dynamic_cast<CModelTreeItem*>(current);
		return item;
	}

	FSObject* currentObject()
	{
		CModelTreeItem* item = currentItem();
		if (item == 0) return 0;

		FSObject* po = item->Object();
		return po;
	}

	void ShowImageViewer(CImageModel* img)
	{
		if (m_tab->count() == 1)
		{
			m_tab->addTab(m_imgView, "Image Viewer");
			m_tab->addTab(m_histo, "Histogram");
		}
		m_imgView->SetImageModel(img);
		m_histo->SetImageModel(img);
	}

	void HideImageViewer()
	{
		if (m_tab->count() != 1)
		{
			m_tab->removeTab(2);
			m_tab->removeTab(1);
		}
		m_imgView->SetImageModel(nullptr);
		m_histo->SetImageModel(nullptr);
	}

	CModelTreeItem* AddItem(
		CModelTreeItem* parent,
		FSObject* po,
		QString txt,
		QString icon = "",
		CPropertyList* props = nullptr,
		CModelTreeItem::Flags flags = CModelTreeItem::Flags())
	{
		CModelTreeItem* pi1 = nullptr;
		if (parent == nullptr) pi1 = new CModelTreeItem(po, m_tree);
		else pi1 = new CModelTreeItem(po, parent);

		pi1->SetFlags(flags);
		pi1->setText(0, txt);
		if (icon.isEmpty() == false) pi1->setIcon(0, QIcon(QString(":/icons/") + icon + ".png"));

		if (pi1->CanDisable())
		{
			if (pi1->IsEnabled())
				pi1->setIcon(1, QIcon(":/icons/check.png"));
			else
				pi1->setIcon(1, QIcon(":/icons/disabled.png"));
		}

		pi1->SetPropertyList(props);


		updateItem(pi1);

		return pi1;
	}

	void setCurrentObject(FSObject* po)
	{
		m_tree->clearSelection();
		m_tree->setCurrentItem(nullptr);
		if (po == nullptr) return;

		std::string name = po->GetName();
		QString s(name.c_str());
		QTreeWidgetItemIterator it(m_tree);
		while (*it)
		{
			QString t = (*it)->text(0);
			string st = t.toStdString();
			if ((*it)->text(0) == s)
			{
				(*it)->setSelected(true);
				m_tree->setCurrentItem(*it);
				break;
			}
			++it;
		}
//		m_props->Refresh();
	}

	void clear()
	{
		// hide the image viewer
		HideImageViewer();

		name->clear();

		// rebuild the tree
		m_props->Update(0);
		m_tree->clear();
	}

	void updateItem(CModelTreeItem* item)
	{
		if (item == nullptr) return;

		Post::CGLObject* po = dynamic_cast<Post::CGLObject*>(item->Object());
		if (po == 0) return;

		if (po->IsActive())
		{
			QFont font = item->font(0);
			font.setItalic(false);
			item->setFont(0, font);
			item->setIcon(1, QIcon(":/icons/check.png"));
		}
		else
		{
			QFont font = item->font(0);
			font.setItalic(true);
			item->setFont(0, font);
			item->setIcon(1, QIcon(":/icons/disabled.png"));
		}
	}
};

CPostModelPanel::CPostModelPanel(CMainWindow* pwnd, QWidget* parent) : CWindowPanel(pwnd, parent), ui(new Ui::CPostModelPanel)
{
	ui->setupUi(this);

	QObject::connect(this, SIGNAL(postObjectStateChanged()), pwnd, SLOT(OnPostObjectStateChanged()));
	QObject::connect(this, SIGNAL(postObjectPropsChanged(FSObject*)), pwnd, SLOT(OnPostObjectPropsChanged(FSObject*)));
}

CGLModelDocument* CPostModelPanel::GetActiveDocument()
{
	return dynamic_cast<CGLModelDocument*>(GetMainWindow()->GetDocument());
}

void CPostModelPanel::selectObject(FSObject* po)
{
	ui->setCurrentObject(po);
}

FSObject* CPostModelPanel::selectedObject()
{
	return ui->currentObject();
}

void CPostModelPanel::UpdateView()
{
	QTreeWidgetItem* psel = ui->m_tree->currentItem();
	if (psel && (psel->text(0) == QString("View")))
	{
		on_postModel_currentItemChanged(psel, psel);
	}
}

void CPostModelPanel::Update(bool breset)
{
	if (breset)
	{
		BuildModelTree();
	}
	else
	{
		on_postModel_currentItemChanged(ui->m_tree->currentItem(), 0);
	}
}

void setPlotIcon(Post::CGLPlot* plot, CModelTreeItem* it)
{
	if      (dynamic_cast<Post::CGLPlaneCutPlot    *>(plot)) it->setIcon(0, QIcon(QString(":/icons/cut.png")));
	else if (dynamic_cast<Post::CGLVectorPlot      *>(plot)) it->setIcon(0, QIcon(QString(":/icons/vectors.png")));
	else if (dynamic_cast<Post::CGLSlicePlot       *>(plot)) it->setIcon(0, QIcon(QString(":/icons/sliceplot.png")));
	else if (dynamic_cast<Post::CGLIsoSurfacePlot  *>(plot)) it->setIcon(0, QIcon(QString(":/icons/isosurface.png")));
	else if (dynamic_cast<Post::CGLStreamLinePlot  *>(plot)) it->setIcon(0, QIcon(QString(":/icons/streamlines.png")));
	else if (dynamic_cast<Post::CGLParticleFlowPlot*>(plot)) it->setIcon(0, QIcon(QString(":/icons/particle.png")));
	else if (dynamic_cast<Post::GLVolumeFlowPlot   *>(plot)) it->setIcon(0, QIcon(QString(":/icons/flow.png")));
	else if (dynamic_cast<Post::GLTensorPlot       *>(plot)) it->setIcon(0, QIcon(QString(":/icons/tensor.png")));
	else if (dynamic_cast<Post::CGLMirrorPlane     *>(plot)) it->setIcon(0, QIcon(QString(":/icons/mirror.png")));
	else if (dynamic_cast<Post::GLPointProbe       *>(plot)) it->setIcon(0, QIcon(QString(":/icons/probe.png")));
	else if (dynamic_cast<Post::GLRuler            *>(plot)) it->setIcon(0, QIcon(QString(":/icons/ruler.png")));
	else if (dynamic_cast<Post::CGLLinePlot        *>(plot)) it->setIcon(0, QIcon(QString(":/icons/wire.png")));
	else if (dynamic_cast<Post::CGLPointPlot       *>(plot)) it->setIcon(0, QIcon(QString(":/icons/selectNodes.png")));
	else if (dynamic_cast<Post::GLMusclePath       *>(plot)) it->setIcon(0, QIcon(QString(":/icons/musclepath.png")));
	else if (dynamic_cast<Post::GLPlotGroup        *>(plot)) it->setIcon(0, QIcon(QString(":/icons/folder.png")));
}

void CPostModelPanel::BuildModelTree()
{
	ui->clear();
	CGLModelDocument* pdoc = GetActiveDocument();
	if (pdoc && pdoc->IsValid())
	{
		Post::CGLModel* mdl = pdoc->GetGLModel();
		Post::FEPostModel* fem = mdl->GetFSModel();
		GObject* po = pdoc->GetActiveObject();

		CModelTreeItem* pi1 = nullptr;
		if (mdl)
		{
			pi1 = ui->AddItem(nullptr, nullptr, QString::fromStdString(pdoc->GetDocFileName()), "PostView", new CModelProps(mdl), CModelTreeItem::ALL_FLAGS);
			pi1->setExpanded(true);

			// add the mesh
			if (fem)
			{
				Post::FEPostMesh& mesh = *fem->GetFEMesh(0);
				CModelTreeItem* pi2 = ui->AddItem(pi1, mdl, "Mesh", "mesh", new CMeshProps(fem), CModelTreeItem::ALL_FLAGS);

				// add node sets
				int nsets = mesh.NodeSets() + (po ? po->FENodeSets() : 0);
				CModelTreeItem* pi3 = ui->AddItem(pi2, nullptr, QString("Node Sets (%1)").arg(nsets), "", nullptr, CModelTreeItem::ALL_FLAGS);
				for (int i = 0; i < mesh.NodeSets(); ++i)
				{
					Post::FSNodeSet& nset = mesh.NodeSet(i);
					ui->AddItem(pi3, &nset, QString::fromStdString(nset.GetName()), "selNode", nullptr, CModelTreeItem::CANNOT_DISABLE);
				}
				if (po)
				{
					for (int i = 0; i < po->FENodeSets(); ++i)
					{
						FSNodeSet* pg = po->GetFENodeSet(i);
						ui->AddItem(pi3, pg, QString::fromStdString(pg->GetName()), "selNode", nullptr, CModelTreeItem::CANNOT_DISABLE);
					}
				}

				// add edges
				int nedges = (po ? po->FEEdgeSets() : 0);
				pi3 = ui->AddItem(pi2, nullptr, QString("Edges (%1)").arg(nedges), "", nullptr, CModelTreeItem::ALL_FLAGS);
				if (po)
				{
					for (int i = 0; i < po->FEEdgeSets(); ++i)
					{
						FSEdgeSet* pg = po->GetFEEdgeSet(i);
						ui->AddItem(pi3, pg, QString::fromStdString(pg->GetName()), "selEdge", nullptr, CModelTreeItem::CANNOT_DISABLE);
					}
				}

				// add surfaces
				int nsurf = mesh.Surfaces() + (po ? po->FESurfaces() : 0);
				pi3 = ui->AddItem(pi2, nullptr, QString("Surfaces (%1)").arg(nsurf), "", nullptr, CModelTreeItem::ALL_FLAGS);
				for (int i = 0; i < mesh.Surfaces(); ++i)
				{
					Post::FSSurface& surf = mesh.Surface(i);
					ui->AddItem(pi3, &surf, QString::fromStdString(surf.GetName()), "selFace", nullptr, CModelTreeItem::CANNOT_DISABLE);
				}
				if (po)
				{
					for (int i = 0; i < po->FESurfaces(); ++i)
					{
						FSSurface* pg = po->GetFESurface(i);
						ui->AddItem(pi3, pg, QString::fromStdString(pg->GetName()), "selFace", nullptr, CModelTreeItem::CANNOT_DISABLE);
					}
				}

				// add element sets
				int eset = mesh.ElemSets() + (po ? po->FEElemSets() : 0);
				pi3 = ui->AddItem(pi2, nullptr, QString("Element Sets (%1)").arg(eset), "", nullptr, CModelTreeItem::ALL_FLAGS);
				for (int i = 0; i < mesh.ElemSets(); ++i)
				{
					Post::FSElemSet& part = mesh.ElemSet(i);
					ui->AddItem(pi3, &part, QString::fromStdString(part.GetName()), "selElem", nullptr, CModelTreeItem::CANNOT_DISABLE);
				}
				if (po)
				{
					for (int i = 0; i < po->FEElemSets(); ++i)
					{
						FSElemSet* pg = po->GetFEElemSet(i);
						ui->AddItem(pi3, pg, QString::fromStdString(pg->GetName()), "selElem", nullptr, CModelTreeItem::CANNOT_DISABLE);
					}
				}
			}

			Post::CGLDisplacementMap* map = mdl->GetDisplacementMap();
			if (map)
			{
				CModelTreeItem::Flags flags = CModelTreeItem::CANNOT_RENAME;
				if (map->IsActive()) flags |= CModelTreeItem::IS_ENABLED;
				ui->AddItem(pi1, map, QString::fromStdString(map->GetName()), "distort", new CObjectProps(map), flags);
			}

			Post::CGLColorMap* col = mdl->GetColorMap();
			if (col)
			{
				CModelTreeItem::Flags flags = CModelTreeItem::CANNOT_RENAME;
				if (col->IsActive()) flags |= CModelTreeItem::IS_ENABLED;
				ui->AddItem(pi1, col, QString::fromStdString(col->GetName()), "colormap", new CObjectProps(col), flags);
			}

			if (fem && fem->PlotObjects())
			{
				int npo = fem->PlotObjects();
				for (int i = 0; i < npo; ++i)
				{
					Post::FEPostModel::PlotObject* po = fem->GetPlotObject(i);
					CModelTreeItem::Flags flags;
					if (po->IsActive()) flags |= CModelTreeItem::IS_ENABLED;
					ui->AddItem(pi1, po, QString::fromStdString(po->GetName()), "", new CObjectProps(po), flags);
				}
			}

			Post::GPlotList& pl = mdl->GetPlotList();
			for (int n = 0; n < pl.Size(); ++n)
			{
				Post::CGLPlot& plot = *pl[n];
				CModelTreeItem::Flags flags;
				if (plot.IsActive()) flags |= CModelTreeItem::IS_ENABLED;
				CModelTreeItem* pi1 = ui->AddItem(nullptr, &plot, QString::fromStdString(plot.GetName()), "", new CObjectProps(&plot), flags);
				setPlotIcon(&plot, pi1);

				Post::GLPlotGroup* pg = dynamic_cast<Post::GLPlotGroup*>(&plot);
				if (pg)
				{
					for (int i = 0; i < pg->Plots(); ++i)
					{
						Post::CGLPlot* plt = pg->GetPlot(i);
						CModelTreeItem* pi2 = ui->AddItem(pi1, plt, QString::fromStdString(plt->GetName()), "", new CObjectProps(plt));
						setPlotIcon(plt, pi2);
					}
				}
			}
		}

		for (int i = 0; i < pdoc->ImageModels(); ++i)
		{
			CImageModel* img = pdoc->GetImageModel(i);
			pi1 = ui->AddItem(nullptr, img, QString::fromStdString(img->GetName()), "image", new CImageModelProps(img));

			for (int j = 0; j < img->ImageRenderers(); ++j)
			{
				Post::CGLImageRenderer* render = img->GetImageRenderer(j);

				CModelTreeItem::Flags flags;
				if (render->IsActive()) flags |= CModelTreeItem::IS_ENABLED;

				Post::CVolumeRenderer* volRender2 = dynamic_cast<Post::CVolumeRenderer*>(render);
				if (volRender2)
				{
					ui->AddItem(pi1, volRender2, QString::fromStdString(render->GetName()), "volrender", new CObjectProps(volRender2), flags);
				}

				Post::CImageSlicer* imgSlice = dynamic_cast<Post::CImageSlicer*>(render);
				if (imgSlice)
				{
					ui->AddItem(pi1, imgSlice, QString::fromStdString(render->GetName()), "imageslice", new CObjectProps(imgSlice), flags);
				}

				Post::CMarchingCubes* marchCube = dynamic_cast<Post::CMarchingCubes*>(render);
				if (marchCube)
				{
					ui->AddItem(pi1, marchCube, QString::fromStdString(render->GetName()), "marching_cubes", new CObjectProps(marchCube), flags);
				}
			}

			for (int j = 0; j < img->ImageFilters(); ++j)
			{
				CImageFilter* flt = img->GetImageFilter(j);
				if (flt)
				{
					ui->AddItem(pi1, flt, QString::fromStdString(flt->GetName()), "", new CObjectProps(flt));
				}
			}
		}
	
		// view settings
		CGView& view = *pdoc->GetView();
		pi1 = ui->AddItem(nullptr, &view, "View", "view", new CViewProps(*GetMainWindow()->GetGLView()), CModelTreeItem::ALL_FLAGS);

		for (int i=0; i<view.CameraKeys(); ++i)
		{
			GLCameraTransform& key = view.GetKey(i);
			string name = key.GetName();
			ui->AddItem(pi1, &key, QString::fromStdString(name), "view", new CCameraTransformProps(key));
		}

		// saved graphs
		CPostDocument* postDoc = dynamic_cast<CPostDocument*>(GetDocument());
		if (postDoc)
		{
			int n = postDoc->Graphs();
			if (n > 0)
			{
				pi1 = ui->AddItem(nullptr, nullptr, "Saved Graphs", "chart", nullptr, CModelTreeItem::ALL_FLAGS);
				for (int i = 0; i < n; ++i)
				{
					CGraphData* gd = const_cast<CGraphData*>(postDoc->GetGraphData(i));
					ui->AddItem(pi1, gd, QString::fromStdString(gd->GetName()));
				}
			}
		}
	}

	// This can crash if po no longer exists (e.g. after new file is read)
//		if (po) selectObject(po);
}

void CPostModelPanel::on_postModel_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev)
{
	CModelTreeItem* item = dynamic_cast<CModelTreeItem*>(current);
	if (item)
	{
		FSObject* po = item->Object();
		if (po)
		{
			ui->delButton->setEnabled(item->CanDelete());
			if (dynamic_cast<CImageModel*>(po))
			{
				CImageModel* img = dynamic_cast<CImageModel*>(po);
				ui->ShowImageViewer(img);
			}
			else ui->HideImageViewer();
		}
		else 
		{
			ui->HideImageViewer();
			ui->delButton->setEnabled(false);
		}

		ui->name->setText(item->text(0));
		ui->name->setEnabled(item->CanRename());
		ui->m_props->Update(item->GetPropertyList());

		Post::CGLObject* pglo = dynamic_cast<Post::CGLObject*>(po);
		if (pglo)
		{
			if (pglo->IsActive()) item->setIcon(1, QIcon(":/icons/check.png"));
			else item->setIcon(1, QIcon(":/icons/disabled.png"));
		}
	}
	else
	{
		ui->HideImageViewer();
		ui->m_props->Update(0);
		ui->delButton->setEnabled(false);
		ui->name->setText("");
		ui->name->setEnabled(false);
	}
}

void CPostModelPanel::on_postModel_itemClicked(QTreeWidgetItem* treeItem, int column)
{
	if (column != 1) return;
	CModelTreeItem* item = dynamic_cast<CModelTreeItem*>(treeItem);
	if (item->CanDisable() == false) return;
	Post::CGLObject* po = dynamic_cast<Post::CGLObject*>(item->Object());
	if (po == nullptr) return;
	if (po->IsActive())
		po->Activate(false);
	else 
		po->Activate(true);
	ui->updateItem(item);
	emit postObjectStateChanged();
}

void CPostModelPanel::on_postModel_itemDoubleClicked(QTreeWidgetItem* treeItem, int column)
{
	CModelTreeItem* item = dynamic_cast<CModelTreeItem*>(treeItem);
	FSObject* po = item->Object();

	GLCameraTransform* pkey = dynamic_cast<GLCameraTransform*>(po);
	if (pkey)
	{
		CGView* view = GetActiveDocument()->GetView();
		view->SetCurrentKey(pkey);
		GetMainWindow()->RedrawGL();
	}

	CPostDocument* doc = dynamic_cast<CPostDocument*>(GetDocument());
	if (doc == nullptr) return;

	CGraphData* graph = dynamic_cast<CGraphData*>(po);
	if (graph)
	{
		CDataGraphWindow* w = new CDataGraphWindow(GetMainWindow(), doc);
		w->SetData(graph);
		GetMainWindow()->AddGraph(w);
		w->setWindowTitle(QString::fromStdString(graph->GetName()));
		w->show();
	}

	FSNodeSet* pn = dynamic_cast<FSNodeSet*>(po);
	if (pn)
	{
		std::vector<int> items = pn->CopyItems();
		std::vector<int> vitems(items.begin(), items.end());
		doc->SetItemMode(ITEM_NODE);
		doc->DoCommand(new CCmdSelectFENodes(pn->GetMesh(), vitems, false));
	}

	Post::FSNodeSet* pn2 = dynamic_cast<Post::FSNodeSet*>(po);
	if (pn2)
	{
		// the tree is populated from the initial mesh
		// but if the initial mesh is remeshed, we need to find the mesh
		// of the active time step
		CPostDocument* postDoc = dynamic_cast<CPostDocument*>(doc);
		if (postDoc)
		{
			Post::FEPostModel* mdl = postDoc->GetFSModel();
			Post::FEPostMesh* pm = mdl->CurrentState()->GetFEMesh();

			Post::FSNodeSet* pn3 = pm->FindNodeSet(pn2->GetName());
			if (pn3) pn2 = pn3;

			std::vector<int> items = pn2->GetNodeList();
			doc->SetItemMode(ITEM_NODE);
			doc->DoCommand(new CCmdSelectFENodes(pn2->GetMesh(), items, false));
		}
	}

	FSEdgeSet* pe = dynamic_cast<FSEdgeSet*>(po);
	if (pe)
	{
		std::vector<int> items = pe->CopyItems();
		std::vector<int> vitems(items.begin(), items.end());
		doc->SetItemMode(ITEM_EDGE);
		doc->DoCommand(new CCmdSelectFEEdges(pe->GetMesh(), vitems, false));
	}

	FSSurface* ps = dynamic_cast<FSSurface*>(po);
	if (ps)
	{
		std::vector<int> items = ps->CopyItems();
		std::vector<int> vitems(items.begin(), items.end());
		doc->SetItemMode(ITEM_FACE);
		doc->DoCommand(new CCmdSelectFaces(ps->GetMesh(), vitems, false));
	}
	Post::FSSurface* ps2 = dynamic_cast<Post::FSSurface*>(po);
	if (ps2)
	{
		// the tree is populated from the initial mesh
		// but if the initial mesh is remeshed, we need to find the mesh
		// of the active time step
		CPostDocument* postDoc = dynamic_cast<CPostDocument*>(doc);
		if (postDoc)
		{
			Post::FEPostModel* mdl = postDoc->GetFSModel();
			Post::FEPostMesh* pm = mdl->CurrentState()->GetFEMesh();

			Post::FSSurface* ps3 = pm->FindSurface(ps2->GetName());
			if (ps3) ps2 = ps3;

			std::vector<int> items = ps2->GetFaceList();
			doc->SetItemMode(ITEM_FACE);
			doc->DoCommand(new CCmdSelectFaces(ps2->GetMesh(), items, false));
		}
	}

	FSElemSet* pg = dynamic_cast<FSElemSet*>(po);
	if (pg)
	{
		std::vector<int> items = pg->CopyItems();
		std::vector<int> vitems(items.begin(), items.end());
		doc->SetItemMode(ITEM_ELEM);
		doc->DoCommand(new CCmdSelectElements(pg->GetMesh(), vitems, false));
	}
	Post::FSElemSet* pg2 = dynamic_cast<Post::FSElemSet*>(po);
	if (pg2)
	{
		std::vector<int> items = pg2->GetElementList();
		doc->SetItemMode(ITEM_ELEM);
		doc->DoCommand(new CCmdSelectElements(pg2->GetMesh(), items, false));
	}

	GetMainWindow()->UpdateGLControlBar();
	GetMainWindow()->RedrawGL();
}

void CPostModelPanel::on_nameEdit_editingFinished()
{
	QString name = ui->name->text();

	CModelTreeItem* item = ui->currentItem();
	if (item) item->setText(0, name);

	FSObject* po = selectedObject();
	if (dynamic_cast<Post::CGLLegendPlot*>(po))
	{
		Post::CGLLegendPlot* plot = dynamic_cast<Post::CGLLegendPlot*>(po); assert(plot);
		if (plot) plot->ChangeName(name.toStdString());
	}
	else if (po)
	{
		po->SetName(name.toStdString());
	}
	GetMainWindow()->RedrawGL();
}

void CPostModelPanel::on_deleteButton_clicked()
{
	CModelTreeItem* item = ui->currentItem();
	if (item == nullptr) return;

	FSObject* pobj = item->Object();

	CPostDocument* doc = dynamic_cast<CPostDocument*>(GetDocument());
	if (doc == nullptr) return;

	Post::CGLObject* po = dynamic_cast<Post::CGLObject*>(pobj);
	CGraphData* graph = dynamic_cast<CGraphData*>(pobj);
	if (po)
	{
		// TODO: This is a hack to avoid a crash when the image viewer updates itself
		//       after the image model is deleted. 
		ui->HideImageViewer();
		
		doc->DeleteObject(po);
		item->SetObject(0);
		Update(true);
		GetMainWindow()->RedrawGL();
	}
	else if (graph)
	{
		doc->DeleteGraph(graph);
		item->SetObject(0);
		Update(true);
		GetMainWindow()->RedrawGL();
	}
	else if (dynamic_cast<FSNodeSet*>(po))
	{
		GObject* poa = doc->GetActiveObject();
		if (poa)
		{
			poa->RemoveFENodeSet(dynamic_cast<FSNodeSet*>(po));
			Update(true);
		}
	}
	else if (dynamic_cast<FSEdgeSet*>(po))
	{
		GObject* poa = doc->GetActiveObject();
		if (poa)
		{
			poa->RemoveFEEdgeSet(dynamic_cast<FSEdgeSet*>(po));
			Update(true);
		}
	}
	else if (dynamic_cast<FSSurface*>(po))
	{
		GObject* poa = doc->GetActiveObject();
		if (poa)
		{
			poa->RemoveFESurface(dynamic_cast<FSSurface*>(po));
			Update(true);
		}
	}
	else if (dynamic_cast<FSElemSet*>(po))
	{
		GObject* poa = doc->GetActiveObject();
		if (poa)
		{
			poa->RemoveFEElemSet(dynamic_cast<FSElemSet*>(po));
			Update(true);
		}
	}
	else if (dynamic_cast<CImageFilter*>(pobj))
	{
		CImageFilter* imf = dynamic_cast<CImageFilter*>(pobj);
		CImageModel* mdl = imf->GetImageModel();
		mdl->RemoveFilter(imf);
		delete imf;
		Update(true);
	}
	else QMessageBox::information(this, "FEBio Studio", "Cannot delete this object");
}

void CPostModelPanel::on_props_dataChanged()
{
	FSObject* po = selectedObject();
	if (po) po->Update();

	emit postObjectPropsChanged(po);
}

void CPostModelPanel::ShowContextMenu(QContextMenuEvent* ev)
{
	FSObject* po = ui->currentObject();
	if (po == nullptr) return;

	Post::CGLModel* pmdl = dynamic_cast<Post::CGLModel*>(po);
	if (pmdl)
	{
		QMenu menu(this);
		menu.addAction("Show All Elements", this, SLOT(OnShowAllElements()));
		menu.exec(ev->globalPos());
		return;
	}

	Post::FSNodeSet* ns = dynamic_cast<Post::FSNodeSet*>(po);
	if (ns)
	{
		QMenu menu(this);
		menu.addAction("Select Nodes", this, SLOT(OnSelectNodes()));
		menu.exec(ev->globalPos());
		return;
	}
	::FSNodeSet* ns2 = dynamic_cast<::FSNodeSet*>(po);
	if (ns2)
	{
		QMenu menu(this);
		menu.addAction("Select Nodes", this, SLOT(OnSelectNodes()));
		menu.exec(ev->globalPos());
		return;
	}

	Post::FSSurface* ps = dynamic_cast<Post::FSSurface*>(po);
	if (ps)
	{
		QMenu menu(this);
		menu.addAction("Select Faces", this, SLOT(OnSelectFaces()));
		menu.exec(ev->globalPos());
		return;
	}
	::FSSurface* ps2 = dynamic_cast<::FSSurface*>(po);
	if (ps2)
	{
		QMenu menu(this);
		menu.addAction("Select Faces", this, SLOT(OnSelectFaces()));
		menu.exec(ev->globalPos());
		return;
	}

	Post::FSElemSet* pg = dynamic_cast<Post::FSElemSet*>(po);
	if (pg)
	{
		QMenu menu(this);
		menu.addAction("Select Elements", this, SLOT(OnSelectElements()));
		menu.addAction("Hide Elements", this, SLOT(OnHideElements()));
		menu.exec(ev->globalPos());
		return;
	}
	::FSElemSet* pg2 = dynamic_cast<::FSElemSet*>(po);
	if (pg2)
	{
		QMenu menu(this);
		menu.addAction("Select Elements", this, SLOT(OnSelectElements()));
		menu.addAction("Hide Elements", this, SLOT(OnHideElements()));
		menu.exec(ev->globalPos());
		return;
	}

	Post::CGLPlot* plot = dynamic_cast<Post::CGLPlot*>(po);
	if (plot)
	{
		QMenu menu(this);
		bool addRenderOptions = true;

		bool addGroups = true;
		std::vector<Post::GLPlotGroup*> groups;
		Post::CGLModel* mdl = plot->GetModel();
		if (mdl)
		{
			for (int i = 0; i < mdl->Plots(); ++i)
			{
				Post::GLPlotGroup* pg = dynamic_cast<Post::GLPlotGroup*>(mdl->Plot(i));
				if (pg) groups.push_back(pg);
			}
		}

		if (dynamic_cast<Post::GLPointProbe*>(po))
		{
			Post::GLPointProbe* probe = dynamic_cast<Post::GLPointProbe*>(po);

			menu.addSeparator();
			menu.addAction("Export data ...", this, SLOT(OnExportProbeData()));
		}

		if (dynamic_cast<Post::GLCurveProbe*>(po))
		{
			Post::GLCurveProbe* pc = dynamic_cast<Post::GLCurveProbe*>(po);
			menu.addSeparator();
			menu.addAction("Import points ...", this, SLOT(OnImportCurveProbePoints()));
			if (pc->Points() > 0)
			{
				menu.addAction("Plot data ...", this, SLOT(OnCurveProbePlotData()));
				menu.addAction("Plot time averaged data ...", this, SLOT(OnCurveProbePlotTimeAveragedData()));
			}
		}

		if (dynamic_cast<Post::GLMusclePath*>(po))
		{
			Post::GLMusclePath* path = dynamic_cast<Post::GLMusclePath*>(po);
			menu.addSeparator();
			menu.addAction("Export data ...", this, SLOT(OnExportMusclePathData()));
			menu.addAction("Swap end points", this, SLOT(OnSwapMusclePathEndPoints()));
		}

		if (dynamic_cast<Post::GLPlotGroup*>(po))
		{
			addGroups = false;
		}

		if (addGroups && (groups.empty() == false))
		{
			QMenu* subMenu = new QMenu("Move to group");
			Post::GLPlotGroup* pg = plot->GetGroup();
			if (pg != nullptr) { QAction* pa = subMenu->addAction("(none)", this, SLOT(OnMoveToGroup())); pa->setData(-1); }
			for (int i = 0; i < groups.size(); ++i) {
				if (pg != groups[i]) {
					QAction* pa = subMenu->addAction(QString::fromStdString(groups[i]->GetName()),this, SLOT(OnMoveToGroup()));
					pa->setData(i);
				}
			}
			if (menu.isEmpty() == false) menu.addSeparator();
			menu.addMenu(subMenu);
		}

		if (addRenderOptions)
		{
			if (menu.isEmpty() == false) menu.addSeparator();
			menu.addAction("Move up", this, SLOT(OnMoveUpInRenderingQueue()));
			menu.addAction("Move down", this, SLOT(OnMoveDownInRenderingQueue()));
		}

		menu.exec(ev->globalPos());
		return;
	}

	CImageModel* img = dynamic_cast<CImageModel*>(po);
	if (img)
	{
		QMenu menu(this);
		menu.addAction("Export image ...", this, SLOT(OnExportImage()));
		menu.exec(ev->globalPos());
		return;
	}

	Post::CMarchingCubes* mc = dynamic_cast<Post::CMarchingCubes*>(po);
	if (mc)
	{
		QMenu menu(this);
		menu.addAction("Export surface ...", this, SLOT(OnExportMCSurface()));
		menu.exec(ev->globalPos());
		return;
	}

	Post::FEPostModel::PointObject* pointObj = dynamic_cast<Post::FEPostModel::PointObject*>(po);
	if (pointObj && (pointObj->m_tag == 0))
	{
		QMenu menu(this);
		menu.addAction("Load transform from file ...", this, SLOT(OnLoadTransform()));
		menu.exec(ev->globalPos());
		return;
	}
}

void CPostModelPanel::OnSelectNodes()
{
	FSObject* po = ui->currentObject();
	if (po == nullptr) return;

	Post::FSNodeSet* pg = dynamic_cast<Post::FSNodeSet*>(po);
	if (pg)
	{
		CGLModelDocument* pdoc = GetActiveDocument();
		FSMesh* mesh = pdoc->GetGLModel()->GetFSModel()->GetFEMesh(0);
		pdoc->SetItemMode(ITEM_NODE);
		vector<int>pgl = pg->GetNodeList();
		pdoc->DoCommand(new CCmdSelectFENodes(mesh, pgl, false));
	}

	::FSNodeSet* pg2 = dynamic_cast<::FSNodeSet*>(po);
	if (pg2)
	{
		CGLModelDocument* pdoc = GetActiveDocument();
		FSMesh* mesh = pdoc->GetGLModel()->GetFSModel()->GetFEMesh(0);
		pdoc->SetItemMode(ITEM_NODE);
		vector<int> items = pg2->CopyItems();
		vector<int> pgl;
		pgl.insert(pgl.begin(), items.begin(), items.end());
		pdoc->DoCommand(new CCmdSelectFENodes(mesh, pgl, false));
	}

	GetMainWindow()->UpdateGLControlBar();
	GetMainWindow()->RedrawGL();
}

void CPostModelPanel::OnSelectFaces()
{
	FSObject* po = ui->currentObject();
	if (po == nullptr) return;

	Post::FSSurface* pg = dynamic_cast<Post::FSSurface*>(po);
	if (pg)
	{
		CGLModelDocument* pdoc = GetActiveDocument();
		FSMesh* mesh = pdoc->GetGLModel()->GetFSModel()->GetFEMesh(0);
		pdoc->SetItemMode(ITEM_FACE);
		vector<int>pgl = pg->GetFaceList();
		pdoc->DoCommand(new CCmdSelectFaces(mesh, pgl, false));
	}
	::FSSurface* pg2 = dynamic_cast<::FSSurface*>(po);
	if (pg2)
	{
		CGLModelDocument* pdoc = GetActiveDocument();
		FSMesh* mesh = pdoc->GetGLModel()->GetFSModel()->GetFEMesh(0);
		pdoc->SetItemMode(ITEM_FACE);
		vector<int> items = pg2->CopyItems();
		vector<int> pgl;
		pgl.insert(pgl.begin(), items.begin(), items.end());
		pdoc->DoCommand(new CCmdSelectFaces(mesh, pgl, false));
	}

	GetMainWindow()->UpdateGLControlBar();
	GetMainWindow()->RedrawGL();
}

void CPostModelPanel::OnSelectElements()
{
	FSObject* po = ui->currentObject();
	if (po == nullptr) return;

	CGLModelDocument* pdoc = GetActiveDocument();
	FSMesh* mesh = pdoc->GetGLModel()->GetFSModel()->GetFEMesh(0);
	Post::FSElemSet* pg = dynamic_cast<Post::FSElemSet*>(po);
	if (pg)
	{
		pdoc->SetItemMode(ITEM_ELEM);
        vector<int>pgl = pg->GetElementList();
		pdoc->DoCommand(new CCmdSelectElements(mesh, pgl, false));
	}
	::FSElemSet* pg2 = dynamic_cast<::FSElemSet*>(po);
	if (pg2)
	{
		pdoc->SetItemMode(ITEM_ELEM);
		vector<int> items = pg2->CopyItems();
		vector<int> pgl;
		pgl.insert(pgl.begin(), items.begin(), items.end());
		pdoc->DoCommand(new CCmdSelectElements(mesh, pgl, false));
	}

	GetMainWindow()->UpdateGLControlBar();
	GetMainWindow()->RedrawGL();
}

void CPostModelPanel::OnHideElements()
{
	FSObject* po = ui->currentObject();
	if (po == nullptr) return;
	CGLModelDocument* pdoc = GetActiveDocument();
	FSMesh* mesh = pdoc->GetGLModel()->GetFSModel()->GetFEMesh(0);

	Post::FSElemSet* pg = dynamic_cast<Post::FSElemSet*>(po);
	if (pg)
	{
		pdoc->DoCommand(new CCmdHideElements(mesh, pg->GetElementList()));
	}
	::FSElemSet* pg2 = dynamic_cast<::FSElemSet*>(po);
	if (pg2)
	{
		vector<int> items = pg2->CopyItems();
		vector<int> pgl;
		pgl.insert(pgl.begin(), items.begin(), items.end());
		pdoc->DoCommand(new CCmdHideElements(mesh, pgl));
	}

	GetMainWindow()->RedrawGL();
}

void CPostModelPanel::OnShowAllElements()
{
	CGLModelDocument* pdoc = GetActiveDocument();
	if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

	FSMesh* mesh = pdoc->GetGLModel()->GetFSModel()->GetFEMesh(0);
	if (mesh)
	{
		ForAllElements(*mesh, [](FEElement_& el) {
			el.Unhide();
		});
		mesh->UpdateItemVisibility();
		GetMainWindow()->RedrawGL();
	}
}

void CPostModelPanel::OnMoveToGroup()
{
	QAction* pa = qobject_cast<QAction*>(QObject::sender());
	if (pa == nullptr) return;

	Post::CGLPlot* plot = dynamic_cast<Post::CGLPlot*>(ui->currentObject());
	if (plot == nullptr) return;

	Post::CGLModel* mdl = plot->GetModel();
	if (mdl == nullptr) return;

	Post::GLPlotGroup* currentGroup = plot->GetGroup();

	int n = pa->data().toInt();
	if (n == -1)
	{
		if (currentGroup) {
			currentGroup->RemovePlot(plot);
			mdl->AddPlot(plot, false);
		}
	}
	else
	{
		std::vector<Post::GLPlotGroup*> groups;
		for (int i = 0; i < mdl->Plots(); ++i)
		{
			Post::GLPlotGroup* pg = dynamic_cast<Post::GLPlotGroup*>(mdl->Plot(i));
			if (pg) groups.push_back(pg);
		}

		if ((n >= 0) && (n < groups.size()))
		{
			if (currentGroup) currentGroup->RemovePlot(plot);
			else mdl->RemovePlot(plot);

			groups[n]->AddPlot(plot, false);
		}
	}

	Update(true);
	selectObject(plot);
}

void CPostModelPanel::OnMoveUpInRenderingQueue()
{
	FSObject* po = ui->currentObject();
	if (po == nullptr) return;

	Post::CGLPlot* plt = dynamic_cast<Post::CGLPlot*>(po);
	if (plt)
	{
		CGLModelDocument* pdoc = GetActiveDocument();
		if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

		Post::GLPlotGroup* pg = plt->GetGroup();
		if (pg == nullptr)
		{
			Post::CGLModel* glm = pdoc->GetGLModel();
			glm->MovePlotUp(plt);
		}
		else pg->MovePlotUp(plt);

		Update(true);
		selectObject(plt);

		GetMainWindow()->RedrawGL();
	}
}

void CPostModelPanel::OnMoveDownInRenderingQueue()
{
	FSObject* po = ui->currentObject();
	if (po == nullptr) return;

	Post::CGLPlot* plt = dynamic_cast<Post::CGLPlot*>(po);
	if (plt)
	{
		CGLModelDocument* pdoc = GetActiveDocument();
		if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

		Post::GLPlotGroup* pg = plt->GetGroup(); 
		if (pg == nullptr)
		{
			Post::CGLModel* glm = pdoc->GetGLModel();
			glm->MovePlotDown(plt);
		}
		else pg->MovePlotDown(plt);

		Update(true);
		selectObject(plt);

		GetMainWindow()->RedrawGL();
	}
}

void CPostModelPanel::OnExportImage()
{
	FSObject* po = ui->currentObject();
	if (po == nullptr) return;

	CImageModel* img = dynamic_cast<CImageModel*>(po);
	if (img == nullptr) return;

    QString filter;

    #ifdef HAS_ITK
        filter = "TIFF (*.tiff);;Raw image (*.raw)";
    #else
        filter = "Raw image (*.raw)";
    #endif

	QString filename = QFileDialog::getSaveFileName(GetMainWindow(), "Export image", "", filter);
	if (filename.isEmpty() == false)
	{
        if(filename.endsWith(".raw"))
        {
            if (img->ExportRAWImage(filename.toStdString()))
            {
                QString msg = QString("Image exported successfully to file\n%1").arg(filename);
                QMessageBox::information(GetMainWindow(), "Export image", msg);
            }
            else
            {
                QString msg = QString("Failed exporting image to file\n%1").arg(filename);
                QMessageBox::critical(GetMainWindow(), "Export image", msg);
            }
        }
        else
        {
            if (img->ExportSITKImage(filename.toStdString()))
            {
                QString msg = QString("Image exported successfully to file\n%1").arg(filename);
                QMessageBox::information(GetMainWindow(), "Export image", msg);
            }
            else
            {
                QString msg = QString("Failed exporting image to file\n%1").arg(filename);
                QMessageBox::critical(GetMainWindow(), "Export image", msg);
            }
        }
	}	
}

void CPostModelPanel::OnLoadTransform()
{
	CPostDocument* pdoc = dynamic_cast<CPostDocument*>(GetActiveDocument());
	if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

	Post::CGLModel* glm = pdoc->GetGLModel();
	if (glm == nullptr) return;

	FSObject* po = ui->currentObject();
	Post::FEPostModel::PointObject* pointObj = dynamic_cast<Post::FEPostModel::PointObject*>(po);
	if (pointObj == nullptr) return;

	QString fileName = QFileDialog::getOpenFileName(this, "Open Kinemat file");
	if (fileName.isEmpty() == false)
	{
		std::string filename = fileName.toStdString();
		FEKinemat kine;
		if (kine.ReadKine(filename.c_str()) == false)
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed reading file.");
			return;
		}
		int NS = kine.States();
		if (NS == 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "No transform data found in file.");
			return;
		}

		int nid = pointObj->m_id;
		if (nid < 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "Internal error: Invalid point object ID.");
			return;
		}

		Post::CGLModel& mdl = *glm;
		Post::FEPostModel& fem = *mdl.GetFSModel();
		for (int i = 0; i < fem.GetStates(); ++i)
		{
			Post::FEState& state = *fem.GetState(i);
			int n = (i < NS ? i : NS - 1);
			FEKinemat::STATE& kineState = kine.GetState(n);

			vec3d p = kineState.D[0].translate();
			mat3d Q = kineState.D[0].rotate();

			Post::OBJ_POINT_DATA* pd = &(state.m_objPt[nid]);
			pd->pos = p;
			pd->rot = Q;
			pd->data->set(0, to_vec3f(pd->pos));
			pd->data->set(1, to_mat3f(Q));
		}
		mdl.Update(true);
		GetMainWindow()->RedrawGL();
	}
}

void CPostModelPanel::OnExportMCSurface()
{
	FSObject* po = ui->currentObject();
	Post::CMarchingCubes* mc = dynamic_cast<Post::CMarchingCubes*>(po);
	if (mc)
	{
		QFileDialog dlg;
		QString fileName = dlg.getSaveFileName(this, "Export surface", "", "STL mesh (*.stl)");
		if (fileName.isEmpty() == false)
		{
			string filename = fileName.toStdString();
			FSMesh mesh;
			mc->GetMesh(mesh);
			FSProject dummy;
			STLExport stl(dummy);

			bool b = stl.Write(filename.c_str(), &mesh);
			if (b) QMessageBox::information(this, "Export surface", "File written successfully.");
			else QMessageBox::critical(this, "Export surface", "Failed exporting surface.");
		}
	}
}

void CPostModelPanel::OnExportProbeData()
{
	Post::GLPointProbe* probe = dynamic_cast<Post::GLPointProbe*>(ui->currentObject());
	if (probe == nullptr) return;

	CGLModelDocument* pdoc = GetActiveDocument();
	if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

	Post::CGLModel* glm = pdoc->GetGLModel();
	if (glm == nullptr) return;

	Post::FEPostModel* fem = glm->GetFSModel();

	int nfield = glm->GetColorMap()->GetEvalField();

	bool ok = true;
	QStringList ops = QStringList() << "selected probe" << "all probes";
	QString op = QInputDialog::getItem(this, "Export data", "Export option", ops, 0, false, &ok);
	if (op.isEmpty() || (ok == false)) return;
	int nop = ops.indexOf(op);

	QString filename = QFileDialog::getSaveFileName(GetMainWindow(), "Export data", "", "Text file (*.txt)");
	if (filename.isEmpty() == false)
	{
		string sfile = filename.toStdString();
		const char* szfile = sfile.c_str();
		FILE* fp = fopen(szfile, "wt");
		if (nop == 0) // selected probe
		{
			fprintf(fp, "x,y,z,value\n");

			for (int nstep = 0; nstep < fem->GetStates(); ++nstep)
			{
				vec3d r = probe->Position(nstep);
				double val = 0.0;
				if ((nfield > 0) && probe->TrackModelData())
						val = probe->DataValue(nfield, nstep);

				fprintf(fp, "%lg,%lg,%lg,%lg\n", r.x, r.y, r.z, val);
			}
		}
		else if (nop == 1) // all probes
		{
			for (Post::GLPlotIterator it(glm); it != nullptr; ++it)
			{
				Post::CGLPlot* po = it;
				Post::GLPointProbe* probe_i = dynamic_cast<Post::GLPointProbe*>(po);
				if (probe_i)
				{
					string s = probe_i->GetName();
					fprintf(fp, "%s\n", s.c_str());

					fprintf(fp, "x,y,z,value\n");

					for (int nstep = 0; nstep < fem->GetStates(); ++nstep)
					{
						vec3d r = probe_i->Position(nstep);
						double val = 0.0;
						if ((nfield > 0) && probe_i->TrackModelData())
							val = probe_i->DataValue(nfield, nstep);

						fprintf(fp, "%lg,%lg,%lg,%lg\n", r.x, r.y, r.z, val);
					}
				}
			}
		}
		fclose(fp);
	}

	QMessageBox::information(GetMainWindow(), "Export", "Data export successful!");
}

void CPostModelPanel::OnImportCurveProbePoints()
{
	CGLModelDocument* pdoc = GetActiveDocument();
	if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

	Post::CGLModel* glm = pdoc->GetGLModel();
	if (glm == nullptr) return;

	Post::FEPostModel* fem = glm->GetFSModel();

	Post::GLCurveProbe* po = dynamic_cast<Post::GLCurveProbe*>(ui->currentObject());
	if (po == nullptr) return;

	QString filename = QFileDialog::getOpenFileName(GetMainWindow(), "Import data", "", "All files (*)");
	if (filename.isEmpty() == false)
	{
		QFile file(filename);
		if (file.open(QFile::ReadOnly | QFile::Text))
		{
			QTextStream txt(&file);
			QString data = txt.readAll();

			CDlgImportData dlg(data, DataType::DOUBLE, 3);
			if (dlg.exec())
			{
				QList<QList<double> > val = dlg.GetDoubleValues();
				std::vector<vec3d> points;
				for (QList<double>& row : val)
				{
					vector<double> d;
					for (double di : row) d.push_back(di);
					assert(d.size() == 3);
					vec3d p(d[0], d[1], d[2]);
					points.push_back(p);
				}

				po->SetPoints(points);
			}
		}
		else QMessageBox::critical(this, "Import data", "Failed importing points");

		Update(true);
		selectObject(po);
		GetMainWindow()->RedrawGL();
	}
}

void CPostModelPanel::OnExportMusclePathData()
{
	Post::GLMusclePath* po = dynamic_cast<Post::GLMusclePath*>(ui->currentObject());
	if (po == nullptr) return;

	CGLModelDocument* pdoc = GetActiveDocument();
	if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

	Post::CGLModel* glm = pdoc->GetGLModel();
	if (glm == nullptr) return;

	Post::FEPostModel* fem = glm->GetFSModel();
	if (fem == nullptr) return;

	bool ok = true;
	QStringList ops = QStringList() << "selected path" << "all paths";
	QString op = QInputDialog::getItem(this, "Export data", "Export option", ops, 0, false, &ok);
	if (op.isEmpty() || (ok == false)) return;
	int nop = ops.indexOf(op);

	QString filename = QFileDialog::getSaveFileName(GetMainWindow(), "Export data", "", "CSV file (*.csv)");
	if (filename.isEmpty() == false)
	{
		string sfile = filename.toStdString();
		const char* szfile = sfile.c_str();
		FILE* fp = fopen(szfile, "wt");
		if (fp == nullptr)
		{
			QMessageBox::critical(GetMainWindow(), "Export", "Failed to export data!");
			return;
		}
		if (nop == 0) // selected path
		{
			fprintf(fp, "length, x0, y0, z0, x1, y1, x1, xd, yd, zd, tx, ty, tz\n");
			for (int nstep = 0; nstep < fem->GetStates(); ++nstep)
			{
				// see double GLMusclePath::DataValue(int field, int step)
				const int MAX_DATA = 13;
				for (int ndata = 1; ndata <= MAX_DATA; ++ndata)
				{
					double v = po->DataValue(ndata, nstep);
					fprintf(fp, "%lg", v);
					if (ndata != MAX_DATA) fprintf(fp, ", ");
				}
				fprintf(fp, "\n");
			}
		}
		else if (nop == 1) // all paths
		{
			for (Post::GLPlotIterator it(glm); it != nullptr; ++it)
			{
				Post::CGLPlot* plt = it;
				Post::GLMusclePath* pm = dynamic_cast<Post::GLMusclePath*>(plt);
				if (pm)
				{
					fprintf(fp, "%s\n", pm->GetName().c_str());
					fprintf(fp, "length, x0, y0, z0, x1, y1, x1, xd, yd, zd, tx, ty, tz\n");
					for (int nstep = 0; nstep < fem->GetStates(); ++nstep)
					{
						// see double GLMusclePath::DataValue(int field, int step)
						const int MAX_DATA = 13;
						for (int ndata = 1; ndata <= MAX_DATA; ++ndata)
						{
							double v = pm->DataValue(ndata, nstep);
							fprintf(fp, "%lg", v);
							if (ndata != MAX_DATA) fprintf(fp, ", ");
						}
						fprintf(fp, "\n");
					}
				}
			}
		}
		fclose(fp);
	}

	QMessageBox::information(GetMainWindow(), "Export", "Data export successful!");
}

void CPostModelPanel::OnSwapMusclePathEndPoints()
{
	Post::GLMusclePath* po = dynamic_cast<Post::GLMusclePath*>(ui->currentObject());
	if (po == nullptr) return;

	po->SwapEndPoints();
	Update(true);
	selectObject(po);
	GetMainWindow()->RedrawGL();
}

void CPostModelPanel::OnCurveProbePlotData()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(GetDocument());
	if (doc == nullptr) return;

	Post::GLCurveProbe* po = dynamic_cast<Post::GLCurveProbe*>(ui->currentObject());
	if (po)
	{
		int N = po->Points();
		vector<double> xpoints = po->SectionLenghts(false);
		vector<double> ypoints(N, 0.0);
#pragma omp parallel for
		for (int i = 0; i < N; ++i)
		{
			ypoints[i] = po->GetPointValue(i);
		}

		CPlotData* data = new CPlotData;
		for (int i = 0; i < po->Points(); ++i)
		{
			data->addPoint(xpoints[i], ypoints[i]);
		}
		data->setLabel(QString::fromStdString(po->GetName()));
		data->setLineColor(toQColor(po->GetColor()));
		data->setFillColor(toQColor(po->GetColor()));

		CGraphData* graph = new CGraphData;
		graph->m_data.push_back(data);

		CDataGraphWindow* w = new CDataGraphWindow(GetMainWindow(), doc);
		w->SetData(graph);
		GetMainWindow()->AddGraph(w);
		w->setWindowTitle(QString::fromStdString(po->GetName()));
		w->show();
	}
}

void CPostModelPanel::OnCurveProbePlotTimeAveragedData()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(GetDocument());
	if (doc == nullptr) return;

	Post::GLCurveProbe* po = dynamic_cast<Post::GLCurveProbe*>(ui->currentObject());
	if (po)
	{
		Post::CGLModel* mdl = po->GetModel();
		if (mdl == nullptr) return;

		TIMESETTINGS& time = doc->GetTimeSettings();

		int N = po->Points();
		vector<double> xpoints = po->SectionLenghts(false);
		vector<double> ypoints(N, 0.0);
#pragma omp parallel for
		for (int i = 0; i < N; ++i)
		{
			double y = 0.0;
			for (int n = time.m_start; n <= time.m_end; n++)
			{
				y += po->GetPointValue(i, n);
			}
			y /= (time.m_end - time.m_start + 1);
			ypoints[i] = y;
		}

		CPlotData* data = new CPlotData;
		for (int i = 0; i < N; ++i)
		{
			data->addPoint(xpoints[i], ypoints[i]);
		}

		data->setLabel(QString::fromStdString(po->GetName()));
		data->setLineColor(toQColor(po->GetColor()));
		data->setFillColor(toQColor(po->GetColor()));

		CGraphData* graph = new CGraphData;
		graph->m_data.push_back(data);

		CDataGraphWindow* w = new CDataGraphWindow(GetMainWindow(), doc);
		w->SetData(graph);
		GetMainWindow()->AddGraph(w);
		w->setWindowTitle(QString::fromStdString(po->GetName()));
		w->show();
	}
}
