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

#include "MaterialPanel.h"
#include <QBoxLayout>
#include <QTreeWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QSplitter>
#include <QLabel>
#include <QCheckBox>
#include <QToolButton>
#include "MainWindow.h"
#include "GLModelDocument.h"
#include "PropertyListView.h"
#include <PostLib/FEPostModel.h>
#include <PostLib/Material.h>
#include <PostGL/GLModel.h>
#include "GLHighlighter.h"
#include "MatEditButton.h"

class MaterialProps : public CPropertyList
{
public:
	enum {
		RENDER_MODE,
		OPACITY,
		OPACITY_MODE,
		SHOW_MESH,
		MESH_COLOR,
		CLIP_MESH
	};

public:
	// NOTE: Changes to the list should be reflected in void CMaterialPanel::on_matprops_dataChanged(int nprop)
	MaterialProps()
	{
		m_mat = 0;
		addProperty("Render mode"      , CProperty::Enum)->setEnumValues(QStringList() << "default" << "solid" << "wireframe");
		addProperty("Opacity"          , CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Opacity mode"     , CProperty::Enum)->setEnumValues(QStringList() << "constant" << "normal-weigthed" << "value-weigthed");
		addProperty("Show Mesh"        , CProperty::Bool);
		addProperty("Mesh color"       , CProperty::Color );
		addProperty("Clip"             , CProperty::Bool);
	}

	void SetMaterial(Post::Material* pmat) { m_mat = pmat; }

	QVariant GetPropertyValue(int i)
	{
		QVariant v;
		if (m_mat)
		{
			switch (i)
			{
			case RENDER_MODE : v = m_mat->m_nrender; break;
			case OPACITY     : v = m_mat->transparency; break;
			case OPACITY_MODE: v = m_mat->m_ntransmode; break;
			case SHOW_MESH   : v = m_mat->bmesh; break;
			case MESH_COLOR  : v = toQColor(m_mat->meshcol); break;
			case CLIP_MESH   : v = m_mat->bclip; break;
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
			case RENDER_MODE : m_mat->m_nrender = v.toInt(); break;
			case OPACITY     : m_mat->transparency = v.toFloat(); break;
			case OPACITY_MODE: m_mat->m_ntransmode = v.toInt(); break;
			case SHOW_MESH   : m_mat->bmesh = v.toBool(); break;
			case MESH_COLOR  : m_mat->meshcol  = toGLColor(v.value<QColor>()); break;
			case CLIP_MESH   : m_mat->bclip = v.toBool(); break;
			}
		}
	}

private:
	Post::Material*	m_mat;
};

class Ui::CMaterialPanel
{
public:
	QTableWidget*			m_list;
	::CPropertyListView*	m_prop;
	QLineEdit* m_flt;
	QToolButton* highlightButton;
	CMatEditButton* matEditButton;
	QLineEdit* nameEdit;

	bool update;

public:
	void setupUi(::CMaterialPanel* parent)
	{
		update = true;

		QVBoxLayout* pg = new QVBoxLayout;
		pg->setContentsMargins(0,0,0,0);

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(highlightButton = new QToolButton);
		h->addWidget(new QLabel("Filter:"));
		h->addWidget(m_flt = new QLineEdit); m_flt->setObjectName("filter");
		pg->addLayout(h);

		highlightButton->setIcon(QIcon(":/icons/select_highlight.png"));
		highlightButton->setObjectName("highlightButton");
		highlightButton->setAutoRaise(true);
		highlightButton->setToolTip("<font color=\"black\">Toggle selection highlighting");
		highlightButton->setCheckable(true);
		highlightButton->setChecked(false);

		QSplitter* psplitter = new QSplitter;
		psplitter->setOrientation(Qt::Vertical);
		pg->addWidget(psplitter);

		m_list = new QTableWidget;
		m_list->setColumnCount(3);
		QHeaderView* hh = m_list->horizontalHeader();
		hh->setDefaultSectionSize(24);
		hh->setMinimumSectionSize(24);
		hh->setSectionResizeMode(0, QHeaderView::Stretch);
		hh->setSectionResizeMode(1, QHeaderView::ResizeToContents);
		hh->setSectionResizeMode(2, QHeaderView::ResizeToContents);
		hh->hide();
		m_list->verticalHeader()->hide();
		m_list->setObjectName(QStringLiteral("materialList"));
		m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
		m_list->setSelectionBehavior(QAbstractItemView::SelectRows);

		QHBoxLayout* hl = new QHBoxLayout;
		hl->addWidget(new QLabel("Name:"), 0, Qt::AlignTop);
		hl->addWidget(nameEdit = new QLineEdit, 0, Qt::AlignTop);
		hl->addWidget(matEditButton = new CMatEditButton, 0, Qt::AlignTop);
		matEditButton->setObjectName("matEditButton");
		nameEdit->setObjectName("nameEdit");

		QWidget* w = new QWidget;
		QVBoxLayout* pvl = new QVBoxLayout;
		pvl->setContentsMargins(0,0,0,0);
		pvl->addLayout(hl);

		m_prop = new ::CPropertyListView;
		m_prop->setObjectName("matprops");
		pvl->addWidget(m_prop);
		w->setLayout(pvl);

		psplitter->addWidget(m_list);
		psplitter->addWidget(w);

		parent->setLayout(pg);

		QMetaObject::connectSlotsByName(parent);
	}

