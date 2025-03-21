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

#include "FiberODFWidget.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QOpenGLFunctions>
#include <QSizePolicy>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QMenu>
#include <QLabel>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QPainter>
#include <QMouseEvent>
#include <QCheckBox>
#include <QMessageBox>
#include <QGroupBox>
#include <fstream>
#include <vector>
#include <complex>
#include "DynamicStackedWidget.h"
#include <PostLib/ColorMap.h>
#include "MainWindow.h"
#include "ModelDocument.h"
#include <FEMLib/FSModel.h>
#include <FEMLib/GMaterial.h>
#include <FEMLib/FECoreMaterial.h>
#include <GLWLib/GLWidget.h>
#include <FEBioLink/FEBioClass.h>
#include <FEMLib/FECoreMaterial.h>
#include <FECore/fecore_enum.h>
#include <FECore/mat3d.h>
#include <FECore/mathalg.h>
#include <FEAMR/SpherePointsGenerator.h>
#include <ImageLib/FiberODFAnalysis.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FEMesh.h>
#include <MeshLib/FEElementData.h>
#include "DlgStartThread.h"
#include "PropertyList.h"
#include "PropertyListForm.h"
#include "PropertyList.h"
#include <XML/XMLWriter.h>

#ifdef __APPLE__
#include <OpenGL/GLU.h>
#else
#include <GL/glu.h>
#endif

using std::vector;
using std::complex;
using sphere = SpherePointsGenerator;

CFiberGLWidget::CFiberGLWidget() : m_ODF(nullptr), m_analysis(nullptr)
{
    setMouseTracking(true);

    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);
}

void CFiberGLWidget::setAnalysis(CFiberODFAnalysis* analysis)
{
    m_analysis = analysis;
}

void CFiberGLWidget::setODF(CODF* odf)
{
    m_ODF = odf;
}

void CFiberGLWidget::initializeGL()
{
    GLfloat amb1[] = { .09f, .09f, .09f, 1.f };
	GLfloat dif1[] = { .8f, .8f, .8f, 1.f };

	glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDepthFunc(GL_LEQUAL);

	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//	glShadeModel(GL_FLAT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(0.5f);

	// enable lighting and set default options
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, amb1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, dif1);

	glEnable(GL_POLYGON_OFFSET_FILL);

	// enable color tracking for diffuse color of materials
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glPointSize(7.0f);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);


    glClearColor(0.3, 0.3, 0.3, 1);

    m_ptriad = new GLTriad(0, 0, 50, 50);
	m_ptriad->align(GLW_ALIGN_LEFT | GLW_ALIGN_BOTTOM);
}

void CFiberGLWidget::resizeGL(int w, int h)
{
    QOpenGLWidget::resizeGL(w, h);
}

void CFiberGLWidget::paintGL()
{
    glEnable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the projection Matrix to ortho2d so we can draw some stuff on the screen
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.2, 1.2, -1.2, 1.2, 0.01, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    m_cam.SetTargetDistance(50);
    m_cam.PositionInScene();

	if (m_analysis && m_ODF) m_analysis->renderODFMesh(m_ODF, &m_cam);

    // render the GL widgets
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width(), height(), 0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    m_ptriad->setOrientation(m_cam.GetOrientation());
    m_ptriad->draw(&painter);
}

void CFiberGLWidget::mousePressEvent(QMouseEvent* ev)
{
    int x = ev->x();
	int y = ev->y();

    m_x0 = ev->pos().x();
	m_y0 = ev->pos().y();


    ev->accept();
}

void CFiberGLWidget::mouseMoveEvent(QMouseEvent* ev)
{
    bool but1 = (ev->buttons() & Qt::LeftButton);
    if(!but1) return;

    bool balt   = (ev->modifiers() & Qt::AltModifier     ? true : false);

    int x = ev->pos().x();
	int y = ev->pos().y();

    if (balt)
    {
        quatd qz = quatd((y - m_y0)*0.01f, vec3d(0, 0, 1));
        m_cam.Orbit(qz);
    }
    else
    {
        quatd qx = quatd((y - m_y0)*0.01f, vec3d(1, 0, 0));
        quatd qy = quatd((x - m_x0)*0.01f, vec3d(0, 1, 0));

        m_cam.Orbit(qx);
        m_cam.Orbit(qy);
    }

    m_x0 = x;
    m_y0 = y;

    repaint();

    m_cam.Update(true);

    ev->accept();

}

int CFiberGLWidget::heightForWidth(int w) const 
{
    return w;
}

//-------------------------------------------------------------------------------
class CODFPropertyList1 : public CPropertyList
{
public:
	CODFPropertyList1(CFiberODFAnalysis* odfAnalysis)
	{
		addProperty("X divisions", CProperty::Int);
		addProperty("Y divisions", CProperty::Int);
		addProperty("Z divisions", CProperty::Int);
		addProperty("Overlap fraction", CProperty::Float);
	}

