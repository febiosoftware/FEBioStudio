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
#include "DlgSettings.h"
#include "MainWindow.h"
#include "Document.h"
#include "DocManager.h"
#include "GLView.h"
#include <QToolButton>
#include <QPushButton>
#include <QListWidget>
#include <QApplication>
#include <QStyleHints>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QStackedWidget>
#include <QGroupBox>
#include <QFileDialog>
#include <QSplitter>
#include "PropertyList.h"
#include "PropertyListForm.h"
#include "PropertyListView.h"
#include "CColorButton.h"
#include <GLWLib/convert.h>
#include <FSCore/Palette.h>
#include <PostGL/GLColorMap.h>
#include "ModelDocument.h"
#include "RepositoryPanel.h"
#include "units.h"
#include "DlgSetRepoFolder.h"
#include "IconProvider.h"
#include "PostDocument.h"
#include <PostGL/GLModel.h>
#include "PaletteViewer.h"
#include <GLLib/GLScene.h>
#include <FSCore/ColorMapManager.h>

//-----------------------------------------------------------------------------
class CBackgroundProps : public CDataPropertyList
{
public:
	CBackgroundProps()
	{
		addColorProperty(&m_bg1, "Background color 1");
		addColorProperty(&m_bg2, "Background color 2");
		addColorProperty(&m_fg, "Foreground color");

		QStringList styles;
		styles << "Color 1" << "Color 2" << "Horizontal" << "Vertical";
		addEnumProperty(&m_nstyle, "Background style")->setEnumValues(styles);

		m_nstyle = 0;
	}

public:
	QColor m_bg1, m_bg2;
	QColor	m_fg;
	int		m_nstyle;
};

//-----------------------------------------------------------------------------
class CDisplayProps : public CDataPropertyList
{
public:
	CDisplayProps()
	{
		addDoubleProperty(&m_nodeSize, "Size of nodes");
		addDoubleProperty(&m_lineSize, "Line width");
		addColorProperty (&m_meshColor, "Mesh color"  );
		addDoubleProperty(&m_meshOpacity, "Mesh opacity")->setFloatRange(0, 1);
		addBoolProperty  (&m_bnormal, "Show normals"  );
		addDoubleProperty(&m_scaleNormal, "Normals scale factor");
		addBoolProperty(&m_showHighlights, "Enable highlighting");
		addBoolProperty(&m_identifyBackfacing, "Identify backfacing faces");
		QStringList vconv;
		vconv <<"First-angle projection (XZ)"<<"First-angle projection (XY)"<<"Third-angle projection (XY)";
		addEnumProperty(&m_nconv, "Multiview projection")->setEnumValues(vconv);
		addEnumProperty(&m_ntrans, "Object transparency mode")->setEnumValues(QStringList() << "None" << "Selected only" << "Unselected only");
		addBoolProperty(&m_dozsorting, "Improved Transparency");
		addEnumProperty(&m_defaultFGColorOption, "Default text color option")->setEnumValues(QStringList() << "Theme" << "Custom");
		addColorProperty(&m_defaultFGColor, "Custom text color");
		addIntProperty(&m_tagFontSize, "Tag font size")->setIntRange(5, 100);
	}

public:
	double	m_nodeSize;
	double	m_lineSize;
	QColor  m_meshColor;
	double	m_meshOpacity;
	bool	m_bnormal;
	double	m_scaleNormal;
	int		m_nconv;
	int		m_ntrans;
	bool	m_dozsorting;
	int		m_defaultFGColorOption;
	QColor	m_defaultFGColor;
	bool	m_showHighlights;
	bool	m_identifyBackfacing;
	int		m_tagFontSize;
};

//-----------------------------------------------------------------------------
class CPhysicsProps : public CDataPropertyList
{
public:
	CPhysicsProps()
	{
		addBoolProperty  (&m_showRigidBodies, "Show rigid bodies");
		addBoolProperty  (&m_showRigidJoints, "Show rigid joints");
		addBoolProperty  (&m_showRigidLabels, "Show rigid labels");
		addBoolProperty  (&m_showRigidWalls , "Show rigid walls" );
		addDoubleProperty(&m_fiberScale   , "Fiber scale factor"  );
		addBoolProperty  (&m_showMatAxes  ,"Show material axes"   );
		addBoolProperty  (&m_showHiddenFibers, "Show fibers/axes on hidden parts");
	}

public:
	bool	m_showRigidBodies;
	bool	m_showRigidJoints;
	bool	m_showRigidLabels;
	bool	m_showRigidWalls;
	bool	m_showMatAxes;
	double	m_fiberScale;
	bool	m_showHiddenFibers;
};

//-----------------------------------------------------------------------------
class CUIProps : public CDataPropertyList
{
public:
	CUIProps(QDialog* parent, CMainWindow* wnd) : m_dlg(parent), m_wnd(wnd)
	{
		addBoolProperty(&m_apply, "Emulate apply action");
		addBoolProperty(&m_bcmd , "Clear undo stack on save");
		addProperty("Recent files list", CProperty::Action)->info = QString("Clear");
		addIntProperty(&m_autoSaveInterval, "AutoSave Interval (s)");
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		if (i == 2)
		{
			if (QMessageBox::question(m_dlg, "FEBio Studio", "Are you sure you want to clear all the recent files list.\nThis can not be undone!") == QMessageBox::Yes)
			{
				m_wnd->ClearRecentFilesList();
			}
		}
		else CDataPropertyList::SetPropertyValue(i, v);
	}

public:
	QDialog*	m_dlg;
	CMainWindow* m_wnd;
	bool	m_apply;
	bool	m_bcmd;
	int		m_theme;
	int		m_autoSaveInterval;
};

//-----------------------------------------------------------------------------
class CSelectionProps : public CDataPropertyList
{
public:
	CSelectionProps()
	{
		addBoolProperty(&m_bconnect, "Select connected");
		addEnumProperty(&m_ntagInfo, "Tag info")->setEnumValues(QStringList() << "Off" << "Item numbers" << "Item numbers and connecting nodes");
		addBoolProperty(&m_backface, "Ignore backfacing items");
		addBoolProperty(&m_binterior, "Ignore interior items");
		addBoolProperty(&m_bpart    , "Respect partitions");
		m_bconnect = false;
		m_ntagInfo = 0;
	}

public:
	bool	m_bconnect;
	int		m_ntagInfo;
	bool	m_backface;
	bool	m_binterior;
	bool	m_bpart;
};

