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

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QProgressBar>
#include <QApplication>
#include <QDesktopServices>
#include <QStyleHints>
#include <QPainter>
#include <QPen>
#include <QPalette>
#include <QMouseEvent>
#include <QToolBar>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "DlgPluginRepo.h"
#include "PluginListWidget.h"
#include "PublicationWidgetView.h"
#include "PluginManager.h"
#include "WrapLabel.h"
#include "MainWindow.h"

CFrameButton::CFrameButton(QString text, QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    label = new QLabel(text);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    label->setAlignment(Qt::AlignCenter);

    QFont font =  label->font();
    font.setPointSize(14);
    label->setFont(font);

    layout->addWidget(label);

    setLayout(layout);

    defaultColor = qApp->palette().color(QPalette::Highlight);

    if(qApp->styleHints()->colorScheme() != Qt::ColorScheme::Dark)
    {
        defaultColor = defaultColor.lighter();
    }
    else
    {
        defaultColor = defaultColor.darker();
    }

    backgroundColor = defaultColor;

    setAttribute(Qt::WA_Hover, true);
    setCursor(Qt::PointingHandCursor);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void CFrameButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPalette palette = qApp->palette();

    // Customize border color, thickness, and corner radius
    QColor borderColor = palette.color(QPalette::WindowText);
    // Customize border and fill
    int borderWidth = 2;
    int radius = 15;

    QRectF borderRect = rect().adjusted(borderWidth / 2.0, borderWidth / 2.0,
                                        -borderWidth / 2.0, -borderWidth / 2.0);

    // First draw background
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroundColor);
    painter.drawRoundedRect(borderRect, radius, radius);

    // Then draw the border
    QPen pen(borderColor, borderWidth);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(borderRect, radius, radius);
}

QSize CFrameButton::sizeHint() const
{
    // Size hint = label's sizeHint + border padding
    QSize labelSize = label->sizeHint();
    int padding = 20; // 10 on each side
    return labelSize + QSize(padding, padding);
}

void CFrameButton::enterEvent(QEnterEvent* event)
{
    backgroundColor = qApp->palette().color(QPalette::Highlight);
    update();
}

void CFrameButton::leaveEvent(QEvent* event)
{
    backgroundColor = defaultColor;
    update();
}

void CFrameButton::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton) 
    {
        emit clicked();
    }
}


class Ui::CDlgPluginRepo
{
public:
    QStackedWidget* stackedWidget;
    ::PluginListWidget* pluginListWidget;

    QLabel* loadingMessage;
    QProgressBar* downloadProgress;

    QToolBar* toolBar;
    QPushButton* backButton;
    QLabel* imageLabel;
    QLabel* statusLabel;
    QLabel* downloadsLabel;
    QLabel* nameLabel;
    QLabel* ownerLabel;
    QLabel* descriptionLabel;
    CFrameButton* downloadButton;
    CFrameButton* loadButton;
    CFrameButton* unloadButton;
    CFrameButton* deleteButton;
    CFrameButton* sourceButton;
    
    ::CPublicationWidgetView* publicationWidget;

