/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
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
#include "stdafx.h"
#include "DlgRemoteProgress.h"
#include "RemoteJob.h"
#include <QBoxLayout>
#include <QProgressBar>
#include <QLabel>

CDlgRemoteProgress::CDlgRemoteProgress(CRemoteJob* job, QWidget* parent, bool send, const QString& localFile) : QDialog(parent), m_job(job)
{
	assert(job);
	setMinimumWidth(400);
	QVBoxLayout* l = new QVBoxLayout;
	QLabel* task = new QLabel("");
	l->addWidget(task);
	l->addWidget(m_prg = new QProgressBar);
	m_prg->setRange(0, 100);
	setLayout(l);

	QObject::connect(m_job, &CRemoteJob::fileTransferFinished, this, &QDialog::accept);
	QObject::connect(m_job, &CRemoteJob::progressUpdate, m_prg, &QProgressBar::setValue);

	if (send)
	{
		task->setText("Sending file to server ...");
		m_job->SendLocalFile();
	}
	else if (!localFile.isEmpty())
	{
		task->setText("Fetching remote file ...");
		m_job->GetRemoteFile(localFile);
	}
	else
	{
		task->setText("Fetching remote files ...");
		m_job->GetRemoteFiles();
	}
}