//-----------------------------------------------------------------------------
class CLightingProps : public CPropertyList
{
public:
	CLightingProps()
	{
		addProperty("Enable lighting", CProperty::Bool);
		addProperty("Diffuse intensity", CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Ambient intensity", CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Render shadows", CProperty::Bool);
		addProperty("Shadow intensity", CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Light direction"  , CProperty::Vec3);
		addProperty("Environment map"  , CProperty::Resource);
		addProperty("Use environment map"  , CProperty::Bool);

		m_blight = true;
		m_diffuse = 0.7f;
		m_ambient = 0.3f;
		m_bshadow = false;
		m_shadow = 0.1f;
	}

	QVariant GetPropertyValue(int i)
	{
		QVariant v;
		switch (i)
		{
		case 0: return m_blight; break;
		case 1: return m_diffuse; break;
		case 2: return m_ambient; break;
		case 3: return m_bshadow; break;
		case 4: return m_shadow; break;
		case 5: return vecToString(m_pos); break;
		case 6: return m_envmap; break;
		case 7: return m_useEV; break;
		}
		return v;
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		switch (i)
		{
		case 0: m_blight = v.toBool(); break;
		case 1: m_diffuse = v.toFloat(); break;
		case 2: m_ambient = v.toFloat(); break;
		case 3: m_bshadow = v.toBool(); break;
		case 4: m_shadow = v.toFloat(); break;
		case 5: m_pos = stringToVec(v.toString()); break;
		case 6: m_envmap = v.toString(); break;
		case 7: m_useEV = v.toBool(); break;
		}
	}

public:
	bool	m_blight;
	float	m_diffuse;
	float	m_ambient;
	bool	m_bshadow;
	float	m_shadow;
	vec3f	m_pos;
	QString m_envmap;
	bool	m_useEV;
};

//-----------------------------------------------------------------------------
class CCameraProps : public CPropertyList
{
public:
	CCameraProps()
	{
		addProperty("Animate camera", CProperty::Bool);
		addProperty("Animation speed", CProperty::Float)->setFloatRange(0.0, 1.0);
		addProperty("Animation bias", CProperty::Float)->setFloatRange(0.0, 1.0);

		m_banim = true;
		m_speed = 0.8f;
		m_bias = 0.8f;
	}

	QVariant GetPropertyValue(int i)
	{
		QVariant v;
		switch (i)
		{
		case 0: return m_banim; break;
		case 1: return m_speed; break;
		case 2: return m_bias; break;
		}
		return v;
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		switch (i)
		{
		case 0: m_banim = v.toBool(); break;
		case 1: m_speed = v.toFloat(); break;
		case 2: m_bias = v.toFloat(); break;
		}
	}

public:
	bool	m_banim;
	float	m_speed;
	float	m_bias;
};

//-----------------------------------------------------------------------------
class CPostProps : public CPropertyList
{
public:
	CPostProps()
	{
		addProperty("Default colormap range", CProperty::Enum)->setEnumValues(QStringList() << "dynamic" << "static");
		m_defrng = 0;
	}

	QVariant GetPropertyValue(int i)
	{
		QVariant v;
		switch (i)
		{
		case 0: return m_defrng; break;
		}
		return v;
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		switch (i)
		{
		case 0: m_defrng = v.toInt(); break;
		}
	}

public:
	int	m_defrng;
};


//=================================================================================================
ColorGradient::ColorGradient(QWidget* parent) : QWidget(parent)
{
	m_map.jet();
}

QSize ColorGradient::sizeHint() const
{
	return QSize(20, 20);
}

void ColorGradient::setColorMap(const CColorMap& m)
{
	m_map = m;
	repaint();
}

void ColorGradient::paintEvent(QPaintEvent* ev)
{
	QRect rt = rect();
	QPainter p(this);

	int r = 3;
	int x0 = rt.left();
	int x1 = rt.right();
	int y0 = rt.top();
	int y1 = y0 + 2 * r;
	int y2 = rt.bottom();
	int w = rt.width();

	p.fillRect(x0, y0, w, y1 - y0, Qt::white);

	int ncol = m_map.Colors();
	for (int i = 0; i<ncol; ++i)
	{
		float x = m_map.GetColorPos(i);
		int xi = x0 + (int)(w*x) - r;
		p.setPen(Qt::gray);
		p.setBrush(Qt::gray);
		p.drawEllipse(xi, y0, 2 * r, 2 * r);
	}

	for (int i = 0; i<rt.width(); ++i)
	{
		float w = (float)i / rt.width();
		GLColor c = m_map.map(w);
		p.setPen(QColor(c.r, c.g, c.b));
		p.drawLine(x0 + i, y1, x0 + i, y2);
	}
}

//=================================================================================================
CPaletteWidget::CPaletteWidget(QWidget* parent) : QWidget(parent)
{
	QLabel* label = new QLabel("Current palette:");
	label->setFixedWidth(120);
	label->setAlignment(Qt::AlignRight | Qt::AlignCenter);

	// build the palette selector
	pal = new QComboBox; label->setBuddy(pal); pal->setObjectName("select");
	CPaletteManager& PM = CPaletteManager::GetInstance();
	int pals = PM.Palettes();
	for (int i = 0; i < pals; ++i)
	{
		pal->addItem(QString::fromStdString(PM.Palette(i).Name()));
	}
	pal->setCurrentIndex(PM.CurrentIndex());

	// build the layout
	QHBoxLayout* h0 = new QHBoxLayout;
	h0->addWidget(label);
	h0->addWidget(pal);

	QPushButton* load = new QPushButton("Load Palette ..."); load->setObjectName("load");
	QPushButton* save = new QPushButton("Save Palette ..."); save->setObjectName("save");
	QPushButton* create = new QPushButton("Create palette from materials ..."); create->setObjectName("create");
	QPushButton* apply = new QPushButton("Apply palette to materials ..."); apply->setObjectName("apply");

	QVBoxLayout* buttons = new QVBoxLayout;
	buttons->addWidget(load);
	buttons->addWidget(save);
	buttons->addWidget(create);
	buttons->addWidget(apply);

	QHBoxLayout* h1 = new QHBoxLayout;
	h1->addWidget(view = new CPaletteViewer);
	h1->addLayout(buttons);

	view->setPalette(PM.CurrentPalette());

	QVBoxLayout* pl = new QVBoxLayout;
	pl->addLayout(h0);
	pl->addLayout(h1);
	pl->addStretch();

	setLayout(pl);

	QMetaObject::connectSlotsByName(this);
}

int CPaletteWidget::currentPalette() const
{
	return pal->currentIndex();
}

void CPaletteWidget::on_select_currentIndexChanged(int n)
{
	CPaletteManager& PM = CPaletteManager::GetInstance();
	if ((n >= 0) && (n < PM.Palettes()))
	{
		view->setPalette(PM.Palette(n));
	}
}

void CPaletteWidget::on_load_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Load Palette", "", "FBS Palette (*.xml)");
	if (!fileName.isEmpty())
	{
		CPaletteManager& PM = CPaletteManager::GetInstance();
		string filename = fileName.toStdString();
		if (!PM.Load(filename))
		{
			QMessageBox::critical(this, "Load Palette", "Failed to load palette from file.");
		}
		else
		{
			const CPalette& newPalette = PM.Palette(PM.Palettes() - 1);
			pal->addItem(QString::fromStdString(newPalette.Name()));
		}
	}
}

