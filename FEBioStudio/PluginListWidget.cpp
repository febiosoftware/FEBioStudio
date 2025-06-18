/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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

#include <QApplication>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QResizeEvent>
#include <QPixmap>
#include <QPainter>
#include "PluginListWidget.h"
#include "PluginManager.h"

class Ui::PluginListWidget
{
public:
    QLabel* imageLabel;
    QLabel* nameLabel;
    QLabel* ownerLabel;
    QLabel* statusLabel;

public:

    void setupUi(::PluginThumbnail* parent, const Plugin& plugin)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setAlignment(Qt::AlignHCenter);
        imageLabel = new QLabel;

        QByteArray imageDataByteArray = QByteArray::fromBase64(plugin.imageData);

        image.loadFromData(imageDataByteArray);
        
        imageLabel->setPixmap(image.scaled(150, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imageLabel->setAlignment(Qt::AlignHCenter);

        nameLabel = new QLabel(QString::fromStdString("<b>" + plugin.name + "</b>"));
        nameLabel->setAlignment(Qt::AlignHCenter);
        ownerLabel = new QLabel(QString::fromStdString("<i>" + plugin.owner + "</i>"));
        ownerLabel->setAlignment(Qt::AlignHCenter);
        statusLabel = new QLabel(QString::fromStdString(plugin.description));
        statusLabel->setAlignment(Qt::AlignHCenter);
        statusLabel->setWordWrap(true);

        layout->addWidget(imageLabel);
        layout->addWidget(nameLabel);
        layout->addWidget(ownerLabel);
        layout->addWidget(statusLabel);

        parent->setLayout(layout);
        parent->setFixedSize(200, 200);

        parent->setAttribute(Qt::WA_Hover, true);
        parent->setCursor(Qt::PointingHandCursor);

        backgroundColor = qApp->palette().color(QPalette::Window);
        m_id = plugin.id;
    }

public:
    QColor backgroundColor;
    QPixmap image;
    int m_id;

};

PluginThumbnail::PluginThumbnail(const Plugin& plugin)
    : ui(new Ui::PluginListWidget)
{
    ui->setupUi(this, plugin);

    SetStatus(plugin.status);
}

void PluginThumbnail::SetStatus(int status)
{
    QPixmap pluginImg = ui->imageLabel->pixmap();

    QPixmap emblem;

    switch (status)
    {
    case PLUGIN_BROKEN:
        emblem = QPixmap(":/icons/emblems/warning.png").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setToolTip("This plugin is broken and cannot be used.");
        ui->statusLabel->setText("Broken");
        break;
    case PLUGIN_NOT_INSTALLED:
        setToolTip("This plugin is not installed.");
        ui->statusLabel->setText("Not Installed");
        break;
    case PLUGIN_OUT_OF_DATE:
        emblem = QPixmap(":/icons/emblems/caution.png").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setToolTip("This plugin is out of date.");
        ui->statusLabel->setText("Out of Date");
        break;
    case PLUGIN_UP_TO_DATE:
        emblem = QPixmap(":/icons/emblems/greenCheck.png").scaled(25, 25, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setToolTip("This plugin is up to date.");
        ui->statusLabel->setText("Up to Date");
        break;
    }

    QPainter painter(&pluginImg);
	painter.drawPixmap(pluginImg.width() - emblem.width(), pluginImg.height() - emblem.height(), emblem);

    ui->imageLabel->setPixmap(pluginImg);
}

int PluginThumbnail::getID()
{
    return ui->m_id;
}

void PluginThumbnail::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int radius = 15;

    painter.setPen(Qt::NoPen);
    painter.setBrush(ui->backgroundColor);
    painter.drawRoundedRect(rect(), radius, radius);
}

void PluginThumbnail::enterEvent(QEnterEvent* event)
{
    ui->backgroundColor = qApp->palette().color(QPalette::Highlight);
    // update();
}

void PluginThumbnail::leaveEvent(QEvent* event)
{
    ui->backgroundColor = qApp->palette().color(QPalette::Window);
}

void PluginThumbnail::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) 
    {
        emit clicked(ui->m_id);
    }
}
    
PluginListWidget::PluginListWidget()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Search Bar
    // QHBoxLayout* searchLayout = new QHBoxLayout();
    // searchInput = new QLineEdit(this);
    // searchInput->setPlaceholderText("Search plugins...");
    // QPushButton* searchButton = new QPushButton("Search", this);

    // searchLayout->addWidget(searchInput);
    // searchLayout->addWidget(searchButton);

    // mainLayout->addLayout(searchLayout);

    // Scrollable Plugin Grid
    scrollArea = new QScrollArea(this);
    QWidget* scrollWidget = new QWidget();
    gridLayout = new QGridLayout(scrollWidget);
    scrollWidget->setLayout(gridLayout);

    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea);

    setLayout(mainLayout);
}

void PluginListWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateGridLayout();
}

void PluginListWidget::AddPlugin(const Plugin& plugin)
{
    PluginThumbnail* card = new PluginThumbnail(plugin);
    pluginCards.append(card);
    
    updateGridLayout();
}

void PluginListWidget::Clear()
{
    qDeleteAll(pluginCards);
    pluginCards.clear();
    updateGridLayout();
}

void PluginListWidget::updateGridLayout() 
{
    int columns = width() / 220; // Adjust columns based on available width
    if (columns < 1) columns = 1;

    QLayoutItem* item;
    while ((item = gridLayout->takeAt(0)) != nullptr) 
    {
        delete item;
    }

    int row = 0, col = 0;
    for (PluginThumbnail* card : pluginCards) 
    {
        gridLayout->addWidget(card, row, col);

        connect(card, &PluginThumbnail::clicked, this, &PluginListWidget::pluginThumbnailClicked);

        col++;
        if (col >= columns) 
        {
            col = 0;
            row++;
        }
    }
}