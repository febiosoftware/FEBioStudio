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
#include "MainWindow.h"
#include "Document.h"
#include "PropertyListView.h"
#include <PostGL/GLModel.h>
#include <PostLib/FEModel.h>
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
#include <PostLib/ImageSlicer.h>
#include <PostLib/ImageModel.h>
#include <PostLib/GLImageRenderer.h>
#include <PostLib/MarchingCubes.h>
#include <PostGL/GLMirrorPlane.h>
#include "ObjectProps.h"
#include <CUILib/ImageViewer.h>
#include <CUILib/HistogramViewer.h>
#include <PostLib/GView.h>
#include "PostDoc.h"

//-----------------------------------------------------------------------------
class CModelProps : public CPropertyList
{
public:
	CModelProps(Post::CGLModel* fem) : m_fem(fem) 
	{
		addProperty("Element subdivions"       , CProperty::Int)->setIntRange(0, 100).setAutoValue(true);
		addProperty("Render mode"              , CProperty::Enum, "Render mode")->setEnumValues(QStringList() << "default" << "wireframe" << "solid");
		addProperty("Render undeformed outline", CProperty::Bool);
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
		case 1: v = m_fem->m_nrender; break;
		case 2: v = m_fem->m_bghost; break;
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
		case 0: m_fem->m_nDivs    = v.toInt(); break;
		case 1: m_fem->m_nrender  = v.toInt(); break;
		case 2: m_fem->m_bghost   = v.toBool(); break;
		case 3: m_fem->m_line_col = toGLColor(v.value<QColor>());
		case 4: m_fem->m_node_col = toGLColor(v.value<QColor>());
		case 5: m_fem->m_sel_col  = toGLColor(v.value<QColor>());
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
	CMeshProps(Post::FEModel* fem) : m_fem(fem)
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
			Post::FEPostMesh& mesh = *m_fem->GetFEMesh(0);
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
	Post::FEModel*	m_fem;
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
	}

	QVariant GetPropertyValue(int i)
	{
		CGLCamera& cam = m_view.GetCamera();
		quatd q = cam.GetOrientation();
		float w = q.GetAngle()*180.f/PI;
		vec3d v = q.GetVector()*w;

		vec3d r = cam.GetPosition();
		float d = cam.GetTargetDistance();

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
		CGLCamera& cam = m_view.GetCamera();
		quatd q = cam.GetOrientation();
		float w = q.GetAngle()*180.f/PI;
		vec3d v = q.GetVector()*w;

		vec3d r = cam.GetPosition();
		float d = cam.GetTargetDistance();

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
		cam.SetOrientation(q);

		cam.SetTarget(r);
		cam.SetTargetDistance(d);

		cam.Update(true);
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
	}

private:
	Post::CImageModel*	m_img;
};

//-----------------------------------------------------------------------------
class CModelTreeItem : public QTreeWidgetItem
{
public:
	CModelTreeItem(Post::CGLObject* po, QTreeWidget* tree) : QTreeWidgetItem(tree), m_po(po) {}
	CModelTreeItem(Post::CGLObject* po, QTreeWidgetItem* item) : QTreeWidgetItem(item), m_po(po) {}

	Post::CGLObject* Object() { return m_po; }

	void SetObject(Post::CGLObject* po) { m_po = po; }

private:
	Post::CGLObject* m_po;
};

//-----------------------------------------------------------------------------
class Ui::CPostModelPanel
{
public:
	QTreeWidget*			m_tree;
	::CPropertyListView*	m_props;
	QVector<CPropertyList*>	m_list;

	CImageViewer*	m_imgView;

	CHistogramViewer* m_histo;

	QTabWidget*	m_tab;


	QLineEdit* name;
	QCheckBox* enabled;