void CPaletteWidget::on_save_clicked()
{
	CPaletteManager& PM = CPaletteManager::GetInstance();
	int n = currentPalette();
	if ((n < 0) || (n >= PM.Palettes())) return;
	const CPalette& pal = PM.Palette(n);

	QString fileName = QFileDialog::getSaveFileName(this, "Save Palette", "", "FBS Palette (*.xml)");
	if (!fileName.isEmpty())
	{
		string filename = fileName.toStdString();
		if (!PM.Save(filename, pal))
		{
			QMessageBox::critical(this, "Save Palette", "Failed to save current palette to file.");
		}
	}
}

void CPaletteWidget::on_create_clicked()
{
	QString name = QInputDialog::getText(this, "Create palette", "Name:");
	if (!name.isEmpty())
	{
		CPalette palette(name.toStdString());
		CDocument* doc = CDocument::GetActiveDocument();

		CModelDocument* modelDoc = dynamic_cast<CModelDocument*>(doc);
		if (modelDoc)
		{
			FSModel* fem = modelDoc->GetFSModel();
			if (fem)
			{
				for (int i = 0; i < fem->Materials(); ++i)
				{
					GMaterial* mat = fem->GetMaterial(i);
					palette.AddColor(mat->GetColor());
				}
			}
		}

		CPostDocument* postDoc = dynamic_cast<CPostDocument*>(doc);
		if (postDoc)
		{
			Post::FEPostModel* fem = postDoc->GetFSModel();
			if (fem)
			{
				for (int i = 0; i < fem->Materials(); ++i)
				{
					Post::Material* mat = fem->GetMaterial(i);
					palette.AddColor(mat->diffuse);
				}
			}
		}

		if (palette.Colors() > 0)
		{
			CPaletteManager& PM = CPaletteManager::GetInstance();
			PM.AddPalette(palette);
			pal->addItem(name);
		}
	}
}

void CPaletteWidget::on_apply_clicked()
{
	CPaletteManager& PM = CPaletteManager::GetInstance();
	int n = currentPalette();
	if ((n < 0) || (n >= PM.Palettes())) return;
	const CPalette& pal = PM.Palette(n);
	int ncol = pal.Colors();

	CDocument* doc = CDocument::GetActiveDocument();

	CModelDocument* modelDoc = dynamic_cast<CModelDocument*>(doc);
	if (modelDoc)
	{
		FSModel* fem = modelDoc->GetFSModel();
		if (fem)
		{
			for (int i = 0; i < fem->Materials(); ++i)
			{
				GMaterial* mat = fem->GetMaterial(i);
				mat->SetColor(pal.Color(i % ncol));
			}
		}
	}

	CPostDocument* postDoc = dynamic_cast<CPostDocument*>(doc);
	if (postDoc)
	{
		Post::FEPostModel* fem = postDoc->GetFSModel();
		if (fem)
		{
			for (int i = 0; i < fem->Materials(); ++i)
			{
				Post::Material* mat = fem->GetMaterial(i);
				mat->setColor(pal.Color(i % ncol));
			}
		}
	}
}

//-----------------------------------------------------------------------------
CColormapWidget::CColormapWidget(QWidget* parent) : QWidget(parent)
{
	QHBoxLayout* h = new QHBoxLayout;
	QComboBox* l = m_maps = new QComboBox;
	l->setMinimumWidth(120);
	l->setCurrentIndex(0);

	QPushButton* newButton = new QPushButton("New ...");
	QPushButton* delButton = new QPushButton("Delete");
	QPushButton* editButton = new QPushButton("Edit ...");
	h->addWidget(new QLabel("Select color map:"));
	h->addWidget(l);
	h->addWidget(newButton);
	h->addWidget(delButton);
	h->addWidget(editButton);
	h->addStretch();

	QVBoxLayout* v = new QVBoxLayout;
	v->addLayout(h);

	h = new QHBoxLayout;

	m_spin = new QSpinBox;
	m_spin->setRange(2, 9);
	m_spin->setMaximumWidth(75);
	m_spin->setMinimumWidth(50);

	m_default = new QCheckBox("Set as default");

	h->addWidget(new QLabel("Number of colors:"));
	h->addWidget(m_spin);
	h->addStretch();
	v->addWidget(m_default);
	v->addLayout(h);

	m_grid = new QGridLayout;

	v->addLayout(m_grid);

	QPushButton* invertButton = new QPushButton("Invert");
	h = new QHBoxLayout();
	h->addStretch();
	h->addWidget(invertButton);
	v->addLayout(h);
	v->addStretch();

	h = new QHBoxLayout();
	h->addWidget(m_grad = new ColorGradient);
	v->addLayout(h);

	setLayout(v);

	m_currentMap = 0;
	m_changed = false;

	QObject::connect(l, SIGNAL(currentIndexChanged(int)), this, SLOT(currentMapChanged(int)));
	QObject::connect(m_spin, SIGNAL(valueChanged(int)), this, SLOT(onSpinValueChanged(int)));
	QObject::connect(newButton, SIGNAL(clicked()), this, SLOT(onNew()));
	QObject::connect(delButton, SIGNAL(clicked()), this, SLOT(onDelete()));
	QObject::connect(editButton, SIGNAL(clicked()), this, SLOT(onEdit()));
	QObject::connect(invertButton, SIGNAL(clicked()), this, SLOT(onInvert()));
	QObject::connect(m_default, SIGNAL(stateChanged(int)), this, SLOT(onSetDefault(int)));

	updateMaps();
	currentMapChanged(0);
}

void CColormapWidget::updateMaps()
{
	m_maps->clear();
	for (int i = 0; i < ColorMapManager::ColorMaps(); ++i)
	{
		string name = ColorMapManager::GetColorMapName(i);
		m_maps->addItem(QString(name.c_str()));
	}
}

