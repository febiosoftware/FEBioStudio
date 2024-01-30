/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2024 University of Utah, The Trustees of Columbia University in
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
#include "DlgMatrixInspector.h"
#include <QLayout>
#include <QDialogButtonBox>
#include <QTableView>
#include <QPainter>
#include <QSplitter>
#include <QScrollBar>
#include <QMouseEvent>
#include <QLabel>
#include <QComboBox>
#include <QScrollArea>
#include <FECore/FEGlobalMatrix.h>

MatrixDensityView::MatrixDensityView(CDlgMatrixInspector* dlg, QWidget* parent) : QWidget(parent), m_dlg(dlg) {}

void MatrixDensityView::paintEvent(QPaintEvent* paintEvent) 
{
	QPainter painter(this);

	drawBackground(painter);

	drawMatrixProfile(painter);

	setMinimumSize(300, 300);

	if (m_sel.isValid()) drawSelection(painter);
}

void MatrixDensityView::drawBackground(QPainter& painter)
{
	QRect rt = rect();
	painter.fillRect(rt, QColor::fromRgb(200, 200, 200));
	if ((m.columns() == 0) || (m.rows() == 0)) return;

	int N = (rt.width() > rt.height() ? rt.height() : rt.width());
	painter.fillRect(0, 0, N, N, QColor::fromRgb(255, 255, 255));
}

void MatrixDensityView::drawMatrixProfile(QPainter& painter)
{
	if ((m.columns() == 0) || (m.rows() == 0)) return;

	double maxM = m_maxM;
	if (maxM == 0) maxM = 1;

	double minM = m_minM;
	if (minM == 0) minM = -1;

	QRect rt = rect();
	int N = (rt.width() > rt.height() ? rt.height() : rt.width());
	for (int i = 0; i < m.rows(); ++i)
		for (int j = 0; j < m.columns(); ++j)
		{
			size_t x0 = (N * j) / m.columns();
			size_t y0 = (N * i) / m.rows();

			size_t x1 = (N * (j + 1)) / m.columns();
			size_t y1 = (N * (i + 1)) / m.rows();

			if ((m_colorMode == 0) || (m_colorMode == 1))
			{
				if (m[i][j] != 0)
				{
					double w = (double)m[i][j] / (double)maxM;
					QColor c = QColor::fromRgbF(1.0, 1.0 - w, 1.0 - w);
					painter.fillRect(x0, y0, x1 - x0, y1 - y0, c);
				}
			}
			else if ((m_colorMode == 2) || (m_colorMode == 3))
			{
				double mij = m[i][j];
				if (mij > 0)
				{
					double w = (double)m[i][j] / (double)maxM;
					QColor c = QColor::fromRgbF(1.0, 1.0 - w, 1.0 - w);
					painter.fillRect(x0, y0, x1 - x0, y1 - y0, c);
				}
				else if (mij < 0)
				{
					double w = (m[i][j]) / minM;
					QColor c = QColor::fromRgbF(1.0 - w, 1.0 - w, 1.0);
					painter.fillRect(x0, y0, x1 - x0, y1 - y0, c);
				}
			}
		}
}

void MatrixDensityView::drawSelection(QPainter& painter)
{
	QRect rt = rect();
	int N = (rt.width() > rt.height() ? rt.height() : rt.width());

	size_t x0 = (N * m_sel.left()) / m.columns();
	size_t y0 = (N * m_sel.top()) / m.rows();

	size_t x1 = (N * (m_sel.right())) / m.columns();
	size_t y1 = (N * (m_sel.bottom())) / m.rows();

	painter.setPen(QPen(Qt::green));
	painter.drawRect(x0, y0, x1 - x0 + 1, y1 - y0 + 1);
}

QSize MatrixDensityView::sizeHint() const 
{
	return QSize(300, 300);
}

void MatrixDensityView::resizeEvent(QResizeEvent* ev)
{
	QRect rt = rect();
	int M = (rt.width() > rt.height() ? rt.height() : rt.width());
	int N = M / 2;
	if (N < 1) N = 1;

	SparseMatrixProfile* P = m_K->GetSparseMatrixProfile();
	if (P)
	{
		int NP = P->Rows();
		if (N > NP) N = NP;
		else
		{
			while (NP > N) NP /= 2;
			N = NP;
			if (N < 1) N = 1;
		}
	}

	m_sel = QRect();
	m.resize(N, N); m.zero();
	Update();
}

