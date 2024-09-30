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
#include "DlgWidgetProps.h"
#include <QTabWidget>
#include <QBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QValidator>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QFontComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include "InputWidgets.h"
#include "CColorButton.h"
#include "GLWLib/convert.h"
#include <GLWLib/GLLabel.h>
#include <GLWLib/GLLegendBar.h>
#include <GLWLib/GLTriad.h>
#include "MainWindow.h"

//=================================================================================================
class CFontWidgetUI
{
public:
	QFontComboBox* pfontStyle;
	QSpinBox* pfontSize;
	CColorButton* pfontColor;
	QCheckBox* pfontBold, * pfontItalic;
};

CFontWidget::CFontWidget(QWidget* parent) : QGroupBox("Font", parent), ui(new CFontWidgetUI)
{
	QGridLayout* pgrid = new QGridLayout;
	ui->pfontStyle = new QFontComboBox;
	QLabel* label = new QLabel("Font:"); label->setBuddy(ui->pfontStyle);
	pgrid->addWidget(label, 0, 0); pgrid->addWidget(ui->pfontStyle, 0, 1, 1, 3);
	ui->pfontSize = new QSpinBox;
	label = new QLabel("Size:"); label->setBuddy(ui->pfontSize);
	pgrid->addWidget(label, 1, 0); pgrid->addWidget(ui->pfontSize, 1, 1);
	ui->pfontColor = new CColorButton;
	label = new QLabel("Color:"); label->setBuddy(ui->pfontColor);
	pgrid->addWidget(label, 0, 4, 1, 1, Qt::AlignRight); pgrid->addWidget(ui->pfontColor, 0, 5);
	pgrid->addWidget(ui->pfontBold   = new QCheckBox("bold"  ), 1, 2);
	pgrid->addWidget(ui->pfontItalic = new QCheckBox("italic"), 1, 3);

	QPushButton* pb = new QPushButton("Make default");
	pb->setToolTip("Set this as the default font for widgets.");
	pgrid->addWidget(pb, 2, 0);
	setLayout(pgrid);

	QObject::connect(pb, SIGNAL(clicked(bool)), this, SLOT(onMakeDefault()));
}

CFontWidget::~CFontWidget()
{
	delete ui;
}

void CFontWidget::setFont(const QFont& font, const QColor& col)
{
	ui->pfontStyle->setCurrentFont(font);
	ui->pfontSize->setValue(font.pointSize());
	ui->pfontBold->setChecked(font.bold());
	ui->pfontItalic->setChecked(font.italic());
	ui->pfontColor->setColor(col);
}

QFont CFontWidget::getFont() const
{
	QFont font(ui->pfontStyle->currentFont().family(), ui->pfontSize->value());
	font.setBold(ui->pfontBold->isChecked());
	font.setItalic(ui->pfontItalic->isChecked());
	return font;
}

QColor CFontWidget::getFontColor() const
{
	return ui->pfontColor->color();
}

void CFontWidget::onMakeDefault()
{
	QFont font = getFont();
	GLWidget::set_default_font(font);
}

//=================================================================================================
class CPositionWidget : public QWidget
{
public:
	CIntInput *px, *py;
	CIntInput *pw, *ph;

	void setPosition(int x, int y, int w, int h)
	{
		px->setValue(x);
		py->setValue(y);
		pw->setValue(w);
		ph->setValue(h);
	}

	int x() const { return px->value(); }
	int y() const { return py->value(); }
	int w() const { return pw->value(); }
	int h() const { return ph->value(); }

public:
	CPositionWidget(QWidget* parent = 0) : QWidget(parent)
	{
		QFormLayout* pform = new QFormLayout;
		pform->addRow("X:", px = new CIntInput);
		pform->addRow("Y:", py = new CIntInput);
		pform->addRow("Width:", pw = new CIntInput);
		pform->addRow("Height:", ph = new CIntInput);
		setLayout(pform);
	}
};