void CColormapWidget::onSetDefault(int nstate)
{
	if (nstate == Qt::Checked)
	{
		int n = m_maps->currentIndex();
		ColorMapManager::SetDefaultMap(n);
		m_default->setDisabled(true);
	}
	else
		m_default->setDisabled(false);
}

void CColormapWidget::onNew()
{
	int n = ColorMapManager::UserColorMaps() + 1;
	QString name = QString("user%1").arg(n);
	bool bok = true;
	QString newName = QInputDialog::getText(this, "New color map", "name:", QLineEdit::Normal, name, &bok);
	if (bok && (newName.isEmpty() == false))
	{
		const CColorMap& map = ColorMapManager::GetColorMap(m_currentMap);
		string sname = newName.toStdString();
		ColorMapManager::AddColormap(sname, map);

		updateMaps();
		m_maps->setCurrentIndex(ColorMapManager::ColorMaps() - 1);
	}
}

void CColormapWidget::onDelete()
{
	if (ColorMapManager::RemoveColormap(m_currentMap) == false)
	{
		QMessageBox::critical(this, "Delete Colormap", "Cannot delete default color maps.");
	}
	else
	{
		m_maps->removeItem(m_currentMap);
	}
}

void CColormapWidget::onEdit()
{
	string sname = ColorMapManager::GetColorMapName(m_currentMap);
	QString name = QString::fromStdString(sname);
	bool bok = true;
	QString newName = QInputDialog::getText(this, "Edit color map", "name:", QLineEdit::Normal, name, &bok);
	if (bok && (newName.isEmpty() == false))
	{
		ColorMapManager::SetColorMapName(m_currentMap, newName.toStdString());
		m_maps->setItemText(m_currentMap, newName);
	}
}

void CColormapWidget::onInvert()
{
	m_map.Invert();
	updateColorMap(m_map);
	m_changed = true;
}

void CColormapWidget::updateColorMap(const CColorMap& map)
{
	clearGrid();

	m_spin->setValue(map.Colors());

	QLineEdit* l;
	CColorButton* b;
	for (int i = 0; i<map.Colors(); ++i)
	{
		QColor c = toQColor(map.GetColor(i));
		float f = map.GetColorPos(i);

		m_grid->addWidget(new QLabel(QString("Color %1").arg(i + 1)), i, 0, Qt::AlignRight);
		m_grid->addWidget(l = new QLineEdit, i, 1); l->setValidator(new QDoubleValidator); l->setText(QString::number(f)); l->setMaximumWidth(100);
		m_grid->addWidget(b = new CColorButton, i, 2); b->setColor(c);

		QObject::connect(l, SIGNAL(editingFinished()), this, SLOT(onDataChanged()));
		QObject::connect(b, SIGNAL(colorChanged(QColor)), this, SLOT(onDataChanged()));
	}

	m_grad->setColorMap(map);
}

void CColormapWidget::clearGrid()
{
	while (m_grid->count())
	{
		QLayoutItem* item = m_grid->takeAt(0);
		delete item->widget();
		delete item;
	}
}

void CColormapWidget::Apply()
{
	ColorMapManager::SetColormap(m_currentMap, m_map);
}

void CColormapWidget::currentMapChanged(int n)
{
	if (m_changed && (m_currentMap >= 0))
	{
		if (QMessageBox::question(this, "FEBio Studio", "The current map was changed. Do you want to keep the changes?") == QMessageBox::Yes)
		{
			ColorMapManager::SetColormap(m_currentMap, m_map);
			emit colormapChanged(m_currentMap);
		}
	}

	m_currentMap = n;
	m_changed = false;
	if (n != -1)
	{
		m_currentMap = n;
		const CColorMap& tex = ColorMapManager::GetColorMap(m_currentMap);
		m_map = tex;

		updateColorMap(ColorMapManager::GetColorMap(n));

		int defaultMap = ColorMapManager::GetDefaultMap();
		if (n == defaultMap)
		{
			m_default->setChecked(true);
			m_default->setDisabled(true);
		}
		else
		{
			m_default->setChecked(false);
			m_default->setEnabled(true);
		}
	}
	else m_default->setDisabled(true);
}

void CColormapWidget::onDataChanged()
{
	CColorMap& map = m_map;
	for (int i = 0; i<map.Colors(); ++i)
	{
		QLineEdit* pos = dynamic_cast<QLineEdit*>(m_grid->itemAtPosition(i, 1)->widget()); assert(pos);
		if (pos)
		{
			float f = pos->text().toFloat();
			map.SetColorPos(i, f);
		}

		CColorButton* col = dynamic_cast<CColorButton*>(m_grid->itemAtPosition(i, 2)->widget()); assert(col);
		if (col)
		{
			QColor c = col->color();
			map.SetColor(i, toGLColor(c));
		}
	}
	m_grad->setColorMap(map);
	m_changed = true;
}

void CColormapWidget::onSpinValueChanged(int n)
{
	if (m_map.Colors() != n)
	{
		m_map.SetColors(n);
		updateColorMap(m_map);
		m_changed = true;
	}
}

//-----------------------------------------------------------------------------
CUnitWidget::CUnitWidget(CMainWindow* wnd, QWidget* parent) : QWidget(parent), m_wnd(wnd)
{
	m_ops = new QComboBox;

	QStringList units = Units::SupportedUnitSystems();
	m_us = new QComboBox();
	m_us->addItems(units);

	m_unit = Units::GetUnitSystem();
	m_us->setCurrentIndex(m_unit);

	QFormLayout* f = new QFormLayout();
	f->setContentsMargins(0,0,0,0);
	f->addRow("Change for:", m_ops);
	f->addRow("Unit system:", m_us);

	QGroupBox* bu = new QGroupBox("Base units");
	QFormLayout* fb = new QFormLayout;
	fb->setLabelAlignment(Qt::AlignRight);
	fb->addRow("Length:"       , m_name[ 0] = new QLabel);
	fb->addRow("Mass:"         , m_name[ 1] = new QLabel);
	fb->addRow("Time:"         , m_name[ 2] = new QLabel);
	fb->addRow("Temperature:"  , m_name[ 3] = new QLabel);
	fb->addRow("Current:"      , m_name[ 4] = new QLabel);
	fb->addRow("Substance:"    , m_name[ 5] = new QLabel);
	bu->setLayout(fb);

	QGroupBox* bd = new QGroupBox("Derived units");
	QFormLayout* fd = new QFormLayout;
	fd->setLabelAlignment(Qt::AlignRight);
	fd->addRow("Force:"        , m_name[ 6] = new QLabel);
	fd->addRow("Pressure:"     , m_name[ 7] = new QLabel);
	fd->addRow("Energy:"       , m_name[ 8] = new QLabel);
	fd->addRow("Power:"        , m_name[ 9] = new QLabel);
    fd->addRow("Voltage:"      , m_name[10] = new QLabel);
    fd->addRow("Concentration:", m_name[11] = new QLabel);
	bd->setLayout(fd);

	QVBoxLayout* el = new QVBoxLayout;
	el->setContentsMargins(0,0,0,0);
	el->addWidget(bu);
	el->addWidget(bd);

	m_edit = new QWidget;
	m_edit->setLayout(el);

	QVBoxLayout* l = new QVBoxLayout;
	l->addLayout(f);
	l->addWidget(m_edit);
	l->addStretch();
	setLayout(l);

	if (m_unit == 0) m_edit->hide();

	QObject::connect(m_us, SIGNAL(currentIndexChanged(int)), this, SLOT(OnUnitSystemChanged(int)));
	QObject::connect(m_ops, SIGNAL(currentIndexChanged(int)), this, SLOT(OnUnitOptionChanged(int)));

	OnUnitSystemChanged(m_unit);
}

