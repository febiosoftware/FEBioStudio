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
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QSpinBox>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include "DlgWarpImage.h"
#include "PostDocument.h"
#include <PostLib/FEPostModel.h>
#include <PostGL/GLModel.h>
#include "ImageThread.h"
#include <ImageLib/3DImage.h>
#include "DlgStartThread.h"
#include "MainWindow.h"
#include "IconProvider.h"
#include <ImageLib/ImageModel.h>

// converts a string to a list of numbers. 
// Note: no white space allowed in the string.
// Note: the numbers are converted to zero-base
static bool string_to_int_list(char* sz, std::vector<int>& list)
{
	// remove all white-space
	char* ch = sz;
	int l = 0;
	while (*ch)
	{
		if (isspace(*ch) == 0)
		{
			sz[l] = *ch;
			l++;
		}
		++ch;
	}
	sz[l] = 0;

	// make sure there
	if (strlen(sz) == 0) return false;

	list.clear();
	ch = sz;
	int n0 = -1, n1 = -1, nn = -1;
	do
	{
		if (n0 < 0) n0 = (int)atoi(ch) - 1;
		else if (n1 < 0) n1 = (int)atoi(ch) - 1;
		else if (nn < 0) nn = (int)atoi(ch);

		while (isdigit(*ch)) ++ch;
		switch (*ch)
		{
		case ':': ++ch; break;
		case ',': ++ch;
		case '\0':
		{
			if (n0 >= 0)
			{
				if (n1 == -1) n1 = n0;
				if (nn == -1) nn = 1;

				if ((n0 <= n1) && (nn>0))
					for (int n = n0; n <= n1; n += nn) list.push_back(n);
				else if ((n1 <= n0) && (nn<0))
					for (int n = n0; n >= n1; n += nn) list.push_back(n);
			}

			n0 = -1;
			n1 = -1;
			nn = -1;
		}
		break;
		default:
			return false;
		}
	} while (*ch);

	return true;
}

class Ui::CDlgWarpImage
{
public:
    QComboBox* img;
    QLineEdit* states;
    QLabel* statesExample;
    QLineEdit* dir;
    QPushButton* browse;
    QLineEdit* filename;
    QSpinBox* digits;
    QComboBox* type;
    QLabel* filenameExample;

public:
    CDlgWarpImage(CPostDocument* doc, ::CMainWindow* wnd) : doc(doc), wnd(wnd) {}

    void setupUi(::CDlgWarpImage* parent)
    {
        QVBoxLayout* layout = new QVBoxLayout;

        QFormLayout* form = new QFormLayout;

        img = new QComboBox;

        for(int i = 0; i < doc->ImageModels(); ++i)
        {
            img->addItem(doc->GetImageModel(i)->GetName().c_str());
        }

        form->addRow("Image:", img);

        states = new QLineEdit(QString::number(doc->GetActiveState() + 1));
        form->addRow("States:", states);

        statesExample = new QLabel("(e.g. 1,2,3:6,10:100:5)");
        form->addRow("", statesExample);

        QHBoxLayout* dirLayout = new QHBoxLayout;
        dirLayout->setContentsMargins(0, 0, 0, 0);

        dirLayout->addWidget(dir = new QLineEdit);

        browse = new QPushButton;
        browse->setIcon(CIconProvider::GetIcon("open"));
        dirLayout->addWidget(browse);

        form->addRow("Output Dir:", dirLayout);

        filename = new QLineEdit;
        filename->setText("warped");
        form->addRow("Filename:", filename);
        
        type = new QComboBox;
        type->addItems(QStringList() << "TIFF" << "NRRD" << "RAW");
        form->addRow("Image Type:", type);
        
        digits = new QSpinBox;
        digits->setValue(4);
        digits->setMinimum(1);
        digits->setMaximum(100);
        form->addRow("Digits:", digits);

        layout->addLayout(form);

        form->addRow("Example Filename:", filenameExample = new QLabel);
        
        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttonBox);

        parent->setLayout(layout);
        parent->setWindowTitle("Warp Image");

        parent->on_filename_changed();

