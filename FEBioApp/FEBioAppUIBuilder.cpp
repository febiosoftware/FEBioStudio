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
#include "FEBioAppUIBuilder.h"
#include <XML/XMLReader.h>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QTabWidget>
#include <QFileInfo>
#include <QPlainTextEdit>
#include <CUILib/GLSceneView.h>
#include <CUILib/InputWidgets.h>
#include <CUILib/PlotWidget.h>
#include "FEBioAppWidget.h"
#include "ActionButton.h"
#include "GLFEBioScene.h"
#include <FECore/FEParam.h>
#include <FECore/FECoreKernel.h>
#include <FECore/ElementDataRecord.h>
#include <FEBioLib/FEBioModel.h>
#include <algorithm>

class CElementDataSource : public CFEBioModelDataSource
{
public:
	CElementDataSource(ElementDataRecord* x, ElementDataRecord* y, CPlotData* plotData) : m_xval(x), m_yval(y), m_data(plotData) {}

	void Clear() override
	{
		m_data->clear();
	}

	void Update(double time) override
	{
		double x = (m_xval ? m_xval->Evaluate(m_xval->m_item[0], 0) : time);
		double y = m_yval->Evaluate(m_yval->m_item[0], 0);
		m_data->addPoint(x, y);
	}

private:
	ElementDataRecord* m_xval;
	ElementDataRecord* m_yval;

	CPlotData* m_data;
};

//====================================================================
FEBioAppUIBuilder::FEBioAppUIBuilder() : ui(nullptr), app(nullptr)
{

}

FEBioAppWidget* FEBioAppUIBuilder::error()
{
	if (ui) delete ui;
	ui = nullptr;
	return nullptr;
}

FEBioAppWidget* FEBioAppUIBuilder::BuildUIFromFile(QString filePath, FEBioApp* app)
{
	if (app == nullptr) return nullptr;
	this->app = app;

	ui = nullptr;

	XMLReader xml;
	std::string filename = filePath.toStdString();
	if (xml.Open(filename.c_str()) == false) return nullptr;

	XMLTag tag;
	if (xml.FindTag("febio_app", tag) == false) return nullptr;

	QFileInfo fi(filePath);
	m_appFolder = fi.absolutePath();

	try {
		++tag;
		do
		{
			if      (tag == "Model") { if (parseModel(tag) == false) return error(); }
			else if (tag == "GUI"  ) { if (parseGUI(tag) == false) return error(); }
			else tag.skip();

			++tag;
		} while (!tag.isend());
	}
	catch (...)
	{

	}

	return ui;
}

bool FEBioAppUIBuilder::parseModel(XMLTag& tag)
{
	XMLReader& xml = *tag.m_preader;

	const char* szid = tag.AttributeValue("id", true);
	if (szid == nullptr) szid = "fem";

	const char* szfile = tag.AttributeValue("file");

	const char* sztask = tag.AttributeValue("task", true);
	const char* szctrl = tag.AttributeValue("task_input", true);

	assert(tag.isleaf());

	// add the new model
	QString fileName = m_appFolder + QString("/%1").arg(szfile);
	bool b = app->LoadModelFromFile(fileName.toStdString()); // TODO: Did I move the febio_cb in this function?

	if (sztask)
	{
		if (szctrl)
		{
			QString ctrlFile = m_appFolder + QString("/%1").arg(szctrl);
			app->SetTask(sztask, ctrlFile.toStdString());
		}
		else
			app->SetTask(sztask);
	}

	return b;
}

bool FEBioAppUIBuilder::parseGUI(XMLTag& tag)
{
	ui = new FEBioAppWidget(app);
	QVBoxLayout* layout = new QVBoxLayout(ui);
	return parseGUITags(tag, layout);
}

bool FEBioAppUIBuilder::parseGUITags(XMLTag& tag, QBoxLayout* playout)
{
	// loop over all tags
	++tag;
	do
	{
		if      (tag == "label"     ) parseLabel    (tag, playout);
		else if (tag == "vgroup"    ) parseVGroup   (tag, playout);
		else if (tag == "hgroup"    ) parseHGroup   (tag, playout);
		else if (tag == "tab_group" ) parseTabGroup (tag, playout);
		else if (tag == "stretch"   ) parseStretch  (tag, playout);
		else if (tag == "button"    ) parseButton   (tag, playout);
		else if (tag == "input"     ) parseInput    (tag, playout);
		else if (tag == "input_list") parseInputList(tag, playout);
		else if (tag == "graph"     ) parseGraph    (tag, playout);
		else if (tag == "plot3d"    ) parsePlot3d   (tag, playout);
		else if (tag == "output"    ) parseOutput   (tag, playout);
		else tag.skip();

		++tag;
	}
	while (!tag.isend());

	return true;
}