void CUnitWidget::showAllOptions(bool b)
{
	m_ops->clear();
	if (b)
	{
		m_ops->addItems(QStringList() << "New models" << "Current model" << "All open models");
		m_ops->setCurrentIndex(1);
	}
	else
		m_ops->addItems(QStringList() << "New models");
}

int CUnitWidget::getOption()
{
	return m_ops->currentIndex();
}

void CUnitWidget::setUnit(int n)
{
	m_unit = n;
	m_us->setCurrentIndex(n);
}

void CUnitWidget::OnUnitSystemChanged(int n)
{
	m_unit = n;

	if (n == 0) m_edit->hide();
	else m_edit->show();

	m_name[ 0]->setText(Units::GetUnitString(n, Units::LENGTH));
	m_name[ 1]->setText(Units::GetUnitString(n, Units::MASS));
	m_name[ 2]->setText(Units::GetUnitString(n, Units::TIME));
	m_name[ 3]->setText(Units::GetUnitString(n, Units::TEMPERATURE));
	m_name[ 4]->setText(Units::GetUnitString(n, Units::CURRENT));
	m_name[ 5]->setText(Units::GetUnitString(n, Units::SUBSTANCE));
	m_name[ 6]->setText(Units::GetUnitString(n, Units::FORCE));
	m_name[ 7]->setText(Units::GetUnitString(n, Units::PRESSURE));
	m_name[ 8]->setText(Units::GetUnitString(n, Units::ENERGY));
	m_name[ 9]->setText(Units::GetUnitString(n, Units::POWER));
    m_name[10]->setText(Units::GetUnitString(n, Units::VOLTAGE));
    m_name[11]->setText(Units::GetUnitString(n, Units::CONCENTRATION));
}

//-----------------------------------------------------------------------------
void CUnitWidget::OnUnitOptionChanged(int n)
{
	if (n == 0) m_us->setCurrentIndex(m_wnd->GetDefaultUnitSystem());
	else if (n == 1)
	{
		CGLDocument* doc = m_wnd->GetGLDocument();
		if (doc) m_us->setCurrentIndex(doc->GetUnitSystem());
	}
}

//-----------------------------------------------------------------------------
CRepoSettingsWidget::CRepoSettingsWidget(QWidget* parent) : QWidget(parent)
{
}

void CRepoSettingsWidget::setupUi()
{
	QVBoxLayout* layout = new QVBoxLayout;
	layout->setAlignment(Qt::AlignTop);

	QHBoxLayout* pathLayout = new QHBoxLayout;
	pathLayout->addWidget(new QLabel("Repository Folder:"));
	pathLayout->addWidget(repoPathEdit = new QLineEdit(repoPath));

	QAction* openFileDialog = new QAction;
	openFileDialog->setObjectName("openFileDialog");
	openFileDialog->setIcon(CIconProvider::GetIcon("open"));

	QToolButton* pathButton = new QToolButton;
	pathButton->setDefaultAction(openFileDialog);
	pathLayout->addWidget(pathButton);

	layout->addLayout(pathLayout);

	this->setLayout(layout);

	QObject::connect(openFileDialog, &QAction::triggered, this, &CRepoSettingsWidget::pathButton_clicked);
}

void CRepoSettingsWidget::pathButton_clicked()
{
	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::Directory);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setDirectory(repoPathEdit->text());

	if(dlg.exec())
	{
		QStringList files = dlg.selectedFiles();
		QString fileName = files.first();

		repoPathEdit->setText(fileName);
	}
}

//-----------------------------------------------------------------------------

CUpdateSettingsWidget::CUpdateSettingsWidget(QDialog* settings, CMainWindow* wnd, QWidget* parent)
	: QWidget(parent), m_settings(settings), m_wnd(wnd) 
{
	QVBoxLayout* layout = new QVBoxLayout;
	layout->setAlignment(Qt::AlignTop);

	QLabel* updateLabel = new QLabel("Click the button below to check for an update to the latest releases of FEBio and\nFEBio Studio.");
	// updateLabel->setWordWrap(true);

	layout->addWidget(updateLabel);

	QPushButton* updateButton = new QPushButton("Update to Latest Release");
	updateButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	layout->addWidget(updateButton);

	QFrame* line = new QFrame();
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	line->setLineWidth(1);

	layout->addWidget(line);

	QLabel* devUpdateLabel = new QLabel("Click the button below to update FEBio and FEBio Studio to the latest development versions. "
			"These versions contain the latest bugfixes and features but are potentially unstable. Please proceed with caution.");
	devUpdateLabel->setWordWrap(true);
	
	layout->addWidget(devUpdateLabel);

	QPushButton* devUpdateButton = new QPushButton("Update to Development Version");
	devUpdateButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	layout->addWidget(devUpdateButton);

	this->setLayout(layout);

	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QObject::connect(updateButton, &QPushButton::clicked, this, &CUpdateSettingsWidget::updateButton_clicked);
	QObject::connect(devUpdateButton, &QPushButton::clicked, this, &CUpdateSettingsWidget::updateDevButton_clicked);
}

void CUpdateSettingsWidget::updateButton_clicked()
{
	m_settings->close();
	m_wnd->on_actionUpdate_triggered();
}

