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

#include "stdafx.h"
#include "ImageViewer.h"
#include <QGraphicsView>
#include <QBoxLayout>
#include <QSlider>
#include <ImageLib/ImageModel.h>
#include <ImageLib/3DImage.h>
#include <QGraphicsScene>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QLabel>
#include <QComboBox>
#include <PostLib/FEPostModel.h>
#include <PostGL/GLModel.h>
using namespace Post;

class CImageViewer::Ui
{
public:
	CImageModel*		m_img;

	QGraphicsScene*	m_gs;
	QGraphicsPathItem*	m_path;

	QGraphicsView*	m_gv;
	QSlider*		m_slider;

	QComboBox*	m_overlay;

public:
	void setup(QWidget* w)
	{
		m_img = nullptr;
		m_path = nullptr;

		QVBoxLayout* l = new QVBoxLayout;
		l->setContentsMargins(1,1,1,1); //was setMargin

		m_slider = new QSlider;
		m_slider->setOrientation(Qt::Horizontal);
		m_slider->setTickPosition(QSlider::TicksBelow);

		QHBoxLayout* h = new QHBoxLayout;
		QLabel* label = new QLabel("overlay");
		h->addWidget(label);
		m_overlay = new QComboBox;
		m_overlay->addItem("None");
		m_overlay->addItem("Outline");
//		m_overlay->addItem("Slice");
		label->setBuddy(m_overlay);
		label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		h->addWidget(m_overlay);

		m_gs = new QGraphicsScene;

		m_gv = new QGraphicsView;
		m_gv->setScene(m_gs);

		l->addWidget(m_slider);
		l->addLayout(h);
		l->addWidget(m_gv);
		w->setLayout(l);

		QObject::connect(m_slider, SIGNAL(valueChanged(int)), w, SLOT(onSliderChanged(int)));
		QObject::connect(m_overlay, SIGNAL(currentIndexChanged(int)), w, SLOT(onOverlayChanged(int)));
	}
};

CImageViewer::CImageViewer(QWidget* parent) : QWidget(parent), ui(new CImageViewer::Ui)
{
	ui->setup(this);
}

void CImageViewer::SetImageModel(CImageModel* img)
{
	if (ui->m_img)
	{
		CGLModel* mdl = ui->m_img->GetModel();
		FEPostModel* fem = (mdl ? mdl->GetFSModel() : nullptr);
		if (fem) fem->RemoveDependant(this);
	}

	ui->m_img = img;
	if (img)
	{
		C3DImage& im3d = *ui->m_img->Get3DImage();

		int n = im3d.Depth();
		ui->m_slider->setRange(0, n-1);

		int m = n / 100 + 1;
		ui->m_slider->setTickInterval(m);

		ui->m_slider->setSingleStep(1);
		ui->m_slider->setPageStep(m);

		CGLModel* mdl = img->GetModel();
		FEPostModel* fem = (mdl ? mdl->GetFSModel() : nullptr);
		if (fem) fem->AddDependant(this);
	}
	Update();
}

void CImageViewer::onSliderChanged(int val)
{
	Update();
}

void CImageViewer::onOverlayChanged(int val)
{
	Update();
}

void CImageViewer::Update(FEPostModel* fem)
{
	UpdatePath();
}

void CImageViewer::Update()
{
	if (ui->m_path)
	{
		delete ui->m_path;
		ui->m_path = nullptr;
	}

	ui->m_gs->clear();
	if (ui->m_img == nullptr) return;

	C3DImage& im3d = *ui->m_img->Get3DImage();
	int NX = im3d.Width();
	int NY = im3d.Height();

	CImage im;
	int slice = ui->m_slider->value();
	im3d.GetSliceZ(im, slice);
	ui->m_slider->setToolTip(QString::number(slice));

	QImage qim(im.GetBytes(), im.Width(), im.Height(), im.Width(), QImage::Format::Format_Grayscale8);

	QPixmap pixmap = QPixmap::fromImage(qim);

	ui->m_gs->setSceneRect(0, 0, im.Width(), im.Height());
	QGraphicsPixmapItem* item = ui->m_gs->addPixmap(pixmap);
	ui->m_gv->fitInView(item, Qt::AspectRatioMode::KeepAspectRatio);

	UpdatePath();
}

void CImageViewer::UpdatePath()
{
	if (ui->m_path)
	{
		delete ui->m_path;
		ui->m_path = nullptr;
	}

	if (ui->m_img == nullptr) return;

	if (ui->m_overlay->currentIndex() == 0) return;

	if (ui->m_overlay->currentIndex() == 1)
	{
		CGLModel* mdl = ui->m_img->GetModel();
		if (mdl == nullptr) return;

		FSMesh* mesh = mdl->GetActiveMesh();
		if (mesh == nullptr) return;

		C3DImage& im3d = *ui->m_img->Get3DImage();
		int NX = im3d.Width();
		int NY = im3d.Height();

		BOX b = ui->m_img->GetBoundingBox();
		int slice = ui->m_slider->value();
		int NZ = im3d.Depth();
		if (NZ == 1) NZ = 2;
		double h = b.z0 + slice * (b.z1 - b.z0) / (NZ - 1);

		QPen pen(QColor::fromRgb(0, 255, 0), 1.0);

		QPainterPath path;

		vec3d r[4];
		for (int i = 0; i < mesh->Faces(); ++i)
		{
			FSFace& face = mesh->Face(i);
			int nf = face.Nodes();
			int nup = 0;
			for (int j = 0; j < nf; ++j)
			{
				r[j] = mesh->Node(face.n[j]).r;
				if (r[j].z >= h) nup++;
			}

			// see if the face intersects the plane at height h
			if ((nup > 0) && (nup < nf))
			{
				vec3f p[2];
				int np = 0;
				for (int j = 0; j < nf; ++j)
				{
					vec3d& a = r[j];
					vec3d& b = r[(j + 1) % nf];

					if ((a.z - h)*(b.z - h) < 0)
					{
						double w = (h - a.z) / (b.z - a.z);

						p[np].x = a.x + w*(b.x - a.x);
						p[np].y = a.y + w*(b.y - a.y);
						p[np].z = h;

						np++;
					}
					if (np == 2) break;
				}

				// Add a line segment
				if (np == 2)
				{
					double x0 = NX*(p[0].x - b.x0) / (b.x1 - b.x0);
					double x1 = NX*(p[1].x - b.x0) / (b.x1 - b.x0);
					double y0 = NY*(p[0].y - b.y0) / (b.y1 - b.y0);
					double y1 = NY*(p[1].y - b.y0) / (b.y1 - b.y0);

					path.moveTo(x0, y0);
					path.lineTo(x1, y1);
				}
			}
		}

		if (path.isEmpty() == false)
		{
			ui->m_path = ui->m_gs->addPath(path, pen);
		}
	}
}

