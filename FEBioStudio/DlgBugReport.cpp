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

#include "DlgBugReport.h"
#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QUrl>
#include <QDesktopServices>

class Ui::CDlgBugReport
{
public:
    QComboBox* software;
    QComboBox* action;


public:
    void setupUI(::CDlgBugReport* parent)
    {
        QVBoxLayout* layout = new QVBoxLayout;

        layout->addWidget(new QLabel("Which program would you like to make a report for?"));
        
        software = new QComboBox;
        software->addItem("FEBio Studio (this GUI frontend)");
        software->addItem("FEBio (the FE solver)");
        layout->addWidget(software);

        layout->addWidget(new QLabel("Would you like to..."));

        action = new QComboBox;
        action->addItem("View current bug reports/issues");
        action->addItem("Create a new bug report/issue (requires GitHub login)");
        layout->addWidget(action);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        QObject::connect(buttonBox, &QDialogButtonBox::accepted, parent, &::CDlgBugReport::accept);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, parent, &::CDlgBugReport::reject);
        
        layout->addWidget(buttonBox);
        
        parent->setLayout(layout);

        parent->setWindowTitle("Submit Bug Report");
    }

};

CDlgBugReport::CDlgBugReport(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgBugReport)
{
    ui->setupUI(this);
}

void CDlgBugReport::accept()
{
    QString url = "https://github.com/febiosoftware/";

    if(ui->software->currentIndex() == 0)
    {
        url += "FEBioStudio/issues";
    }
    else
    {
        url += "FEBio/issues";
    }

    if(ui->action->currentIndex() == 1)
    {
        url += "/new?template=🐛-bug-report.md&labels=from%20fbs&type=Bug";
    }

    QDesktopServices::openUrl(QUrl(url));

    QDialog::accept();
}