	QVariant GetPropertyValue(int i) override
	{
		if (m_odfAnalysis == nullptr) return QVariant();

		switch (i)
		{
		case 0: return m_odfAnalysis->GetIntValue  (CFiberODFAnalysis::XDIV); break;
		case 1: return m_odfAnalysis->GetIntValue  (CFiberODFAnalysis::YDIV); break;
		case 2: return m_odfAnalysis->GetIntValue  (CFiberODFAnalysis::ZDIV); break;
		case 3: return m_odfAnalysis->GetFloatValue(CFiberODFAnalysis::OVERLAP); break;
		}

		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		if (m_odfAnalysis == nullptr) return;

		switch (i)
		{
		case 0: m_odfAnalysis->SetIntValue  (CFiberODFAnalysis::XDIV   , v.toInt() ); break;
		case 1: m_odfAnalysis->SetIntValue  (CFiberODFAnalysis::YDIV   , v.toInt() ); break;
		case 2: m_odfAnalysis->SetIntValue  (CFiberODFAnalysis::ZDIV   , v.toInt() ); break;
		case 3: m_odfAnalysis->SetFloatValue(CFiberODFAnalysis::OVERLAP, v.toDouble() ); break;
		}
	}

	void SetAnalysis(CFiberODFAnalysis* odfAnalysis) { m_odfAnalysis = odfAnalysis; }

private:
	CFiberODFAnalysis* m_odfAnalysis = nullptr;
};

//-------------------------------------------------------------------------------
class CODFPropertyList2 : public CPropertyList
{
public:
	CODFPropertyList2(CFiberODFAnalysis* odfAnalysis)
	{
		addProperty("Harmonic Order", CProperty::Int);
		addProperty("Low Freq Cutoff (pixels)", CProperty::Float);
		addProperty("High Freq Cutoff (pixels)", CProperty::Float);
		addProperty("Butterworth fraction", CProperty::Float);
		addProperty("Butterworth steepness", CProperty::Float);
		addProperty("Do fitting analysis", CProperty::Bool);
	}

	QVariant GetPropertyValue(int i) override
	{
		if (m_odfAnalysis == nullptr) return QVariant();

		switch (i)
		{
		case 0: return m_odfAnalysis->GetIntValue  (CFiberODFAnalysis::ORDER); break;
		case 1: return m_odfAnalysis->GetFloatValue(CFiberODFAnalysis::T_LOW); break;
		case 2: return m_odfAnalysis->GetFloatValue(CFiberODFAnalysis::T_HIGH); break;
		case 3: return m_odfAnalysis->GetFloatValue(CFiberODFAnalysis::BW_FRACTION); break;
		case 4: return m_odfAnalysis->GetFloatValue(CFiberODFAnalysis::BW_STEEPNESS); break;
		case 5: return m_odfAnalysis->GetBoolValue (CFiberODFAnalysis::FITTING); break;
		}

		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		if (m_odfAnalysis == nullptr) return;

		switch (i)
		{
		case 0: m_odfAnalysis->SetIntValue  (CFiberODFAnalysis::ORDER  , v.toInt() ); break;
		case 1: m_odfAnalysis->SetFloatValue(CFiberODFAnalysis::T_LOW  , v.toDouble() ); break;
		case 2: m_odfAnalysis->SetFloatValue(CFiberODFAnalysis::T_HIGH , v.toDouble() ); break;
		case 3: m_odfAnalysis->SetFloatValue(CFiberODFAnalysis::BW_FRACTION, v.toDouble()); break;
		case 4: m_odfAnalysis->SetFloatValue(CFiberODFAnalysis::BW_STEEPNESS, v.toDouble()); break;
		case 5: m_odfAnalysis->SetBoolValue (CFiberODFAnalysis::FITTING, v.toBool() ); break;
		}
	}

	void SetAnalysis(CFiberODFAnalysis* odfAnalysis) { m_odfAnalysis = odfAnalysis; }

private:
	CFiberODFAnalysis* m_odfAnalysis = nullptr;
};

//-------------------------------------------------------------------------------
class ODFParamsWidget : public QWidget
{
public:
	ODFParamsWidget(QWidget* parent = nullptr) : QWidget(parent)
	{
		m_stack = new QStackedWidget;

		// page 1 (generate subvolumes)
		QGroupBox* page1 = new QGroupBox("Step 1: Generate subvolumes");
		QVBoxLayout* l1 = new QVBoxLayout;
		m_form1 = new CPropertyListForm;
		m_form1->setPropertyList(m_props1 = new CODFPropertyList1(nullptr));
		l1->addWidget(m_form1);
		l1->addStretch();
		l1->addWidget(m_gen = new QPushButton("Generate subvolumes"));

		QHBoxLayout* h1 = new QHBoxLayout;
		h1->addStretch();
		h1->addWidget(m_next = new QPushButton("Next"));
		l1->addLayout(h1);

		setLayout(l1);
		page1->setLayout(l1);
		m_stack->addWidget(page1);

		// page 2 (generate ODFs)
		QGroupBox* page2 = new QGroupBox("Step 2: Generate ODFs");
		QVBoxLayout* l2 = new QVBoxLayout;
		m_form2 = new CPropertyListForm;
		m_form2->setPropertyList(m_props2 = new CODFPropertyList2(nullptr));
		l2->addWidget(m_form2);
		l2->addStretch();
		l2->addWidget(m_run = new QPushButton("Run"));

		QMenu* runMenu = new QMenu;
		m_runAll = runMenu->addAction("Process all subvolumes");
		m_runSel = runMenu->addAction("Process selected only");
		m_run->setMenu(runMenu);

		QHBoxLayout* h2 = new QHBoxLayout;
		h2->addWidget(m_back = new QPushButton("Back"));
		h2->addStretch();
		l2->addLayout(h2);

		setLayout(l2);
		page2->setLayout(l2);
		m_stack->addWidget(page2);

		//  build layout
		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addWidget(m_stack);
		setLayout(mainLayout);
	}

