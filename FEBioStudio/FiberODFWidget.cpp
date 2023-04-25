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
#include <fstream>
#include <vector>
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
#include <ImageLib/FiberODFAnalysis.h>
#include <FEAMR/spherePoints.h>
#include "DlgStartThread.h"
#include "PropertyList.h"
#include <XML/XMLWriter.h>

#ifdef __APPLE__
#include <OpenGL/GLU.h>
#else
#include <GL/glu.h>
#endif

#include <iostream>

using std::vector;
using std::complex;

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
	glLineWidth(1.5f);

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

    // set the projection Matrix to ortho2d so we can draw some stuff on the screen
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.2, 1.2, -1.2, 1.2, 0.01, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    m_cam.SetTargetDistance(50);
    m_cam.Transform();

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

    QOpenGLWidget::paintGL();
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
class Ui::CFiberODFWidget
{
public:
    DynamicStackedWidget* stack;
    
    QPushButton* runButton;

    QWidget* firstPage;

    QWidget* secondPage;
    QTabWidget* tabs;
    QComboBox* odfSelector;
    QCheckBox* odfCheck;
    CFiberGLWidget* glWidget;
    
    QWidget* sphHarmTab;
    QTableWidget* sphHarmTable;
    QPushButton* copyToMatButton;
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
        
        stack = new DynamicStackedWidget;

        firstPage = new QWidget;
        QVBoxLayout* firstPageLayout = new QVBoxLayout;
        firstPageLayout->addWidget(new QLabel("Click Run to\ngenerate ODF(s)."));

        firstPage->setLayout(firstPageLayout);
        stack->addWidget(firstPage);

        secondPage = new QWidget;
        QVBoxLayout* secondPageLayout = new QVBoxLayout;
        secondPageLayout->setContentsMargins(0,0,0,0);

		QHBoxLayout* odfl = new QHBoxLayout;
		odfl->addWidget(new QLabel("Select ODF:"));
		odfl->addWidget(odfSelector = new QComboBox);
		odfl->addWidget(odfCheck = new QCheckBox); odfCheck->setChecked(true);
		QSizePolicy sp = odfSelector->sizePolicy();
		odfSelector->setSizePolicy(QSizePolicy::Expanding, sp.verticalPolicy());
		secondPageLayout->addLayout(odfl);
        secondPageLayout->addWidget(tabs = new QTabWidget);

        QWidget* odfTab = new QWidget;
        QVBoxLayout* odfTabLayout = new QVBoxLayout;
        odfTabLayout->setContentsMargins(0,0,0,0);

        odfTabLayout->addWidget(glWidget = new CFiberGLWidget);

        odfTab->setLayout(odfTabLayout);
        tabs->addTab(odfTab, "ODF");

        QWidget* sphHarmTab = new QWidget;
        QVBoxLayout* sphHarmTabLayout = new QVBoxLayout;
        sphHarmTabLayout->setContentsMargins(0,0,0,0);

        sphHarmTable = new QTableWidget(0,1);
        sphHarmTable->horizontalHeader()->hide();
		sphHarmTable->horizontalHeader()->setStretchLastSection(true);
        sphHarmTabLayout->addWidget(sphHarmTable);

        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonLayout->setContentsMargins(0,0,0,0);

        buttonLayout->addWidget(copyToMatButton = new QPushButton("Copy to Material"));
        buttonLayout->addWidget(saveToXMLButton = new QPushButton("Save to CSV..."));
        
        saveMenu = new QMenu;
        saveMenu->addAction(saveODFs = new QAction("ODFs"));
        saveMenu->addAction(saveSphHarm = new QAction("Spherical Harmonics"));
        saveMenu->addAction(saveStats = new QAction("Statistics"));

        saveToXMLButton->setMenu(saveMenu);

        sphHarmTabLayout->addLayout(buttonLayout);

        sphHarmTab->setLayout(sphHarmTabLayout);
        tabs->addTab(sphHarmTab, "Spherical Harmonics");

		QWidget* fitTab = new QWidget;
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
		tabs->addTab(fitTab, "Analysis");

        secondPage->setLayout(secondPageLayout);
        stack->addWidget(secondPage);

        layout->addWidget(stack);

        layout->addWidget(runButton = new QPushButton("Run"));