void CUpdateSettingsWidget::updateDevButton_clicked()
{
	m_settings->close();
	m_wnd->on_actionUpdate_triggered(true);
}


//-----------------------------------------------------------------------------
CFEBioSettingsWidget::CFEBioSettingsWidget(QWidget* parent) : QWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout;
	layout->setAlignment(Qt::AlignTop);

	m_loadConfig = new QCheckBox("Load FEBio config file on startup.");
	layout->addWidget(m_loadConfig);

	QHBoxLayout* pathLayout = new QHBoxLayout;
	pathLayout->addWidget(new QLabel("Config file: "));
	m_configEdit = new QLineEdit;
	pathLayout->addWidget(m_configEdit);

	QToolButton* pathButton = new QToolButton;
	pathButton->setIcon(CIconProvider::GetIcon("open"));
	pathLayout->addWidget(pathButton);

	layout->addLayout(pathLayout);

	QFormLayout* f = new QFormLayout;
	f->addRow("FEBio SDK Include path: ", m_sdkInc = new QLineEdit);
	f->addRow("FEBio SDK Library path: ", m_sdkLib = new QLineEdit);
	f->addRow("FEBio create plugin path:", m_pluginPath = new QLineEdit);
	layout->addLayout(f);
	 
	this->setLayout(layout);

	QObject::connect(pathButton, SIGNAL(clicked()), this, SLOT(editConfigFilePath()));
}

bool CFEBioSettingsWidget::GetLoadConfigFlag() { return m_loadConfig->isChecked(); }
QString CFEBioSettingsWidget::GetConfigFileName() { return m_configEdit->text(); }

void CFEBioSettingsWidget::SetLoadConfigFlag(bool b) { m_loadConfig->setChecked(b); }
void CFEBioSettingsWidget::SetConfigFileName(QString s) { m_configEdit->setText(s); }

QString CFEBioSettingsWidget::GetSDKIncludePath() const { return m_sdkInc->text(); }
void CFEBioSettingsWidget::SetSDKIncludePath(const QString& s) { m_sdkInc->setText(s); }

QString CFEBioSettingsWidget::GetSDKLibraryPath() const { return m_sdkLib->text(); }
void CFEBioSettingsWidget::SetSDKLibraryPath(const QString& s) { m_sdkLib->setText(s); }

QString CFEBioSettingsWidget::GetCreatePluginPath() const { return m_pluginPath->text(); }
void CFEBioSettingsWidget::SetCreatePluginPath(const QString& s) { m_pluginPath->setText(s); }

void CFEBioSettingsWidget::editConfigFilePath()
{
	QFileDialog dlg(this);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setDirectory(m_configEdit->text());
	if (dlg.exec())
	{
		QStringList files = dlg.selectedFiles();
		QString fileName = files.first();
		m_configEdit->setText(fileName);
	}
}

//-----------------------------------------------------------------------------
class Ui::CDlgSettings
{
public:
	CBackgroundProps*	m_bg;
	CDisplayProps*		m_display;
	CPhysicsProps*		m_physics;
	CUIProps*			m_ui;
	QDialogButtonBox*	buttonBox;
	CPaletteWidget*		m_pal;
	CColormapWidget*	m_map;
	CSelectionProps*	m_select;
	CLightingProps*		m_light;
	CCameraProps*		m_cam;
	CPostProps*			m_post;
	CUnitWidget*		m_unit;
	CRepoSettingsWidget*	m_repo;
	CUpdateSettingsWidget*	m_update;
	CFEBioSettingsWidget*	m_febio;

	::CPropertyListView*	bg_panel;
	::CPropertyListView*	di_panel;
	::CPropertyListView*	ph_panel;
	::CPropertyListForm*	ui_panel;
	::CPropertyListView*	se_panel;
	::CPropertyListView*	li_panel;
	::CPropertyListView*	ca_panel;
	::CPropertyListView*	po_panel;

	GLViewSettings& ops;
	CGLDocument* doc = nullptr;

public:
	CDlgSettings(QDialog* parent, ::CMainWindow* wnd, GLViewSettings& vs) : ops(vs)
	{
		m_bg = new CBackgroundProps;
		m_display = new CDisplayProps;
		m_physics = new CPhysicsProps;
		m_ui = new CUIProps(parent, wnd);
		m_pal = new CPaletteWidget;
		m_map = new CColormapWidget;
		m_select = new CSelectionProps;
		m_light = new CLightingProps;
		m_cam = new CCameraProps;
		m_post = new CPostProps;
		m_unit = new CUnitWidget(wnd);
		m_repo = new CRepoSettingsWidget;
		m_update = new CUpdateSettingsWidget(parent, wnd);
		m_febio = new CFEBioSettingsWidget;
	}

	void setupUi(::CDlgSettings* pwnd, ::CMainWindow* mwnd)
	{
		bg_panel = new ::CPropertyListView;
		di_panel = new ::CPropertyListView;
		ph_panel = new ::CPropertyListView;
		ui_panel = new ::CPropertyListForm;
		se_panel = new ::CPropertyListView;
		li_panel = new ::CPropertyListView;
		ca_panel = new ::CPropertyListView;
		po_panel = new ::CPropertyListView;
		m_repo->setupUi();

		QStackedWidget* stack = new QStackedWidget;
		QListWidget* list = new QListWidget;

		stack->addWidget(bg_panel); list->addItem("Background");
		stack->addWidget(ca_panel); list->addItem("Camera");
		stack->addWidget(m_map   ); list->addItem("Colormap");
		stack->addWidget(di_panel); list->addItem("Display");
		stack->addWidget(m_febio ); list->addItem("FEBio");
		stack->addWidget(li_panel); list->addItem("Lighting");
		stack->addWidget(m_pal   ); list->addItem("Palette");
		stack->addWidget(ph_panel); list->addItem("Physics");
		stack->addWidget(po_panel); list->addItem("Post Options");
		stack->addWidget(se_panel); list->addItem("Selection");
		stack->addWidget(ui_panel); list->addItem("UI");
		stack->addWidget(m_unit); list->addItem("Units");
		stack->addWidget(m_repo); list->addItem("Model Repository");

		if(mwnd->updaterPresent())
		{
			stack->addWidget(m_update); list->addItem("Auto Update");
		}
		
		list->setResizeMode(QListView::ResizeMode::Adjust);

		QSplitter* split = new QSplitter;
		split->addWidget(list);
		split->addWidget(stack);
		split->setStretchFactor(0, 1);
		split->setStretchFactor(1, 2);

		QVBoxLayout* pg = new QVBoxLayout(pwnd);
		pg->addWidget(split);

		QHBoxLayout* bl = new QHBoxLayout;
		bl->setContentsMargins(0,0,0,0);

		QPushButton* resetButton = new QPushButton("Reset");
		bl->addWidget(resetButton);
		bl->addStretch();

		buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
		bl->addWidget(buttonBox);

		pg->addLayout(bl);

		QObject::connect(buttonBox, SIGNAL(accepted()), pwnd, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), pwnd, SLOT(reject()));
		QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), pwnd, SLOT(onClicked(QAbstractButton*)));
		QObject::connect(list, SIGNAL(currentRowChanged(int)), stack, SLOT(setCurrentIndex(int)));
		QObject::connect(resetButton, SIGNAL(clicked()), pwnd, SLOT(onReset()));
		QObject::connect(m_map, SIGNAL(colormapChanged(int)), pwnd, SLOT(onColormapChanged(int)));
	}
};


