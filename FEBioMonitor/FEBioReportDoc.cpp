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
#include "FEBioReportDoc.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

CFEBioReportDoc::CFEBioReportDoc(CMainWindow* wnd) : CDocument(wnd)
{
	m_bValid = false;
	SetDocTitle("Report");
	SetIcon(":/icons/report.png");
}

void CFEBioReportDoc::setJob(CFEBioJob* job)
{
	m_jobName = QString::fromStdString(job->GetName());
	m_febFile = QString::fromStdString(job->GetFEBFileName());
	m_logFile = QString::fromStdString(job->GetLogFileName());
	m_pltFile = QString::fromStdString(job->GetPlotFileName());

	m_report = QString::fromStdString(job->m_jobReport);

	m_timingInfo = job->m_timingInfo;
	m_modelStats = job->m_modelStats;
	m_stepStats  = job->m_stepStats;
	m_timestepStats = job->m_timestepStats;

	QString title = QString("Report [%1]").arg(m_jobName);
	SetDocTitle(title.toStdString());

	m_bValid = true;
}

bool CFEBioReportDoc::SaveDocument()
{
	return false;
}

bool extractString(const QString& str, const QString& key, QString& out, QChar sep = ':')
{
	if (str.contains(key))
	{
		int start = str.indexOf(sep) + 1;
		int end = str.indexOf('\n', start);
		if (end < 0) end = str.length();
		out = str.mid(start, end - start).trimmed();
		return true;
	}
	return false;
}

bool extractInt(const QString& str, const QString& key, int& out, QChar sep = ':')
{
	QString tmp;
	if (extractString(str, key, tmp, sep))
	{
		out = tmp.toInt();
		return true;
	}
	return false;
}

bool extractFloat(const QString& str, const QString& key, double& out, QChar sep = ':')
{
	QString tmp;
	if (extractString(str, key, tmp, sep))
	{
		out = tmp.toDouble();
		return true;
	}
	return false;
}

double extractSeconds(const QString& str)
{
	int lpos = str.indexOf('(');
	if (lpos >= 0)
	{
		int rpos = str.indexOf("sec", lpos + 1);
		QString t = str.mid(lpos + 1, rpos - lpos - 1).trimmed();
		return t.toDouble();
	}
	return 0.0;
}

