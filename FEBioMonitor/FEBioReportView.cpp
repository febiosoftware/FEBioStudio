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

		QString s = "<html><head><style>table, th, td { border: 1px solid black; border-collapse: collapse; } th, td { padding: 5px; } th { background-color:#808080;}</style></head>";
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
	void table_headings(const QStringList& v)
	{
		open("tr");
		for (auto& d : v)
		{
			open("th");
			append(d);
			close("th");
		}
		close("tr");
	}
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


class BarchartBuilder
{
public:
	BarchartBuilder(int w, int h) : pix(w, h)
	{
		pix.fill(Qt::white);
	}

	void addBar(double val, QColor c)
	{
		m_data.push_back(std::make_pair(val, c));
	}

	QPixmap& pixmap() { Build(); return pix; }

private:
	void Build()
	{
		const int leftMargin = 50;
		const int rightMargin = 10;
		const int topMargin = 10;
		const int bottomMargin = 30;

		QPainter p(&pix);
		p.setRenderHint(QPainter::RenderHint::Antialiasing);
		QRect rt = pix.rect();
		int W = rt.width() - (leftMargin + rightMargin);
		int H = rt.height() - (topMargin + bottomMargin);
		p.setPen(Qt::NoPen);

		// get the maximum value in the data
		double maxVal = 0.0;
		for (auto it : m_data) if (it.first > maxVal) maxVal = it.first;

		// draw the horizontal grid lines
		int steps = 5;
		if (maxVal >= 100) steps = 10;
		if (maxVal <= 5) { maxVal = 5; steps = 1; }

		int maxY = steps * ceil(maxVal / steps);
		if (maxY == 0) { maxY = 1; steps = 1; }

		for (int y = 0; y <= maxY; y += steps)
		{
			int y0 = topMargin + H - (int)(H * y / maxY);
			p.setPen(QPen(Qt::lightGray, 1.0));
			p.drawLine(leftMargin, y0, leftMargin + W, y0); // horizontal lines
			p.setPen(Qt::black);
			p.drawText(leftMargin - 30, y0 + 5, QString::number(y)); // y-axis labels
		}

		int N = m_data.size();
		for (int i = 0; i < N; ++i)
		{
			auto it = m_data[i];
			double v = it.first;
			if (v > 0.0)
			{
				int x0 = leftMargin + i * W / N;
				int x1 = leftMargin + (i + 1) * W / N - 2;
				int y0 = topMargin + H;
				int y1 = topMargin + H - (int)(H * v / maxY);
				p.setPen(Qt::PenStyle::NoPen);
				p.setBrush(it.second);
				p.drawRect(x0, y0, x1 - x0, y1 - y0);
			}
		}

		p.setPen(QPen(Qt::black, 1.0));
		p.drawLine(leftMargin, topMargin + H, leftMargin + W, topMargin + H); // x-axis
		p.drawLine(leftMargin, topMargin, leftMargin, topMargin + H); // y-axis
	}

private:
	QPixmap pix;
	std::vector<std::pair<double, QColor>> m_data;
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
		view->setLayout(l);
	}
};

CFEBioReportView::CFEBioReportView(CMainWindow* wnd) : CDocumentView(wnd), ui(new CFEBioReportView::Ui)
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