	void setAnalysis(CFiberODFAnalysis* odfAnalysis)
	{
		m_props1->SetAnalysis(odfAnalysis);
		m_props2->SetAnalysis(odfAnalysis);
		m_form1->updateData();
		m_form2->updateData();

		if (odfAnalysis && (odfAnalysis->ODFs() == 0))
		{
			m_stack->setCurrentIndex(0);
		}
	}

	void nextStep()
	{
		m_stack->setCurrentIndex(1);
	}

	void prevStep()
	{
		m_stack->setCurrentIndex(0);
	}

public:
	CODFPropertyList1* m_props1 = nullptr;
	CODFPropertyList2* m_props2 = nullptr;
	CPropertyListForm* m_form1 = nullptr;
	CPropertyListForm* m_form2 = nullptr;
	QPushButton* m_gen = nullptr;
	QPushButton* m_run = nullptr;
	QPushButton* m_next = nullptr;
	QPushButton* m_back = nullptr;
	QStackedWidget* m_stack = nullptr;
	QAction* m_runAll = nullptr;
	QAction* m_runSel = nullptr;
};

//-------------------------------------------------------------------------------
class Ui::CFiberODFWidget
{
public:
    QTabWidget* tabs;
    QComboBox* odfSelector;
    QCheckBox* odfCheck;
    CFiberGLWidget* glWidget;

	// tabs
	ODFParamsWidget* odfParams;
	QWidget* odfTab;
    QWidget* sphHarmTab;
	QWidget* fitTab;

    QTableWidget* sphHarmTable;
    QPushButton* copyToMatButton;
    QMenu* copyMenu;
    QAction* copyODF;
    QAction* copyEFD;
    QPushButton* saveToXMLButton;
    QMenu* saveMenu;
    QAction* saveSphHarm;
    QAction* saveODFs;
    QAction* saveStats;

	// analysis tab widgets
	QLineEdit* pos;
	QLineEdit* meanDir;
	QLineEdit* FA;
	QLineEdit* GFA;
    QLineEdit* meanIntensity;
	QLineEdit* EFD_alpha;
	QLineEdit* VM3_beta;
    QLineEdit* VM3_phi;
    QLineEdit* VM3_theta;

public:
    void setupUI(::CFiberODFWidget* parent)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);
        

        QWidget* w = new QWidget;
        QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->setContentsMargins(0,0,0,0);

		QHBoxLayout* odfl = new QHBoxLayout;
		odfl->addWidget(new QLabel("Select subvolume:"));
		odfl->addWidget(odfSelector = new QComboBox);
		odfl->addWidget(odfCheck = new QCheckBox); odfCheck->setChecked(true);
		QSizePolicy sp = odfSelector->sizePolicy();
		odfSelector->setSizePolicy(QSizePolicy::Expanding, sp.verticalPolicy());
		mainLayout->addLayout(odfl);
		mainLayout->addWidget(tabs = new QTabWidget);

		// parameters tab
		odfParams = new ODFParamsWidget;
		tabs->addTab(odfParams, "Parameters");

		// odf tab
        odfTab = new QWidget;
        QVBoxLayout* odfTabLayout = new QVBoxLayout;
        odfTabLayout->setContentsMargins(0,0,0,0);

        odfTabLayout->addWidget(glWidget = new CFiberGLWidget);

        odfTab->setLayout(odfTabLayout);
//        tabs->addTab(odfTab, "ODF");

		// spherical harmonics tab
        sphHarmTab = new QWidget;
        QVBoxLayout* sphHarmTabLayout = new QVBoxLayout;
        sphHarmTabLayout->setContentsMargins(0,0,0,0);

        sphHarmTable = new QTableWidget(0,1);
        sphHarmTable->horizontalHeader()->hide();
		sphHarmTable->horizontalHeader()->setStretchLastSection(true);
        sphHarmTabLayout->addWidget(sphHarmTable);

        sphHarmTab->setLayout(sphHarmTabLayout);
//        tabs->addTab(sphHarmTab, "Spherical Harmonics");

		// stats tab
		fitTab = new QWidget;
		QFormLayout* fitTabLayout = new QFormLayout;
		fitTabLayout->setLabelAlignment(Qt::AlignRight);
		fitTabLayout->addRow("Position:", pos = new QLineEdit); pos->setReadOnly(true);
		fitTabLayout->addRow("Mean Direction:", meanDir = new QLineEdit); meanDir->setReadOnly(true);
		fitTabLayout->addRow("FA:", FA = new QLineEdit); FA->setReadOnly(true);
		fitTabLayout->addRow("GFA:", GFA = new QLineEdit); GFA->setReadOnly(true);
        fitTabLayout->addRow("Mean Intensity:", meanIntensity = new QLineEdit); meanIntensity->setReadOnly(true);
		fitTabLayout->addRow("EFD alpha:", EFD_alpha = new QLineEdit); EFD_alpha->setReadOnly(true);
		fitTabLayout->addRow("VM3 beta:", VM3_beta = new QLineEdit); VM3_beta->setReadOnly(true);
        fitTabLayout->addRow("VM3 phi:", VM3_phi = new QLineEdit); VM3_phi->setReadOnly(true);
        fitTabLayout->addRow("VM3 theta:", VM3_theta = new QLineEdit); VM3_theta->setReadOnly(true);

		fitTab->setLayout(fitTabLayout);