void FEBioAppUIBuilder::parseLabel(XMLTag& tag, QBoxLayout* playout)
{
	std::string name, id;
	for (XMLAtt& att : tag.m_att)
	{
		if (strcmp(att.name(), "text") == 0)
		{
			name = att.cvalue();
		}
		else if (strcmp(att.name(), "id") == 0)
		{
			id = att.cvalue();
		}
	}

	QLabel* plabel = new QLabel(QString::fromStdString(name));
	if (id.empty() == false) plabel->setObjectName(QString::fromStdString(id));
	QFont f("Times", 14, QFont::Bold);
	plabel->setFont(f);
	plabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	playout->addWidget(plabel);
}

void FEBioAppUIBuilder::parseButton(XMLTag& tag, QBoxLayout* playout)
{
	assert(tag.isleaf());

	const char* szid     = tag.AttributeValue("id", true);
	const char* sztxt    = tag.AttributeValue("text", true);
	const char* szaction = tag.AttributeValue("onClick", true);

	CActionButton* pb = new CActionButton(sztxt);
	if (szid) pb->setObjectName(szid);
	pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	if (szaction)
	{
		pb->setAction(szaction);
		QObject::connect(pb, &CActionButton::doAction, app, &FEBioApp::runScript);
	}

	playout->addWidget(pb);
}

void FEBioAppUIBuilder::parseGraph(XMLTag& tag, QBoxLayout* playout)
{
	const char *sztxt = nullptr, *szid = nullptr;
	int size[2] = { 400, 400 };

	for (XMLAtt& att : tag.m_att)
	{
		if      (att.m_name == "text") sztxt = att.cvalue();
		else if (att.m_name == "id"  ) szid  = att.cvalue();
		else if (att.m_name == "size") att.value(size, 2);
	}

	XMLReader& xml = *tag.m_preader;

	FECoreKernel& fecore = FECoreKernel::GetInstance();

	FEBioModel* fem = app->GetFEBioModel();

	CPlotWidget* plotWidget = new CPlotWidget(0);
	if (szid) plotWidget->setObjectName(szid);
	if (sztxt) plotWidget->setTitle(QString(sztxt));
//	plotWidget->showLegend(false);

	QObject::connect(ui, &FEBioAppWidget::modelFinished, plotWidget, &CPlotWidget::OnZoomToFit);

	if (!tag.isleaf())
	{
		++tag;
		do
		{
			if (tag == "data")
			{
				const char* sztxt = tag.AttributeValue("text", true);

				CPlotData* data = new CPlotData;
				if (sztxt) data->setLabel(sztxt);

				const char* sztype = tag.AttributeValue("type", true);
				if (sztype == 0)
				{
					ElementDataRecord* val_x = nullptr;
					ElementDataRecord* val_y = nullptr;

					++tag;
					do
					{
						if (tag == "x")
						{
							const char* szvar = tag.AttributeValue("var");
							const char* szid = tag.AttributeValue("elemId");
							int eid = atoi(szid);

							val_x = new ElementDataRecord(fem);
							val_x->SetData(szvar);
							val_x->SetItemList({ eid });
						}
						else if (tag == "y")
						{
							const char* szvar = tag.AttributeValue("var");
							const char* szid = tag.AttributeValue("elemId");
							int eid = atoi(szid);

							val_y = new ElementDataRecord(fem);
							val_y->SetData(szvar);
							val_y->SetItemList({ eid });
						}
						++tag;
					} while (!tag.isend());


					CElementDataSource* src = new CElementDataSource(val_x, val_y, data);
					app->AddModelDataSource(src);
					++tag;
				}
				else if (strcmp(sztype, "static") == 0)
				{
					++tag;
					do
					{
						double p[2];
						tag.value(p, 2);
						data->addPoint(p[0], p[1]);
						++tag;
					} while (!tag.isend());
					++tag;
				}
				plotWidget->addPlotData(data);
			}
			else xml.SkipTag(tag);
		} while (!tag.isend());
	}

	plotWidget->setMinimumSize(QSize(size[0], size[1]));

	playout->addWidget(plotWidget, 1);
}