	QPushButton* autoUpdate;
	QPushButton* apply;

public:
	void setupUi(::CPostModelPanel* parent)
	{
		QVBoxLayout* pg = new QVBoxLayout(parent);
		pg->setMargin(0);
		
		QSplitter* psplitter = new QSplitter;
		psplitter->setOrientation(Qt::Vertical);
		pg->addWidget(psplitter);

		m_tree = new QTreeWidget;
		m_tree->setObjectName(QStringLiteral("postModel"));
		m_tree->setColumnCount(1);
		m_tree->setHeaderHidden(true);

		QWidget* w = new QWidget;
		QVBoxLayout* pvl = new QVBoxLayout;
		pvl->setMargin(0);
		
		QHBoxLayout* phl = new QHBoxLayout;
		enabled = new QCheckBox; enabled->setObjectName("enabled");

		phl->addWidget(enabled);
		phl->addWidget(name = new QLineEdit); name->setObjectName("nameEdit");

		QPushButton* del = new QPushButton("Delete"); del->setObjectName("deleteButton");
		phl->addWidget(del);
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

	Post::CGLObject* currentObject()
	{
		QTreeWidgetItem* current = m_tree->currentItem();
		CModelTreeItem* item = dynamic_cast<CModelTreeItem*>(current);
		if (item == 0) return 0;

		Post::CGLObject* po = item->Object();
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
};

CPostModelPanel::CPostModelPanel(CMainWindow* pwnd, QWidget* parent) : CCommandPanel(pwnd, parent), ui(new Ui::CPostModelPanel)
{
	ui->setupUi(this);

	QObject::connect(this, SIGNAL(postObjectStateChanged()), pwnd, SLOT(OnPostObjectStateChanged()));
	QObject::connect(this, SIGNAL(postObjectPropsChanged(Post::CGLObject*)), pwnd, SLOT(OnPostObjectPropsChanged(Post::CGLObject*)));
}

CPostDoc* CPostModelPanel::GetActiveDocument()
{
	CDocument* doc = GetMainWindow()->GetDocument();
	if (doc->FEBioJobs() == 0) return nullptr;
	return GetMainWindow()->GetActiveDocument();
}

void CPostModelPanel::selectObject(Post::CGLObject* po)
{
	if (po == 0) ui->m_tree->clearSelection();
	else
	{
		std::string name = po->GetName();
		QString s(name.c_str());
		QTreeWidgetItemIterator it(ui->m_tree);
		while (*it)
		{
			QString t = (*it)->text(0);
			string st = t.toStdString();
			if ((*it)->text(0) == s)
			{
				(*it)->setSelected(true);
				ui->m_tree->setCurrentItem(*it);
//				on_modelTree_currentItemChanged(*it, 0);
				break;
			}
			++it;
		}
	}
}

Post::CGLObject* CPostModelPanel::selectedObject()
{
	CModelTreeItem* item = dynamic_cast<CModelTreeItem*>(ui->m_tree->currentItem());
	if (item == nullptr) return nullptr;
	return item->Object();
}

void CPostModelPanel::UpdateView()
{
	QTreeWidgetItem* psel = ui->m_tree->currentItem();
	if (psel && (psel->text(0) == QString("View")))
	{
		on_postModel_currentItemChanged(psel, psel);
	}
}

void CPostModelPanel::Update()
{
	Update(true);
}

void CPostModelPanel::Update(bool breset)
{
	if (breset)
	{
//		CModelTreeItem* item = dynamic_cast<CModelTreeItem*>(ui->m_tree->currentItem());
/*		CGLObject* po = 0;
		if (item)
		{
			po = item->Object();
		}
*/
		// clear all property lists
		if (ui->m_list.isEmpty() == false)
		{
			QVector<CPropertyList*>::iterator it;
			for (it=ui->m_list.begin(); it != ui->m_list.end(); ++it) delete (*it);
			ui->m_list.clear();
		}

		// clear object list
		m_obj.clear();

		// hide the image viewer
		ui->HideImageViewer();

		ui->name->clear();

		// rebuild the tree
		CPostDoc* pdoc = GetActiveDocument();
		ui->m_props->Update(0);
		ui->m_tree->clear();
		if (pdoc && pdoc->IsValid())
		{
			Post::FEModel* fem = pdoc->GetFEModel();
			Post::CGLModel* mdl = pdoc->GetGLModel();

			CModelTreeItem* pi1 = nullptr;
			if (mdl)
			{
				pi1 = new CModelTreeItem(0, ui->m_tree);
				pi1->setText(0, QString::fromStdString(pdoc->GetTitle()));
				pi1->setIcon(0, QIcon(QString(":/icons/postview.png")));
				ui->m_list.push_back(new CModelProps(mdl));
				pi1->setData(0, Qt::UserRole, (int)(ui->m_list.size() - 1));
				m_obj.push_back(0);
				pi1->setExpanded(true);

				// add the mesh
				if (fem)
				{
					CModelTreeItem* pi2 = new CModelTreeItem(0, pi1);
					pi2->setText(0, "Mesh");
					pi2->setIcon(0, QIcon(QString(":/icons/mesh.png")));
					ui->m_list.push_back(new CMeshProps(fem));
					pi2->setData(0, Qt::UserRole, (int)(ui->m_list.size() - 1));
					m_obj.push_back(0);
				}

				Post::CGLDisplacementMap* map = mdl->GetDisplacementMap();
				if (map)
				{
					CModelTreeItem* pi2 = new CModelTreeItem(map, pi1);
					pi2->setText(0, QString::fromStdString(map->GetName()));
					//				pi2->setTextColor(0, map && map->IsActive() ? Qt::black : Qt::gray);
					pi2->setIcon(0, QIcon(QString(":/icons/distort.png")));
					ui->m_list.push_back(new CObjectProps(map));
					pi2->setData(0, Qt::UserRole, (int)(ui->m_list.size() - 1));
					m_obj.push_back(map);
				}

				Post::CGLColorMap* col = mdl->GetColorMap();
				if (col)
				{
					CModelTreeItem* pi2 = new CModelTreeItem(col, pi1);
					pi2->setText(0, QString::fromStdString(col->GetName()));
					//			pi2->setTextColor(0, col->IsActive() ? Qt::black : Qt::gray);
					pi2->setIcon(0, QIcon(QString(":/icons/colormap.png")));
					ui->m_list.push_back(new CObjectProps(col));
					pi2->setData(0, Qt::UserRole, (int)(ui->m_list.size() - 1));
					m_obj.push_back(0);
				}

				Post::GPlotList& pl = mdl->GetPlotList();
				for (int n = 0; n < pl.Size(); ++n)
				{
					Post::CGLPlot& plot = *pl[n];
					CModelTreeItem* pi1 = new CModelTreeItem(&plot, ui->m_tree);

					if (dynamic_cast<Post::CGLPlaneCutPlot    *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/cut.png")));
					else if (dynamic_cast<Post::CGLVectorPlot      *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/vectors.png")));
					else if (dynamic_cast<Post::CGLSlicePlot       *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/slice.png")));
					else if (dynamic_cast<Post::CGLIsoSurfacePlot  *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/isosurface.png")));
					else if (dynamic_cast<Post::CGLStreamLinePlot  *>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/streamlines.png")));
					else if (dynamic_cast<Post::CGLParticleFlowPlot*>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/particle.png")));
					else if (dynamic_cast<Post::GLTensorPlot*>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/tensor.png")));
					else if (dynamic_cast<Post::CGLMirrorPlane*>(&plot)) pi1->setIcon(0, QIcon(QString(":/icons/mirror.png")));

					string name = plot.GetName();

					pi1->setText(0, name.c_str());
					//				pi1->setTextColor(0, plot.IsActive() ? Qt::black : Qt::gray);
					ui->m_list.push_back(new CObjectProps(&plot));
					pi1->setData(0, Qt::UserRole, (int)(ui->m_list.size() - 1));
					m_obj.push_back(&plot);
				}
			}

/*			for (int i = 0; i < pdoc->ImageModels(); ++i)
			{
				Post::CImageModel* img = pdoc->GetImageModel(i);

				CModelTreeItem* pi1 = new CModelTreeItem(img, ui->m_tree);
				pi1->setText(0, QString::fromStdString(img->GetName()));
				pi1->setIcon(0, QIcon(QString(":/icons/image.png")));
				ui->m_list.push_back(new CImageModelProps(img));
				pi1->setData(0, Qt::UserRole, (int)(ui->m_list.size() - 1));
				m_obj.push_back(img);
				pi1->setExpanded(true);

				for (int j = 0; j < img->ImageRenderers(); ++j)
				{
					Post::CGLImageRenderer* render = img->GetImageRenderer(j);

					Post::CVolRender* volRender = dynamic_cast<Post::CVolRender*>(render);
					if (volRender)
					{
						CModelTreeItem* pi = new CModelTreeItem(volRender, pi1);
						pi->setText(0, QString::fromStdString(render->GetName()));
						//				pi->setTextColor(0, volRender->IsActive() ? Qt::black : Qt::gray);
						pi->setIcon(0, QIcon(QString(":/icons/volrender.png")));
						ui->m_list.push_back(new CObjectProps(volRender));
						pi->setData(0, Qt::UserRole, (int)(ui->m_list.size() - 1));
						m_obj.push_back(volRender);
					}

					Post::CImageSlicer* imgSlice = dynamic_cast<Post::CImageSlicer*>(render);
					if (imgSlice)
					{
						CModelTreeItem* pi = new CModelTreeItem(imgSlice, pi1);
						pi->setText(0, QString::fromStdString(render->GetName()));
						//				pi->setTextColor(0, imgSlice->IsActive() ? Qt::black : Qt::gray);
						pi->setIcon(0, QIcon(QString(":/icons/imageslice.png")));
						ui->m_list.push_back(new CObjectProps(imgSlice));
						pi->setData(0, Qt::UserRole, (int)(ui->m_list.size() - 1));
						m_obj.push_back(imgSlice);
					}

					Post::CMarchingCubes* marchCube = dynamic_cast<Post::CMarchingCubes*>(render);
					if (marchCube)
					{
						CModelTreeItem* pi = new CModelTreeItem(marchCube, pi1);
						pi->setText(0, QString::fromStdString(render->GetName()));
						//				pi->setTextColor(0, imgSlice->IsActive() ? Qt::black : Qt::gray);
						pi->setIcon(0, QIcon(QString(":/icons/marching_cubes.png")));
						ui->m_list.push_back(new CObjectProps(marchCube));
						pi->setData(0, Qt::UserRole, (int)(ui->m_list.size() - 1));
						m_obj.push_back(marchCube);
					}
				}
			}
*/	
			Post::CGView& view = *pdoc->GetView();
			pi1 = new CModelTreeItem(&view, ui->m_tree);
			pi1->setText(0, "View");
			pi1->setIcon(0, QIcon(QString(":/icons/view.png")));
			ui->m_list.push_back(new CViewProps(*GetMainWindow()->GetGLView()));
			pi1->setData(0, Qt::UserRole, (int) (ui->m_list.size() - 1));
			pi1->setExpanded(true);
			m_obj.push_back(0);

			for (int i=0; i<view.CameraKeys(); ++i)
			{
				GLCameraTransform& key = view.GetKey(i);
				CModelTreeItem* pi2 = new CModelTreeItem(&key, pi1);

				string name = key.GetName();
				pi2->setText(0, name.c_str());

				pi2->setIcon(0, QIcon(QString(":/icons/view.png")));
				ui->m_list.push_back(new CCameraTransformProps(key));
				pi2->setData(0, Qt::UserRole, (int) (ui->m_list.size() - 1));
				m_obj.push_back(&key);
			}
		}

		// This can crash PostView if po no longer exists (e.g. after new file is read)
//		if (po) selectObject(po);
	}
	else
	{
		on_postModel_currentItemChanged(ui->m_tree->currentItem(), 0);
	}
}

void CPostModelPanel::on_postModel_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev)
{
	if (current)
	{
		CModelTreeItem* item = dynamic_cast<CModelTreeItem*>(current);
		if (item)
		{
			Post::CGLObject* po = item->Object();
			if (po)
			{
				ui->enabled->setEnabled(true);
				ui->enabled->setChecked(po->IsActive());

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
			}
		}
		else
		{
			ui->HideImageViewer();
			ui->enabled->setEnabled(false);
			ui->enabled->setChecked(false);
		}

		ui->name->setText(current->text(0));
		QVariant v = current->data(0, Qt::UserRole);
		ui->m_props->Update(ui->m_list[v.toInt()]);
	}
	else
	{
		ui->HideImageViewer();
		ui->m_props->Update(0);
	}
}

void CPostModelPanel::on_postModel_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
	int n = item->data(0, Qt::UserRole).toInt();
	GLCameraTransform* pkey = dynamic_cast<GLCameraTransform*>(m_obj[n]);
	if (pkey)
	{
		Post::CGView* view = GetActiveDocument()->GetView();
		view->SetCurrentKey(pkey);
		GetMainWindow()->GetGLView()->GetCamera().SetTransform(*pkey);
		GetMainWindow()->RedrawGL();
	}
}

void CPostModelPanel::on_nameEdit_editingFinished()
{
	QString name = ui->name->text();
	QTreeWidgetItem* item = ui->m_tree->currentItem();
	if (item) item->setText(0, name);

	Post::CGLObject* po = selectedObject();
	if (po)
	{
		po->ChangeName(name.toStdString());
		GetMainWindow()->RedrawGL();
	}
}

void CPostModelPanel::on_deleteButton_clicked()
{
	CModelTreeItem* item = dynamic_cast<CModelTreeItem*>(ui->m_tree->currentItem());
	if (item)
	{
		QVariant v = item->data(0, Qt::UserRole);
		int n = v.toInt();
		Post::CGLObject* po = m_obj[n];
		if (po)
		{
			GetActiveDocument()->DeleteObject(po);
			item->SetObject(0);
			Update(true);
			GetMainWindow()->RedrawGL();
		}
		else QMessageBox::information(this, "FEBio Studio", "Cannot delete this object");
	}
}

void CPostModelPanel::on_props_dataChanged()
{
	Post::CGLObject* po = selectedObject();
	if (po) po->Update();

	emit postObjectPropsChanged(po);
}

void CPostModelPanel::on_enabled_stateChanged(int nstate)
{
	QTreeWidgetItem* current = ui->m_tree->currentItem();
	CModelTreeItem* item = dynamic_cast<CModelTreeItem*>(current);
	if (item == 0) return;

	Post::CGLObject* po = item->Object();
	if (po == 0) return;

	if (nstate == Qt::Unchecked)
	{
		po->Activate(false);
//		item->setTextColor(0, Qt::gray);
	}
	else if (nstate == Qt::Checked)
	{
		po->Activate(true);
//		item->setTextColor(0, Qt::black);
	}

	emit postObjectStateChanged();
}
/*
void CModelViewer::on_autoUpdate_toggled(bool b)
{
	ui->apply->setEnabled(!b);	
}

void CModelViewer::on_applyButton_clicked()
{
	CGLObject* po = ui->currentObject();
	if (po)
	{
		CGLModel* mdl = po->GetModel();
		if (mdl) po->Update(mdl->currentTimeIndex(), 0.f, true);
	}
}
*/