void MatrixDensityView::SetGlobalMatrix(FEGlobalMatrix* K)
{
	m_K = K;
	if (K) Update();
}

void MatrixDensityView::SetSelection(QRect sel)
{
	if (m_K == nullptr) return;
	size_t N = m_K->Rows();
	if (N < 1) return;

	size_t x0 = (m.columns() * sel.left()) / N;
	size_t y0 = (m.rows()    * sel.top()) / N;

	size_t x1 = (m.columns() * sel.right()) / N;
	size_t y1 = (m.rows()    * sel.bottom()) / N;

	if (x0 < 0) x0 = 0; if (x0 >= m.columns()) x0 = m.columns() - 1;
	if (x1 < 0) x1 = 0; if (x1 >= m.columns()) x1 = m.columns() - 1;
	if (y0 < 0) y0 = 0; if (y0 >= m.rows()) y0 = m.rows() - 1;
	if (y1 < 0) y1 = 0; if (y1 >= m.rows()) y1 = m.rows() - 1;

	m_sel = QRect(x0, y0, x1 - x0 + 1, y1 - y0 + 1);

	update();
}

void MatrixDensityView::mousePressEvent(QMouseEvent* ev)
{
	size_t x = ev->pos().x();
	size_t y = ev->pos().y();
	updateView(x, y);
}

void MatrixDensityView::mouseMoveEvent(QMouseEvent* ev)
{
	if ((ev->buttons() & Qt::LeftButton))
	{
		size_t x = ev->pos().x();
		size_t y = ev->pos().y();
		updateView(x, y);
	}
}

void MatrixDensityView::wheelEvent(QWheelEvent* ev)
{
	int y = ev->angleDelta().y();
	QSize widgetSize = size();
	if (y > 0)
	{
		int newWidth  = (10 * widgetSize.width()) / 9;
		int newHeight = (10 * widgetSize.height()) / 9;
		resize(newWidth, newHeight);
	}
	else if (y < 0)
	{
		int newWidth = (9 * widgetSize.width()) / 10;
		int newHeight = (9 * widgetSize.height()) / 10;
		resize(newWidth, newHeight);
	}
}

void MatrixDensityView::updateView(size_t x, size_t y)
{
	QRect rt = rect();
	size_t N = (rt.width() > rt.height() ? rt.height() : rt.width());

	if ((x < 0) || (x >= N) || (y < 0) || (y >= N)) return;
	if (m_K == nullptr) return;

	int NR = m_K->Rows();
	x = (NR * x) / N;
	y = (NR * y) / N;
	m_dlg->updateView(x, y);

}

void MatrixDensityView::Update()
{
	if (m_K == nullptr) return;
	SparseMatrixProfile* P = m_K->GetSparseMatrixProfile();
	if (P == nullptr) return;
	SparseMatrix* A = m_K->GetSparseMatrixPtr();

	int N = m.rows();
	if (N == 0) return;

	size_t NC = P->Columns();
	size_t NR = P->Rows();
	m.zero();
	m_minM = 0;
	m_maxM = 0;
	for (size_t j = 0; j < NC; ++j)
	{
		SparseMatrixProfile::ColumnProfile& CP = P->Column(j);
		for (size_t l = 0; l < CP.size(); ++l)
		{
			SparseMatrixProfile::RowEntry& rowSpan = CP[l];
			for (size_t i = rowSpan.start; i <= rowSpan.end; ++i)
			{
				int x = (N*i) / NR;
				int y = (N*j) / NC;
				switch (m_colorMode)
				{
				case 0: m[x][y] += 1; break;
				case 1: if (A->get(i, j) != 0.0) m[x][y] += 1; break;
				case 2: m[x][y] += A->get(i, j); break;
				case 3: m[x][y] += A->get(i, j) - A->get(j, i); break;
				}
						
				if (m[x][y] > m_maxM) m_maxM = m[x][y];
				if (m[x][y] < m_minM) m_minM = m[x][y];
			}
		}
	}
	update();
}

void MatrixDensityView::SetColorMode(int option)
{
	m_colorMode = option;
	Update();
}

class MatrixModel : public QAbstractTableModel
{
public:
	MatrixModel(FEGlobalMatrix* M) : m_K(M) 
	{
		if (M) m_A = M->GetSparseMatrixPtr();
	}

public: // overrides from base class

