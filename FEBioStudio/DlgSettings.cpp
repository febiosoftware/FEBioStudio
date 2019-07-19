#include "stdafx.h"
#include "DlgSettings.h"
#include "MainWindow.h"
#include "Document.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QDialogButtonBox>
#include "PropertyList.h"
#include "PropertyListView.h"

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
		addBoolProperty  (&m_bnormal, "Show normals"  );
		addDoubleProperty(&m_scaleNormal, "Normals scale factor");
        QStringList vconv;
        vconv <<"First-angle projection (XZ)"<<"First-angle projection (XY)"<<"Third-angle projection (XY)";
        addEnumProperty(&m_nconv, "Multiview projection")->setEnumValues(vconv);
	}

public:
	double	m_nodeSize;
	double	m_lineSize;
	QColor  m_meshColor;
	bool	m_bnormal;
	double	m_scaleNormal;
    int     m_nconv;
};

//-----------------------------------------------------------------------------
class CPhysicsProps : public CDataPropertyList
{
public:
	CPhysicsProps()
	{
		addBoolProperty  (&m_showRigidJoints, "Show rigid joints");
		addBoolProperty  (&m_showRigidWalls , "Show rigid walls" );
		addBoolProperty  (&m_showFibers     , "Show material fibers");
		addDoubleProperty(&m_fiberScale   , "Fiber scale factor"  );
		addBoolProperty  (&m_showMatAxes  ,"Show material axes"   );
		addBoolProperty  (&m_showHiddenFibers, "Show fibers/axes on hidden parts");
	}

public:
	bool	m_showRigidJoints;
	bool	m_showRigidWalls;
	bool	m_showFibers;
	bool	m_showMatAxes;
	double	m_fiberScale;
	bool	m_showHiddenFibers;
};

//-----------------------------------------------------------------------------
class CUIProps : public CDataPropertyList
{
public:
	CUIProps()
	{
		addBoolProperty(&m_apply, "Emulate apply action");
		addBoolProperty(&m_bcmd , "Clear undo stack on save");
		addEnumProperty(&m_theme, "Theme")->setEnumValues(QStringList() << "Default" << "Dark");
	}

public:
	bool	m_apply;
	bool	m_bcmd;
	int		m_theme;
};

//-----------------------------------------------------------------------------
class Ui::CDlgSettings
{
public:
	CBackgroundProps*	m_bg;
	CDisplayProps*		m_display;
	CPhysicsProps*		m_physics;
	CUIProps*			m_ui;
	QDialogButtonBox*	buttonBox;

	::CPropertyListView*	panel[4];

public:
	CDlgSettings()
	{
		m_bg = new CBackgroundProps;
		m_display = new CDisplayProps;
		m_physics = new CPhysicsProps;
		m_ui = new CUIProps;
	}

	void setupUi(::CDlgSettings* pwnd)
	{
		QVBoxLayout* pg = new QVBoxLayout(pwnd);

		QTabWidget* pt = new QTabWidget;

		panel[0] = new ::CPropertyListView;
		panel[1] = new ::CPropertyListView;
		panel[2] = new ::CPropertyListView;
		panel[3] = new ::CPropertyListView;

		pt->addTab(panel[0], "Background");
		pt->addTab(panel[1], "Display");
		pt->addTab(panel[2], "Physics");
		pt->addTab(panel[3], "UI");
		pg->addWidget(pt);

		buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
		pg->addWidget(buttonBox);

		QObject::connect(buttonBox, SIGNAL(accepted()), pwnd, SLOT(accept()));
		QObject::connect(buttonBox, SIGNAL(rejected()), pwnd, SLOT(reject()));
		QObject::connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), pwnd, SLOT(onClicked(QAbstractButton*)));
		QObject::connect(pt, SIGNAL(currentChanged(int)), pwnd, SLOT(onTabChanged(int)));
	}
};


CDlgSettings::CDlgSettings(CMainWindow* pwnd) : ui(new Ui::CDlgSettings)
{
	m_pwnd = pwnd;

	CDocument* pdoc = m_pwnd->GetDocument();
	VIEW_SETTINGS& view = pdoc->GetViewSettings();

	ui->m_bg->m_bg1 = toQColor(view.m_col1);
	ui->m_bg->m_bg2 = toQColor(view.m_col2);
	ui->m_bg->m_fg = toQColor(view.m_fgcol);
	ui->m_bg->m_nstyle = view.m_nbgstyle;

	ui->m_display->m_meshColor = toQColor(view.m_mcol);
	ui->m_display->m_nodeSize = (double) view.m_node_size;
	ui->m_display->m_lineSize = (double) view.m_line_size;
	ui->m_display->m_bnormal = view.m_bnorm;
	ui->m_display->m_scaleNormal = view.m_scaleNormals;
    ui->m_display->m_nconv = view.m_nconv;

	ui->m_physics->m_showRigidJoints = view.m_brigid;
	ui->m_physics->m_showRigidWalls = view.m_bwall;
	ui->m_physics->m_showFibers = view.m_bfiber;
	ui->m_physics->m_fiberScale = view.m_fiber_scale;
	ui->m_physics->m_showMatAxes = view.m_blma;
	ui->m_physics->m_showHiddenFibers = view.m_showHiddenFibers;

	ui->m_ui->m_apply = (view.m_apply == 1);
	ui->m_ui->m_bcmd = view.m_clearUndoOnSave;
	ui->m_ui->m_theme = pwnd->currentTheme();

	ui->setupUi(this);
	resize(400, 300);
}

void CDlgSettings::showEvent(QShowEvent* ev)
{
	ui->panel[0]->Update(ui->m_bg);
	ui->panel[1]->Update(ui->m_display);
	ui->panel[2]->Update(ui->m_physics);
	ui->panel[3]->Update(ui->m_ui);
}

void CDlgSettings::onTabChanged(int n)
{
	if ((n >= 0) && (n < 4))
	{
		ui->panel[n]->FitGeometry();
	}
}

void CDlgSettings::apply()
{
	CDocument* pdoc = m_pwnd->GetDocument();

	VIEW_SETTINGS& view = pdoc->GetViewSettings();

	view.m_col1 = toGLColor(ui->m_bg->m_bg1);
	view.m_col2 = toGLColor(ui->m_bg->m_bg2);
	view.m_fgcol = toGLColor(ui->m_bg->m_fg);
	view.m_nbgstyle = ui->m_bg->m_nstyle;

	view.m_mcol = toGLColor(ui->m_display->m_meshColor);
	view.m_node_size = (float) ui->m_display->m_nodeSize;
	view.m_line_size = (float) ui->m_display->m_lineSize;
	view.m_bnorm = ui->m_display->m_bnormal;
	view.m_scaleNormals = ui->m_display->m_scaleNormal;
    view.m_nconv = ui->m_display->m_nconv;

	view.m_brigid = ui->m_physics->m_showRigidJoints;
	view.m_bwall = ui->m_physics->m_showRigidWalls;
	view.m_bfiber = ui->m_physics->m_showFibers;
	view.m_fiber_scale = ui->m_physics->m_fiberScale;
	view.m_blma = ui->m_physics->m_showMatAxes;
	view.m_showHiddenFibers = ui->m_physics->m_showHiddenFibers;

	view.m_apply = (ui->m_ui->m_apply ? 1 : 0);
	view.m_clearUndoOnSave = ui->m_ui->m_bcmd;
	
	m_pwnd->setCurrentTheme(ui->m_ui->m_theme);

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