CDlgSettings::CDlgSettings(CMainWindow* pwnd, CGLDocument* doc, GLViewSettings& vs) : ui(new Ui::CDlgSettings(this, pwnd, vs))
{
	m_pwnd = pwnd;
	ui->doc = doc;
	setWindowTitle("Options");

	UpdateSettings();

	if (pwnd->GetDocManager()->Documents())
	{
		ui->m_unit->showAllOptions(true);
		if (doc) ui->m_unit->setUnit(doc->GetUnitSystem());
	}
	else
	{
		ui->m_unit->showAllOptions(false);
		ui->m_unit->setUnit(pwnd->GetDefaultUnitSystem());
	}

	ui->m_repo->repoPath = pwnd->GetDatabasePanel()->GetRepositoryFolder();

	ui->setupUi(this, m_pwnd);

	pwnd->GetDatabasePanel()->GetRepositoryFolder();
}

void CDlgSettings::UpdateSettings()
{
	GLViewSettings& view = ui->ops;

	ui->m_bg->m_bg1 = toQColor(view.m_col1);
	ui->m_bg->m_bg2 = toQColor(view.m_col2);
	ui->m_bg->m_fg = toQColor(view.m_fgcol);
	ui->m_bg->m_nstyle = view.m_nbgstyle;

	ui->m_display->m_meshColor = toQColor(view.m_meshColor);
	double a = view.m_meshColor.a/255.0;
	if (a < 0) a = 0; else if (a > 1) a = 1;
	ui->m_display->m_meshOpacity = a;
	ui->m_display->m_nodeSize = (double)view.m_node_size;
	ui->m_display->m_lineSize = (double)view.m_line_size;
	ui->m_display->m_bnormal = view.m_bnorm;
	ui->m_display->m_scaleNormal = view.m_scaleNormals;
	ui->m_display->m_showHighlights = view.m_showHighlights;
	ui->m_display->m_identifyBackfacing = view.m_identifyBackfacing;
	ui->m_display->m_nconv = view.m_nconv;
	ui->m_display->m_ntrans = view.m_transparencyMode;
	ui->m_display->m_dozsorting = view.m_bzsorting;
	ui->m_display->m_defaultFGColorOption = view.m_defaultFGColorOption;
	ui->m_display->m_defaultFGColor = toQColor(view.m_defaultFGColor);
	ui->m_display->m_tagFontSize = view.m_tagFontSize;

	ui->m_physics->m_showRigidBodies = view.m_brigid;
	ui->m_physics->m_showRigidJoints = view.m_bjoint;
	ui->m_physics->m_showRigidLabels = view.m_showRigidLabels;
	ui->m_physics->m_showRigidWalls = view.m_bwall;
	ui->m_physics->m_fiberScale = view.m_fiber_scale;
	ui->m_physics->m_showMatAxes = view.m_blma;
	ui->m_physics->m_showHiddenFibers = view.m_showHiddenFibers;

	ui->m_ui->m_apply = (view.m_apply == 1);
	ui->m_ui->m_bcmd = m_pwnd->clearCommandStackOnSave();
	ui->m_ui->m_autoSaveInterval = m_pwnd->autoSaveInterval();

	ui->m_select->m_bconnect = view.m_bconn;
	ui->m_select->m_ntagInfo = view.m_ntagInfo;
	ui->m_select->m_backface = view.m_bcullSel;
	ui->m_select->m_binterior = view.m_bext;
	ui->m_select->m_bpart = view.m_bpart;

	ui->m_light->m_blight  = view.m_bLighting;
	ui->m_light->m_diffuse = view.m_diffuse;
	ui->m_light->m_ambient = view.m_ambient;
	ui->m_light->m_bshadow = view.m_bShadows;
	ui->m_light->m_shadow  = view.m_shadow_intensity;
	ui->m_light->m_pos     = view.m_light;
	ui->m_light->m_envmap  = m_pwnd->GetEnvironmentMap();
	ui->m_light->m_useEV   = view.m_use_environment_map;

	if (ui->doc && ui->doc->GetScene())
	{
		GLScene* scene = ui->doc->GetScene();
		GLCamera& cam = scene->GetCamera();
		ui->m_cam->m_banim = true;
		ui->m_cam->m_bias = cam.GetCameraBias();
		ui->m_cam->m_speed = cam.GetCameraSpeed();
	}

	ui->m_post->m_defrng = Post::CGLColorMap::m_defaultRngType;

	ui->m_febio->SetLoadConfigFlag(m_pwnd->GetLoadConfigFlag());
	ui->m_febio->SetConfigFileName(m_pwnd->GetConfigFileName());

	ui->m_febio->SetSDKIncludePath(m_pwnd->GetSDKIncludePath());
	ui->m_febio->SetSDKLibraryPath(m_pwnd->GetSDKLibraryPath());
	ui->m_febio->SetCreatePluginPath(m_pwnd->GetCreatePluginPath());
}

void CDlgSettings::showEvent(QShowEvent* ev)
{
	UpdateUI();
}

void CDlgSettings::UpdateUI()
{
	ui->bg_panel->Update(ui->m_bg);
	ui->di_panel->Update(ui->m_display);
	ui->ph_panel->Update(ui->m_physics);
	ui->ui_panel->setPropertyList(ui->m_ui);
	ui->se_panel->Update(ui->m_select);
	ui->li_panel->Update(ui->m_light);
	ui->ca_panel->Update(ui->m_cam);
	ui->po_panel->Update(ui->m_post);
}