    void setupUI(::CDlgPluginRepo* dlg, CPluginManager* manager)
    {
        m_manager = manager;

        QVBoxLayout* l = new QVBoxLayout;

        stackedWidget = new QStackedWidget;

        QWidget* loadingWidget = new QWidget;
        QHBoxLayout* loadingLayout = new QHBoxLayout;
        loadingLayout->setContentsMargins(0, 0, 0, 0);
        loadingLayout->addStretch();

        QVBoxLayout* innerLoadingLayout = new QVBoxLayout;
        innerLoadingLayout->addStretch();
        innerLoadingLayout->addWidget(loadingMessage = new QLabel("Connecting ..."));
        innerLoadingLayout->addWidget(downloadProgress = new QProgressBar);
        downloadProgress->hide();
        innerLoadingLayout->addStretch();

        loadingLayout->addLayout(innerLoadingLayout);
        loadingLayout->addStretch();
        loadingWidget->setLayout(loadingLayout);
        stackedWidget->addWidget(loadingWidget);

        pluginListWidget = new ::PluginListWidget;
        stackedWidget->addWidget(pluginListWidget);

        QWidget* pluginCard = new QWidget;
        QVBoxLayout* pluginCardLayout = new QVBoxLayout;

        toolBar = new QToolBar;
        toolBar->addWidget(backButton = new QPushButton("Back"));
        pluginCardLayout->addWidget(toolBar);

        QHBoxLayout* pluginBottomLayout = new QHBoxLayout;

        QVBoxLayout* pluginLeftLayout = new QVBoxLayout;
        pluginLeftLayout->setAlignment(Qt::AlignHCenter);
        pluginLeftLayout->addWidget(imageLabel = new QLabel);
        imageLabel->setAlignment(Qt::AlignHCenter);

        QHBoxLayout* statusLayout = new QHBoxLayout;
        statusLayout->setContentsMargins(0, 0, 0, 0);
        statusLayout->addStretch();
        statusLayout->addWidget(new QLabel("Status:"));
        statusLayout->addWidget(statusLabel = new QLabel);
        statusLayout->addStretch();
        pluginLeftLayout->addLayout(statusLayout);

        QHBoxLayout* downloadsLayout = new QHBoxLayout;
        downloadsLayout->setContentsMargins(0, 0, 0, 0);
        downloadsLayout->addStretch();
        downloadsLayout->addWidget(new QLabel("Downloads:"));
        downloadsLayout->addWidget(downloadsLabel = new QLabel);
        downloadsLayout->addStretch();
        pluginLeftLayout->addLayout(downloadsLayout);

        downloadButton = new CFrameButton("Download Plugin");
        downloadButton->hide();
        pluginLeftLayout->addWidget(downloadButton, 0, Qt::AlignHCenter);

        loadButton = new CFrameButton("Load Plugin");
        loadButton->hide();
        pluginLeftLayout->addWidget(loadButton, 0, Qt::AlignHCenter);

        unloadButton = new CFrameButton("Unload Plugin");
        unloadButton->hide();
        pluginLeftLayout->addWidget(unloadButton, 0, Qt::AlignHCenter);

        deleteButton = new CFrameButton("Delete Plugin");
        deleteButton->hide();
        pluginLeftLayout->addWidget(deleteButton, 0, Qt::AlignHCenter);

        pluginLeftLayout->addStretch();
        pluginBottomLayout->addLayout(pluginLeftLayout, 0);

        QVBoxLayout* pluginRightLayout = new QVBoxLayout;

        QHBoxLayout* nameLayout = new QHBoxLayout;
        nameLayout->setContentsMargins(0, 0, 0, 0);

        nameLayout->addWidget(nameLabel = new QLabel);
        QFont font = nameLabel->font();
        font.setBold(true);
        font.setPointSize(18);
        nameLabel->setFont(font);

        QLabel* byLabel = new QLabel("by");
        QFont byFont = byLabel->font();
        byFont.setPointSize(18);
        byLabel->setFont(byFont);

        nameLayout->addWidget(byLabel);

        nameLayout->addWidget(ownerLabel = new QLabel);
        QFont font2 = ownerLabel->font();
        font2.setItalic(true);
        font2.setPointSize(18);
        ownerLabel->setFont(font2);

        nameLayout->addStretch();

        pluginRightLayout->addLayout(nameLayout);

        pluginRightLayout->addWidget(descriptionLabel = new QLabel);
        descriptionLabel->setWordWrap(true);

        pluginRightLayout->addWidget(publicationWidget = new ::CPublicationWidgetView(::CPublicationWidgetView::LIST, false));

        sourceButton = new CFrameButton("Source Code");
        pluginRightLayout->addWidget(sourceButton, 0, Qt::AlignRight);

        pluginRightLayout->addStretch();

        pluginBottomLayout->addLayout(pluginRightLayout, 1);

        pluginCardLayout->addLayout(pluginBottomLayout);
        pluginCard->setLayout(pluginCardLayout);

        stackedWidget->addWidget(pluginCard);

        l->addWidget(stackedWidget);
        

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);
		
        
		l->addWidget(bb);

        QObject::connect(pluginListWidget, &::PluginListWidget::pluginThumbnailClicked, dlg, &::CDlgPluginRepo::on_pluginThumbnail_clicked);
        QObject::connect(backButton, &QPushButton::clicked, dlg, &::CDlgPluginRepo::on_BackButton_clicked);
        QObject::connect(downloadButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_downloadButton_clicked);
        QObject::connect(deleteButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_deleteButton_clicked);
        QObject::connect(loadButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_loadButton_clicked);
        QObject::connect(unloadButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_unloadButton_clicked);
        QObject::connect(sourceButton, &CFrameButton::clicked, dlg, &::CDlgPluginRepo::on_sourceButton_clicked);

		QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &::CDlgPluginRepo::reject);
		dlg->setLayout(l);
    }

    void setActivePlugin(int id)
    {
        Plugin* plugin = m_manager->GetPlugin(id);

        m_pluginID = plugin->id;
        nameLabel->setText(plugin->name.c_str());
        ownerLabel->setText(plugin->owner.c_str());
        downloadsLabel->setText(QString::number(plugin->downloads));
        descriptionLabel->setText(plugin->description.c_str());

        QByteArray image(plugin->imageData);
        if (image.size() > 0)
        {
            QByteArray imageDataByteArray = QByteArray::fromBase64(image);

            QPixmap pixmap;
            pixmap.loadFromData(imageDataByteArray);
            imageLabel->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
        }
        else
        {
            imageLabel->clear();
        }

        publicationWidget->clear();

        for(auto& pub : plugin->publications)
        {
            publicationWidget->addPublication(pub);
        }

        hideButtons();
        
        if(plugin->status == PLUGIN_BROKEN)
        {
            statusLabel->setText("Broken");
            downloadButton->show();
        }
        else if(plugin->status ==  PLUGIN_NOT_INSTALLED)
        {
            statusLabel->setText("Not Installed");
            downloadButton->show();
        }
        else if(plugin->status == PLUGIN_OUT_OF_DATE)
        {
            statusLabel->setText("Out of Date");
            downloadButton->show();

            if(plugin->loaded)
            {
                unloadButton->show();
            }
            else
            {
                loadButton->show();
            }

            deleteButton->show();
        }
        else if(plugin->status == PLUGIN_UP_TO_DATE)
        {
            statusLabel->setText("Up to Date");

            if(plugin->loaded)
            {
                unloadButton->show();
            }
            else
            {
                loadButton->show();
            }

            deleteButton->show();
        }

        stackedWidget->setCurrentIndex(2);
    }

    void hideButtons()
    {
        downloadButton->hide();
        loadButton->hide();
        unloadButton->hide();
        deleteButton->hide();
    }

public:
    CPluginManager* m_manager;

    int m_pluginID = -1;
};

CDlgPluginRepo::CDlgPluginRepo(CPluginManager* manager, QWidget *parent)
    : QDialog(parent), ui(new Ui::CDlgPluginRepo)
{
    resize(800, 500);
    setWindowTitle("Plugin Repository");

    ui->setupUI(this, manager);

    connect(ui->m_manager, &CPluginManager::DownloadFinished, this, &CDlgPluginRepo::DownloadFinished);
    connect(ui->m_manager, &CPluginManager::PluginsReady, this, &CDlgPluginRepo::on_PluginsReady);
    connect(ui->m_manager, &CPluginManager::PluginsReady, this, &CDlgPluginRepo::on_PluginsReady);
    connect(ui->m_manager, &CPluginManager::HTMLError, this, &CDlgPluginRepo::on_HTMLError);

    ui->m_manager->Connect();
}

void CDlgPluginRepo::AddPublication(QVariantMap& data)
{
    ui->publicationWidget->addPublication(data);
}

void CDlgPluginRepo::DownloadFinished()
{
    ui->setActivePlugin(ui->m_pluginID);
}

void CDlgPluginRepo::downloadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    ui->downloadProgress->setRange(0, bytesTotal);
    ui->downloadProgress->setValue(bytesSent);
}

void CDlgPluginRepo::on_PluginsReady()
{
    ui->pluginListWidget->Clear();

    auto& plugins = ui->m_manager->GetPlugins();

    for (auto& [id, plugin] : plugins)
    {
        ui->pluginListWidget->AddPlugin(plugin);
    }
    
    ui->stackedWidget->setCurrentIndex(1);
}

void CDlgPluginRepo::on_HTMLError(QString& message, bool bclose)
{
    QMessageBox::warning(this, "Error", message);

    if(bclose)
    {
        close();
    }
    else
    {
        ui->setActivePlugin(ui->m_pluginID);
    }
}

void CDlgPluginRepo::on_UnknownError(QString& message)
{
    QMessageBox::warning(this, "Error", message);

    ui->setActivePlugin(ui->m_pluginID);
}

void CDlgPluginRepo::on_pluginThumbnail_clicked(int id)
{
    ui->setActivePlugin(id);
}

void CDlgPluginRepo::on_BackButton_clicked()
{
    on_PluginsReady();
}

void CDlgPluginRepo::on_downloadButton_clicked()
{
    ui->downloadProgress->setValue(0);
    ui->downloadProgress->show();
    ui->loadingMessage->setText("Downloading " + ui->nameLabel->text() + " ...");

    ui->stackedWidget->setCurrentIndex(0);

    ui->m_manager->DownloadPlugin(ui->m_pluginID);
}

void CDlgPluginRepo::on_deleteButton_clicked()
{
    if(!ui->m_manager->DeletePlugin(ui->m_pluginID))
    {
        QMessageBox::warning(this, "Error", "Unable to delete plugin.");
    }

    ui->setActivePlugin(ui->m_pluginID);
}

void CDlgPluginRepo::on_loadButton_clicked()
{
    if(!ui->m_manager->LoadPlugin(ui->m_pluginID))
    {
        QMessageBox::warning(this, "Error", "Unable to load plugin.");
    }

    ui->setActivePlugin(ui->m_pluginID);
}

void CDlgPluginRepo::on_unloadButton_clicked()
{
    if(!ui->m_manager->UnloadPlugin(ui->m_pluginID))
    {
        QMessageBox::warning(this, "Error", "Unable to unload plugin.");
    }

    ui->setActivePlugin(ui->m_pluginID);
}

void CDlgPluginRepo::on_sourceButton_clicked()
{
    Plugin* plugin = ui->m_manager->GetPlugin(ui->m_pluginID);
    assert(plugin);

    if(plugin)
    {
        QDesktopServices::openUrl(QUrl(plugin->sourceURL.c_str()));
    }
}