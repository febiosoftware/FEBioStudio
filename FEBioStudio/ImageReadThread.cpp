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

#include <QMessageBox>
#include <QLabel>
#include <QProgressBar>
#include <QBoxLayout>
#include <QCloseEvent>
#include "ImageReadThread.h"
#include <PostLib/ImageModel.h>

CImageReadThread::CImageReadThread(Post::CImageModel* imgModel)
    : m_imgModel(imgModel)
{

}
    
void CImageReadThread::run()
{
    m_success = true;

    try
    {
        if(!m_imgModel->Load())
        {
            m_success = false;
            m_error = "Failed importing image data.";
        }
    }
    catch(std::runtime_error& e)
    {
        m_success = false;
        m_error = e.what();
    }
}


class Ui::CDlgStartImageThread
{
public:
    CImageReadThread*	m_thread;

public:
	QLabel*			m_task;
	QProgressBar*	m_progress;

public:
    void setupUI(QDialog* parent, CImageReadThread* thread)
    {
        m_thread = thread;

        QVBoxLayout* layout = new QVBoxLayout;

        layout->addWidget(m_task = new QLabel("Reading Image Data..."));
        
        m_progress = new QProgressBar();
        m_progress->setMinimum(0);
        m_progress->setMaximum(0);
        layout->addWidget(m_progress);

        parent->setLayout(layout);
    }

};


CDlgStartImageThread::CDlgStartImageThread(QWidget* parent, CImageReadThread* thread)
    : QDialog(parent), ui(new Ui::CDlgStartImageThread)
{
    ui->setupUI(this, thread);

    connect(thread, &QThread::finished, this, &CDlgStartImageThread::threadFinished);
    thread->start();
}

void CDlgStartImageThread::threadFinished()
{
    ui->m_thread->deleteLater();

    if(!ui->m_thread->getSuccess())
    {
        QMessageBox::critical(this, "FEBio Studio", ui->m_thread->getError());

        reject();
        return;
    }

    accept();
}

void CDlgStartImageThread::closeEvent(QCloseEvent* ev)
{
    QMessageBox::critical(this, "FEBio Studio", "This operation cannot be canceled.");

    ev->ignore();
}