	int rowCount(const QModelIndex& parent) const override
	{
		return (m_A ? m_A->Rows() : 0);
	}

	int columnCount(const QModelIndex& parent) const override
	{
		return (m_A ? m_A->Columns() : 0);
	}

	QVariant data(const QModelIndex& index, int role) const override
	{
		if (m_A == nullptr) return QVariant();
		if (index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole))
		{
			int row = index.row();
			int col = index.column();
			if (m_A->check(row, col))
			{
				double d = m_A->get(row, col);
				return QVariant(d);
			}
		}
		if (index.isValid() && (role == Qt::BackgroundRole))
		{
			int row = index.row();
			int col = index.column();
			if (m_A->check(row, col))
			{
				double d = m_A->get(row, col);
				if      (d > 0) return QBrush(QColor::fromRgb(255, 220, 255));
				else if (d < 0) return QBrush(QColor::fromRgb(220, 255, 255));
			}
			else
			{
				return QBrush(QColor::fromRgb(128, 128, 128));
			}
		}
		if (index.isValid() && (role == Qt::FontRole))
		{
			int row = index.row();
			int col = index.column();
			QFont font;
			if (row == col) font.setBold(true);
			return font;
		}
		return QVariant();
	}

private:
	FEGlobalMatrix* m_K = nullptr;
	SparseMatrix* m_A = nullptr;
};

class CDlgMatrixInspector::Ui 
{
public:
	QTableView* view;
	MatrixDensityView* densView;

public:
	void setup(CDlgMatrixInspector* dlg)
	{
		QLayout* l = new QVBoxLayout;
		view = new QTableView();
		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);

		QSplitter* splitter = new QSplitter(Qt::Horizontal);
		splitter->addWidget(view);

		QWidget* matView = new QWidget;
		QVBoxLayout* matViewLayout = new QVBoxLayout;
		QHBoxLayout* h = new QHBoxLayout;
		QComboBox* colorMode = new QComboBox;
		colorMode->addItems(QStringList() << "sparsity pattern" << "non-zeroes" << "values" << "symmetric difference");
		h->addWidget(new QLabel("color mode:"));
		h->addWidget(colorMode);
		h->addStretch();
		matViewLayout->addLayout(h);

		QScrollArea* scroll = new QScrollArea;
		scroll->setWidget(densView = new MatrixDensityView(dlg));
		matViewLayout->addWidget(scroll);
		matView->setLayout(matViewLayout);

		splitter->addWidget(matView);
		l->addWidget(splitter);
		l->addWidget(bb);
		dlg->setLayout(l);

		QScrollBar* vertScroll = view->verticalScrollBar();
		QScrollBar* horzScroll = view->horizontalScrollBar();

		QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::reject);
		QObject::connect(bb, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
		QObject::connect(vertScroll, &QScrollBar::valueChanged, dlg, &CDlgMatrixInspector::onViewScroll);
		QObject::connect(horzScroll, &QScrollBar::valueChanged, dlg, &CDlgMatrixInspector::onViewScroll);
		QObject::connect(colorMode, &QComboBox::currentIndexChanged, densView, &MatrixDensityView::SetColorMode);
	}

	QRect GetVisibleMatrixRegion()
	{
		QRect boundingRect = view->rect();
		int row0 = view->rowAt(boundingRect.top());
		int row1 = view->rowAt(boundingRect.bottom());
		int col0 = view->columnAt(boundingRect.left());
		int col1 = view->columnAt(boundingRect.right());
		return QRect(col0, row0, col1 - col0 + 1, row1 - row0 + 1);
	}
};

CDlgMatrixInspector::CDlgMatrixInspector(QWidget* parent) : QDialog(parent), ui(new CDlgMatrixInspector::Ui)
{
	setWindowTitle("Matrix Inspector");
	setMinimumSize(900, 600);
	ui->setup(this);
}

void CDlgMatrixInspector::SetGlobalMatrix(FEGlobalMatrix* M)
{
	ui->view->setModel(new MatrixModel(M));
	ui->densView->SetGlobalMatrix(M);
	ui->densView->SetSelection(ui->GetVisibleMatrixRegion());
}

void CDlgMatrixInspector::onViewScroll()
{
	ui->densView->SetSelection(ui->GetVisibleMatrixRegion());
}

void CDlgMatrixInspector::updateView(int x, int y)
{
	ui->view->scrollTo(ui->view->model()->index(y, x));
}
