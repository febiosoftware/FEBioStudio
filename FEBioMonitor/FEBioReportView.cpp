/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2024 University of Utah, The Trustees of Columbia University in
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
#include "FEBioReportView.h"
#include "FEBioReportDoc.h"
#include <QTextEdit>
#include <QBoxLayout>
#include "../FEBioStudio/MainWindow.h"
#include <QPixmap>
#include <QPainter>

class HTMLComposer
{
	enum HtmlAlign {
		Align_Default,
		Align_Left,
		Align_Right
	};

public:
	HTMLComposer() {}

	QString text() const { 

		QString s = "<html><head><style>table, th, td { border: 1px solid black; border-collapse: collapse; } th, td { padding: 5px; }</style></head>";
		s += "<body>" + m_txt + "</body>";
		return s; 
	}

	void heading1(const QString& s) { append(wrap("h1", s)); }
	void heading2(const QString& s) { append(wrap("h2", s)); }
	void paragraph(const QString& s) { append(wrap("p", s)); }
	void table(const QStringList& v)
	{
		QString s;
		for (auto d : v)
		{
			s += wrap("td", d);
		}
		s = wrap("tr", s);
		s = wrap("table", s);
		append(s);
	}

	void div_start() { open("div"); }
	void div_end() { close("div"); }

	void table_start() { open("table"); }
	void table_end() { close("table"); }
	void table_row(const QStringList& v)
	{
		int n = 0;
		open("tr");
		int align = Align_Default;
		for (auto& d : v)
		{
			if (d == align_right) align = Align_Right;
			else
			{
				switch (align)
				{
				case Align_Default: open("td"); break;
				case Align_Left   : open("td align=left"); break;
				case Align_Right  : open("td align=right"); break;
				}
				append(d);
				close("td");
				align = Align_Default;
			}
			n++;
		}
		close("tr");
	}

	void image(const QString& name)
	{
		append(QString("<img src=\"%1\">").arg(name));
	}

public:
	static QString align_left;
	static QString align_right;

private:
	QString wrap(const QString& tag, const QString& val) { return QString("<%1>%2</%1>").arg(tag).arg(val); }
	void append(const QString& s) { m_txt += s; }
	void open(const QString& s)
	{
		m_txt += QString("<%1>").arg(s);
	}
	void close(const QString& s)
	{
		m_txt += QString("</%1>").arg(s);
	}

private:
	QString m_txt;
};

QString HTMLComposer::align_left = "__align_left";
QString HTMLComposer::align_right = "__align_right";

class PiechartBuilder
{
public:
	PiechartBuilder(int w, int h) : pix(w, h) 
	{
		pix.fill(QColor::fromRgb(0,0,0,0));
		m_start = 0;
	}
	
	void addSlice(double span, const QBrush& b, const QString& label = "")
	{
		QPainter p(&pix);
		p.setRenderHint(QPainter::RenderHint::Antialiasing);
		QRect rt = pix.rect();
		int W = rt.width();
		int H = rt.height();
		p.setPen(Qt::NoPen);
		p.setBrush(b); 
		int n0 = (int)(5760.0 * m_start);
		int n1 = (int)(5760.0 * span);
		p.drawPie(rt, n0, n1);

		if (!label.isEmpty() && (span > 2.0/360.0))
		{
			double a = 2*PI*(m_start + span * 0.5);
			double ca = cos(a);
			double sa = sin(a);
			double x = W / 2 + W * 0.35 * ca;
			double y = H / 2 - H * 0.35 * sa;
			QFont f = p.font();
			f.setPixelSize(10);
			p.setFont(f);
			p.setPen(QPen(Qt::black));
			p.drawText(x, y, label);
		}

		m_start += span;
	}

	QPixmap& pixmap() { return pix; }

private:
	QPixmap pix;
	double	m_start;
};

class CFEBioReportView::Ui
{
public:
	QTextEdit* txt;

public:
	void setup(CFEBioReportView* view)
	{
		QVBoxLayout* l = new QVBoxLayout;
		txt = new QTextEdit;
		txt->setReadOnly(true);
		l->addWidget(txt);
		txt->setHtml("<h1>Report</h1><p>All is good!</p>");
		view->setLayout(l);
	}
};

CFEBioReportView::CFEBioReportView(CMainWindow* wnd) : QWidget(wnd), ui(new CFEBioReportView::Ui)
{
	ui->setup(this);
}

QStringList composeRow(const QString& label, double sec, double f, const QString& description)
{
	QStringList sl;
	sl << QString("<b>%1</b>").arg(label);
	sl << HTMLComposer::align_right << QString("%1 sec.").arg(sec, 10, 'f', 2);
	sl << HTMLComposer::align_right << QString("%1%").arg(f * 100.0, 0, 'f', 2);
	sl << description;
	return sl;
}

struct TimingEntry
{
	QString label;
	double sec;
	double f;
	QString desc;
	QColor col;
};