void FEBioAppUIBuilder::parseInputList(XMLTag& tag, QBoxLayout* playout)
{
	const char* sztitle = tag.AttributeValue("text", true);
	const char* szparams = tag.AttributeValue("params");

	assert(tag.isleaf());

	int naction = -1;

	vector<FEParamValue> paramList;
	paramList = app->GetFEBioParameterList(szparams);

	if (!paramList.empty())
	{
		QGroupBox* pg = 0;
		if (sztitle) pg = new QGroupBox(sztitle);

		QFormLayout* pf = new QFormLayout;
		for (size_t i = 0; i < paramList.size(); ++i)
		{
			FEParamValue& pi = paramList[i];

			CFEBioParamEdit* paramEdit = new CFEBioParamEdit(ui);
			paramEdit->SetParameter(pi);

			switch (pi.type())
			{
			case FE_PARAM_DOUBLE: paramEdit->SetEditor(new CFloatInput()); break;
			case FE_PARAM_BOOL  : paramEdit->SetEditor(new QCheckBox  ()); break;
			case FE_PARAM_INT   : paramEdit->SetEditor(new CIntInput  ()); break;
			default:
				assert(false);
			}

			QWidget* pw = paramEdit->GetEditor();
			if (pw)
			{
//				pw->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

				// add it to the row
				pf->addRow(QString::fromStdString(pi.param()->name()), pw);
			}
			else delete paramEdit;
		}

		if (pg) { pg->setLayout(pf); playout->addWidget(pg); }
		else playout->addLayout(pf);
	}
	else
	{
		printf("ERROR: Cannot find property: %s\n", tag.szvalue());
	}
}

void FEBioAppUIBuilder::parseVGroup(XMLTag& tag, QBoxLayout* playout)
{
	const char* szname = tag.AttributeValue("title", true);

	QGroupBox* pg = 0;
	if (szname) pg = new QGroupBox(szname);

	QBoxLayout* pl = new QVBoxLayout;
	parseGUITags(tag, pl);

	if (pg)
	{
		pg->setLayout(pl);
		playout->addWidget(pg);
	}
	else playout->addLayout(pl);
}

void FEBioAppUIBuilder::parseHGroup(XMLTag& tag, QBoxLayout* playout)
{
	const char* szname = tag.AttributeValue("title", true);

	QGroupBox* pg = 0;
	if (szname) pg = new QGroupBox(szname);

	QBoxLayout* pl = new QHBoxLayout;
	parseGUITags(tag, pl);

	if (pg)
	{
		pg->setLayout(pl);
		playout->addWidget(pg);
	}
	else playout->addLayout(pl);
}

void FEBioAppUIBuilder::parseStretch(XMLTag& tag, QBoxLayout* playout)
{
	playout->addStretch();
}

