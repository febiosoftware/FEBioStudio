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
#include <QDir>
#include "DlgWarpImage.h"
#include "PostDocument.h"
#include <PostLib/FEPostModel.h>
#include <PostGL/GLModel.h>

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
    QLineEdit* dir;
    QLineEdit* filename;

public:
    CDlgWarpImage(CPostDocument* doc) : doc(doc) {}

    void setupUi(QDialog* parent)
    {

        QVBoxLayout* layout = new QVBoxLayout;

        QFormLayout* form = new QFormLayout;

        img = new QComboBox;

        for(int i = 0; i < doc->ImageModels(); ++i)
        {
            img->addItem(doc->GetImageModel(i)->GetName().c_str());
        }

        form->addRow("Image:", img);

        states = new QLineEdit;
        states->setPlaceholderText("(e.g.:1,2,3:6,10:100:5)");
        form->addRow("States:", states);

        form->addRow("Output Dir:", dir = new QLineEdit);
        form->addRow("Filename:", filename = new QLineEdit);

        layout->addLayout(form);
        
        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        layout->addWidget(buttonBox);

        parent->setLayout(layout);
        parent->setWindowTitle("Warp Image");

        QObject::connect(buttonBox, &QDialogButtonBox::accepted, parent, &QDialog::accept);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, parent, &QDialog::reject);
    }

public:
    CPostDocument* doc;
};

CDlgWarpImage::CDlgWarpImage(CPostDocument* doc, QWidget* parent)
    : QDialog(parent), ui(new Ui::CDlgWarpImage(doc))
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

    QDir dir = ui->dir->text();
    QString filename = ui->filename->text();

    int digits = states.size() / 10 + 1;
    for(int time : states)
    {
        ui->doc->GetFSModel()->SetCurrentTimeIndex(time);
        ui->doc->GetGLModel()->Update(true);

        // create the image warp filter
        Post::CGLModel& mdl = *ui->doc->GetGLModel();
        
        WarpImageFilter* warp = new WarpImageFilter(&mdl);
        img->AddImageFilter(warp);
        img->ApplyFilters();

        QString currentFilename = filename + QString::number(time).rightJustified(digits, '0') + ".tiff";

        img->ExportSITKImage(dir.filePath(currentFilename).toStdString());

        img->RemoveFilter(warp);
    }

    QDialog::accept();
}