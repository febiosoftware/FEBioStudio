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
#include "DlgConvertFEBio.h"
#include "MainWindow.h"
#include "DlgExportFEBio.h"
#include <QBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>

class CDlgConvertFEBioUI
{
public:
	QListWidget*	fileList;
	QLineEdit*		outPath;
	QComboBox*		outFormat;

public:
	void setup(QDialog* dlg)
	{
		dlg->setWindowTitle("Batch Convert");
		dlg->setMinimumSize(QSize(800, 600));

		QVBoxLayout* l = new QVBoxLayout;

		QPushButton* addFiles = new QPushButton("Add files ...");
		QPushButton* delFile = new QPushButton("Remove");
		QPushButton* clearFiles = new QPushButton("Clear");
		QVBoxLayout* b = new QVBoxLayout;
		b->addWidget(addFiles);
		b->addWidget(delFile);
		b->addWidget(clearFiles);
		b->addStretch();

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(fileList = new QListWidget);
		fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
		h->addLayout(b);
		l->addLayout(h);

		QPushButton* selectOutPath = new QPushButton("Select...");
		h = new QHBoxLayout;
		h->addWidget(new QLabel("Output folder:"));
		h->addWidget(outPath = new QLineEdit);
		h->addWidget(selectOutPath);
		l->addLayout(h);

		// NOTE: make sure the order here matches the order in CDlgExportFEBio
		outFormat = new QComboBox;
		outFormat->addItem("febio_spec 4.0");
		outFormat->addItem("febio_spec 3.0");
		outFormat->addItem("febio_spec 2.5");
		outFormat->addItem("febio_spec 2.0");
		outFormat->addItem("febio_spec 1.2");
		outFormat->setCurrentIndex(0);
		QPushButton* options = new QPushButton("More options...");
		h = new QHBoxLayout;
		h->addWidget(new QLabel("Output format:"));
		h->addWidget(outFormat);
		h->addWidget(options);
		h->addStretch();
		l->addLayout(h);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(addFiles, SIGNAL(clicked()), dlg, SLOT(on_addFiles()));
		QObject::connect(delFile, SIGNAL(clicked()), dlg, SLOT(on_removeFile()));
		QObject::connect(clearFiles, SIGNAL(clicked()), dlg, SLOT(on_clearFiles()));
		QObject::connect(selectOutPath, SIGNAL(clicked()), dlg, SLOT(on_selectOutPath()));
		QObject::connect(options, SIGNAL(clicked()), dlg, SLOT(on_moreOptions()));
	}
};

CDlgConvertFEBio::CDlgConvertFEBio(CMainWindow* wnd) : QDialog(wnd), ui(new CDlgConvertFEBioUI)
{
	ui->setup(this);

	m_compress = false;
	m_bexportSelections = true;
	m_writeNotes = true;

	for (int i = 0; i < MAX_SECTIONS; ++i)
	{
		m_nsection[i] = true;
	}
}

QStringList CDlgConvertFEBio::getFileNames()
{
	QStringList fileNames;
	for (int i = 0; i < ui->fileList->count(); ++i)
	{
		QListWidgetItem* it = ui->fileList->item(i);
		fileNames.push_back(it->text());
	}
	return fileNames;
}

QString CDlgConvertFEBio::getOutPath()
{
	return ui->outPath->text();
}

int CDlgConvertFEBio::getOutputFormat()
{
	return ui->outFormat->currentIndex();
}

void CDlgConvertFEBio::on_addFiles()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select Files", "", "FEBio files (*.feb)");
	if (fileNames.isEmpty() == false)
	{
		for (int i = 0; i < fileNames.size(); ++i)
		{
			QString fileName = fileNames[i];
			if (ui->fileList->findItems(fileName, Qt::MatchExactly).isEmpty())
			{
				ui->fileList->addItem(fileName);
			}
		}
	}
}

void CDlgConvertFEBio::on_selectOutPath()
{
	QString dir = QFileDialog::getExistingDirectory();
	if (dir.isEmpty() == false)
	{
		ui->outPath->setText(dir);
	}
}

void CDlgConvertFEBio::on_removeFile()
{
	QList<QListWidgetItem*> items = ui->fileList->selectedItems();
	for (QListWidgetItem* it : items) delete it;
}

void CDlgConvertFEBio::on_clearFiles()
{
	ui->fileList->clear();
}

void CDlgConvertFEBio::on_moreOptions()
{
	CDlgExportFEBio dlg(this);

	dlg.m_nversion = getOutputFormat();
	dlg.m_writeNotes = m_writeNotes;
	dlg.m_compress = m_compress;
	dlg.m_bexportSelections = m_bexportSelections;

	if (dlg.exec())
	{
		ui->outFormat->setCurrentIndex(dlg.m_nversion);
		m_writeNotes = dlg.m_writeNotes;
		m_compress = dlg.m_compress;
		m_bexportSelections = dlg.m_bexportSelections;
		for (int i = 0; i < MAX_SECTIONS; ++i) m_nsection[i] = dlg.m_nsection[i];
	}
}

void CDlgConvertFEBio::accept()
{
	if (ui->fileList->count() == 0)
	{
		QMessageBox::critical(this, "Batch Convert", "You need to add at least one file.");
		return;
	}

	QString outPath = getOutPath();
	if (outPath.isEmpty())
	{
		QMessageBox::critical(this, "Batch Convert", "You need to specify the output folder.");
		return;
	}

	QDir out(outPath);
	if (out.exists() == false)
	{
		QMessageBox::critical(this, "Batch Convert", "The output folder does not exist.\nPlease specify a valid folder path.");
		return;
	}

	QDialog::accept();
}
