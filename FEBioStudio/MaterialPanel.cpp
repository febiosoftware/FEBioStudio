#include "MaterialPanel.h"
#include <QBoxLayout>
#include <QTreeWidget>
#include <QListWidget>
#include <QSplitter>
#include <QLabel>
#include <QCheckBox>
#include <QToolButton>
#include "MainWindow.h"
#include "Document.h"
#include "PostDoc.h"
#include "PropertyListView.h"
#include <PostLib/FEPostModel.h>
#include <PostLib/FEMaterial.h>
#include <PostGL/GLModel.h>

class MaterialProps : public CPropertyList
{
public:
	// NOTE: Changes to the list should be reflected in void CMaterialPanel::on_props_dataChanged(int nprop)
	MaterialProps()
	{
		m_mat = 0;
		addProperty("Render mode"      , CProperty::Enum, "Render mode")->setEnumValues(QStringList() << "default" << "wireframe" << "solid");
		addProperty("Color"            , CProperty::Color );
//		addProperty("Ambient"          , CProperty::Color );
		addProperty("Specular color"   , CProperty::Color );
		addProperty("Emission color"   , CProperty::Color );
		addProperty("Mesh color"       , CProperty::Color );
		addProperty("Shininess"        , CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Transparency"     , CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Transparency mode", CProperty::Enum, "Transparency mode")->setEnumValues(QStringList() << "constant" << "normal-weighed");
		addProperty("Show Mesh"        , CProperty::Bool);
		addProperty("Cast shadows"     , CProperty::Bool);
		addProperty("Clip"             , CProperty::Bool);
	}

	void SetMaterial(Post::FEMaterial* pmat) { m_mat = pmat; }

	QVariant GetPropertyValue(int i)
	{
		QVariant v;
		if (m_mat)
		{
			switch (i)
			{
			case 0: v = m_mat->m_nrender; break;
			case 1: v = toQColor(m_mat->diffuse); break;
//			case 2: v = toQColor(m_mat->ambient); break;
			case 2: v = toQColor(m_mat->specular); break;
			case 3: v = toQColor(m_mat->emission); break;
			case 4: v = toQColor(m_mat->meshcol); break;
			case 5: v = m_mat->shininess; break;
			case 6: v = m_mat->transparency; break;
			case 7: v = m_mat->m_ntransmode; break;
			case 8: v = m_mat->bmesh; break;
			case 9: v = m_mat->bcast_shadows; break;
			case 10: v = m_mat->bclip; break;
			}
		}
		return v;
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		if (m_mat)
		{
			switch (i)
			{
			case 0: m_mat->m_nrender = v.toInt(); break;
			case 1: m_mat->diffuse  = m_mat->ambient = toGLColor(v.value<QColor>()); break;
//			case 2: m_mat->ambient  = toGLColor(v.value<QColor>()); break;
			case 2: m_mat->specular = toGLColor(v.value<QColor>()); break;
			case 3: m_mat->emission = toGLColor(v.value<QColor>()); break;
			case 4: m_mat->meshcol  = toGLColor(v.value<QColor>()); break;
			case 5: m_mat->shininess = v.toFloat(); break;
			case 6: m_mat->transparency = v.toFloat(); break;
			case 7: m_mat->m_ntransmode = v.toInt(); break;
			case 8: m_mat->bmesh = v.toBool(); break;
			case 9: m_mat->bcast_shadows = v.toBool(); break;
			case 10: m_mat->bclip = v.toBool(); break;
			}
		}
	}

private:
	Post::FEMaterial*	m_mat;
};

class Ui::CMaterialPanel
{
public:
	QListWidget*			m_list;
	::CPropertyListView*	m_prop;
	QLineEdit*	name;
	QToolButton* pshow;
	QToolButton* pcheck;

	bool update;

public:
	void setupUi(::CMaterialPanel* parent)
	{
		update = true;

		QVBoxLayout* pg = new QVBoxLayout;
		pg->setMargin(0);

		QSplitter* psplitter = new QSplitter;
		psplitter->setOrientation(Qt::Vertical);
		pg->addWidget(psplitter);

		m_list = new QListWidget;
		m_list->setObjectName(QStringLiteral("materialList"));
		m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);

		QWidget* w = new QWidget;
		QVBoxLayout* pvl = new QVBoxLayout;
		pvl->setMargin(0);

		pshow = new QToolButton; pshow->setObjectName("showButton");
		pshow->setIcon(QIcon(":/icons/eye.png"));
		pshow->setCheckable(true);
		pshow->setToolTip("<font color=\"black\">Show or hide material");

