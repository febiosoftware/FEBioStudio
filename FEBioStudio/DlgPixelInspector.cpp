/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2023 University of Utah, The Trustees of Columbia University in
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
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QScrollBar>
#include "DlgPixelInspector.h"
#include "PixelInfoSource.h"
#include "ImageToolBar.h"

class Ui::CDlgPixelInspector
{
public:
    QTableWidget* table;

public:
    void setupUI(::CDlgPixelInspector* parent)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        
        int size = parent->GetRadius()*2+1;
        table = new QTableWidget(size,size);
        // table->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionMode(QAbstractItemView::NoSelection);
        table->setFocusPolicy(Qt::NoFocus);
        layout->addWidget(table);
        
        layout->setSizeConstraint(QLayout::SetFixedSize);

        parent->setLayout(layout);

        parent->setWindowTitle("Pixel Inspector");
    }

};


CDlgPixelInspector::CDlgPixelInspector(QWidget* parent, CImageToolBar* toolbar, CPixelInfoSource* source)
    : m_toolbar(toolbar), m_radius(3), m_source(source), ui(new Ui::CDlgPixelInspector), QDialog(parent)
{
    ui->setupUI(this);

    m_source->SetInspector(this);
    m_source->UpdatePixelInfo();
    UpdateData();
}

void CDlgPixelInspector::SetInfoSource(CPixelInfoSource* source)
{
    if(m_source)
    {
        m_source->SetInspector(nullptr);
    }

    m_source = source;
    m_source->SetInspector(this);
    m_source->UpdatePixelInfo();
    UpdateData();
}

void CDlgPixelInspector::UpdateData()
{
    QPoint startIndices = m_source->GetStartIndices();
    std::vector<QString> vals = m_source->GetPixelVals();
    std::vector<QColor> colors = m_source->GetPixelColors();

    // Update Cells
    int row = 0;
    int col = 0;
    for(int i = 0; i < vals.size(); i++)
    {
        QTableWidgetItem* item = new QTableWidgetItem(vals.at(i));
        item->setTextAlignment(Qt::AlignCenter);
        
        QColor& color = colors.at(i);
        item->setBackground(QBrush(color));

        if(color.value() < 128)
        {
            item->setForeground(QBrush(QColorConstants::White));
        }
        else
        {
            item->setForeground(QBrush(QColorConstants::Black));
        }

        ui->table->setItem(row, col, item);

        col++;
        if(col == m_radius*2+1)
        {
            col = 0;
            row++;
        }
    }

    // Update Headers
    QStringList horizontalHeaders, verticalHeaders;
    for(int i = 0; i < m_radius*2+1; i++)
    {
        horizontalHeaders << QString::number(startIndices.x() + i);
        verticalHeaders << QString::number(startIndices.y() - i);
    }
    ui->table->setHorizontalHeaderLabels(horizontalHeaders);
    ui->table->setVerticalHeaderLabels(verticalHeaders);

    // Resize columns and widget
    ui->table->resizeColumnsToContents();

    int tableWidth = 2 + ui->table->verticalHeader()->width();
    for(int i = 0; i < ui->table->columnCount(); i++)
    {
        tableWidth += ui->table->columnWidth(i);
    }
    
    int tableHeight = 2 + ui->table->horizontalHeader()->height();
    for(int i = 0; i < ui->table->rowCount(); i++)
    {
        tableHeight += ui->table->rowHeight(i);
    }

    ui->table->setFixedWidth(tableWidth);
    ui->table->setFixedHeight(tableHeight);
}

void CDlgPixelInspector::done(int r)
{
    m_toolbar->InspectorClosed();
    m_source->SetInspector(nullptr);

    QDialog::done(r);
}