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
#pragma once
#include <QDialog>
#include <QThread>

class CustomThread : public QThread
{
	Q_OBJECT

public:
	CustomThread();

	virtual bool hasProgress();

	virtual double progress();

	virtual const char* currentTask();

	virtual void stop();

signals:
	void resultReady(bool);
};

class CDlgStartThreadUI;

class CDlgStartThread : public QDialog
{
	Q_OBJECT

public:
	CDlgStartThread(QWidget* parent, CustomThread* thread);

	void closeEvent(QCloseEvent* ev) override;

	void accept();

	bool GetReturnCode();

	void setTask(const QString& taskString);

private slots:
	void threadFinished(bool b);
	void checkProgress();
	void cancel();

private:
	CDlgStartThreadUI*	ui;
};