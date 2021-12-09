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
#include "MainWindow.h"
#include "Document.h"
#include "PropertyListView.h"
#include <PostGL/GLModel.h>
#include <PostLib/FEPostModel.h>
#include <PostLib/GLObject.h>
#include <PostGL/GLPlot.h>
#include <PostGL/GLPlaneCutPlot.h>
#include <PostGL/GLVectorPlot.h>
#include <PostGL/GLSlicePLot.h>
#include <PostGL/GLIsoSurfacePlot.h>
#include <PostGL/GLLinePlot.h>
#include <PostGL/GLStreamLinePlot.h>
#include <PostGL/GLParticleFlowPlot.h>
#include <PostGL/GLTensorPlot.h>
#include <ImageLib/3DImage.h>
#include <PostLib/VolRender.h>
#include <PostLib/VolumeRender2.h>
#include <PostLib/ImageSlicer.h>
#include <PostLib/ImageModel.h>
#include <PostLib/GLImageRenderer.h>
#include <PostLib/MarchingCubes.h>
#include <PostGL/GLMirrorPlane.h>
#include "ObjectProps.h"
#include <CUILib/ImageViewer.h>
#include <CUILib/HistogramViewer.h>
#include "GLView.h"
#include "PostDocument.h"
#include "GraphWindow.h"
#include "Commands.h"

//-----------------------------------------------------------------------------
class CModelProps : public CPropertyList
{
public:
	CModelProps(Post::CGLModel* fem) : m_fem(fem) 
	{
		addProperty("Element subdivions"       , CProperty::Int)->setIntRange(0, 100).setAutoValue(true);
		addProperty("Render undeformed outline", CProperty::Bool);
		addProperty("Undeformed outline color" , CProperty::Color);
		addProperty("Outline color"            , CProperty::Color);
		addProperty("Node color"               , CProperty::Color);
		addProperty("Selection color"          , CProperty::Color);
		addProperty("Shells as hexes"          , CProperty::Bool);
		addProperty("Shell reference surface"  , CProperty::Enum, "set the shell reference surface")->setEnumValues(QStringList() << "Mid surface" << "bottom surface" << "top surface");
		addProperty("Smoothing angle"          , CProperty::Float);
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
		case 8: v = m_fem->GetSmoothingAngle(); break;
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
		case 8: m_fem->SetSmoothingAngle(v.toDouble());  break;
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
		addProperty("Shell Elements", CProperty::Int, "Number of shell elemetns")->setFlags(CProperty::Visible);
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
	CImageModelProps(Post::CImageModel* img)
	{
		m_img = img;

		addProperty("show box", CProperty::Bool);
		addProperty("x-min", CProperty::Float);
		addProperty("y-min", CProperty::Float);
		addProperty("z-min", CProperty::Float);
		addProperty("x-max", CProperty::Float);
		addProperty("y-max", CProperty::Float);
		addProperty("z-max", CProperty::Float);
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
		}
		m_img->SetBoundingBox(box);
	}

private:
	Post::CImageModel*	m_img;
};

//-----------------------------------------------------------------------------
class CModelTreeItem : public QTreeWidgetItem
{
public:
	enum Flag {
		CANNOT_RENAME = 0x01,
		CANNOT_DELETE = 0x02,
		CANNOT_DISABLE = 0x04,
		ALL_FLAGS = 0x07
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
	QCheckBox* enabled;

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
		m_tree->setObjectName(QStringLiteral("postModel"));
		m_tree->setColumnCount(1);
		m_tree->setHeaderHidden(true);

		QWidget* w = new QWidget;
		QVBoxLayout* pvl = new QVBoxLayout;
		
    pvl->setContentsMargins(0,0,0,0);
		
		QHBoxLayout* phl = new QHBoxLayout;
		enabled = new QCheckBox; enabled->setObjectName("enabled");

		phl->addWidget(enabled);
		phl->addWidget(name = new QLineEdit); name->setObjectName("nameEdit");

		delButton = new QPushButton("Delete"); delButton->setObjectName("deleteButton");
		phl->addWidget(delButton);
		phl->addStretch();

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

	void ShowImageViewer(Post::CImageModel* img)
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

		pi1->setText(0, txt);
		if (icon.isEmpty() == false) pi1->setIcon(0, QIcon(QString(":/icons/") + icon + ".png"));

		pi1->SetPropertyList(props);

		pi1->SetFlags(flags);

		updateItem(pi1);

		return pi1;
	}