class Ui::CDlgBoxProps
{
public:
	CPositionWidget* ppos;
	QLineEdit* ptext;
	QCheckBox* pshadow;
	QComboBox* align;
	CColorButton*	pshadowCol;
	QComboBox* pbgstyle;
	CColorButton *bgCol1, *bgCol2;
	CFontWidget* pfont;
	QDialogButtonBox* buttonBox;
	QComboBox* bglinemode;
	QSpinBox* bglinesize;
	CColorButton* bglinecol;

public:
	void setupUi(QDialog* parent)
	{
		// main layout
		QVBoxLayout* pv = new QVBoxLayout(parent);

		// create tab widget
		QTabWidget* tab = new QTabWidget;

		// text properties
		QWidget* textPage = new QWidget;
		QVBoxLayout* textPageLayout = new QVBoxLayout;

		QGroupBox* pg = new QGroupBox("Text");
		QVBoxLayout* pvg = new QVBoxLayout;
		QHBoxLayout* phb = new QHBoxLayout;
			QLabel* label = new QLabel("Text:");
			ptext = new QLineEdit;
			label->setBuddy(ptext);
			phb->addWidget(label);
			phb->addWidget(ptext);
		pvg->addLayout(phb);

		phb = new QHBoxLayout;
			pshadow = new QCheckBox("text shadow");
			pshadowCol = new CColorButton;
			phb->addWidget(pshadow);
			phb->addWidget(pshadowCol);
			phb->addStretch();
		pvg->addLayout(phb);

		QHBoxLayout* alignLayout = new QHBoxLayout;
			align = new QComboBox;
			align->addItems(QStringList() << "Left" << "Centered" << "Right");
			alignLayout->addWidget(new QLabel("Align:"));
			alignLayout->addWidget(align);
			alignLayout->addStretch();
		pvg->addLayout(alignLayout);
		pg->setLayout(pvg);
		textPageLayout->addWidget(pg);

		pfont = new CFontWidget;
		textPageLayout->addWidget(pfont);

		pg = new QGroupBox("Background");
		QFormLayout* pform = new QFormLayout;
		QStringList items; items << "None" << "Color 1" << "Color 2" << "Horizontal gradient" << "Vertical gradient";
		pbgstyle = new QComboBox;
		pbgstyle->addItems(items);

		pform->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
		pform->addRow("Fill mode:", pbgstyle);
		pform->addRow("Color1:", bgCol1 = new CColorButton);
		pform->addRow("Color2:", bgCol2 = new CColorButton);

		bglinemode = new QComboBox;
		bglinemode->addItems(QStringList() << "None" << "Solid");

		pform->addRow("Line mode:", bglinemode);
		pform->addRow("Line width:", bglinesize = new QSpinBox); bglinesize->setRange(1, 10);
		pform->addRow("Line color:", bglinecol = new CColorButton);
		pg->setLayout(pform);
		textPageLayout->addWidget(pg);

		textPage->setLayout(textPageLayout);

		// position properties
		ppos = new CPositionWidget;

		// add all the tabs
		tab->addTab(textPage, "Text");
		tab->addTab(ppos, "Position");
		pv->addWidget(tab);

		// create the dialog button box
		buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(buttonBox);
		QObject::connect(buttonBox, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), parent, SLOT(onClicked(QAbstractButton*)));
	}
};

CDlgBoxProps::CDlgBoxProps(GLWidget* widget, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgBoxProps)
{
	ui->setupUi(this);

	pw = widget;
	GLLabel* pb = dynamic_cast<GLLabel*>(pw);

	ui->ppos->setPosition(widget->x(), widget->y(), widget->w(), widget->h());

	ui->ptext->setText(widget->get_label());
	ui->pshadow->setChecked(pb->m_bshadow);
	ui->pshadowCol->setColor(toQColor(pb->m_shc));

	ui->pbgstyle->setCurrentIndex(pb->get_bg_style());
	ui->bgCol1->setColor(toQColor(pb->get_bg_color(0)));
	ui->bgCol2->setColor(toQColor(pb->get_bg_color(1)));

	ui->bglinemode->setCurrentIndex(pb->get_bg_line_style());
	ui->bglinesize->setValue(pb->get_bg_line_size());
	ui->bglinecol->setColor(toQColor(pb->get_bg_line_color()));

	QFont font = pb->get_font();
	ui->pfont->setFont(font, toQColor(pb->get_fg_color()));
}

void CDlgBoxProps::apply()
{
	int x = ui->ppos->x();
	int y = ui->ppos->y();
	int w = ui->ppos->w();
	int h = ui->ppos->h();

	std::string text = ui->ptext->text().toStdString();

	pw->resize(x, y, w, h);
	pw->copy_label(text.c_str());

	GLLabel* pb = dynamic_cast<GLLabel*>(pw);
	pb->m_bshadow = ui->pshadow->isChecked();
	pb->m_shc = toGLColor(ui->pshadowCol->color());

	pb->set_bg_style(ui->pbgstyle->currentIndex());
	pb->set_bg_color(toGLColor(ui->bgCol1->color()), toGLColor(ui->bgCol2->color()));
	pb->set_bg_line_style(ui->bglinemode->currentIndex());
	pb->set_bg_line_size(ui->bglinesize->value());
	pb->set_bg_line_color(toGLColor(ui->bglinecol->color()));

	QFont font = ui->pfont->getFont();
	pb->set_font(font);
	pb->set_fg_color(toGLColor(ui->pfont->getFontColor()));

	int n = ui->align->currentIndex(); assert(n >= 0);
	pb->m_align = n;

	CMainWindow* wnd = dynamic_cast<CMainWindow*>(parentWidget());
	if (wnd) wnd->RedrawGL();
}

