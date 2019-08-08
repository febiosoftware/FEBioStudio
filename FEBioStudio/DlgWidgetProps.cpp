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
#include <GLWLib/GLWidget.h>
#include "CIntInput.h"
#include "CColorButton.h"
#include "PostViewLib/convert.h"
#include "MainWindow.h"

class CFontWidget : public QGroupBox
{
public:
	QFontComboBox*	pfontStyle;
	QSpinBox*		pfontSize;
	CColorButton*	pfontColor;
	QCheckBox		*pfontBold, *pfontItalic;

	void setFont(const QFont& font, const QColor& col)
	{
		pfontStyle->setCurrentFont(font);
		pfontSize->setValue(font.pointSize());
		pfontBold->setChecked(font.bold());
		pfontItalic->setChecked(font.italic());
		pfontColor->setColor(col);
	}

	QFont getFont() const
	{
		QFont font(pfontStyle->currentFont().family(), pfontSize->value());
		font.setBold(pfontBold->isChecked());
		font.setItalic(pfontItalic->isChecked());
		return font;
	}

	QColor getFontColor() const
	{
		return pfontColor->color();
	}

public:
	CFontWidget(QWidget* parent = 0) : QGroupBox("Font", parent)
	{
		QGridLayout* pgrid = new QGridLayout;
		pfontStyle = new QFontComboBox;
		QLabel* label = new QLabel("Font:"); label->setBuddy(pfontStyle);
		pgrid->addWidget(label, 0, 0); pgrid->addWidget(pfontStyle, 0, 1, 1, 3);
		pfontSize = new QSpinBox;
		label = new QLabel("Size:"); label->setBuddy(pfontSize);
		pgrid->addWidget(label, 1, 0); pgrid->addWidget(pfontSize, 1, 1);
		pfontColor = new CColorButton;
		label = new QLabel("Color:"); label->setBuddy(pfontColor);
		pgrid->addWidget(label, 0, 4, 1, 1, Qt::AlignRight); pgrid->addWidget(pfontColor, 0, 5);
		pgrid->addWidget(pfontBold   = new QCheckBox("bold"  ), 1, 2);
		pgrid->addWidget(pfontItalic = new QCheckBox("italic"), 1, 3);
		setLayout(pgrid);
	}
};

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
	CColorButton*	pshadowCol;
	QComboBox* pbgstyle;
	CColorButton *bgCol1, *bgCol2;
	CFontWidget* pfont;
	QDialogButtonBox* buttonBox;

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
		pform->addRow("Background style:", pbgstyle);
		pform->addRow("Color1:", bgCol1 = new CColorButton);
		pform->addRow("Color2:", bgCol2 = new CColorButton);
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
	GLBox* pb = dynamic_cast<GLBox*>(pw);

	ui->ppos->setPosition(widget->x(), widget->y(), widget->w(), widget->h());

	ui->ptext->setText(widget->get_label());
	ui->pshadow->setChecked(pb->m_bshadow);
	ui->pshadowCol->setColor(toQColor(pb->m_shc));

	ui->pbgstyle->setCurrentIndex(pb->get_bg_style());
	ui->bgCol1->setColor(toQColor(pb->get_bg_color(0)));
	ui->bgCol2->setColor(toQColor(pb->get_bg_color(1)));

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

	GLBox* pb = dynamic_cast<GLBox*>(pw);
	pb->m_bshadow = ui->pshadow->isChecked();
	pb->m_shc = toGLColor(ui->pshadowCol->color());

	pb->set_bg_style(ui->pbgstyle->currentIndex());
	pb->set_bg_color(toGLColor(ui->bgCol1->color()), toGLColor(ui->bgCol2->color()));

	QFont font = ui->pfont->getFont();
	pb->set_font(font);
	pb->set_fg_color(toGLColor(ui->pfont->getFontColor()));

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
				plabelFont = new CFontWidget;
				labelsPageLayout->addWidget(plabelFont);
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

CDlgLegendProps::CDlgLegendProps(GLWidget* widget, CMainWindow* parent) : QDialog(parent), ui(new Ui::CDlgLegendProps), m_wnd(parent)
{
	ui->setupUi(this);

	pw = widget;
	GLLegendBar* pb = dynamic_cast<GLLegendBar*>(pw);

	ui->pshowLabels->setChecked(pb->ShowLabels());
	ui->pprec->setValue(pb->GetPrecision());

	QFont labelFont = pb->get_font();
	ui->plabelFont->setFont(labelFont, toQColor(pb->get_fg_color()));

	ui->placement->setCurrentIndex(pb->GetLabelPosition());

	ui->ppos->setPosition(widget->x(), widget->y(), widget->w(), widget->h());
}

void CDlgLegendProps::apply()
{
	int x = ui->ppos->x();
	int y = ui->ppos->y();
	int w = ui->ppos->w();
	int h = ui->ppos->h();
	pw->resize(x, y, w, h);

	GLLegendBar* pb = dynamic_cast<GLLegendBar*>(pw);
	pb->ShowLabels(ui->pshowLabels->isChecked());
	pb->SetPrecision(ui->pprec->value());
	pb->SetLabelPosition(ui->placement->currentIndex());

	QFont labelFont = ui->plabelFont->getFont();
	pb->set_font(labelFont);
	pb->set_fg_color(toGLColor(ui->plabelFont->getFontColor()));

	if (m_wnd) m_wnd->RedrawGL();
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