//		tabs->addTab(fitTab, "Analysis");

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->setContentsMargins(0,0,0,0);

        buttonLayout->addWidget(copyToMatButton = new QPushButton("Copy to Material..."));
        copyMenu = new QMenu;
        copyMenu->addAction(copyODF = new QAction("FiberODF Material"));
        copyMenu->addAction(copyEFD = new QAction("EFD Material"));
        copyToMatButton->setMenu(copyMenu);

        buttonLayout->addWidget(saveToXMLButton = new QPushButton("Save to XML..."));
        saveMenu = new QMenu;
        saveMenu->addAction(saveODFs = new QAction("ODFs"));
        saveMenu->addAction(saveSphHarm = new QAction("Spherical Harmonics"));
        saveMenu->addAction(saveStats = new QAction("Statistics"));
        saveToXMLButton->setMenu(saveMenu);

		mainLayout->addLayout(buttonLayout);

        w->setLayout(mainLayout);
        
        layout->addWidget(w);

        parent->setLayout(layout);

    }

	void hideTabs()
	{
		tabs->removeTab(3);
		tabs->removeTab(2);
		tabs->removeTab(1);
	}

	void showTabs()
	{
		tabs->addTab(odfTab, "ODF");
		tabs->addTab(sphHarmTab, "Spherical Harmonics");
		tabs->addTab(fitTab, "Analysis");
	}

    void update(CFiberODFAnalysis* analysis)
    {
        glWidget->setAnalysis(analysis);
        glWidget->setODF(nullptr);

        if(!analysis || analysis->ODFs() == 0)
        {
			odfSelector->hide();
			odfParams->setAnalysis(analysis);
			hideTabs();
        }
        else if(analysis->ODFs() == 1)
        {
            odfSelector->hide();
            glWidget->setODF(analysis->GetODF(0));
			odfParams->setAnalysis(analysis);
			updateData(analysis);
			showTabs();
        }
        else
        {
            odfSelector->blockSignals(true);
            odfSelector->clear();

            for(int index = 0; index < analysis->ODFs(); index++)
            {
                odfSelector->addItem(QString("subvolume %1").arg(index));
            }

            odfSelector->blockSignals(false);
            odfSelector->setCurrentIndex(0);
            glWidget->setODF(analysis->GetODF(0));
            odfSelector->show();
			odfParams->setAnalysis(analysis);

			updateData(analysis);
			showTabs();
		}
    }

	int currentODF() 
	{ 
		if (odfSelector->isVisible() == false) return 0;
		else return odfSelector->currentIndex(); 
	}

	void updateData(CFiberODFAnalysis* analysis)
	{
		updateTable(analysis);
		updateFittingTab(analysis);
	}

private:
	void updateTable(CFiberODFAnalysis* analysis)
	{
		sphHarmTable->clear();
		int nodf = currentODF();
		if ((nodf < 0) || (nodf >= analysis->ODFs())) return;
		CODF* current = analysis->GetODF(nodf);

		vector<double>& sph = current->m_sphHarmonics;
		if (sph.empty()) return;

		sphHarmTable->setRowCount(sph.size());

		QStringList headers;
		QTableWidgetItem* ti;
		for(int i=0; i<sph.size(); ++i)
		{
			double val = sph[i];
			sphHarmTable->setItem(i, 0, ti = new QTableWidgetItem(QString::number(val)));
			ti->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
			headers << QString::number(i);
		}

		sphHarmTable->setVerticalHeaderLabels(headers);
	}

	void updateFittingTab(CFiberODFAnalysis* analysis)
	{
		int nodf = currentODF();
		if ((nodf < 0) || (nodf >= analysis->ODFs())) return;
		CODF* odf = analysis->GetODF(nodf);
		pos->setText(Vec3dToString(odf->m_position));
		meanDir->setText(Vec3dToString(odf->m_meanDir));
		FA->setText(QString::number(odf->m_FA));
		GFA->setText(QString::number(odf->m_GFA));
        meanIntensity->setText(QString::number(odf->m_meanIntensity));
		EFD_alpha->setText(Vec3dToString(odf->m_EFD_alpha));
		VM3_beta->setText(QString::number(odf->m_VM3_beta.x));
        VM3_phi->setText(QString::number(odf->m_VM3_beta.y));
        VM3_theta->setText(QString::number(odf->m_VM3_beta.z));
	}
};