	QString GetFilterText()
	{
		return m_flt->text();
	}

	void setColor(QTableWidgetItem* item, GLColor c)
	{
		QColor c2 = QColor::fromRgb(c.r, c.g, c.b);
		QColor c1 = c2.lighter();
		QColor c3 = c2.darker();

		QRadialGradient g(QPointF(8, 8), 12);
		g.setColorAt(0.0, c1);
		g.setColorAt(0.2, c2);
		g.setColorAt(1.0, c3);

		QPixmap pix(24, 24);
		// NOTE: This was commented out since this makes the icons scale incorrectly. 
//        pix.setDevicePixelRatio(m_list->devicePixelRatio());
		pix.fill(Qt::transparent);
		QPainter p(&pix);
		p.setRenderHint(QPainter::Antialiasing);
		p.setPen(Qt::PenStyle::NoPen);
		p.setBrush(QBrush(g));
		p.drawEllipse(2, 2, 20, 20);
		p.end();
		item->setIcon(QIcon(pix));
	}

	void setMaterial(Post::Material* pmat)
	{
		GLMaterial mat;
		if (pmat)
		{
			mat.type = GLMaterial::PLASTIC;
			mat.diffuse = pmat->diffuse;
			mat.ambient = pmat->ambient;
			mat.specular = pmat->specular;
			mat.shininess = pmat->shininess;
			mat.reflection = pmat->reflectivity;
			matEditButton->setMaterial(mat);
		}
	}

	void setName(const QString& name)
	{
		nameEdit->setText(name);
	}

	void SetItemColor(int index, GLColor c)
	{
		QTableWidgetItem* item = m_list->item(index, 0);
		setColor(item, c);
	}