	void setCurrentObject(FSObject* po)
	{
		if (po == 0) m_tree->clearSelection();
		else
		{
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
		}
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
		}
		else
		{
			QFont font = item->font(0);
			font.setItalic(true);
			item->setFont(0, font);
		}
	}
};

CPostModelPanel::CPostModelPanel(CMainWindow* pwnd, QWidget* parent) : CCommandPanel(pwnd, parent), ui(new Ui::CPostModelPanel)
{
	ui->setupUi(this);

	QObject::connect(this, SIGNAL(postObjectStateChanged()), pwnd, SLOT(OnPostObjectStateChanged()));
	QObject::connect(this, SIGNAL(postObjectPropsChanged(FSObject*)), pwnd, SLOT(OnPostObjectPropsChanged(FSObject*)));
}

CPostDocument* CPostModelPanel::GetActiveDocument()
{
	return GetMainWindow()->GetPostDocument();
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

void CPostModelPanel::BuildModelTree()
{
	ui->clear();
	CPostDocument* pdoc = GetActiveDocument();
	if (pdoc && pdoc->IsValid())
	{
		Post::FEPostModel* fem = pdoc->GetFSModel();
		Post::CGLModel* mdl = pdoc->GetGLModel();

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

				for (int i = 0; i < mesh.Parts(); ++i)
				{
					Post::FEPart& part = mesh.Part(i);
					ui->AddItem(pi2, &part, QString::fromStdString(part.GetName()), "", nullptr, CModelTreeItem::ALL_FLAGS);
				}

				GObject* po = pdoc->GetActiveObject();
				if (po)
				{
					for (int i = 0; i < po->FENodeSets(); ++i)
					{
						FENodeSet* pg = po->GetFENodeSet(i);
						ui->AddItem(pi2, pg, QString::fromStdString(pg->GetName()), "selNode", nullptr, CModelTreeItem::CANNOT_DISABLE);
					}

					for (int i = 0; i < po->FEEdgeSets(); ++i)
					{
						FEEdgeSet* pg = po->GetFEEdgeSet(i);
						ui->AddItem(pi2, pg, QString::fromStdString(pg->GetName()), "selEdge", nullptr, CModelTreeItem::CANNOT_DISABLE);
					}

					for (int i = 0; i < po->FESurfaces(); ++i)
					{
						FESurface* pg = po->GetFESurface(i);
						ui->AddItem(pi2, pg, QString::fromStdString(pg->GetName()), "selFace", nullptr, CModelTreeItem::CANNOT_DISABLE);
					}

					for (int i = 0; i < po->FEParts(); ++i)
					{
						FEPart* pg = po->GetFEPart(i);
						ui->AddItem(pi2, pg, QString::fromStdString(pg->GetName()), "selElem", nullptr, CModelTreeItem::CANNOT_DISABLE);
					}
				}
			}

			Post::CGLDisplacementMap* map = mdl->GetDisplacementMap();
			if (map)
			{
				ui->AddItem(pi1, map, QString::fromStdString(map->GetName()), "distort", new CObjectProps(map), CModelTreeItem::CANNOT_RENAME);
			}

			Post::CGLColorMap* col = mdl->GetColorMap();
			if (col)
			{
				ui->AddItem(pi1, col, QString::fromStdString(col->GetName()), "colormap", new CObjectProps(col), CModelTreeItem::CANNOT_RENAME);
			}

			if (fem && fem->PlotObjects())
			{
				int npo = fem->PlotObjects();
				for (int i = 0; i < npo; ++i)
				{
					Post::FEPostModel::PlotObject* po = fem->GetPlotObject(i);
					ui->AddItem(pi1, po, QString::fromStdString(po->GetName()), "", new CObjectProps(po));
				}
			}

			Post::GPlotList& pl = mdl->GetPlotList();
			for (int n = 0; n < pl.Size(); ++n)
			{
				Post::CGLPlot& plot = *pl[n];
				CModelTreeItem* pi1 = ui->AddItem(nullptr, &plot, QString::fromStdString(plot.GetName()), "", new CObjectProps(&plot));

				if      (dynamic_cast<Post::CGLPlaneCutPlot    *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/cut.png")));
				else if (dynamic_cast<Post::CGLVectorPlot      *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/vectors.png")));
				else if (dynamic_cast<Post::CGLSlicePlot       *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/slice.png")));
				else if (dynamic_cast<Post::CGLIsoSurfacePlot  *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/isosurface.png")));
				else if (dynamic_cast<Post::CGLStreamLinePlot  *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/streamlines.png")));
				else if (dynamic_cast<Post::CGLParticleFlowPlot*>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/particle.png")));
				else if (dynamic_cast<Post::GLTensorPlot       *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/tensor.png")));
				else if (dynamic_cast<Post::CGLMirrorPlane     *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/mirror.png")));
			}
		}

		for (int i = 0; i < pdoc->ImageModels(); ++i)
		{
			Post::CImageModel* img = pdoc->GetImageModel(i);
			pi1 = ui->AddItem(nullptr, img, QString::fromStdString(img->GetName()), "image", new CImageModelProps(img));

			for (int j = 0; j < img->ImageRenderers(); ++j)
			{
				Post::CGLImageRenderer* render = img->GetImageRenderer(j);

				Post::CVolRender* volRender = dynamic_cast<Post::CVolRender*>(render);
				if (volRender)
				{
					ui->AddItem(pi1, volRender, QString::fromStdString(render->GetName()), "volrender", new CObjectProps(volRender));
				}

				Post::CVolumeRender2* volRender2 = dynamic_cast<Post::CVolumeRender2*>(render);
				if (volRender2)
				{
					ui->AddItem(pi1, volRender2, QString::fromStdString(render->GetName()), "volrender", new CObjectProps(volRender2));
				}

				Post::CImageSlicer* imgSlice = dynamic_cast<Post::CImageSlicer*>(render);
				if (imgSlice)
				{
					ui->AddItem(pi1, imgSlice, QString::fromStdString(render->GetName()), "imageslice", new CObjectProps(imgSlice));
				}

				Post::CMarchingCubes* marchCube = dynamic_cast<Post::CMarchingCubes*>(render);
				if (marchCube)
				{
					ui->AddItem(pi1, marchCube, QString::fromStdString(render->GetName()), "marching_cubes", new CObjectProps(marchCube));
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
		int n = pdoc->Graphs();
		if (n > 0)
		{
			pi1 = ui->AddItem(nullptr, nullptr, "Saved Graphs", "chart", nullptr, CModelTreeItem::ALL_FLAGS);
			for (int i = 0; i < n; ++i)
			{
				CGraphData* gd = const_cast<CGraphData*>(pdoc->GetGraphData(i));
				ui->AddItem(pi1, gd, QString::fromStdString(gd->GetName()));
			}
		}
	}

	// This can crash PostView if po no longer exists (e.g. after new file is read)
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

			ui->enabled->setEnabled(item->CanDisable());
			Post::CGLObject* glo = dynamic_cast<Post::CGLObject*>(po);
			if (glo)
			{
				ui->enabled->setChecked(glo->IsActive());
			}

			if (dynamic_cast<Post::CImageModel*>(po))
			{
				Post::CImageModel* img = dynamic_cast<Post::CImageModel*>(po);
				ui->ShowImageViewer(img);
			}
			else ui->HideImageViewer();
		}
		else 
		{
			ui->HideImageViewer();
			ui->enabled->setEnabled(false);
			ui->enabled->setChecked(true);
			ui->delButton->setEnabled(false);
		}

		ui->name->setText(item->text(0));
		ui->name->setEnabled(item->CanRename());
		ui->m_props->Update(item->GetPropertyList());
	}
	else
	{
		ui->HideImageViewer();
		ui->m_props->Update(0);
		ui->enabled->setEnabled(false);
		ui->enabled->setChecked(false);
		ui->delButton->setEnabled(false);
		ui->name->setText("");
		ui->name->setEnabled(false);
	}
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

	CGraphData* graph = dynamic_cast<CGraphData*>(po);
	if (graph)
	{
		CDataGraphWindow* w = new CDataGraphWindow(GetMainWindow(), GetActiveDocument());
		w->SetData(graph);
		GetMainWindow()->AddGraph(w);
		w->setWindowTitle(QString::fromStdString(graph->GetName()));
		w->show();
	}

	CGLDocument* doc = GetDocument();

	FENodeSet* pn = dynamic_cast<FENodeSet*>(po);
	if (pn)
	{
		std::list<int> items = pn->CopyItems();
		std::vector<int> vitems(items.begin(), items.end());
		doc->SetItemMode(ITEM_NODE);
		doc->DoCommand(new CCmdSelectFENodes(pn->GetMesh(), vitems, false));
	}

	FEEdgeSet* pe = dynamic_cast<FEEdgeSet*>(po);
	if (pe)
	{
		std::list<int> items = pe->CopyItems();
		std::vector<int> vitems(items.begin(), items.end());
		doc->SetItemMode(ITEM_EDGE);
		doc->DoCommand(new CCmdSelectFEEdges(pe->GetMesh(), vitems, false));
	}

	FESurface* ps = dynamic_cast<FESurface*>(po);
	if (ps)
	{
		std::list<int> items = ps->CopyItems();
		std::vector<int> vitems(items.begin(), items.end());
		doc->SetItemMode(ITEM_FACE);
		doc->DoCommand(new CCmdSelectFaces(ps->GetMesh(), vitems, false));
	}

	FEPart* pg = dynamic_cast<FEPart*>(po);
	if (pg)
	{
		std::list<int> items = pg->CopyItems();
		std::vector<int> vitems(items.begin(), items.end());
		doc->SetItemMode(ITEM_ELEM);
		doc->DoCommand(new CCmdSelectElements(pg->GetMesh(), vitems, false));
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

	CPostDocument* doc = GetActiveDocument();

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
	else if (dynamic_cast<FENodeSet*>(po))
	{
		GObject* poa = doc->GetActiveObject();
		if (poa)
		{
			poa->RemoveFENodeSet(dynamic_cast<FENodeSet*>(po));
			Update(true);
		}
	}
	else if (dynamic_cast<FEEdgeSet*>(po))
	{
		GObject* poa = doc->GetActiveObject();
		if (poa)
		{
			poa->RemoveFEEdgeSet(dynamic_cast<FEEdgeSet*>(po));
			Update(true);
		}
	}
	else if (dynamic_cast<FESurface*>(po))
	{
		GObject* poa = doc->GetActiveObject();
		if (poa)
		{
			poa->RemoveFESurface(dynamic_cast<FESurface*>(po));
			Update(true);
		}
	}
	else if (dynamic_cast<FEPart*>(po))
	{
		GObject* poa = doc->GetActiveObject();
		if (poa)
		{
			poa->RemoveFEPart(dynamic_cast<FEPart*>(po));
			Update(true);
		}
	}
	else QMessageBox::information(this, "FEBio Studio", "Cannot delete this object");
}

void CPostModelPanel::on_props_dataChanged()
{
	FSObject* po = selectedObject();
	if (po) po->Update();

	emit postObjectPropsChanged(po);
}

void CPostModelPanel::on_enabled_stateChanged(int nstate)
{
	CModelTreeItem* item = ui->currentItem();
	if (item == 0) return;

	Post::CGLObject* po = dynamic_cast<Post::CGLObject*>(item->Object());
	if (po == 0) return;

	if (nstate == Qt::Unchecked)
	{
		po->Activate(false);
	}
	else if (nstate == Qt::Checked)
	{
		po->Activate(true);
	}

	ui->updateItem(item);

	emit postObjectStateChanged();
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

	Post::FEPart* pg = dynamic_cast<Post::FEPart*>(po);
	if (pg)
	{
		QMenu menu(this);
		menu.addAction("Select Elements", this, SLOT(OnSelectElements()));
		menu.addAction("Hide Elements", this, SLOT(OnHideElements()));
		menu.exec(ev->globalPos());
		return;
	}
}

void CPostModelPanel::OnSelectElements()
{
	FSObject* po = ui->currentObject();
	if (po == nullptr) return;

	Post::FEPart* pg = dynamic_cast<Post::FEPart*>(po);
	if (pg)
	{
		CPostDocument* pdoc = GetActiveDocument();
		FEMesh* mesh = pdoc->GetFSModel()->GetFEMesh(0);
		pdoc->SetItemMode(ITEM_ELEM);
        vector<int>pgl = pg->GetElementList();
		pdoc->DoCommand(new CCmdSelectElements(mesh, pgl, false));
		GetMainWindow()->UpdateGLControlBar();
		GetMainWindow()->RedrawGL();
	}
}

void CPostModelPanel::OnHideElements()
{
	FSObject* po = ui->currentObject();
	if (po == nullptr) return;

	Post::FEPart* pg = dynamic_cast<Post::FEPart*>(po);
	if (pg)
	{
		CPostDocument* pdoc = GetActiveDocument();
		FEMesh* mesh = pdoc->GetFSModel()->GetFEMesh(0);

		pdoc->DoCommand(new CCmdHideElements(mesh, pg->GetElementList()));
		GetMainWindow()->RedrawGL();
	}
}

void CPostModelPanel::OnShowAllElements()
{
	CPostDocument* pdoc = GetActiveDocument();
	if ((pdoc == nullptr) || (pdoc->IsValid() == false)) return;

	FEMesh* mesh = pdoc->GetFSModel()->GetFEMesh(0);
	if (mesh)
	{
		ForAllElements(*mesh, [](FEElement_& el) {
			el.Unhide();
		});
		mesh->UpdateItemVisibility();
		GetMainWindow()->RedrawGL();
	}
}
