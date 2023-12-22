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
#include <QPushButton>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QTabWidget>
#include "GLSceneView.h"
#include "InputWidgets.h"
#include "PlotWidget.h"
#include "MainWindow.h"
#include "FEBioAppDocument.h"
#include "FEBioAppView.h"
#include <FECore/FEParam.h>
#include <FECore/FECoreKernel.h>
#include <FECore/ElementDataRecord.h>
#include <FEBioLib/FEBioModel.h>

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

FEBioAppUIBuilder::FEBioAppUIBuilder() : ui(nullptr), app(nullptr)
{

}

QWidget* FEBioAppUIBuilder::error()
{
	if (ui) delete ui;
	ui = nullptr;
	return nullptr;
}

QWidget* FEBioAppUIBuilder::BuildUIFromFile(QString filePath, FEBioAppDocument* app)
{
	if (app == nullptr) return nullptr;
	this->app = app;

	XMLReader xml;
	std::string filename = filePath.toStdString();
	if (xml.Open(filename.c_str()) == false) return nullptr;

	XMLTag tag;
	if (xml.FindTag("febio_app", tag) == false) return nullptr;

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

	assert(tag.isleaf());

	// add the new model
	bool b = app->LoadModelFromFile(szfile);

	return b;
}

bool FEBioAppUIBuilder::parseGUI(XMLTag& tag)
{
	ui = new QWidget;
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
		else if (tag == "group"     ) parseGroup    (tag, playout);
		else if (tag == "vgroup"    ) parseVGroup   (tag, playout);
		else if (tag == "hgroup"    ) parseHGroup   (tag, playout);
		else if (tag == "tab_group" ) parseTabGroup (tag, playout);
		else if (tag == "stretch"   ) parseStretch  (tag, playout);
		else if (tag == "button"    ) parseButton   (tag, playout);
		else if (tag == "input"     ) parseInput    (tag, playout);
		else if (tag == "input_list") parseInputList(tag, playout);
		else if (tag == "graph"     ) parseGraph    (tag, playout);
		else if (tag == "plot3d"    ) parsePlot3d   (tag, playout);
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

	const char* sztitle = tag.AttributeValue("text", true);
	const char* szaction = tag.AttributeValue("onClick", true);

	QPushButton* pb = new QPushButton(sztitle);
	pb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	if (szaction)
	{
		if (strcmp(szaction, "fem.solve()") == 0)
		{
			QObject::connect(pb, SIGNAL(clicked()), app, SLOT(runModel()));
		}
	}

	//if (szaction)
	//{
	//	pb->setText(QString(sztitle));
	//	pb->setCode(szaction);
	//}

	playout->addWidget(pb);

//	QObject::connect(pb, SIGNAL(runCode(QString&)), m_dlg, SLOT(RunCode(QString&)));
}