void CDlgBoxProps::accept()
{
	apply();
	QDialog::accept();
}

void CDlgBoxProps::onClicked(QAbstractButton* button)
{
	if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) apply();
}

class Ui::CDlgLegendProps
{
public:
	// Labels page
	QCheckBox* pshowLabels;
	QSpinBox* pprec;
	CFontWidget* plabelFont;

	// Position page
	CPositionWidget* ppos;

	QDialogButtonBox* buttonBox;
	QComboBox* placement;
	QComboBox* orientation;
	QDoubleSpinBox* plineThick;
	QSpinBox* pdivs;
	QCheckBox* psmooth;

	// background
	QComboBox* pbgstyle;
	CColorButton* bgCol1, * bgCol2;
	QComboBox* bglinemode;
	QSpinBox* bglinesize;
	CColorButton* bglinecol;

public:
	void setupUi(QDialog* parent)
	{
		// main layout
		QVBoxLayout* pv = new QVBoxLayout(parent);

		// create tab widget
		QTabWidget* tab = new QTabWidget;

		// Labels page
		QWidget* labelsPage = new QWidget;
			QVBoxLayout* labelsPageLayout = new QVBoxLayout;
				QHBoxLayout* ph = new QHBoxLayout;
					ph->addWidget(pshowLabels = new QCheckBox("show labels"));
					QLabel* plabel = new QLabel("precision");
					placement = new QComboBox;
					placement->addItem("Left/Top");
					placement->addItem("Right/Bottom");
					ph->addWidget(plabel);
					ph->addWidget(pprec = new QSpinBox); plabel->setBuddy(pprec); pprec->setRange(1, 7);
					ph->addWidget(placement);
					ph->addStretch();
				labelsPageLayout->addLayout(ph);

				QFormLayout* pol = new QFormLayout;
				orientation = new QComboBox;
				orientation->addItems(QStringList() << "Horizontal" << "Vertical");
				plineThick = new QDoubleSpinBox;
				plineThick->setRange(0.0, 100.0);
				pol->addRow("Orientation", orientation);
				pol->addRow("Line thickness", plineThick);
				pol->addRow("Divisions", pdivs = new QSpinBox);
				pol->addRow("Smooth texture", psmooth = new QCheckBox);
				pdivs->setRange(1, 100);
				labelsPageLayout->addLayout(pol);
				plabelFont = new CFontWidget;
				labelsPageLayout->addWidget(plabelFont);

				// background props
				QGroupBox* pg = new QGroupBox("Background");
				QFormLayout* pform = new QFormLayout;
				QStringList items; items << "None" << "Color 1" << "Color 2" << "Horizontal gradient" << "Vertical gradient";
				pbgstyle = new QComboBox;
				pbgstyle->addItems(items);

				bglinemode = new QComboBox;
				bglinemode->addItems(QStringList() << "None" << "Solid");
				
				pform->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
				pform->addRow("Fill mode:", pbgstyle);
				pform->addRow("Color1:", bgCol1 = new CColorButton);
				pform->addRow("Color2:", bgCol2 = new CColorButton);

				pform->addRow("Line mode:", bglinemode);
				pform->addRow("Line width:", bglinesize = new QSpinBox); bglinesize->setRange(1, 10);
				pform->addRow("Line color:", bglinecol = new CColorButton);
				pg->setLayout(pform);
				labelsPageLayout->addWidget(pg);

			labelsPage->setLayout(labelsPageLayout);

		// Position page
		ppos = new CPositionWidget;

		// add all the tabs
		tab->addTab(labelsPage, "Options");
		tab->addTab(ppos, "Position");
		pv->addWidget(tab);

		// create the dialog button box
		buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(buttonBox);
		QObject::connect(buttonBox, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), parent, SLOT(onClicked(QAbstractButton*)));
	}
};

