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
#include <QDialogButtonBox>
#include <QCloseEvent>
#include "ImageThread.h"
#include <PostLib/ImageModel.h>
#include <ImageLib/ImageFilter.h>

CImageThread::CImageThread(Post::CImageModel* imgModel)
    : m_imgModel(imgModel), m_error("")
{

}

//--------------------------------------------------------------------

CImageReadThread::CImageReadThread(Post::CImageModel* imgModel)
    : CImageThread(imgModel)
{

}
    
void CImageReadThread::run()
{
    m_success = true;

    emit newStatus("Reading Image Data...");

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

//--------------------------------------------------------------------

CImageFilterThread::CImageFilterThread(Post::CImageModel* imgModel)
    : CImageThread(imgModel), m_canceled(false)
{

}
    
void CImageFilterThread::run()
{
    m_success = true;

    try
    {
        for(int index = 0; index < m_imgModel->ImageFilters(); index++)
        {
            emit newStatus(QString("Applying %1...").arg(m_imgModel->GetImageFilter(index)->GetName().c_str()));

            if(m_canceled)
            {
                break;
            }

            m_imgModel->GetImageFilter(index)->ApplyFilter();
        }

        if(m_canceled)
        {
            m_success = false;
            return;
        }
    }
    catch(std::runtime_error& e)
    {
        m_success = false;
        m_error = e.what();
    }
}

void CImageFilterThread::cancel()
{
    m_canceled = true;
}


class Ui::CDlgStartImageThread
{
public:
    CImageThread*	m_thread;
    bool            m_cancelable;

public:
	QLabel*			m_status;
	QProgressBar*	m_progress;
    QDialogButtonBox* m_box;

public:
    void setupUI(QDialog* parent, CImageThread* thread)
    {
        m_thread = thread;
        m_cancelable = dynamic_cast<CImageFilterThread*>(thread) != nullptr;

        QVBoxLayout* layout = new QVBoxLayout;

        layout->addWidget(m_status = new QLabel(""));
        
        m_progress = new QProgressBar();
        m_progress->setMinimum(0);
        m_progress->setMaximum(0);
        layout->addWidget(m_progress);

        if(m_cancelable)
        {
            layout->addWidget(m_box = new QDialogButtonBox(QDialogButtonBox::Cancel));
        }

        parent->setLayout(layout);
    }

};


CDlgStartImageThread::CDlgStartImageThread(CImageThread* thread, QWidget* parent)
    : QDialog(parent), ui(new Ui::CDlgStartImageThread)
{
    ui->setupUI(this, thread);

    connect(thread, &QThread::finished, this, &CDlgStartImageThread::threadFinished);
    connect(thread, &CImageThread::newStatus, this, &CDlgStartImageThread::on_status_changed);

    if(ui->m_cancelable)
    {
        connect(ui->m_box, &QDialogButtonBox::rejected, this, &CDlgStartImageThread::on_canceled);
    }


    thread->start();
}

void CDlgStartImageThread::threadFinished()
{
    ui->m_thread->deleteLater();

    if(!ui->m_thread->getSuccess())
    {
        if(!QString(ui->m_thread->getError()).isEmpty())
        {
            QMessageBox::critical(this, "FEBio Studio", ui->m_thread->getError());
        }

        reject();
        return;
    }

    accept();
}

void CDlgStartImageThread::on_canceled()
{
    dynamic_cast<CImageFilterThread*>(ui->m_thread)->cancel();

    ui->m_status->setText("Canceling...");
}

void CDlgStartImageThread::on_status_changed(QString status)
{
    ui->m_status->setText(status);
}

void CDlgStartImageThread::closeEvent(QCloseEvent* ev)
{
    if(ui->m_cancelable)
    {
        on_canceled();
    }
    else
    {
        QMessageBox::critical(this, "FEBio Studio", "This operation cannot be canceled.");
    }

    ev->ignore();    
}