void FEBioAppUIBuilder::parseGraph(XMLTag& tag, QBoxLayout* playout)
{
	const char* sztxt = tag.AttributeValue("text");

	// size of graph
	int size[2] = { 400, 400 };

	XMLReader& xml = *tag.m_preader;

	FECoreKernel& fecore = FECoreKernel::GetInstance();

	FEBioModel* fem = app->GetFEBioModel();

	int nplt = 0;

	CPlotWidget* plotWidget = new CPlotWidget(0);
	plotWidget->setTitle(QString(sztxt));
	plotWidget->showLegend(false);

	if (!tag.isleaf())
	{
		++tag;
		do
		{
			if (tag == "size")
			{
				int user_size[2];
				int nread = tag.value(user_size, 2);
				if (nread == 2)
				{
					if (user_size[0] > 100) size[0] = user_size[0];
					if (user_size[1] > 100) size[1] = user_size[1];
				}
				++tag;
			}
			else if (tag == "data")
			{
				nplt++;

				char szname[256] = { 0 };
				const char* sz = tag.AttributeValue("title", true);
				if (sz) strcpy(szname, sz);
				else sprintf(szname, "plot%d", nplt);

				ElementDataRecord* val_x = nullptr;
				ElementDataRecord* val_y = nullptr;

				const char* sztype = tag.AttributeValue("type", true);
				if (sztype == 0)
				{
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

					CPlotData* data = new CPlotData;

					CElementDataSource* src = new CElementDataSource(val_x, val_y, data);
					app->AddModelDataSource(src);

					plotWidget->addPlotData(data);

					++tag;
				}
				else if (strcmp(sztype, "static") == 0)
				{
//					CStaticDataSource* src = new CStaticDataSource;
					++tag;
					do
					{
						double p[2];
						tag.value(p, 2);

//						src->AddPoint(QPointF(p[0], p[1]));
						++tag;
					} while (!tag.isend());

//					plotWidget->AddData(src, szname);

					++tag;
				}
			}
			else xml.SkipTag(tag);
		} while (!tag.isend());
	}

	plotWidget->setMinimumSize(QSize(size[0], size[1]));

	playout->addWidget(plotWidget, 1);
//	m_dlg->AddGraph(pg);
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

			if (pi.type() == FE_PARAM_DOUBLE)
			{
				paramEdit->SetEditor(new CFloatInput());
			}
			if (pi.type() == FE_PARAM_BOOL)
			{
//				pw = new QCheckBox;
			}
			if (pi.type() == FE_PARAM_INT)
			{
//				pw = new QLineEdit;
			}

			QWidget* pw = paramEdit->GetEditor();
			if (pw)
			{
//				pw->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

				// add it to the row
				pf->addRow(QString::fromStdString(pi.param()->name()), pw);
//				m_dlg->AddInputParameter(pin);
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

void FEBioAppUIBuilder::parseGroup(XMLTag& tag, QBoxLayout* playout)
{
	const char* szalign = tag.AttributeValue("align");
	const char* szname = tag.AttributeValue("title", true);

	QGroupBox* pg = 0;
	if (szname) pg = new QGroupBox(szname);

	QBoxLayout* pl = 0;
	if (strcmp(szalign, "vertical") == 0) pl = new QVBoxLayout;
	else if (strcmp(szalign, "horizontal") == 0) pl = new QHBoxLayout;

	parseGUITags(tag, pl);

	if (pg)
	{
		pg->setLayout(pl);
		playout->addWidget(pg);
	}
	else playout->addLayout(pl);
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
//			CFloatSlider* slider = new CFloatSlider;
//			slider->setFloatRange(rng[0], rng[1], rng[2]);
//			pw = slider;
//			pi->SetWidget(slider);
		}
		else
		{
			CFloatInput* edit = new CFloatInput;
			pi->SetEditor(edit);
		}
	}
	if (param.type() == FE_PARAM_BOOL)
	{
		QCheckBox* pcheck = new QCheckBox;
		pi->SetEditor(pcheck);
	}
	if (param.type() == FE_PARAM_INT)
	{
//		QLineEdit* pedit = new QLineEdit; pedit->setValidator(new QIntValidator);
//		pi->SetWidget(pedit); pw = pedit;
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
	char sz[256] = { 0 };
	strcpy(sz, tag.AttributeValue("title"));

	// size of plot view
	int size[2] = { 400, 400 };

	bool brange = false;
	double rng[2];
	const char* szmap = "displacement";
	XMLReader& xml = *tag.m_preader;
	double bgc[3] = { 0.8, 0.8, 1.0 };
	double fgc[3] = { 0.0, 0.0, 0.0 };
	double w[3] = { 0, 0, 0 };
	double smoothingAngle = 60.0;
	int timeFormat = 0;
	int modelId = -1;
	if (!tag.isleaf())
	{
		++tag;
		do
		{
			if (tag == "size")
			{
				int user_size[2];
				int nread = tag.value(user_size, 2);
				if (nread == 2)
				{
					if (user_size[0] > 100) size[0] = user_size[0];
					if (user_size[1] > 100) size[1] = user_size[1];
				}
				++tag;
			}
			else if (tag == "model")
			{
//				const char* szmodel = tag.szvalue();
//				modelId = m_data->GetModelIndex(szmodel); assert(modelId >= 0);
//				if (modelId < 0) throw 1;
				++tag;
			}
			else if (tag == "bg_color")
			{
				tag.value(bgc, 3);
				++tag;
			}
			else if (tag == "fg_color")
			{
				tag.value(fgc, 3);
				++tag;
			}
			else if (tag == "map")
			{
				const char* sztype = tag.AttributeValue("type");
				if (sztype) szmap = sztype;

				if (tag.isleaf()) ++tag;
				else
				{
					++tag;
					do
					{
						if (tag == "range")
						{
							brange = true;
							tag.value(rng, 2);
						}
						++tag;
					} while (!tag.isend());
					++tag;
				}
			}
			else if (tag == "rotation")
			{
				tag.value(w, 3);
				++tag;
			}
			else if (tag == "smoothing_angle")
			{
				tag.value(smoothingAngle);
				++tag;
			}
			else if (tag == "time_format")
			{
				tag.value(timeFormat);
				++tag;
			}
			else
			{
				xml.SkipTag(tag);
				++tag;
			}
		} while (!tag.isend());
	}

	CGLSceneView* pgl = new CGLSceneView(app->GetMainWindow());
	pgl->setMinimumSize(QSize(size[0], size[1]));
	pgl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	playout->addWidget(pgl);

/*	pgl->SetTimeFormat(timeFormat);
	pgl->SetSmoothingAngle(smoothingAngle);	// must be set before SetFEModel is called
	pgl->SetFEModel(m_data, modelId);
	pgl->SetDataSource(szmap);
	pgl->SetRotation(w[0], w[1], w[2]);
	if (brange) pgl->SetDataRange(rng[0], rng[1]);
	pgl->SetBackgroundColor(bgc[0], bgc[1], bgc[2]);
	pgl->SetForegroundColor(fgc[0], fgc[1], fgc[2]);
	m_dlg->AddPlot3D(pgl);
*/
}