QStringList composeRow(const std::vector<int>& a)
{
	QStringList sl;
	for (int v : a) sl.push_back(QString::number(v));
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

void CFEBioReportView::setDocument(CDocument* doc)
{
	ui->txt->clear();

	QTextEdit* txt = ui->txt;
	QTextDocument* t = txt->document(); assert(t);
	if (t == nullptr) return;

	CFEBioReportDoc* report = dynamic_cast<CFEBioReportDoc*>(doc);
	if ((report == nullptr) || !report->IsValid())
	{
		QTextCursor cursor(t);
		QTextImageFormat imf;
		imf.setName(":/icons/error_small.png");
		cursor.insertImage(imf, QTextFrameFormat::Position::FloatLeft);
		cursor.insertText("No data available.");
	}
	else
	{
		HTMLComposer html;

		html.heading1("FEBio Job Report");
		html.heading2("Files");
		html.paragraph("Files used in the job.");
		html.table_start();
		html.table_row({ "<b>input</b>", report->m_febFile });
		html.table_row({ "<b>log</b>"  , report->m_logFile });
		html.table_row({ "<b>plot</b>" , report->m_pltFile });
		html.table_end();

		// Model Stats
		html.heading2("Stats");
		html.paragraph("Overall statistics.");
		html.table_start();
		ModelStats stats = report->m_modelStats;
		html.table_row({ "<b>timesteps</b>"   , HTMLComposer::align_right, QString::number(stats.ntimeSteps   ), "Total number of time steps completed."});
		html.table_row({ "<b>total iters</b>"  , HTMLComposer::align_right, QString::number(stats.ntotalIters  ), "Total number of Quasi-Newton iterations."});
		html.table_row({ "<b>total RHS</b>"    , HTMLComposer::align_right, QString::number(stats.ntotalRHS    ), "Total number of residual evaluations."});
		html.table_row({ "<b>total reforms</b>", HTMLComposer::align_right, QString::number(stats.ntotalReforms), "Total number of stiffness matrix reformations."});
		html.table_end();

		// Step stats
		if (!report->m_stepStats.empty())
		{
			html.paragraph("Breakdown of stats per step.");
			html.table_start();
			html.table_headings({ "step", "timesteps", "total iters", "total RHS", "total reforms" });
			for (int i = 0; i < report->m_stepStats.size(); ++i)
			{
				ModelStats& s = report->m_stepStats[i];
				html.table_row(composeRow({ i + 1, s.ntimeSteps, s.ntotalIters, s.ntotalRHS, s.ntotalReforms }));
			}
			html.table_end();
		}

		// iteration stats
		std::vector<TimeStepStats> iters = report->m_timestepStats;
		if (!iters.empty())
		{
			BarchartBuilder bar(800, 400);

			for (const TimeStepStats& s : iters)
			{
				bar.addBar(s.nrhs, (s.status == 0 ? Qt::red : Qt::darkGreen));
			}

			html.paragraph("Right hand side evaluations per time step.");
			html.paragraph(""); // Added to make sure there is a line break before the bar chart
			t->addResource(QTextDocument::ImageResource, QUrl("barchart"), bar.pixmap());
			html.image("barchart");
		}

		// Timings
		TimingInfo ti = report->m_timingInfo;
		double tot = ti.total_time;
		if (tot == 0) tot = 1.0; // avoid division by zero
		std::vector<TimingEntry> entries;
		if (ti.input_time         > 0) entries.push_back({ "input"    , ti.input_time        , ti.input_time         / tot, "Time to process the input file" , QColor::fromRgb(200, 224, 16) });
		if (ti.init_time          > 0) entries.push_back({ "init"     , ti.init_time         , ti.init_time          / tot, "Time to initialize all model data" , QColor::fromRgb(220, 120, 60) });
		if (ti.io_time            > 0) entries.push_back({ "output"   , ti.io_time           , ti.io_time            / tot, "Time to write output files (plot, dmp, data)" , QColor::fromRgb(255, 255, 0) });
		if (ti.total_reform       > 0) entries.push_back({ "reforms"  , ti.total_reform      , ti.total_reform       / tot, "Time spent reforming the stiffness matrix" , QColor::fromRgb(175, 0, 230) });
		if (ti.total_stiff        > 0) entries.push_back({ "stiff"    , ti.total_stiff       , ti.total_stiff        / tot, "Time spent evaluating the stiffness matrix" , QColor::fromRgb(0, 201, 87) });
		if (ti.total_rhs          > 0) entries.push_back({ "rhs"      , ti.total_rhs         , ti.total_rhs          / tot, "Time spent evaluating the residual (i.e. all forces, including internal and external)" , QColor::fromRgb(255, 127, 80) });
		if (ti.total_update       > 0) entries.push_back({ "update"   , ti.total_update      , ti.total_update       / tot, "Time spent updating model (i.e. applying increments to solution and reevaluating model state)" , QColor::fromRgb(0, 165, 103) });
		if (ti.total_qn           > 0) entries.push_back({ "QN"       , ti.total_qn          , ti.total_qn           / tot, "Time evaluating the Quasi-Newton updates" , QColor::fromRgb(200, 200, 250) });
		if (ti.total_ls_factor    > 0) entries.push_back({ "factor"   , ti.total_ls_factor   , ti.total_ls_factor    / tot, "Time spent factorizing stiffness matrix" , QColor::fromRgb(32, 165, 218) });
		if (ti.total_ls_backsolve > 0) entries.push_back({ "backsolve", ti.total_ls_backsolve, ti.total_ls_backsolve / tot, "Time spent doing backsolves" , QColor::fromRgb(32, 100, 158) });
		if (ti.total_serialize    > 0) entries.push_back({ "serialize", ti.total_serialize   , ti.total_serialize    / tot, "Time spent serializing model data" , QColor::fromRgb(218, 165, 32) });
		if (ti.total_callback     > 0) entries.push_back({ "callback" , ti.total_callback    , ti.total_callback     / tot, "Time spent in callback routines (excl. output)" , QColor::fromRgb(208, 32, 32) });
		if (ti.total_other        > 0) entries.push_back({ "other"    , ti.total_other       , ti.total_other        / tot, "Time spent outside of timed routines" , QColor::fromRgb(255, 0, 255) });

		if (!entries.empty())
		{
			std::sort(entries.begin(), entries.end(), [](TimingEntry& a, TimingEntry& b) { return a.sec > b.sec; });

			html.heading2("Timings");
			html.paragraph("Breakdown of total runtime.");
			html.table_start();
			html.table_row(composeRow("Total time", ti.total_time, 1.0, "Total time to run the job"));
			for (auto& e : entries)
				html.table_row(composeRow(e.label, e.sec, e.f, e.desc));
			html.table_end();

			std::sort(entries.begin(), entries.end(), [](TimingEntry& a, TimingEntry& b) { return a.sec < b.sec; });
			PiechartBuilder pie(400, 400);
			for (auto& e : entries)
				pie.addSlice(e.f, e.col, e.label);

			t->addResource(QTextDocument::ImageResource, QUrl("piechart"), pie.pixmap());

			html.image("piechart");
		}

		txt->setHtml(html.text());
	}
}
