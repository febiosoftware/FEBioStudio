/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2026 University of Utah, The Trustees of Columbia University in
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
#pragma once

#include <QString>
#include <QStringList>

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
	void heading3(const QString& s) { append(wrap("h3", s)); }
	void paragraph(const QString& s) { append(wrap("p", s)); }

	QString pre(const QString& s) { return wrap("pre", s); }
	QString italic(const QString& s) { return wrap("i", s); }
	QString bold(const QString& s) { return wrap("b", s); }

	QString color(const QString& s, const QString& c) { return QString("<font color=\"%1\">%2</font>").arg(c).arg(s); }
	QString bg_color(const QString& s, const QString& c) { return QString("<span style=\"background-color:%1\">%2</span>").arg(c).arg(s); }

	// tables
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
				case Align_Left: open("td align=left"); break;
				case Align_Right: open("td align=right"); break;
				}
				append(d);
				close("td");
				align = Align_Default;
			}
			n++;
		}
		close("tr");
	}

	// unordered list
	void unordered_list_start() { open("ul"); }
	void unordered_list_end() { close("ul"); }
	void list_item(const QString& s) { append(wrap("li", s)); }

	// image
	void image(const QString& name)
	{
		append(QString("<div><img src=\"%1\"></div>").arg(name));
	}

	// link
	QString link(const QString& url, const QString& name = "")
	{
		return QString("<a href=\"%1\">%2</a>").arg(url).arg(name.isEmpty() ? url : name);
	}

	// break
	void break_line() { append("<br>"); }

	// divs
	void div_start() { open("div"); }
	void div_end() { close("div"); }

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
