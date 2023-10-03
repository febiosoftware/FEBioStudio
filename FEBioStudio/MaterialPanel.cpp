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
#include <QListWidget>
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
	QListWidget*			m_list;
	::CPropertyListView*	m_prop;
	QLineEdit*	name;
	QToolButton* pshow;
	QToolButton* pcheck;
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

		m_list = new QListWidget;
		m_list->setObjectName(QStringLiteral("materialList"));
		m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);

		QWidget* w = new QWidget;
		QVBoxLayout* pvl = new QVBoxLayout;
		pvl->setContentsMargins(0,0,0,0);

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

	QString GetFilterText()
	{
		return m_flt->text();
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

	ui->m_list->clear();
	ui->m_prop->Update(0);
	m_pmat->SetMaterial(0);

	CPostDocument* pdoc = GetActiveDocument();
	if (pdoc == nullptr) return;

	Post::FEPostModel* fem = pdoc->GetFSModel();
	if (fem)
	{
		QString flt = ui->GetFilterText();

		int nmat = fem->Materials();
		for (int i=0; i<nmat; ++i)
		{
			Post::Material& mat = *fem->GetMaterial(i);

			QString name(mat.GetName());
			if (flt.isEmpty() || name.contains(flt, Qt::CaseInsensitive))
			{
				QListWidgetItem* it = new QListWidgetItem(mat.GetName());
				it->setData(Qt::UserRole, i);
				ui->m_list->addItem(it);
				ui->setColor(it, mat.diffuse);
			}
		}

		if (nmat > 0)
			ui->m_list->setCurrentRow(0);

		UpdateStates();
	}
}

void CMaterialPanel::UpdateStates()
{
	CPostDocument* pdoc = GetActiveDocument();
	Post::FEPostModel* fem = pdoc->GetFSModel();
	if (fem == 0) return;

	int nmat = fem->Materials();
	int items = ui->m_list->count();
	for (int i=0; i<items; ++i)
	{
		QListWidgetItem* pi = ui->m_list->item(i);

		int imat = pi->data(Qt::UserRole).toInt();
		if ((imat >= 0) && (imat < nmat))
		{
			Post::Material& mat = *fem->GetMaterial(imat);
			QFont font = pi->font();
			font.setItalic(!mat.visible());
			font.setBold(mat.enabled());
			pi->setFont(font);
//			pi->setBackgroundColor((mat.enabled() ? Qt::white : Qt::yellow));
		}
	}
}

void CMaterialPanel::on_materialList_currentRowChanged(int nrow)
{
	CPostDocument* doc = GetActiveDocument();
	if (doc && doc->IsValid())
	{
		if ((nrow >= 0) && (nrow < ui->m_list->count()))
		{
			QListWidgetItem* pi = ui->m_list->item(nrow);
			int imat = pi->data(Qt::UserRole).toInt();

			Post::FEPostModel& fem = *doc->GetFSModel();
			if ((imat >= 0) && (imat < fem.Materials()))
			{
				Post::Material* pmat = fem.GetMaterial(imat);
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
}

void CMaterialPanel::on_showButton_toggled(bool b)
{
	if (ui->update == false) return;

	CPostDocument& doc = *GetActiveDocument();
	if (doc.IsValid() == false) return;

	Post::CGLModel& mdl = *doc.GetGLModel();
	Post::FEPostModel& fem = *doc.GetFSModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	QItemSelectionModel* pselect = ui->m_list->selectionModel();
	QModelIndexList selection = pselect->selectedRows();
	int ncount = selection.count();
	for (int i=0; i<ncount; ++i)
	{
		QModelIndex index = selection.at(i);
		QListWidgetItem* it = ui->m_list->item(index.row());
		int nmat = it->data(Qt::UserRole).toInt();

		Post::Material& mat = *fem.GetMaterial(nmat);
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

	CPostDocument& doc = *GetActiveDocument();
	if (doc.IsValid() == false) return;

	Post::CGLModel& mdl = *doc.GetGLModel();
	Post::FEPostModel& fem = *doc.GetFSModel();
	Post::FEPostMesh& mesh = *fem.GetFEMesh(0);

	QItemSelectionModel* pselect = ui->m_list->selectionModel();
	QModelIndexList selection = pselect->selectedRows();
	int ncount = selection.count();
	for (int i=0; i<ncount; ++i)
	{
		QModelIndex index = selection.at(i);
		QListWidgetItem* it = ui->m_list->item(index.row());
		int nmat = it->data(Qt::UserRole).toInt();

		Post::Material& mat = *fem.GetMaterial(nmat);

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
	CPostDocument& doc = *GetActiveDocument();
	QModelIndex n = ui->m_list->currentIndex();
	if (n.isValid())
	{
		QListWidgetItem* it = ui->m_list->item(n.row());
		int nmat = it->data(Qt::UserRole).toInt();

		Post::FEPostModel& fem = *doc.GetFSModel();
		Post::Material& mat = *fem.GetMaterial(nmat);

		string name = ui->name->text().toStdString();
		mat.SetName(name.c_str());
		it->setText(ui->name->text());
	}
}

void CMaterialPanel::SetItemColor(int index, GLColor c)
{
	QListWidgetItem* item = ui->m_list->item(index);
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
	int nmat = ui->m_list->item(currentIndex.row())->data(Qt::UserRole).toInt();
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
			int imat = ui->m_list->item(index.row())->data(Qt::UserRole).toInt();
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