CDlgLegendProps::CDlgLegendProps(GLWidget* widget, CMainWindow* parent) : QDialog(parent), ui(new Ui::CDlgLegendProps), m_wnd(parent)
{
	ui->setupUi(this);

	pw = widget;
	GLLegendBar* pb = dynamic_cast<GLLegendBar*>(pw);

	ui->pshowLabels->setChecked(pb->ShowLabels());
	ui->pprec->setValue(pb->GetPrecision());
	ui->orientation->setCurrentIndex(pb->Orientation());
	ui->plineThick->setValue(pb->LineThickness());
	ui->pdivs->setValue(pb->GetDivisions());
	ui->psmooth->setChecked(pb->SmoothTexture());

	QFont labelFont = pb->get_font();
	ui->plabelFont->setFont(labelFont, toQColor(pb->get_fg_color()));

	ui->placement->setCurrentIndex(pb->GetLabelPosition());

	ui->ppos->setPosition(widget->x(), widget->y(), widget->w(), widget->h());

	ui->pbgstyle->setCurrentIndex(pb->get_bg_style());
	ui->bgCol1->setColor(toQColor(pb->get_bg_color(0)));
	ui->bgCol2->setColor(toQColor(pb->get_bg_color(1)));

	ui->bglinemode->setCurrentIndex(pb->get_bg_line_style());
	ui->bglinesize->setValue(pb->get_bg_line_size());
	ui->bglinecol->setColor(toQColor(pb->get_bg_line_color()));
}

void CDlgLegendProps::apply()
{
	GLLegendBar* pb = dynamic_cast<GLLegendBar*>(pw);
	pb->ShowLabels(ui->pshowLabels->isChecked());
	pb->SetPrecision(ui->pprec->value());
	pb->SetLabelPosition(ui->placement->currentIndex());

	int oldOrient = pb->Orientation();
	int newOrient = ui->orientation->currentIndex();

	QFont labelFont = ui->plabelFont->getFont();
	pb->set_font(labelFont);
	pb->set_fg_color(toGLColor(ui->plabelFont->getFontColor()));

	pb->set_bg_style(ui->pbgstyle->currentIndex());
	pb->set_bg_color(toGLColor(ui->bgCol1->color()), toGLColor(ui->bgCol2->color()));
	pb->set_bg_line_style(ui->bglinemode->currentIndex());
	pb->set_bg_line_size(ui->bglinesize->value());
	pb->set_bg_line_color(toGLColor(ui->bglinecol->color()));

	pb->SetLineThickness((float)ui->plineThick->value());
	pb->SetDivisions(ui->pdivs->value());
	pb->SetSmoothTexture(ui->psmooth->isChecked());

	if (oldOrient != newOrient)
	{
		pb->SetOrientation(newOrient);

		if (m_wnd) m_wnd->RedrawGL();

		// update the positions
		ui->ppos->setPosition(pw->x(), pw->y(), pw->w(), pw->h());
	}
	else
	{
		int x = ui->ppos->x();
		int y = ui->ppos->y();
		int w = ui->ppos->w();
		int h = ui->ppos->h();
		pw->resize(x, y, w, h);
		if (m_wnd) m_wnd->RedrawGL();
	}
}

void CDlgLegendProps::accept()
{
	apply();
	QDialog::accept();
}

void CDlgLegendProps::onClicked(QAbstractButton* button)
{
	if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) apply();
}

class Ui::CDlgTriadProps
{
public:
	// Labels page
	QCheckBox* pshowLabels;
	CFontWidget* plabelFont;

	// Position page
	CPositionWidget* ppos;
	
	QDialogButtonBox* buttonBox;

public:
	void setupUi(QDialog* parent)
	{
		// main layout
		QVBoxLayout* pv = new QVBoxLayout(parent);

		// create tab widget
		QTabWidget* tab = new QTabWidget;

		// Labels page
		QWidget* labelsPage = new QWidget;
			QVBoxLayout* labelsPageLayout = new QVBoxLayout;
				labelsPageLayout->addWidget(pshowLabels = new QCheckBox("show labels"));
				labelsPageLayout->addWidget(plabelFont = new CFontWidget);
			labelsPage->setLayout(labelsPageLayout);

		// Position page
		ppos = new CPositionWidget;

		// add all the tabs
		tab->addTab(labelsPage, "Labels");
		tab->addTab(ppos, "Position");
		pv->addWidget(tab);

		// create the dialog button box
		buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(buttonBox);
		QObject::connect(buttonBox, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), parent, SLOT(onClicked(QAbstractButton*)));
	}
};

CDlgTriadProps::CDlgTriadProps(GLWidget* widget, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgTriadProps)
{
	ui->setupUi(this);

	pw = widget;
	GLTriad* pt = dynamic_cast<GLTriad*>(pw);

	ui->pshowLabels->setChecked(pt->show_coord_labels());

	QFont labelFont = pt->get_font();
	ui->plabelFont->setFont(labelFont, toQColor(pt->get_fg_color()));

	ui->ppos->setPosition(widget->x(), widget->y(), widget->w(), widget->h());
}

