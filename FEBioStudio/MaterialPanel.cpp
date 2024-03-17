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
#include "Document.h"
#include "PostDocument.h"
#include "PropertyListView.h"
#include <PostLib/FEPostModel.h>
#include <PostLib/Material.h>
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
		addProperty("Transparency mode", CProperty::Enum, "Transparency mode")->setEnumValues(QStringList() << "constant" << "normal-weigthed" << "value-weigthed");
		addProperty("Show Mesh"        , CProperty::Bool);
		addProperty("Cast shadows"     , CProperty::Bool);
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
	Post::Material*	m_mat;
};

class Ui::CMaterialPanel
{
public:
	QTableWidget*			m_list;
	::CPropertyListView*	m_prop;
	QLineEdit* m_flt;

	bool update;

public:
	void setupUi(::CMaterialPanel* parent)
	{
		update = true;

		QVBoxLayout* pg = new QVBoxLayout;
		pg->setContentsMargins(0,0,0,0);

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(new QLabel("Filter:"));
		h->addWidget(m_flt = new QLineEdit); m_flt->setObjectName("filter");
		pg->addLayout(h);

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

		QWidget* w = new QWidget;
		QVBoxLayout* pvl = new QVBoxLayout;
		pvl->setContentsMargins(0,0,0,0);


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
};

CMaterialPanel::CMaterialPanel(CMainWindow* pwnd, QWidget* parent) : CCommandPanel(pwnd, parent), ui(new Ui::CMaterialPanel)
{
	ui->setupUi(this);
	m_pmat = new MaterialProps;
}

CPostDocument* CMaterialPanel::GetActiveDocument()
{
	return GetMainWindow()->GetPostDocument();
}

void CMaterialPanel::Update(bool breset)
{
	if (breset == false) return;

	ui->m_list->setRowCount(0);
	ui->m_prop->Update(0);
	m_pmat->SetMaterial(0);

	CPostDocument* pdoc = GetActiveDocument();
	if (pdoc == nullptr) return;

	Post::FEPostModel* fem = pdoc->GetFSModel();
	if (fem)
	{
		QString flt = ui->GetFilterText();


		int nmat = fem->Materials();
		int nrows = 0;
		for (int i = 0; i < nmat; ++i)
		{
			Post::Material& mat = *fem->GetMaterial(i);

			QString name(mat.GetName());
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

			QString name(mat.GetName());
			if (flt.isEmpty() || name.contains(flt, Qt::CaseInsensitive))
			{
				QTableWidgetItem* it = new QTableWidgetItem(mat.GetName());
				it->setFlags(Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled);
				it->setData(Qt::UserRole, i);
				ui->m_list->setItem(nrows, 0, it);
				ui->setColor(it, mat.diffuse);

				it = new QTableWidgetItem();
				it->setTextAlignment(Qt::AlignRight);
				if (mat.bvisible)
					it->setFlags(Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled);
				else
					it->setFlags(Qt::ItemFlag::ItemIsSelectable);
				it->setIcon(QIcon(":/icons/eye.png"));
				it->setTextAlignment(Qt::AlignHCenter);
				ui->m_list->setItem(nrows, 1, it);

				it = new QTableWidgetItem();
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
}

void CMaterialPanel::UpdateStates()
{
	CPostDocument* pdoc = GetActiveDocument();
	Post::FEPostModel* fem = pdoc->GetFSModel();
	if (fem == 0) return;

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
	CPostDocument* doc = GetActiveDocument();
	if (doc && doc->IsValid() && current)
	{
		int imat = current->data(Qt::UserRole).toInt();

		Post::FEPostModel& fem = *doc->GetFSModel();
		if ((imat >= 0) && (imat < fem.Materials()))
		{
			Post::Material* pmat = fem.GetMaterial(imat);
			m_pmat->SetMaterial(pmat);
			ui->m_prop->Update(m_pmat);
		}
	}
}

void CMaterialPanel::on_materialList_itemClicked(QTableWidgetItem* item)
{
	CPostDocument* doc = GetActiveDocument();
	if (doc && doc->IsValid() && item)
	{
		int nrow = item->row();
		int ncol = item->column();
		int imat = ui->m_list->item(nrow, 0)->data(Qt::UserRole).toInt();

		Post::CGLModel& mdl = *doc->GetGLModel();
		Post::FEPostModel& fem = *doc->GetFSModel();
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

			UpdateStates();
			if      (ncol == 1) mdl.UpdateMeshVisibility();
			else if (ncol == 2)
			{
				mdl.UpdateMeshState();
				mdl.ResetAllStates();
				doc->UpdateFEModel(true);
			}
			GetMainWindow()->RedrawGL();
		}
	}
}

void CMaterialPanel::SetItemColor(int index, GLColor c)
{
	QTableWidgetItem* item = ui->m_list->item(index, 0);
	ui->setColor(item, c);
}

void CMaterialPanel::on_filter_textChanged(const QString& txt)
{
	Update(true);
}

void CMaterialPanel::on_matprops_dataChanged(int nprop)
{
	// Get the model
	CPostDocument& doc = *GetActiveDocument();
	Post::CGLModel& mdl = *doc.GetGLModel();
	Post::FEPostModel& fem = *doc.GetFSModel();

	// get the current material
	QModelIndex currentIndex = ui->m_list->currentIndex();
	if (currentIndex.isValid() == false) return;

	// get the current material
	int nmat = ui->m_list->item(currentIndex.row(), 0)->data(Qt::UserRole).toInt();
	Post::Material& currentMat = *fem.GetMaterial(nmat);

	// update color of corresponding item in material list
	if (nprop == 1) SetItemColor(currentIndex.row(), currentMat.diffuse);

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
			if (nprop == 1) SetItemColor(index.row(), mati.diffuse);
		}
	}

	mdl.Update(false);
	GetMainWindow()->RedrawGL();
}