CFiberODFWidget::CFiberODFWidget(CMainWindow* wnd)
    : m_analysis(nullptr), ui(new Ui::CFiberODFWidget), m_wnd(wnd)
{
    ui->setupUI(this);

    connect(ui->odfParams->m_runAll, &QAction::triggered, this, &CFiberODFWidget::on_runAll_triggered);
    connect(ui->odfParams->m_runSel, &QAction::triggered, this, &CFiberODFWidget::on_runSel_triggered);
    connect(ui->odfParams->m_gen, &QPushButton::clicked, this, &CFiberODFWidget::on_genButton_pressed);
    connect(ui->odfParams->m_next, &QPushButton::clicked, this, &CFiberODFWidget::on_nextButton_pressed);
    connect(ui->odfParams->m_back, &QPushButton::clicked, this, &CFiberODFWidget::on_backButton_pressed);
    connect(ui->odfSelector, &QComboBox::currentIndexChanged, this, &CFiberODFWidget::on_odfSelector_currentIndexChanged);
    connect(ui->odfCheck, &QCheckBox::stateChanged, this, &CFiberODFWidget::on_odfCheck_stateChanged);
    connect(ui->copyODF, &QAction::triggered, this, &CFiberODFWidget::on_copyODF_triggered);
    connect(ui->copyEFD, &QAction::triggered, this, &CFiberODFWidget::on_copyEFD_triggered);
    connect(ui->saveODFs, &QAction::triggered, this, &CFiberODFWidget::on_saveODFs_triggered);
    connect(ui->saveSphHarm, &QAction::triggered, this, &CFiberODFWidget::on_saveSphHarm_triggered);
    connect(ui->saveStats, &QAction::triggered, this, &CFiberODFWidget::on_saveStats_triggered);
}

void CFiberODFWidget::setAnalysis(CFiberODFAnalysis* analysis)
{
    m_analysis = analysis;
    
    ui->update(analysis);
}

class ImageAnalysisThread;

class ImageAnalysisLogger : public TaskLogger
{
public:
	ImageAnalysisLogger(ImageAnalysisThread* thread) : m_thread(thread) {}

	void Log(const std::string& msg) override;

private:
	ImageAnalysisThread* m_thread;
};

class ImageAnalysisThread : public CustomThread
{

public:
	ImageAnalysisThread(CImageAnalysis* analysis) : m_analysis(analysis), m_logger(this) {}

	void run() Q_DECL_OVERRIDE
	{
		if (m_analysis)
		{
			m_analysis->SetTaskLogger(&m_logger);
			m_analysis->run();
			m_analysis->SetTaskLogger(nullptr);
		}
		emit resultReady(true);
	}

	void WriteLog(QString msg)
	{
		emit writeLog(msg);
	}

public:
	bool hasProgress() override { return (m_analysis != nullptr); }

	double progress() override { return (m_analysis ? m_analysis->GetProgress().percent : 0.0); }

	const char* currentTask() override { return (m_analysis ? m_analysis->GetProgress().task : ""); }

	void stop() override { if (m_analysis) m_analysis->Terminate(); }

private:
	CImageAnalysis* m_analysis = nullptr;
	ImageAnalysisLogger m_logger;
	bool	m_processSelectedOnly;
};

void ImageAnalysisLogger::Log(const std::string& msg)
{
	assert(m_thread);
	if (m_thread) m_thread->WriteLog(QString::fromStdString(msg));
}

void CFiberODFWidget::on_runAll_triggered(bool b)
{
    if(!m_analysis) return;

	m_analysis->ProcessSelectedOnly(false);
	CDlgStartThread dlg(m_wnd, new ImageAnalysisThread(m_analysis));
	if (dlg.exec())
	{
		bool bsuccess = dlg.GetReturnCode();
		if (bsuccess == false)
		{
			QMessageBox::critical(this, "Apply Image Analysis", "Image analysis failed");
		}
		else
		{
		}
		m_wnd->RedrawGL();
	}

    ui->update(m_analysis);
}

void CFiberODFWidget::on_runSel_triggered(bool b)
{
	if (!m_analysis) return;

	m_analysis->ProcessSelectedOnly(true);
	CDlgStartThread dlg(m_wnd, new ImageAnalysisThread(m_analysis));
	if (dlg.exec())
	{
		bool bsuccess = dlg.GetReturnCode();
		if (bsuccess == false)
		{
			QMessageBox::critical(this, "Apply Image Analysis", "Image analysis failed");
		}
		else
		{
		}
		m_wnd->RedrawGL();
	}

	ui->update(m_analysis);
}

void CFiberODFWidget::on_genButton_pressed()
{
	if (!m_analysis) return;
	m_analysis->GenerateSubVolumes();
	ui->update(m_analysis);
	m_wnd->RedrawGL();
}

void CFiberODFWidget::on_nextButton_pressed()
{
	if (!m_analysis) return;
	if (m_analysis->ODFs() == 0)
	{
		QMessageBox::warning(m_wnd, "ODF Analysis", "You must generate subvolumes before continuing");
		return;
	}
	ui->odfParams->nextStep();
}

void CFiberODFWidget::on_backButton_pressed()
{
	if (!m_analysis) return;
	ui->odfParams->prevStep();
}