        QObject::connect(browse, &QPushButton::clicked, parent, &::CDlgWarpImage::on_browse_clicked);
        QObject::connect(filename, &QLineEdit::textChanged, parent, &::CDlgWarpImage::on_filename_changed);
        QObject::connect(digits, &QSpinBox::valueChanged, parent, &::CDlgWarpImage::on_filename_changed);
        QObject::connect(type, &QComboBox::currentIndexChanged, parent, &::CDlgWarpImage::on_filename_changed);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, parent, &QDialog::accept);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, parent, &QDialog::reject);
    }

public:
    ::CMainWindow* wnd;
    CPostDocument* doc;
};

CDlgWarpImage::CDlgWarpImage(CPostDocument* doc, CMainWindow* parent)
    : QDialog(parent), ui(new Ui::CDlgWarpImage(doc, parent))
{
    ui->setupUi(this);
}

void CDlgWarpImage::accept()
{
    int nimg = ui->img->currentIndex();
    CImageModel* img = ui->doc->GetImageModel(nimg);

    std::vector<int> states;
    char buf[256] = {0}; 
	strcpy(buf, ui->states->text().toStdString().c_str());
    string_to_int_list(buf, states);

    if(states.empty())
    {
        QMessageBox::critical(this, "Error", "No valid states were specified.");
        return;
    }

    QDir dir = ui->dir->text();
    if(!dir.exists())
    {
        QMessageBox::critical(this, "Error", QString("The directory %1 does not exist.").arg(dir.absolutePath()));
        return;
    }

    QString filename = ui->filename->text();

    Post::CGLModel& mdl = *ui->doc->GetGLModel();
    int currentStateIndex = mdl.GetActiveState()->GetID();

    ui->wnd->AddLogEntry("Image Metadata\nFilename\tPixels X\tPixels Y\tPixels Z\tBox X0\tBox Y0\tBox Z0\tBox X1\tBox Y1\tBox Z1\n");

    for(int time : states)
    {
        if(time < 0 || time >= mdl.GetFSModel()->GetStates())
        {
            QMessageBox::warning(this, "Error", QString("Time step %1 does not exist.\nSkipping.").arg(time + 1));
            continue;
        };

        mdl.GetFSModel()->SetCurrentTimeIndex(time);;
        mdl.Update(true);
        
        WarpImageFilter* warp = new WarpImageFilter(&mdl);
        img->AddImageFilter(warp);

        CImageFilterThread* thread = new CImageFilterThread(img);
        thread->setUpdateTask(false);
        
        CDlgStartThread dlg(ui->wnd, thread);
        dlg.setTask("Warping Image at time step " + QString::number(time + 1));

        if(!dlg.exec())
        {
            img->RemoveFilter(warp);
            img->ClearFilters();

            mdl.GetFSModel()->SetCurrentTimeIndex(currentStateIndex);;
            mdl.Update(true);

            return;
        }

        QString currentFilename = filename + QString::number(time + 1).rightJustified(ui->digits->value(), '0') 
            + "." + ui->type->currentText().toLower();

        if(ui->type->currentIndex() == 2)
        {
            img->ExportRAWImage(dir.filePath(currentFilename).toStdString());
        }
        else
        {
            img->ExportSITKImage(dir.filePath(currentFilename).toStdString());
        }

        BOX box = img->GetBoundingBox();

        ui->wnd->AddLogEntry(QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9\t%10\n")
            .arg(currentFilename)
            .arg(img->Get3DImage()->Width())
            .arg(img->Get3DImage()->Height())
            .arg(img->Get3DImage()->Depth())
            .arg(box.x0)
            .arg(box.y0)
            .arg(box.z0)
            .arg(box.x1)
            .arg(box.y1)
            .arg(box.z1));

        img->RemoveFilter(warp);
        img->ClearFilters();
    }

    mdl.GetFSModel()->SetCurrentTimeIndex(currentStateIndex);;
    mdl.Update(true);

    QDialog::accept();
}

void CDlgWarpImage::on_browse_clicked()
{
    ui->dir->setText(QFileDialog::getExistingDirectory());
}

void CDlgWarpImage::on_filename_changed()
{
    ui->filenameExample->setText(ui->filename->text() + 
        QString::number(1).rightJustified(ui->digits->value(), '0') + "." +
        ui->type->currentText().toLower());
}