		pcheck = new QToolButton; pcheck->setObjectName("enableButton");
		pcheck->setIcon(QIcon(":/icons/check.png"));
		pcheck->setCheckable(true);
		pcheck->setToolTip("<font color=\"black\">Enable or disable material");


		QHBoxLayout* ph = new QHBoxLayout;
//		ph->setSpacing(0);
		ph->addWidget(name = new QLineEdit, 2); name->setObjectName("editName");
		ph->addWidget(pshow);
		ph->addWidget(pcheck);
		ph->addStretch();
		pvl->addLayout(ph);

		m_prop = new ::CPropertyListView;
		m_prop->setObjectName("matprops");
		pvl->addWidget(m_prop);
		w->setLayout(pvl);

		psplitter->addWidget(m_list);
		psplitter->addWidget(w);

		parent->setLayout(pg);

		QMetaObject::connectSlotsByName(parent);
	}

	void setColor(QListWidgetItem* item, GLColor c)
	{
		QColor c2 = QColor::fromRgb(c.r, c.g, c.b);
		QColor c1 = c2.lighter();
		QColor c3 = c2.darker();

		QRadialGradient g(QPointF(8, 8), 12);
		g.setColorAt(0.0, c1);
		g.setColorAt(0.2, c2);
		g.setColorAt(1.0, c3);

		QPixmap pix(24, 24);
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		p.setRenderHint(QPainter::Antialiasing);
		p.setPen(Qt::PenStyle::NoPen);
		p.setBrush(QBrush(g));
		p.drawEllipse(2, 2, 20, 20);
		p.end();
		item->setIcon(QIcon(pix));
	}
};

CMaterialPanel::CMaterialPanel(CMainWindow* pwnd, QWidget* parent) : CCommandPanel(pwnd, parent), ui(new Ui::CMaterialPanel)
{
	ui->setupUi(this);
	m_pmat = new MaterialProps;
}

CPostDoc* CMaterialPanel::GetActiveDocument()
{
	CDocument* doc = GetMainWindow()->GetDocument();
	if (doc->FEBioJobs() == 0) return nullptr;
	return GetMainWindow()->GetActiveDocument();
}

void CMaterialPanel::Update(bool breset)
{
	if (breset == false) return;

	ui->m_list->clear();
	ui->m_prop->Update(0);
	m_pmat->SetMaterial(0);

	CPostDoc* pdoc = GetActiveDocument();
	if (pdoc == nullptr) return;

	Post::FEPostModel* fem = pdoc->GetFEModel();
	if (fem)
	{
		int nmat = fem->Materials();
		for (int i=0; i<nmat; ++i)
		{
			Post::FEMaterial& mat = *fem->GetMaterial(i);

			QListWidgetItem* it = new QListWidgetItem(mat.GetName());
			ui->m_list->addItem(it);

			ui->setColor(it, mat.diffuse);
		}

		if (nmat > 0)
			ui->m_list->setCurrentRow(0);

		UpdateStates();
	}
}

void CMaterialPanel::UpdateStates()
{
	CPostDoc* pdoc = GetActiveDocument();
	Post::FEPostModel* fem = pdoc->GetFEModel();
	if (fem == 0) return;

	int nmat = fem->Materials();
	for (int i=0; i<nmat; ++i)
	{
		Post::FEMaterial& mat = *fem->GetMaterial(i);
		QListWidgetItem* pi = ui->m_list->item(i);
		QFont font = pi->font();
		if (mat.visible())
		{
//			pi->setForeground(Qt::black);
			font.setBold(mat.enabled());
			pi->setFont(font);
		}
		else 
		{
//			pi->setForeground(Qt::gray);
			font.setBold(mat.enabled());
			pi->setFont(font);
		}

//		pi->setBackgroundColor((mat.enabled() ? Qt::white : Qt::yellow));
	}
}

void CMaterialPanel::on_materialList_currentRowChanged(int nrow)
{
	CPostDoc* doc = GetActiveDocument();
	if (doc && doc->IsValid())
	{
		Post::FEPostModel& fem = *doc->GetFEModel();
		if ((nrow >= 0) && (nrow < fem.Materials()))
		{
			Post::FEMaterial* pmat = fem.GetMaterial(nrow);
			m_pmat->SetMaterial(pmat);
			ui->m_prop->Update(m_pmat);
			ui->name->setText(QString(pmat->GetName()));

			ui->update = false;
			ui->pcheck->setChecked(pmat->enabled());
			ui->pshow->setChecked(pmat->visible());
			ui->update = true;
		}
	}
}