void CFiberODFWidget::on_odfSelector_currentIndexChanged(int index)
{
    if(!m_analysis) return;

	m_analysis->SelectODF(index);
	CODF* odf = m_analysis->GetODF(index);

	ui->odfCheck->blockSignals(true);
	ui->odfCheck->setChecked(odf->m_active);
	ui->odfCheck->blockSignals(false);

    ui->glWidget->setODF(odf);
    ui->glWidget->repaint();
	ui->updateData(m_analysis);
	m_wnd->RedrawGL();
}

void CFiberODFWidget::on_odfCheck_stateChanged(int state)
{
	int n = ui->currentODF();
	CODF* odf = m_analysis->GetODF(n);
	odf->m_active = (state == Qt::Checked);
	m_wnd->RedrawGL();
}

void CFiberODFWidget::on_copyODF_triggered()
{
    CModelDocument* doc = m_wnd->GetModelDocument();

    if(!m_analysis || m_analysis->ODFs() == 0 || !doc) return;

    FSModel* model = doc->GetFSModel();
    
    FSMaterial* mat = getMaterial("fiberODF");
    if(!mat) return;

    mat->GetProperty(1).Clear();

    int classID = FEBio::GetClassId(FECLASS_ID, "fiber-odf");

    for(int index = 0; index < m_analysis->ODFs(); index++)
    {
        CODF* current = m_analysis->GetODF(index);

        FSModelComponent* fiberODF = FEBio::CreateClass(classID, model);  
        fiberODF->GetParam("shp_harmonics")->SetVectorDoubleValue(current->m_sphHarmonics);
        fiberODF->GetParam("position")->SetVec3dValue(current->m_position);  
        mat->GetProperty(1).AddComponent(fiberODF);
    }  
}

void CFiberODFWidget::on_copyEFD_triggered()
{
    CModelDocument* doc = m_wnd->GetModelDocument();

    if(!m_analysis || m_analysis->ODFs() == 0 || !doc) return;

    // get the currently selected object
	GObject* po = doc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(m_wnd, "Tool", "You must first select an object.");
		return;
	}

	// make sure there is a mesh
	FSMesh* pm = po->GetFEMesh();
	if (pm == 0)
	{
		QMessageBox::critical(m_wnd, "Tool", "The object needs to be meshed before you can copy the EFD parameters.");
		return;
	}

    FSMaterial* mat = getMaterial("continuous fiber distribution");
    if(!mat) return;

    std::vector<mat3ds> efdTensors;
    for(int index = 0; index < m_analysis->ODFs(); index++)
    {
        auto odf = m_analysis->GetODF(index);

        mat3ds current = (odf->m_ev[0]&odf->m_ev[0]*odf->m_el[0]).sym();
        current += (odf->m_ev[1]&odf->m_ev[1]*odf->m_el[1]).sym();
        current += (odf->m_ev[2]&odf->m_ev[2]*odf->m_el[2]).sym();

        efdTensors.push_back(current);
    }

    //get the model and nodeset
	FSModel* ps = doc->GetFSModel();
	GModel& model = ps->GetModel();
    FSMesh* mesh = po->GetFEMesh();

    // ensure unique datamap name
    int num = 1;
    std::string datamapName;
    bool duplicate = false;
    do
    {
        datamapName = "EFD Datamap " + std::to_string(num++);
        duplicate = false;

        for(int index = 0; index < mesh->MeshDataFields(); index++)
        {
            if(mesh->GetMeshDataField(index)->GetName().compare(datamapName) == 0)
            {
                duplicate = true;
                break;
            }
        }
    } while (duplicate);

    // create element data
    int parts = po->Parts();
	FSPartSet* partSet = new FSPartSet(po->GetFEMesh());
    for (int i = 0; i < parts; ++i) partSet->add(i);
    partSet->SetName(datamapName);
    po->AddFEPartSet(partSet);
    

    FEPartData* pdata = new FEPartData(mesh);
    pdata->SetName(datamapName);
    pdata->Create(partSet, DATA_VEC3, DATA_ITEM);
    pm->AddMeshDataField(pdata);

    FEElemList* elemList = pdata->BuildElemList();
    int NE = elemList->Size();
    auto it = elemList->First();

    std::vector<FEElement_*> elems;
    for(int i = 0; i < NE; ++i, ++it)
    {
        elems.push_back(it->m_pi);
    }

    #pragma omp parallel for
    for (int i = 0; i < NE; ++i)
    {
        FEElement_* el = elems[i];
        
        // Calculate the centroid of the element
        vec3d pos(0);
        for(int node = 0; node < el->Nodes(); node++)
        {
            pos += mesh->LocalToGlobal(mesh->Node(el->m_node[node]).pos());
        }
        pos /= (double)el->Nodes();

        // Find the distances between this element and all of the ODFs
        std::vector<std::pair<int, double>> distPairs;
        for(int index = 0; index < m_analysis->ODFs(); index++)
        {
            auto odf = m_analysis->GetODF(index);

            distPairs.emplace_back(index,(m_analysis->GetODF(index)->m_position - pos).Length());
        }

        std::sort(distPairs.begin(), distPairs.end(), 
            [](std::pair<int, double> a, std::pair<int, double> b){ return a.second < b.second; });

        std::vector<double> distances;
        std::vector<mat3ds> currentTensors;
        int numClosest = 8 > efdTensors.size() ? efdTensors.size() : 8;
        for(int index = 0; index < numClosest; index++)
        {
            distances.push_back(distPairs[index].second);
            currentTensors.push_back(efdTensors[distPairs[index].first]);
        }


        double max = *std::max_element(distances.begin(), distances.end());
        double min = *std::min_element(distances.begin(), distances.end());

        double sum = 0;
        for(int index = 0; index < distances.size(); index++)
        {
            distances[index] = 1.0 - (distances[index] - min)/(max - min);
            sum += distances[index];
        }

        for(int index = 0; index < distances.size(); index++)
        {
            distances[index] /= sum;
        }

        // Get interpolated tensor
        mat3ds currentTensor = weightedAverageStructureTensor(currentTensors.data(), distances.data(), currentTensors.size());

        // Eigen decomposition
        double eVal[3];
        vec3d eVec[3];
        currentTensor.eigen(eVal, eVec);

        // Set values in data map
        pdata->set(i*3, eVal[0]);
        pdata->set(i*3+1, eVal[1]);
        pdata->set(i*3+2, eVal[2]);

        // Set mat axis
        el->m_Q = mat3d(eVec[0], eVec[1], eVec[2]);
        el->m_Qactive = true;
    }
    delete elemList;

    int classID = FEBio::GetClassId(FEMATERIALPROP_ID, "ellipsoidal");
    FSModelComponent* edf = FEBio::CreateClass(classID, ps);
    edf->GetParam("spa")->SetParamType(Param_Type::Param_STRING);
    edf->GetParam("spa")->SetStringValue(datamapName);

    mat->GetProperty(1).SetComponent(edf);

    m_wnd->UpdateModel();
}