void CDlgSettings::apply()
{
	GLViewSettings& view = ui->ops;

	view.m_col1 = toGLColor(ui->m_bg->m_bg1);
	view.m_col2 = toGLColor(ui->m_bg->m_bg2);
	view.m_fgcol = toGLColor(ui->m_bg->m_fg);
	view.m_nbgstyle = ui->m_bg->m_nstyle;

	view.m_meshColor = toGLColor(ui->m_display->m_meshColor);
	int a = (int)(255.0*ui->m_display->m_meshOpacity);
	if (a < 0) a = 0; else if (a > 255) a = 255;
	view.m_meshColor.a = a;
	view.m_node_size = (float) ui->m_display->m_nodeSize;
	view.m_line_size = (float) ui->m_display->m_lineSize;
	view.m_bnorm = ui->m_display->m_bnormal;
	view.m_scaleNormals = ui->m_display->m_scaleNormal;
	view.m_showHighlights = ui->m_display->m_showHighlights;
	view.m_identifyBackfacing = ui->m_display->m_identifyBackfacing;
	view.m_nconv = ui->m_display->m_nconv;
	view.m_transparencyMode = ui->m_display->m_ntrans;
	view.m_bzsorting = ui->m_display->m_dozsorting;
	view.m_defaultFGColorOption = ui->m_display->m_defaultFGColorOption;
	view.m_defaultFGColor = toGLColor(ui->m_display->m_defaultFGColor);
	view.m_tagFontSize = ui->m_display->m_tagFontSize;

	if (view.m_defaultFGColorOption == 0)
	{
		int theme = 0;
		if (qApp->styleHints()->colorScheme() != Qt::ColorScheme::Dark) view.m_defaultFGColor = GLColor(0,0,0);
		else view.m_defaultFGColor = GLColor(255, 255, 255);
	}
	GLWidget::set_base_color(view.m_defaultFGColor);

	CGLDocument* doc = m_pwnd->GetGLDocument();
	if (doc)
	{
		CGLView* glview = m_pwnd->GetGLView();
		CGLWidgetManager* wm = (glview ? glview->GetGLWidgetManager() : nullptr);
		if (wm)
		{
			for (int i = 0; i < wm->Widgets(); ++i)
			{
				GLWidget* wi = wm->get(i); 
				if (wi && (wi->isfgc_overridden() == false))
				{
					wi->set_fg_color(view.m_defaultFGColor, false);
				}
			}
		}
	}

	view.m_brigid = ui->m_physics->m_showRigidBodies;
	view.m_bjoint = ui->m_physics->m_showRigidJoints;
	view.m_showRigidLabels = ui->m_physics->m_showRigidLabels;
	view.m_bwall = ui->m_physics->m_showRigidWalls;
	view.m_fiber_scale = ui->m_physics->m_fiberScale;
	view.m_blma = ui->m_physics->m_showMatAxes;
	view.m_showHiddenFibers = ui->m_physics->m_showHiddenFibers;

	view.m_apply = (ui->m_ui->m_apply ? 1 : 0);
	
	view.m_bconn = ui->m_select->m_bconnect;
	view.m_ntagInfo = ui->m_select->m_ntagInfo;
	view.m_bcullSel = ui->m_select->m_backface;
	view.m_bext = ui->m_select->m_binterior;
	view.m_bpart = ui->m_select->m_bpart;

	view.m_bLighting = ui->m_light->m_blight;
	view.m_diffuse   = ui->m_light->m_diffuse;
	view.m_ambient   = ui->m_light->m_ambient;
	view.m_bShadows  = ui->m_light->m_bshadow;
	view.m_shadow_intensity = ui->m_light->m_shadow;
	view.m_light = ui->m_light->m_pos;
	m_pwnd->SetEnvironmentMap(ui->m_light->m_envmap);
	view.m_use_environment_map = ui->m_light->m_useEV;

	if (ui->doc && ui->doc->GetScene())
	{
		GLScene* scene = ui->doc->GetScene();
		GLCamera& cam = scene->GetCamera();
		cam.SetCameraBias(ui->m_cam->m_bias);
		cam.SetCameraSpeed(ui->m_cam->m_speed);
	}

	Post::CGLColorMap::m_defaultRngType = ui->m_post->m_defrng;

	m_pwnd->setClearCommandStackOnSave(ui->m_ui->m_bcmd);
	m_pwnd->setAutoSaveInterval(ui->m_ui->m_autoSaveInterval);

	// update units
	int newUnit = ui->m_unit->m_unit;
	int nops = ui->m_unit->getOption();
	if (nops == 0) m_pwnd->SetDefaultUnitSystem(newUnit);
	else if (nops == 1)
	{
		Units::SetUnitSystem(newUnit);
		if (ui->doc) {
			ui->doc->SetUnitSystem(newUnit); 
			ui->doc->SetModifiedFlag(true);
		}
	}
	else
	{
		Units::SetUnitSystem(newUnit);
		CDocManager* dm = m_pwnd->GetDocManager();
		for (int i = 0; i < dm->Documents(); ++i)
		{
			CGLDocument* doci = dynamic_cast<CGLDocument*>(dm->GetDocument(i));
			if (doci)
			{
				doci->SetUnitSystem(newUnit);
				doci->SetModifiedFlag(true);
			}
		}
	}

	ui->m_map->Apply();

	CPaletteManager& PM = CPaletteManager::GetInstance();
	int n = ui->m_pal->currentPalette();
	if ((n >= 0) && (n < PM.Palettes())) PM.SetCurrentIndex(n);

	m_pwnd->GetDatabasePanel()->SetRepositoryFolder(ui->m_repo->repoPathEdit->text());

	m_pwnd->SetLoadConfigFlag(ui->m_febio->GetLoadConfigFlag());
	m_pwnd->SetConfigFileName(ui->m_febio->GetConfigFileName());
	m_pwnd->SetSDKIncludePath(ui->m_febio->GetSDKIncludePath());
	m_pwnd->SetSDKLibraryPath(ui->m_febio->GetSDKLibraryPath());
	m_pwnd->SetCreatePluginPath(ui->m_febio->GetCreatePluginPath());

	m_pwnd->RedrawGL();
}

void CDlgSettings::accept()
{
	apply();
	QDialog::accept();
}

void CDlgSettings::onClicked(QAbstractButton* button)
{
	if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) apply();
}

void CDlgSettings::onReset()
{
	GLViewSettings& view = ui->ops;
	int ntheme = 0;
    if(qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark) ntheme = 1;
	view.Defaults(ntheme);
	UpdateSettings();
	m_pwnd->RedrawGL();
	UpdateUI();
}
