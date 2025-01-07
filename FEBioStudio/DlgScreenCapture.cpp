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

#include <QBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QPushButton>
#include <QClipboard>
#include <QGuiApplication>
#include "DlgScreenCapture.h"
#include "MainWindow.h"

CDlgScreenCapture::CDlgScreenCapture(CMainWindow* wnd) : m_wnd(wnd), QDialog(wnd)
{
    QVBoxLayout* layout = new QVBoxLayout;

    m_scene = new QGraphicsScene;
    m_view = new QGraphicsView;
    m_view->setScene(m_scene);
	m_imgItem = nullptr;

    layout->addWidget(m_view);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();

    QPushButton* saveButton = new QPushButton("Save Image...");
    buttonLayout->addWidget(saveButton);

    QPushButton* copyButton = new QPushButton("Copy to Clipboard");
    buttonLayout->addWidget(copyButton);

    layout->addLayout(buttonLayout);

    setLayout(layout);

    connect(saveButton, &QPushButton::clicked, this, &CDlgScreenCapture::on_saveButton_clicked);
    connect(copyButton, &QPushButton::clicked, this, &CDlgScreenCapture::on_copyButton_clicked);
}

void CDlgScreenCapture::SetImage(const QImage& img)
{
	m_img = img;
	if (m_imgItem) delete m_imgItem;
	m_imgItem = m_scene->addPixmap(QPixmap::fromImage(img));
}

void CDlgScreenCapture::on_saveButton_clicked()
{
    m_wnd->SaveImage(m_img);
}

void CDlgScreenCapture::on_copyButton_clicked()
{
    QClipboard *clipboard = QGuiApplication::clipboard();

    clipboard->setImage(m_img);
}