void CMaterialPanel::on_showButton_toggled(bool b)
{
	if (ui->update == false) return;

	CPostDoc& doc = *GetActiveDocument();
	if (doc.IsValid() == false) return;

	Post::CGLModel& mdl = *doc.GetGLModel();
	Post::FEPostModel& fem = *doc.GetFEModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	QItemSelectionModel* pselect = ui->m_list->selectionModel();
	QModelIndexList selection = pselect->selectedRows();
	int ncount = selection.count();
	for (int i=0; i<ncount; ++i)
	{
		QModelIndex index = selection.at(i);
		int nmat = index.row();

		Post::FEMaterial& mat = *fem.GetMaterial(nmat);
		if (b)
		{
			mat.show();
			mdl.ShowMaterial(nmat);
		}
		else
		{
			mat.hide();
			mdl.HideMaterial(nmat);
		}
	}

	UpdateStates();
	GetMainWindow()->RedrawGL();
}

void CMaterialPanel::on_enableButton_toggled(bool b)
{
	if (ui->update == false) return;

	CPostDoc& doc = *GetActiveDocument();
	if (doc.IsValid() == false) return;

	Post::CGLModel& mdl = *doc.GetGLModel();
	Post::FEPostModel& fem = *doc.GetFEModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	QItemSelectionModel* pselect = ui->m_list->selectionModel();
	QModelIndexList selection = pselect->selectedRows();
	int ncount = selection.count();
	for (int i=0; i<ncount; ++i)
	{
		QModelIndex index = selection.at(i);
		int nmat = index.row();

		Post::FEMaterial& mat = *fem.GetMaterial(nmat);

		if (b) mat.enable();
		else mat.disable();
	}
	mdl.UpdateMeshState();
	mdl.ResetAllStates();
	doc.UpdateFEModel(true);
	UpdateStates();
	GetMainWindow()->RedrawGL();
}

void CMaterialPanel::on_editName_editingFinished()
{
	CPostDoc& doc = *GetActiveDocument();
	QModelIndex n = ui->m_list->currentIndex();
	if (n.isValid())
	{
		int nmat = n.row();

		Post::FEPostModel& fem = *doc.GetFEModel();
		Post::FEMaterial& mat = *fem.GetMaterial(nmat);

		QListWidgetItem* item = ui->m_list->item(nmat);
		string name = ui->name->text().toStdString();
		mat.SetName(name.c_str());
		item->setText(ui->name->text());
	}
}

void CMaterialPanel::SetItemColor(int nmat, GLColor c)
{
	QListWidgetItem* item = ui->m_list->item(nmat);
	ui->setColor(item, c);
}

void CMaterialPanel::on_matprops_dataChanged(int nprop)
{
	// Get the model
	CPostDoc& doc = *GetActiveDocument();
	Post::CGLModel& mdl = *doc.GetGLModel();
	Post::FEPostModel& fem = *doc.GetFEModel();

	// get the current material
	QModelIndex currentIndex = ui->m_list->currentIndex();
	if (currentIndex.isValid() == false) return;

	// get the current material
	int nmat = currentIndex.row();
	Post::FEMaterial& currentMat = *fem.GetMaterial(nmat);

	// update color of corresponding item in material list
	if (nprop == 1) SetItemColor(nmat, currentMat.diffuse);

	// update all the other selected materials
	QItemSelectionModel* pselect = ui->m_list->selectionModel();
	QModelIndexList selection = pselect->selectedRows();
	int ncount = selection.count();
	for (int i = 0; i<ncount; ++i)
	{
		QModelIndex index = selection.at(i);
		if (index.row() != currentIndex.row())
		{
			int imat = index.row();
			Post::FEMaterial& mati = *fem.GetMaterial(imat);

			switch (nprop)
			{
			case  0: mati.m_nrender     = currentMat.m_nrender; break;
			case  1: 
				mati.diffuse = currentMat.diffuse; 
				mati.ambient = currentMat.ambient;
				break;
			case  2: mati.specular      = currentMat.specular; break;
			case  3: mati.emission      = currentMat.emission; break;
			case  4: mati.meshcol       = currentMat.meshcol; break;
			case  5: mati.shininess     = currentMat.shininess; break;
			case  6: mati.transparency  = currentMat.transparency; break;
			case  7: mati.m_ntransmode  = currentMat.m_ntransmode; break;
			case  8: mati.bmesh         = currentMat.bmesh; break;
			case  9: mati.bcast_shadows = currentMat.bcast_shadows; break;
			case 10: mati.bclip         = currentMat.bclip; break;
			default:
				assert(false);
			};

			// update color of corresponding item in material list
			if (nprop == 1) SetItemColor(imat, mati.diffuse);
		}
	}

	mdl.Update(false);
	GetMainWindow()->RedrawGL();
}