bool CFEBioReportDoc::LoadFromLogFile(const QString& logFile)
{
	QFile file(logFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "Could not open file:" << file.errorString();
		return false;
	}

	QFileInfo fi(logFile);

	m_jobName = fi.fileName();
	m_logFile.clear();
	m_febFile.clear();
	m_pltFile.clear();

	int iters = 0;
	int rhsEvals = 0;
	int reforms = 0;
	m_timestepStats.clear();

	bool processInfoBox = false;

	QString report;

	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine();

		if (m_febFile.isEmpty())
		{
			extractString(line, "Input file", m_febFile);
		}
		if (m_logFile.isEmpty())
		{
			extractString(line, "Log file", m_logFile);
		}
		if (m_pltFile.isEmpty())
		{
			extractString(line, "Plot file", m_pltFile);
		}

		bool match = false;

		if (processInfoBox)
		{
			if (line.contains("WARNING")) report += "Warning: ";
			else if (line.contains("ERROR")) report += "Error: ";
			else if (line.contains("*****"))
			{
				report += "\n";
				processInfoBox = false;
			}
			else
			{
				QString tmp = line;
				tmp.replace('*', ' ');
				report += tmp.trimmed();
			}

			match = true;
		}
		else if (line.contains("*****"))
		{
			processInfoBox = true;
			match = true;
		}

		if (!match) match = extractInt(line, "right hand side evaluations", rhsEvals, '=');
		if (!match) match = extractInt(line, "stiffness matrix reformations", reforms, '=');
		if (!match)
		{
			double time = 0.0;
			match = extractFloat(line, "converged at time", time);
			if (match)
			{
				m_timestepStats.push_back({ iters, rhsEvals, reforms, 1 });
				iters = 0;
				rhsEvals = 0;
				reforms = 0;
			}
		}
		if (!match) match = extractInt(line, "Number of time steps completed", m_modelStats.ntimeSteps);
		if (!match) match = extractInt(line, "Total number of equilibrium iterations", m_modelStats.ntotalIters);
		if (!match) match = extractInt(line, "Total number of right hand evaluations", m_modelStats.ntotalRHS);
		if (!match) match = extractInt(line, "Total number of stiffness reformations", m_modelStats.ntotalReforms);
		if (!match)
		{
			double time = 0.0;
			match = extractFloat(line, "failed to converge", time);
			if (match)
			{
				m_timestepStats.push_back({ iters, rhsEvals, reforms, 0 });
				iters = 0;
				rhsEvals = 0;
				reforms = 0;
			}
		}
		if (!match)
		{
			QString tmp;
			match = extractString(line, "Total elapsed time", tmp);
			if (match)
			{
				double sec = extractSeconds(tmp);
				m_timingInfo.total_time = sec;
			}
		}
		if (!match)
		{
			QString tmp;
			match = extractString(line, "IO-time", tmp);
			if (match)
			{
				double sec = extractSeconds(tmp);
				m_timingInfo.io_time = sec;
			}
		}
		if (!match)
		{
			QString tmp;
			match = extractString(line, "Input time", tmp);
			if (match)
			{
				double sec = extractSeconds(tmp);
				m_timingInfo.input_time = sec;
			}
		}
		if (!match)
		{
			QString tmp;
			match = extractString(line, "Initialization time", tmp);
			if (match)
			{
				double sec = extractSeconds(tmp);
				m_timingInfo.init_time = sec;
			}
		}
		if (!match)
		{
			QString tmp;
			match = extractString(line, "time in linear solver", tmp);
			if (match)
			{
				double sec = extractSeconds(tmp);
				m_timingInfo.total_ls_factor = sec;
			}
		}
		if (!match)
		{
			QString tmp;
			match = extractString(line, "reforming stiffness", tmp);
			if (match)
			{
				double sec = extractSeconds(tmp);
				m_timingInfo.total_reform = sec;
			}
		}
		if (!match)
		{
			QString tmp;
			match = extractString(line, "evaluating stiffness", tmp);
			if (match)
			{
				double sec = extractSeconds(tmp);
				m_timingInfo.total_stiff = sec;
			}
		}
		if (!match)
		{
			QString tmp;
			match = extractString(line, "evaluating residual", tmp);
			if (match)
			{
				double sec = extractSeconds(tmp);
				m_timingInfo.total_rhs = sec;
			}
		}
		if (!match)
		{
			QString tmp;
			match = extractString(line, "model update", tmp);
			if (match)
			{
				double sec = extractSeconds(tmp);
				m_timingInfo.total_update = sec;
			}
		}
		if (!match)
		{
			QString tmp;
			match = extractString(line, "QN updates", tmp);
			if (match)
			{
				double sec = extractSeconds(tmp);
				m_timingInfo.total_qn = sec;
			}
		}
		if (!match)
		{
			QString tmp = line.trimmed();
			bool ok = false;
			int n = tmp.toInt(&ok);
			if (ok) iters = n;
		}
	}

	// if the input file name wasn't set, then this is probably not an FEBio log file. 
	if (m_febFile.isEmpty()) return false;

	m_report = report;

	// calculate the remaining timing info
	double sum = 0;
	sum += m_timingInfo.io_time;
	sum += m_timingInfo.init_time;
	sum += m_timingInfo.input_time;
	sum += m_timingInfo.total_ls_factor;
	sum += m_timingInfo.total_reform;
	sum += m_timingInfo.total_stiff;
	sum += m_timingInfo.total_rhs;
	sum += m_timingInfo.total_update;
	sum += m_timingInfo.total_qn;
	m_timingInfo.total_other = m_timingInfo.total_time - sum;
	if (m_timingInfo.total_other < 0) m_timingInfo.total_other = 0.0;

	file.close(); // optional, as QFile destructor closes the file

	QString title = QString("Report [%1]").arg(m_jobName);
	SetDocTitle(title.toStdString());

	m_bValid = true;
	return true;
}