void FEBioAppUIBuilder::parseInput(XMLTag& tag, QBoxLayout* playout)
{
	// read the title
	char sz[256] = { 0 };
	strcpy(sz, tag.AttributeValue("text"));

	// read the align
	CFEBioParamEdit::AlignOptions nalign = CFEBioParamEdit::AlignOptions::ALIGN_LEFT;

	const char* szalign = tag.AttributeValue("align", true);
	if (szalign)
	{
		if      (strcmp(szalign, "left"       ) == 0) nalign = CFEBioParamEdit::AlignOptions::ALIGN_LEFT;
		else if (strcmp(szalign, "right"      ) == 0) nalign = CFEBioParamEdit::AlignOptions::ALIGN_RIGHT;
		else if (strcmp(szalign, "top"        ) == 0) nalign = CFEBioParamEdit::AlignOptions::ALIGN_TOP;
		else if (strcmp(szalign, "bottom"     ) == 0) nalign = CFEBioParamEdit::AlignOptions::ALIGN_BOTTOM;
		else if (strcmp(szalign, "topleft"    ) == 0) nalign = CFEBioParamEdit::AlignOptions::ALIGN_TOP_LEFT;
		else if (strcmp(szalign, "topright"   ) == 0) nalign = CFEBioParamEdit::AlignOptions::ALIGN_TOP_RIGHT;
		else if (strcmp(szalign, "bottomleft" ) == 0) nalign = CFEBioParamEdit::AlignOptions::ALIGN_BOTTOM_LEFT;
		else if (strcmp(szalign, "bottomright") == 0) nalign = CFEBioParamEdit::AlignOptions::ALIGN_BOTTOM_RIGHT;
		else printf("WARNING: Unknown align value %s\n", szalign);
	}

	// read the parameter
	const char* szparam = tag.AttributeValue("param");
	FEParamValue param = app->GetFEBioParameter(szparam);
	assert(param.isValid());

	// read the range
	double rng[3];
	bool brange = false;
	XMLAtt* rngAtt = tag.AttributePtr("range");
	if (rngAtt)
	{
		rngAtt->value(rng, 3);
		brange = true;
	}

	assert(tag.isempty());

	QBoxLayout* pl = 0;

	if ((nalign == CFEBioParamEdit::AlignOptions::ALIGN_LEFT) || (nalign == CFEBioParamEdit::AlignOptions::ALIGN_RIGHT)) pl = new QHBoxLayout;
	else pl = new QVBoxLayout;

	QLabel* plabel = new QLabel(sz);
	//	plabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	CFEBioParamEdit* pi = new CFEBioParamEdit(ui);
	pi->SetParameter(param);
	if (param.type() == FE_PARAM_DOUBLE)
	{
		if (brange)
		{
			CDoubleSlider* slider = new CDoubleSlider;
			slider->setRange(rng[0], rng[1]);
			slider->setSingleStep(rng[2]);
			pi->SetEditor(slider);
		}
		else
		{
			pi->SetEditor(new CFloatInput);
		}
	}
	if (param.type() == FE_PARAM_BOOL)
	{
		pi->SetEditor(new QCheckBox);
	}
	if (param.type() == FE_PARAM_INT)
	{
		pi->SetEditor(new CIntInput);
	}

	QWidget* pw = pi->GetEditor(); assert(pw);
	if ((pw == nullptr) || !param.isValid())
	{
		delete pi; 
		return;
	}

	pw->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	playout->addLayout(pl);

	switch (nalign)
	{
	case CFEBioParamEdit::AlignOptions::ALIGN_LEFT:
		pl->addWidget(plabel);
		pl->addWidget(pw);
		pl->addStretch();
		break;
	case CFEBioParamEdit::AlignOptions::ALIGN_RIGHT:
		pl->addWidget(pw);
		pl->addWidget(plabel);
		pl->addStretch();
		break;
	case CFEBioParamEdit::AlignOptions::ALIGN_TOP:
		plabel->setAlignment(Qt::AlignHCenter);
		pl->addWidget(plabel);
		pl->addWidget(pw);
		break;
	case CFEBioParamEdit::AlignOptions::ALIGN_BOTTOM:
		pl->addWidget(pw);
		plabel->setAlignment(Qt::AlignHCenter);
		pl->addWidget(plabel);
		break;
	case CFEBioParamEdit::AlignOptions::ALIGN_TOP_LEFT:
		pl->addWidget(plabel);
		pl->addWidget(pw);
		break;
	case CFEBioParamEdit::AlignOptions::ALIGN_TOP_RIGHT:
		plabel->setAlignment(Qt::AlignRight);
		pl->addWidget(plabel);
		pl->addWidget(pw);
		break;
	case CFEBioParamEdit::AlignOptions::ALIGN_BOTTOM_LEFT:
		pl->addWidget(pw);
		pl->addWidget(plabel);
		break;
	case CFEBioParamEdit::AlignOptions::ALIGN_BOTTOM_RIGHT:
		plabel->setAlignment(Qt::AlignRight);
		pl->addWidget(pw);
		pl->addWidget(plabel);
		break;
	default:
		assert(false);
	}
}

void FEBioAppUIBuilder::parseTabGroup(XMLTag& tag, QBoxLayout* playout)
{
	QTabWidget* ptab = new QTabWidget();

	++tag;
	do
	{
		if (tag == "tab")
		{
			QString s(tag.AttributeValue("title"));

			QWidget* pw = new QWidget;
			QVBoxLayout* pl = new QVBoxLayout;
			pl->setContentsMargins(0, 0, 0, 0);
			parseGUITags(tag, pl);
			pw->setLayout(pl);
			ptab->addTab(pw, s);

			++tag;
		}
		else tag.skip();
	} while (!tag.isend());

	playout->addWidget(ptab);
}