	void SetItemName(int index, QString name)
	{
		QTableWidgetItem* item = m_list->item(index, 0);
		item->setText(name);
	}
};

CMaterialPanel::CMaterialPanel(CMainWindow* pwnd, QWidget* parent) : CWindowPanel(pwnd, parent), ui(new Ui::CMaterialPanel)
{
	ui->setupUi(this);
	m_pmat = new MaterialProps;
}

Post::CGLModel* CMaterialPanel::GetActiveModel()
{
	CGLModelDocument* gldoc = dynamic_cast<CGLModelDocument*>(GetMainWindow()->GetDocument());
	if ((gldoc == nullptr) || !gldoc->IsValid()) return nullptr;

	// get the model and make sure the model has a valid post model
	Post::CGLModel* glm = gldoc->GetGLModel();
	if (glm && (glm->GetFSModel() == nullptr)) return nullptr;

	return glm;
}

void CMaterialPanel::Update(bool breset)
{
	if (breset == false) return;

	ui->m_list->setRowCount(0);
	ui->m_prop->Update(0);
	m_pmat->SetMaterial(0);

	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;
	Post::FEPostModel* fem = glm->GetFSModel();

	QString flt = ui->GetFilterText();


		int nmat = fem->Materials();
		int nrows = 0;
		for (int i = 0; i < nmat; ++i)
		{
			Post::Material& mat = *fem->GetMaterial(i);

			QString name = QString::fromStdString(mat.GetName());
			if (flt.isEmpty() || name.contains(flt, Qt::CaseInsensitive))
			{
				nrows++;
			}
		}
		ui->m_list->setRowCount(nrows);
		nrows = 0;
		for (int i=0; i<nmat; ++i)
		{
			Post::Material& mat = *fem->GetMaterial(i);

			QString name = QString::fromStdString(mat.GetName());
			if (flt.isEmpty() || name.contains(flt, Qt::CaseInsensitive))
			{
				QTableWidgetItem* it = new QTableWidgetItem(name);
				it->setFlags(Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled);
				it->setData(Qt::UserRole, i);
				ui->m_list->setItem(nrows, 0, it);
				ui->setColor(it, mat.diffuse);

				it = new QTableWidgetItem();
				it->setData(Qt::UserRole, i);
				it->setTextAlignment(Qt::AlignRight);
				if (mat.bvisible)
					it->setFlags(Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled);
				else
					it->setFlags(Qt::ItemFlag::ItemIsSelectable);
				it->setIcon(QIcon(":/icons/eye.png"));
				it->setTextAlignment(Qt::AlignHCenter);
				ui->m_list->setItem(nrows, 1, it);

				it = new QTableWidgetItem();
				it->setData(Qt::UserRole, i);
				if (mat.benable)
					it->setFlags(Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled);
				else
					it->setFlags(Qt::ItemFlag::ItemIsSelectable);
				it->setIcon(QIcon(":/icons/check.png"));
				it->setTextAlignment(Qt::AlignHCenter);
				ui->m_list->setItem(nrows, 2, it);
				nrows++;
			}
		}

		if (nrows > 0)
			ui->m_list->setCurrentItem(ui->m_list->item(0, 0));

	UpdateStates();
}

void CMaterialPanel::UpdateStates()
{
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;
	Post::FEPostModel* fem = glm->GetFSModel();

	int nmat = fem->Materials();
	int items = ui->m_list->rowCount();
	for (int i=0; i<items; ++i)
	{
		QTableWidgetItem* pi = ui->m_list->item(i, 0);

		int imat = pi->data(Qt::UserRole).toInt();
		if ((imat >= 0) && (imat < nmat))
		{
			Post::Material& mat = *fem->GetMaterial(imat);
			QFont font = pi->font();
			font.setItalic(!mat.visible());
			font.setBold(mat.enabled());
			pi->setFont(font);

			if (mat.bvisible)
				ui->m_list->item(i, 1)->setIcon(QIcon(":/icons/show.png"));
			else
				ui->m_list->item(i, 1)->setIcon(QIcon(":/icons/hide.png"));
		
			if (mat.benable)
				ui->m_list->item(i, 2)->setIcon(QIcon(":/icons/check.png"));
			else
				ui->m_list->item(i, 2)->setIcon(QIcon(":/icons/disabled.png"));
		}
	}
}

void CMaterialPanel::on_materialList_currentItemChanged(QTableWidgetItem* current, QTableWidgetItem* prev)
{
	Post::CGLModel* glm = GetActiveModel();
	if (glm && current)
	{
		int imat = current->data(Qt::UserRole).toInt();

		Post::FEPostModel& fem = *glm->GetFSModel();
		if ((imat >= 0) && (imat < fem.Materials()))
		{
			Post::Material* pmat = fem.GetMaterial(imat);
			m_pmat->SetMaterial(pmat);
			ui->m_prop->Update(m_pmat);
			ui->setMaterial(pmat);

			ui->setName(QString::fromStdString(pmat->GetName()));
		}
	}
}

void CMaterialPanel::on_materialList_itemClicked(QTableWidgetItem* item)
{
	if (ui->update == false) return;

	Post::CGLModel* glm = GetActiveModel();
	if (glm && item)
	{
		int nrow = item->row();
		int ncol = item->column();
		int imat = ui->m_list->item(nrow, 0)->data(Qt::UserRole).toInt();

		Post::CGLModel& mdl = *glm;
		Post::FEPostModel& fem = *mdl.GetFSModel();
		if ((imat >= 0) && (imat < fem.Materials()) && (ncol > 0))
		{
			Post::Material& mat = *fem.GetMaterial(imat);

			if (ncol == 1)
			{
				mat.bvisible = !mat.bvisible;
			}
			else if (ncol == 2)
			{
				mat.benable = !mat.benable;
			}

			if (item->isSelected())
			{
				// update all the other selected materials
				QItemSelectionModel* pselect = ui->m_list->selectionModel();
				QModelIndexList selection = pselect->selectedRows();
				int ncount = selection.count();
				for (int i = 0; i < ncount; ++i)
				{
					QModelIndex index = selection.at(i);
					if (index.row() != nrow)
					{
						int imat = ui->m_list->item(index.row(), 0)->data(Qt::UserRole).toInt();
						Post::Material& mati = *fem.GetMaterial(imat);
						mati.bvisible = mat.bvisible;
						mati.benable = mat.benable;
					}
				}
			}

			UpdateStates();
			if (ncol == 1)
			{
				mdl.UpdateMeshVisibility();
				CGLDocument* postDoc = GetDocument();
				if (postDoc) postDoc->UpdateSelection();
			}
			else if (ncol == 2)
			{
				fem.UpdateMeshState();
				fem.ResetAllStates();
			}
			mdl.Update(true);
		}

		GLHighlighter::ClearHighlights();
		if (ui->highlightButton->isChecked() && (imat >= 0))
		{
			// find all the parts that belong to this material
			CPostObject* po = mdl.GetPostObject();
			if (po)
			{
				vector<GPart*> parts;
				for (int i = 0; i < po->Parts(); ++i)
				{
					GPart* pg = po->Part(i);
					if (pg && pg->GetMaterialID() == imat)
					{
						parts.push_back(pg);
					}
				}

				for (GPart* part : parts)
					GLHighlighter::PickItem(part, 0);
			}
		}

		GetMainWindow()->RedrawGL();
	}
}

void CMaterialPanel::on_filter_textChanged(const QString& txt)
{
	Update(true);
}

void CMaterialPanel::on_highlightButton_toggled(bool)
{
	on_materialList_itemClicked(ui->m_list->currentItem());
}

void CMaterialPanel::on_nameEdit_editingFinished()
{
	// Get the model
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;
	Post::FEPostModel& fem = *glm->GetFSModel();

	// get the current material
	QModelIndex currentIndex = ui->m_list->currentIndex();
	if (currentIndex.isValid() == false) return;

	// get the current material
	int nmat = ui->m_list->item(currentIndex.row(), 0)->data(Qt::UserRole).toInt();
	Post::Material& currentMat = *fem.GetMaterial(nmat);

	QString newName = ui->nameEdit->text();
	if (newName.isEmpty())
	{
		ui->setName(QString::fromStdString(currentMat.GetName()));
	}
	else
	{
		currentMat.SetName(newName.toStdString());
		ui->SetItemName(currentIndex.row(), newName);
	}
}

void CMaterialPanel::on_matEditButton_materialChanged(GLMaterial col)
{
	// Get the model
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;
	Post::FEPostModel& fem = *glm->GetFSModel();

	// get the current material
	QModelIndex currentIndex = ui->m_list->currentIndex();
	if (currentIndex.isValid() == false) return;

	// get the current material
	int nmat = ui->m_list->item(currentIndex.row(), 0)->data(Qt::UserRole).toInt();
	Post::Material& currentMat = *fem.GetMaterial(nmat);

	currentMat.ambient = col.ambient;
	currentMat.diffuse = col.diffuse;
	currentMat.specular = col.specular;
	currentMat.emission = col.emission;
	currentMat.shininess = col.shininess;
	currentMat.reflectivity = col.reflection;

	// update color of corresponding item in material list
	ui->SetItemColor(currentIndex.row(), currentMat.diffuse);

	glm->Update(false);
	GetMainWindow()->RedrawGL();
}

void CMaterialPanel::on_matprops_dataChanged(int nprop)
{
	// Get the model
	Post::CGLModel* glm = GetActiveModel();
	if (glm == nullptr) return;
	Post::FEPostModel& fem = *glm->GetFSModel();

	// get the current material
	QModelIndex currentIndex = ui->m_list->currentIndex();
	if (currentIndex.isValid() == false) return;

	// get the current material
	int nmat = ui->m_list->item(currentIndex.row(), 0)->data(Qt::UserRole).toInt();
	Post::Material& currentMat = *fem.GetMaterial(nmat);

	// update all the other selected materials
	QItemSelectionModel* pselect = ui->m_list->selectionModel();
	QModelIndexList selection = pselect->selectedRows();
	int ncount = selection.count();
	for (int i = 0; i<ncount; ++i)
	{
		QModelIndex index = selection.at(i);
		if (index.row() != currentIndex.row())
		{
			int imat = ui->m_list->item(index.row(), 0)->data(Qt::UserRole).toInt();
			Post::Material& mati = *fem.GetMaterial(imat);

			switch (nprop)
			{
			case MaterialProps::RENDER_MODE : mati.m_nrender     = currentMat.m_nrender; break;
			case MaterialProps::OPACITY     : mati.transparency  = currentMat.transparency; break;
			case MaterialProps::OPACITY_MODE: mati.m_ntransmode  = currentMat.m_ntransmode; break;
			case MaterialProps::SHOW_MESH   : mati.bmesh         = currentMat.bmesh; break;
			case MaterialProps::MESH_COLOR  : mati.meshcol       = currentMat.meshcol; break;
			case MaterialProps::CLIP_MESH   : mati.bclip         = currentMat.bclip; break;
			default:
				assert(false);
			};
		}
	}

	glm->Update(false);
	GetMainWindow()->RedrawGL();
}