FSMaterial* CFiberODFWidget::getMaterial(std::string type)
{
    CModelDocument* doc = m_wnd->GetModelDocument();

    if(!m_analysis || m_analysis->ODFs() == 0 || !doc) return nullptr;

    FSModel* model = doc->GetFSModel();

    std::vector<std::pair<std::string, FSMaterial*>> materials;
    for(int index = 0; index < model->Materials(); index++)
    {
        std::string name = model->GetMaterial(index)->GetName();

        auto current = model->GetMaterial(index)->GetMaterialProperties();

        if(std::string(current->GetTypeString()).find(type) != std::string::npos)
        {
            materials.push_back(std::pair<std::string, FSMaterial*>(name, current));
            continue;
        }

        findMaterials(model->GetMaterial(index)->GetMaterialProperties(), type, name, materials);
    }

    QDialog dlg(m_wnd);
    QVBoxLayout* layout = new QVBoxLayout;
    dlg.setLayout(layout);

    if(materials.size() == 0)
    {
        layout->addWidget(new QLabel(("There are no " + type + " materials in the current model.").c_str()));

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
        layout->addWidget(buttonBox);
        connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        
        dlg.exec();
        
        return nullptr;
    }
    
    layout->addWidget(new QLabel("Choose which material to copy the parameters to"));

    QComboBox* box = new QComboBox;
    for(auto val : materials)
    {
        box->addItem(val.first.c_str());
    }
    layout->addWidget(box);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if(dlg.exec())
    {
        return materials[box->currentIndex()].second;
    }

    return nullptr;
}

void CFiberODFWidget::findMaterials(FSMaterial* mat, std::string type, std::string name, std::vector<std::pair<std::string,FSMaterial*>>& materials)
{
    if(mat->Properties() == 0) return;

    for(int index = 0; index < mat->Properties(); index++)
    {
        std::string temp = name;
        for(int index2 = 0; index2 < mat->GetProperty(index).Size(); index2++)
        {
            FSProperty& currentProp = mat->GetProperty(index);

            name = temp + "-" + currentProp.GetName() + std::to_string(index2+1);

            auto current = dynamic_cast<FSMaterial*>(currentProp.GetComponent(index2));

            if(current)
            {
                if(std::string(current->GetTypeString()).find(type) != std::string::npos)
                {
                    materials.push_back(std::pair<std::string, FSMaterial*>(name, current));
                }

                findMaterials(current, type, name, materials);
            }
        }
    }
}

void CFiberODFWidget::on_saveSphHarm_triggered()
{
    if(!m_analysis || m_analysis->ODFs() == 0) return;

    QString filename = QFileDialog::getSaveFileName(m_wnd, "Save XML", QString(), "XML (*.xml)");

    XMLWriter writer;
    if(!writer.open(filename.toStdString().c_str())) return;

    XMLElement root("sphericalHarmonics");

    writer.add_branch(root);

    std::vector<double> position(3,0);

    for(int i = 0; i < m_analysis->ODFs(); i++)
    {
        CODF* current = m_analysis->GetODF(i);

        XMLElement el("sphHarm");
        writer.add_branch(el);

        XMLElement pos("position");
        position[0] = current->m_position.x;
        position[1] = current->m_position.y;
        position[2] = current->m_position.z;
        pos.value(position);
        writer.add_leaf(pos);

        XMLElement sphHarm("coefficients");
        sphHarm.value(current->m_sphHarmonics);
        writer.add_leaf(sphHarm);

        writer.close_branch();
    }

    writer.close_branch();

    writer.close();
}