void CFEBioReportView::setDocument(CFEBioReportDoc* doc)
{
	ui->txt->clear();
	CFEBioJob* job = doc->getJob();
	QTextEdit* txt = ui->txt;
	QTextDocument* t = txt->document(); assert(t);
	if (t == nullptr) return;

	if (job == nullptr)
	{
		QTextCursor cursor(t);
		QTextImageFormat imf;
		imf.setName(":/icons/error_small.png");
		cursor.insertImage(imf, QTextFrameFormat::Position::FloatLeft);
		cursor.insertText("No data available.");
	}
	else
	{
		TimingInfo ti = job->m_timingInfo;

		std::vector<TimingEntry> entries;
		entries.push_back({ "input"    , ti.input_time        , ti.input_time         / ti.total_time, "Time to process the input file" , QColor::fromRgb(200, 224, 16) });
		entries.push_back({ "init"     , ti.init_time         , ti.init_time          / ti.total_time, "Time to initialize all model data" , QColor::fromRgb(220, 120, 60)});
		entries.push_back({ "output"   , ti.io_time           , ti.io_time            / ti.total_time, "Time to write output files (plot, dmp, data)" , QColor::fromRgb(255, 255, 0)});
		entries.push_back({ "refs"     , ti.total_reform      , ti.total_reform       / ti.total_time, "Time spent reforming the stiffness matrix" , QColor::fromRgb(175, 0, 230) });
		entries.push_back({ "stiff"    , ti.total_stiff       , ti.total_stiff        / ti.total_time, "Time spent evaluating the stiffness matrix" , QColor::fromRgb(0, 201, 87) });
		entries.push_back({ "rhs"      , ti.total_rhs         , ti.total_rhs          / ti.total_time, "Time spent evaluating the residual (i.e. all forces, including internal and external)" , QColor::fromRgb(255, 127, 80) });
		entries.push_back({ "update"   , ti.total_update      , ti.total_update       / ti.total_time, "Time spent updating model (i.e. applying increments to solution and reevaluating model state)" , QColor::fromRgb(0, 165, 103) });
		entries.push_back({ "QN"       , ti.total_qn          , ti.total_qn           / ti.total_time, "Time evaluating the Quasi-Newton updates" , QColor::fromRgb(200, 200, 250) });
		entries.push_back({ "factor"   , ti.total_ls_factor   , ti.total_ls_factor    / ti.total_time, "Time spent factorizing stiffness matrix" , QColor::fromRgb(32, 165, 218) });
		entries.push_back({ "backsolve", ti.total_ls_backsolve, ti.total_ls_backsolve / ti.total_time, "Time spent doing backsolves" , QColor::fromRgb(32, 100, 158) });
		entries.push_back({ "serialize", ti.total_serialize   , ti.total_serialize    / ti.total_time, "Time spent serializing model data" , QColor::fromRgb(218, 165, 32) });
		entries.push_back({ "callback" , ti.total_callback    , ti.total_callback     / ti.total_time, "Time spent in callback routines (excl. output)" , QColor::fromRgb(208, 32, 32) });
		entries.push_back({ "other"    , ti.total_other       , ti.total_other        / ti.total_time, "Time spent outside of timed routines" , QColor::fromRgb(255, 0, 255) });
		std::sort(entries.begin(), entries.end(), [](TimingEntry& a, TimingEntry& b) { return a.sec > b.sec; });

		HTMLComposer html;
		html.heading1("FEBio Job Report");
		html.heading2("Files");
		html.paragraph("Files used in the job.");
		html.table_start();
		html.table_row({ "<b>input</b>", QString::fromStdString(job->GetFEBFileName()) });
		html.table_row({ "<b>log</b>"  , QString::fromStdString(job->GetLogFileName()) });
		html.table_row({ "<b>plot</b>" , QString::fromStdString(job->GetPlotFileName()) });
		html.table_end();
		html.heading2("Stats");
		html.paragraph("Overall statistics.");
		html.table_start();
		ModelStats stats = job->m_stats;
		html.table_row({ "<b>time steps</b>"   , HTMLComposer::align_right, QString::number(stats.ntimeSteps   ), "Total number of time steps completed."});
		html.table_row({ "<b>total iters</b>"  , HTMLComposer::align_right, QString::number(stats.ntotalIters  ), "Total number of Quasi-Newton iterations."});
		html.table_row({ "<b>total RHS</b>"    , HTMLComposer::align_right, QString::number(stats.ntotalRHS    ), "Total number of residual evaluations."});
		html.table_row({ "<b>total reforms</b>", HTMLComposer::align_right, QString::number(stats.ntotalReforms), "Total number of stiffness matrix reformations."});
		html.table_end();
		html.heading2("Timings");
		html.paragraph("Breakdown of total runtime.");
		html.table_start();
		html.table_row(composeRow("Total time", ti.total_time  , 1.0, "Total time to run the job"));
		for (auto& e : entries)
			html.table_row(composeRow(e.label, e.sec, e.f, e.desc));
		html.table_end();

		std::sort(entries.begin(), entries.end(), [](TimingEntry& a, TimingEntry& b) { return a.sec < b.sec; });
		PiechartBuilder pie(400, 400);
		double tot = ti.total_time;
		for (auto& e : entries)
			pie.addSlice(e.f, e.col, e.label);

		t->addResource(QTextDocument::ImageResource, QUrl("piechart"), pie.pixmap());

		html.image("piechart");

		txt->setHtml(html.text());
	}
}