        parent->setLayout(layout);

    }

    void update(CFiberODFAnalysis* analysis)
    {
        glWidget->setAnalysis(analysis);
        glWidget->setODF(nullptr);

        if(!analysis || analysis->ODFs() == 0)
        {
            stack->setCurrentIndex(0);
        }
        else if(analysis->ODFs() == 1)
        {
            odfSelector->hide();
            glWidget->setODF(analysis->GetODF(0));
			stack->setCurrentIndex(1);
			updateData(analysis);
        }
        else
        {
            odfSelector->blockSignals(true);
            odfSelector->clear();

            for(int index = 0; index < analysis->ODFs(); index++)
            {
                odfSelector->addItem(QString("ODF %1").arg(index));
            }

            odfSelector->blockSignals(false);
            odfSelector->setCurrentIndex(0);
            glWidget->setODF(analysis->GetODF(0));
            odfSelector->show();
			stack->setCurrentIndex(1);

			updateData(analysis);
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

    connect(ui->runButton, &QPushButton::pressed, this, &CFiberODFWidget::on_runButton_pressed);
    connect(ui->odfSelector, &QComboBox::currentIndexChanged, this, &CFiberODFWidget::on_odfSelector_currentIndexChanged);
    connect(ui->odfCheck, &QCheckBox::stateChanged, this, &CFiberODFWidget::on_odfCheck_stateChanged);
    connect(ui->copyToMatButton, &QPushButton::pressed, this, &CFiberODFWidget::on_copyToMatButton_pressed);
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
};

void ImageAnalysisLogger::Log(const std::string& msg)
{
	assert(m_thread);
	if (m_thread) m_thread->WriteLog(QString::fromStdString(msg));
}

void CFiberODFWidget::on_runButton_pressed()
{
    if(!m_analysis) return;

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

void CFiberODFWidget::on_copyToMatButton_pressed()
{
    CModelDocument* doc = m_wnd->GetModelDocument();

    if(!m_analysis || m_analysis->ODFs() == 0 || !doc) return;

    FSModel* model = doc->GetFSModel();

    std::vector<std::pair<std::string, FSMaterial*>> materials;
    for(int index = 0; index < model->Materials(); index++)
    {
        std::string name = model->GetMaterial(index)->GetName();

        auto current = model->GetMaterial(index)->GetMaterialProperties();

        if(std::string("fiberODF").compare(current->GetTypeString()) == 0)
        {
            materials.push_back(std::pair<std::string, FSMaterial*>(name, current));
            continue;
        }

        findMaterials(model->GetMaterial(index)->GetMaterialProperties(), name, materials);
    }

    QDialog dlg(m_wnd);
    QVBoxLayout* layout = new QVBoxLayout;

    if(materials.size() == 0)
    {
        layout->addWidget(new QLabel("There are no fiberODF materials in the current model."));

        layout->addWidget(new QDialogButtonBox(QDialogButtonBox::Ok));
        dlg.exec();
        return;
    }
    
    layout->addWidget(new QLabel("Choose which material to copy the ODF(s) to"));

    QComboBox* box = new QComboBox;
    for(auto val : materials)
    {
        box->addItem(val.first.c_str());
    }
    layout->addWidget(box);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    dlg.setLayout(layout);

    connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if(dlg.exec())
    {
        FSMaterial* mat = materials[box->currentIndex()].second;
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
}

void CFiberODFWidget::findMaterials(FSMaterial* mat, std::string name, std::vector<std::pair<std::string,FSMaterial*>>& materials)
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
                if(std::string("fiberODF").compare(current->GetTypeString()) == 0)
                {
                    materials.push_back(std::pair<std::string, FSMaterial*>(name, current));
                }

                findMaterials(current, name, materials);
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

    std::vector<double> position(3,0);

    for(int i = 0; i < NPTS; i++)
    {
        XMLElement current("node");
        position[0] = XCOORDS[i];
        position[1] = YCOORDS[i];
        position[2] = ZCOORDS[i];
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
    std::vector<double> alpha(3,0);
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

        XMLElement alphaEl("EFDaplha");
        alpha[0] = current->m_EFD_alpha.x;
        alpha[1] = current->m_EFD_alpha.y;
        alpha[2] = current->m_EFD_alpha.z;
        alphaEl.value(alpha);
        writer.add_leaf(alphaEl);

        XMLElement vm3Frd("VM3");
        vm3Frd.value(current->m_VM3_FRD);
        writer.add_leaf(vm3Frd);

        XMLElement betaEl("VM3beta");
        betaEl.value(current->m_VM3_beta.x);
        writer.add_leaf(betaEl);
        
        XMLElement phiEl("VM3beta");
        phiEl.value(current->m_VM3_beta.y);
        writer.add_leaf(phiEl);
        
        XMLElement thetaEl("VM3beta");
        thetaEl.value(current->m_VM3_beta.z);
        writer.add_leaf(thetaEl);

        writer.close_branch();
    }

    writer.close_branch();

    writer.close();
}