void CFiberODFWidget::on_saveODFs_triggered()
{
    if(!m_analysis || m_analysis->ODFs() == 0) return;

    QString filename = QFileDialog::getSaveFileName(m_wnd, "Save XML", QString(), "XML (*.xml)");

    XMLWriter writer;
    if(!writer.open(filename.toStdString().c_str())) return;

    XMLElement root("ODFs");
    writer.add_branch(root);

    XMLElement nodes("Nodes");
    writer.add_branch(nodes);

    auto& coords = sphere::GetNodes(FULL);

    std::vector<double> position(3,0);

    for(int i = 0; i < coords.size(); i++)
    {
        XMLElement current("node");
        position[0] = coords[i].x;
        position[1] = coords[i].y;
        position[2] = coords[i].z;
        current.value(position);
        writer.add_leaf(current);
    }

    writer.close_branch();

    for(int i = 0; i < m_analysis->ODFs(); i++)
    {
        CODF* current = m_analysis->GetODF(i);

        XMLElement el("odf");
        writer.add_branch(el);

        XMLElement pos("position");
        position[0] = current->m_position.x;
        position[1] = current->m_position.y;
        position[2] = current->m_position.z;
        pos.value(position);
        writer.add_leaf(pos);

        XMLElement vals("values");
        vals.value(current->m_odf);
        writer.add_leaf(vals);

        writer.close_branch();
    }

    writer.close_branch();

    writer.close();
}

void CFiberODFWidget::on_saveStats_triggered()
{
    if(!m_analysis || m_analysis->ODFs() == 0) return;

    QString filename = QFileDialog::getSaveFileName(m_wnd, "Save XML", QString(), "XML (*.xml)");

    XMLWriter writer;
    if(!writer.open(filename.toStdString().c_str())) return;

    XMLElement root("odfStats");

    writer.add_branch(root);

    std::vector<double> position(3,0);
    std::vector<double> mDir(3,0);
    std::vector<double> eigenVals(3,0);
    std::vector<double> eigenVec1(3,0);
    std::vector<double> eigenVec2(3,0);
    std::vector<double> eigenVec3(3,0);
    std::vector<double> beta(3,0);

    for(int i = 0; i < m_analysis->ODFs(); i++)
    {
        CODF* current = m_analysis->GetODF(i);

        XMLElement el("stats");
        writer.add_branch(el);

        XMLElement pos("position");
        position[0] = current->m_position.x;
        position[1] = current->m_position.y;
        position[2] = current->m_position.z;
        pos.value(position);
        writer.add_leaf(pos);

        XMLElement meanDir("meanDir");
        mDir[0] = current->m_meanDir.x;
        mDir[1] = current->m_meanDir.y;
        mDir[2] = current->m_meanDir.z;
        meanDir.value(mDir);
        writer.add_leaf(meanDir);

        XMLElement meanIntensity("meanIntensity");
        meanIntensity.value(current->m_meanIntensity);
        writer.add_leaf(meanIntensity);

        XMLElement FA("fracAnisotropy");
        FA.value(current->m_FA);
        writer.add_leaf(FA);

        XMLElement efd("GFA");
        efd.value(current->m_GFA);
        writer.add_leaf(efd);

        XMLElement efdFrd("EFD_FRD");
        efdFrd.value(current->m_EFD_FRD);
        writer.add_leaf(efdFrd);

        XMLElement eigenValsEl("EFDEigenVals");
        eigenVals[0] = current->m_el[0];
        eigenVals[1] = current->m_el[1];
        eigenVals[2] = current->m_el[2];
        eigenValsEl.value(eigenVals);
        writer.add_leaf(eigenValsEl);

        XMLElement EFDVec1("EFDVec1");
        eigenVec1[0] = current->m_ev[0].x;
        eigenVec1[1] = current->m_ev[0].y;
        eigenVec1[2] = current->m_ev[0].z;
        EFDVec1.value(eigenVec1);
        writer.add_leaf(EFDVec1);

        XMLElement EFDVec2("EFDVec2");
        eigenVec2[0] = current->m_ev[1].x;
        eigenVec2[1] = current->m_ev[1].y;
        eigenVec2[2] = current->m_ev[1].z;
        EFDVec2.value(eigenVec2);
        writer.add_leaf(EFDVec2);

        XMLElement EFDVec3("EFDVec3");
        eigenVec3[0] = current->m_ev[2].x;
        eigenVec3[1] = current->m_ev[2].y;
        eigenVec3[2] = current->m_ev[2].z;
        EFDVec3.value(eigenVec3);
        writer.add_leaf(EFDVec3);

        XMLElement vm3Frd("VM3");
        vm3Frd.value(current->m_VM3_FRD);
        writer.add_leaf(vm3Frd);

        XMLElement betaEl("VM3_beta");
        betaEl.value(current->m_VM3_beta.x);
        writer.add_leaf(betaEl);
        
        XMLElement phiEl("VM3_phi");
        phiEl.value(current->m_VM3_beta.y);
        writer.add_leaf(phiEl);
        
        XMLElement thetaEl("VM3_theta");
        thetaEl.value(current->m_VM3_beta.z);
        writer.add_leaf(thetaEl);

        writer.close_branch();
    }

    writer.close_branch();

    writer.close();
}