void CDlgTriadProps::apply()
{
	int x = ui->ppos->x();
	int y = ui->ppos->y();
	int w = ui->ppos->w();
	int h = ui->ppos->h();
	pw->resize(x, y, w, h);

	GLTriad* pb = dynamic_cast<GLTriad*>(pw);
	pb->show_coord_labels(ui->pshowLabels->isChecked());

	QFont labelFont = ui->plabelFont->getFont();
	pw->set_font(labelFont);
	pw->set_fg_color(toGLColor(ui->plabelFont->getFontColor()));

	CMainWindow* wnd = dynamic_cast<CMainWindow*>(parentWidget());
	if (wnd) wnd->RedrawGL();
}

void CDlgTriadProps::accept()
{
	apply();
	QDialog::accept();
}

void CDlgTriadProps::onClicked(QAbstractButton* button)
{
	if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) apply();
}

class Ui::CDlgCaptureFrameProps
{
public:
	QComboBox* pformat;
	CIntInput *px, *py;
	CIntInput *pw, *ph;
	QDialogButtonBox* buttonBox;

public:
	void setupUi(QDialog* parent)
	{
		// main layout
		QVBoxLayout* pv = new QVBoxLayout(parent);

		QStringList items;
		items << "User" << "HD 720" << "HD 1080" << "NTSC" << "PAL" << "SVGA" << "VGA";

		QHBoxLayout* phbox = new QHBoxLayout;
		QLabel* label = new QLabel("Format");
		pformat = new QComboBox; label->setBuddy(pformat);
		pformat->addItems(items);
		pformat->setCurrentIndex(0);

		phbox->addWidget(label);
		phbox->addWidget(pformat);
		phbox->addStretch();
		pv->addLayout(phbox);

		QGridLayout* pgrid = new QGridLayout;
		pgrid->addWidget(label = new QLabel("x:"), 0, 0);
		pgrid->addWidget(px = new CIntInput, 0, 1); label->setBuddy(px);

		pgrid->addWidget(label = new QLabel("y:"), 1, 0);
		pgrid->addWidget(py = new CIntInput, 1, 1); label->setBuddy(py);

		pgrid->addWidget(label = new QLabel("width:"), 0, 2);
		pgrid->addWidget(pw = new CIntInput, 0, 3); label->setBuddy(pw);

		pgrid->addWidget(label = new QLabel("height:"), 1, 2);
		pgrid->addWidget(ph = new CIntInput, 1, 3); label->setBuddy(ph);
		pv->addLayout(pgrid);

		// create the dialog button box
		buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(buttonBox);
		QObject::connect(buttonBox, SIGNAL(accepted()), parent, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), parent, SLOT(reject()));
		QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), parent, SLOT(onClicked(QAbstractButton*)));

		QObject::connect(pformat, SIGNAL(currentIndexChanged(int)), parent, SLOT(onFormat(int)));
	}
};

CDlgCaptureFrameProps::CDlgCaptureFrameProps(GLWidget* widget, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgCaptureFrameProps)
{
	ui->setupUi(this);

	pw = widget;

	ui->px->setValue(pw->x());
	ui->py->setValue(pw->y());
	ui->pw->setValue(pw->w());
	ui->ph->setValue(pw->h());
}

void CDlgCaptureFrameProps::apply()
{
	int x = ui->px->value();
	int y = ui->py->value();
	int w = ui->pw->value();
	int h = ui->ph->value();
	pw->resize(x, y, w, h);

	CMainWindow* wnd = dynamic_cast<CMainWindow*>(parentWidget());
	if (wnd) wnd->RedrawGL();
}

void CDlgCaptureFrameProps::accept()
{
	apply();
	QDialog::accept();
}

void CDlgCaptureFrameProps::onClicked(QAbstractButton* button)
{
	if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole) apply();
}

void CDlgCaptureFrameProps::onFormat(int nindex)
{
	int LUT[][2] = {
		{0,0},
		{1280, 720},
		{1920, 1080},
		{720, 480},
		{768, 576},
		{800, 600},
		{640,480}};

	if (nindex == 0)
	{
		ui->pw->setEnabled(true);
		ui->ph->setEnabled(true);
	}
	else
	{
		ui->pw->setValue(LUT[nindex][0]);
		ui->ph->setValue(LUT[nindex][1]);

		ui->pw->setEnabled(false);
		ui->ph->setEnabled(false);
	}
}