void FEBioAppUIBuilder::parsePlot3d(XMLTag& tag, QBoxLayout* playout)
{
	const char* sztxt = nullptr;
	int size[2] = { 400, 400 };
	double bgc[3] = { 0.8, 0.8, 1.0 };
	double rot[3] = { 0, 0, 0 };
	for (XMLAtt& att : tag.m_att)
	{
		if      (att.m_name == "text"    ) sztxt = att.cvalue();
		else if (att.m_name == "size"    ) att.value(size, 2);
		else if (att.m_name == "bg_color")
		{
			if      (strcmp(att.cvalue(), "black") == 0) {bgc[0] = 0.0; bgc[1] = 0.0; bgc[2] = 0.0; }
			else if (strcmp(att.cvalue(), "white") == 0) {bgc[0] = 1.0; bgc[1] = 1.0; bgc[2] = 1.0; }
			else if (strcmp(att.cvalue(), "red"  ) == 0) {bgc[0] = 1.0; bgc[1] = 0.0; bgc[2] = 0.0; }
			else if (strcmp(att.cvalue(), "green") == 0) {bgc[0] = 0.0; bgc[1] = 1.0; bgc[2] = 0.0; }
			else if (strcmp(att.cvalue(), "blue" ) == 0) {bgc[0] = 0.0; bgc[1] = 0.0; bgc[2] = 1.0; }
			else
				att.value(bgc, 3);
		}
		else if (att.m_name == "rotation") att.value(rot, 3);
	}

	GLFEBioScene* scene = new GLFEBioScene(*app->GetFEBioModel());

	bool brange = false;
	double rng[2];
	std::string mapName;
	std::string colMap;
	if (!tag.isleaf())
	{
		++tag;
		do
		{
			if (tag == "map")
			{
				const char* szdata = tag.AttributeValue("data");
				if (szdata) mapName = szdata;

				const char* szmin = tag.AttributeValue("rangeMin", true);
				const char* szmax = tag.AttributeValue("rangeMax", true);
				if (szmin && szmax)
				{
					brange = true;
					rng[0] = atof(szmin);
					rng[1] = atof(szmax);
				}

				const char* szcol = tag.AttributeValue("colorMap", true);
				if (szcol) colMap = szcol;

				++tag;
			}
			else if (tag == "object")
			{
				GLSceneObject* po = new GLSceneObject;

				XMLAtt* acol = tag.AttributePtr("color");
				if (acol)
				{
					double v[3];
					acol->value(v, 3);
					po->SetColor(GLColor::FromRGBf(v[0], v[1], v[2]));
				}

				XMLAtt* apos = tag.AttributePtr("position");
				if (apos)
				{
					double v[3];
					apos->value(v, 3);
					po->SetPosition(vec3d(v[0], v[1], v[2]));
				}

				XMLAtt* arot = tag.AttributePtr("rotation");
				if (arot)
				{
					double v[3];
					arot->value(v, 3);
					vec3d r(v[0], v[1], v[2]);
					quatd q(r * DEG2RAD);
					po->SetRotation(q);
				}

				const char* szfile = tag.AttributeValue("file");

				QString fileName = m_appFolder + QString("/%1").arg(szfile);

				if (po->LoadFromFile(fileName.toStdString()) == false) delete po;
				else scene->AddSceneObject(po);

				++tag;
			}
			else
			{
				tag.skip();
				++tag;
			}
		} while (!tag.isend());
	}

	if (!mapName.empty()) scene->SetDataSourceName(mapName);
	if (colMap.empty() == false) scene->SetColorMap(colMap);
	if (brange) scene->SetDataRange(rng[0], rng[1]);

	double d2r = PI / 180.0;
	vec3d R = vec3d(rot[0], rot[1], rot[2])*d2r;
	if (R.norm2() > 0)
	{
		GLCamera& cam = scene->GetCamera();
		cam.SetOrientation(quatd(R));
		cam.Update(true);
	}
	
	app->AddModelDataSource(scene);

	CGLManagedSceneView* pgl = new CGLManagedSceneView(scene);
	ui->AddRepaintChild(pgl);
	pgl->setMinimumSize(QSize(size[0], size[1]));
	pgl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	GLViewSettings& vs = pgl->GetViewSettings();
	vs.m_col1 = GLColor::FromRGBf(bgc[0], bgc[1], bgc[2]);
	vs.m_col2 = GLColor::FromRGBf(bgc[0], bgc[1], bgc[2]);
	playout->addWidget(pgl);

	scene->SetGLView(pgl);
}

void FEBioAppUIBuilder::parseOutput(XMLTag& tag, QBoxLayout* playout)
{
	const char* szid = tag.AttributeValue("id", true);

	QPlainTextEdit* w = new QPlainTextEdit;
	if (szid) w->setObjectName(szid);

	w->setReadOnly(true);
	w->setFont(QFont("Courier", 11));
	w->setWordWrapMode(QTextOption::NoWrap);

	playout->addWidget(w);

	ui->SetOutputWidget(w);
}
