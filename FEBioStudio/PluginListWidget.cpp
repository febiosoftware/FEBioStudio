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
#include <QProgressBar>
#include <QLabel>
#include <QScrollArea>
#include <QResizeEvent>
#include <QPixmap>
#include <QPainter>
#include "PluginListWidget.h"
#include "PluginManager.h"
#include "DlgPluginRepo.h"

class Ui::PluginThumbnail
{
public:
    QLabel* imageLabel;
    QLabel* nameLabel;
    QLabel* ownerLabel;
    QLabel* statusLabel;

public:

    PluginThumbnail(::PluginThumbnail* parent)
        : m_parent(parent), m_id(0), m_installed(false), m_selected(false)
    {

    }

    void setupUi(const Plugin* plugin)
    {
        m_parent->setAttribute(Qt::WA_Hover, true);
        m_parent->setCursor(Qt::PointingHandCursor);
        m_parent->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);

        backgroundColor = qApp->palette().color(QPalette::Window);

        // This widget is also used to display the status of the repo while
        // it is connecting, or if there is an error. A nullptr value for
        // the plugin is the signal that this object is used for showing the
        // status 
        if(!plugin)
        {
            m_id = 0;

            QVBoxLayout* layout = new QVBoxLayout;
            layout->setAlignment(Qt::AlignHCenter);

            nameLabel = new QLabel;
            ownerLabel = new QLabel("<b>Connecting...</b>");
            statusLabel = new QLabel;

            layout->addWidget(nameLabel, 0, Qt::AlignHCenter);
            layout->addWidget(ownerLabel, 0, Qt::AlignHCenter);
            layout->addWidget(statusLabel, 0, Qt::AlignHCenter);

            m_parent->setLayout(layout);

            return;
        }

        QHBoxLayout* layout = new QHBoxLayout;
        layout->setAlignment(Qt::AlignLeft);
        
        QVBoxLayout* infoLayout = new QVBoxLayout;
        infoLayout->setContentsMargins(0,0,0,0);
        infoLayout->setSpacing(6);
        infoLayout->setAlignment(Qt::AlignVCenter);

        nameLabel = new QLabel(QString::fromStdString("<b>" + plugin->name + "</b>"));
        nameLabel->setAlignment(Qt::AlignLeft);
        QFont nameFont = nameLabel->font();
        nameFont.setBold(true);
        nameLabel->setFont(nameFont);
        
        ownerLabel = new QLabel(QString::fromStdString("<i>" + plugin->owner + "</i>"));
        ownerLabel->setAlignment(Qt::AlignLeft);
        QFont ownerFont = ownerLabel->font();
        ownerFont.setItalic(true);
        ownerLabel->setFont(ownerFont);

        statusLabel = new QLabel(QString::fromStdString(plugin->description));
        statusLabel->setAlignment(Qt::AlignLeft);
        QFont statusFont = statusLabel->font();

        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(ownerLabel);
        infoLayout->addWidget(statusLabel);

        // In order to get the appropriate size for the image label, we need
        // to have the fonts for the other labels, so even though the image label
        // gets added to the layout first, we instantiate it and set the pixmap
        // after adding the other labels
        if(plugin->id > 0)
        {
            QByteArray imageDataByteArray = QByteArray::fromBase64(plugin->imageData);
            pixmap.loadFromData(imageDataByteArray);
        }
        else
        {
            pixmap.load(":/icons/febio_large.png");
        }

        imageLabel = new QLabel;
        imageLabel->setAlignment(Qt::AlignCenter);

        int imageHeight = QFontMetrics(nameFont).height();
        imageHeight += QFontMetrics(ownerFont).height();
        imageHeight += QFontMetrics(statusFont).height();
        imageHeight += infoLayout->spacing()*2;

        pixmap = pixmap.scaled(imageHeight, imageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        imageLabel->setPixmap(pixmap);
        imageLabel->setFixedWidth(imageHeight);
        
        // Make a grayscale version of the image to be used to show download progress
        QImage grayImage = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
        QColor c;
        for (int y = 0; y < pixmap.height(); ++y)
        {
            QRgb *scanLine = reinterpret_cast<QRgb*>(grayImage.scanLine(y));
            
            for (int x = 0; x < pixmap.width(); ++x)
            {
                c.setRgba(scanLine[x]);
                int grayValue = qGray(c.darker().rgb()); 
                scanLine[x] = qRgba(grayValue, grayValue, grayValue, c.alpha()); 
            }
        }
        grayPixmap = QPixmap::fromImage(grayImage);

        layout->addWidget(imageLabel);
        layout->addSpacing(5);
        layout->addLayout(infoLayout);

        m_parent->setLayout(layout);

        m_id = plugin->id;
        m_installed = plugin->localCopy;
    }

public:
    QWidget* m_parent;
    QColor backgroundColor;
    QPixmap pixmap;
    QPixmap grayPixmap;
    int m_id;
    bool m_installed;
    bool m_selected;
};

PluginThumbnail::PluginThumbnail(const Plugin* plugin)
    : ui(new Ui::PluginThumbnail(this))
{
    ui->setupUi(plugin);

    if(plugin) SetStatus(plugin->status);
}

void PluginThumbnail::SetProgress(float progress)
{
    // Start with fully gray
    QPixmap result(ui->grayPixmap.size());
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.drawPixmap(0, 0, ui->grayPixmap);

    // Determine height of the colored portion
    int colorHeight = result.height() * progress;

    if (colorHeight > 0) 
    {
        QRect colorRect(0, result.height() - colorHeight,
                        result.width(), colorHeight);

        painter.setClipRect(colorRect);
        painter.drawPixmap(0, 0, ui->pixmap);
    }

    painter.end();

    ui->imageLabel->setPixmap(result);
}

void PluginThumbnail::SetErrorText(const QString& text)
{
    ui->ownerLabel->setText(text);
}

void PluginThumbnail::SetStatus(int status)
{
    ui->imageLabel->setPixmap(ui->pixmap);

    QPixmap pluginImg = ui->imageLabel->pixmap();

    QPixmap emblem;

    switch (status)
    {
    case PLUGIN_BROKEN:
        emblem = QPixmap(":/icons/emblems/warning.png").scaled(17, 17, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setToolTip("This plugin is broken and cannot be used.");
        ui->statusLabel->setText("Broken");
        break;
    case PLUGIN_UNAVAILABLE:
        emblem = QPixmap(":/icons/emblems/warning.png").scaled(17, 17, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setToolTip("There are no versions of this plugin available for download.");
        ui->statusLabel->setText("Unavailable");
        break;
    case PLUGIN_NOT_INSTALLED:
        setToolTip("This plugin is not installed.");
        ui->statusLabel->setText("Not Installed");
        break;
    case PLUGIN_OUT_OF_DATE:
        emblem = QPixmap(":/icons/emblems/caution.png").scaled(17, 17, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setToolTip("This plugin is out of date.");
        ui->statusLabel->setText("Out of Date");
        break;
    case PLUGIN_UP_TO_DATE:
        emblem = QPixmap(":/icons/emblems/greenCheck.png").scaled(15, 15, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        setToolTip("This plugin is up to date.");
        ui->statusLabel->setText("Up to Date");
        break;
    case PLUGIN_LOCAL:
        setToolTip("This plugin in not part of the repository, but was loaded from a local file.");
        ui->statusLabel->setText("Local Plugin");
        ui->ownerLabel->hide();
        break;
    case PLUGIN_DOWNLOADING:
        setToolTip("Downloading...");
        ui->statusLabel->setText("Downloading...");
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

void PluginThumbnail::setInstalled(bool installed)
{
    ui->m_installed = installed;
}

bool PluginThumbnail::installed()
{
    return ui->m_installed;
}

void PluginThumbnail::setSelected(bool selected)
{
    ui->m_selected = selected;
}

void PluginThumbnail::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QColor color = ui->backgroundColor;
    if(ui->m_selected) color = qApp->palette().color(QPalette::Highlight);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(rect(), 5, 5);
}

void PluginThumbnail::enterEvent(QEnterEvent* event)
{
    ui->backgroundColor = qApp->palette().color(QPalette::Highlight);
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

class Ui::PluginListWidget
{
public:
    CCollapsibleHeader* installedPlugins;
    QVBoxLayout* installedLayout;
    CCollapsibleHeader* repoPlugins;
    QVBoxLayout* repoLayout;
    ::PluginThumbnail* statusThumbnail;

public:

    void setupUi(::PluginListWidget* parent)
    {
        m_parent = parent;

        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->setContentsMargins(0, 0, 0, 0);

        QScrollArea* scrollArea = new QScrollArea;
        QWidget* scrollWidget = new QWidget;
        QVBoxLayout* scrollLayout = new QVBoxLayout;
        scrollLayout->setContentsMargins(0,0,0,0);
        scrollWidget->setLayout(scrollLayout);

        installedPlugins = new CCollapsibleHeader("Installed");
        installedPlugins->SetExpanded(true);

        QWidget* installedWidget = new QWidget;
        installedLayout = new QVBoxLayout;
        installedLayout->setContentsMargins(0,0,0,0);
        installedWidget->setLayout(installedLayout);

        installedPlugins->SetContents(installedWidget);

        scrollLayout->addWidget(installedPlugins);

        repoPlugins = new CCollapsibleHeader("Repository");
        repoPlugins->SetExpanded(true);

        QWidget* repoWidget = new QWidget;
        repoLayout = new QVBoxLayout;
        repoLayout->setContentsMargins(0,0,0,0);
        repoWidget->setLayout(repoLayout);

        repoLayout->addWidget(statusThumbnail = new ::PluginThumbnail(nullptr));
        QObject::connect(statusThumbnail, &::PluginThumbnail::clicked, parent, &::PluginListWidget::on_pluginThumbnailClicked);

        repoPlugins->SetContents(repoWidget);

        scrollLayout->addWidget(repoPlugins);

        scrollLayout->addStretch();
        
        scrollArea->setWidget(scrollWidget);
        scrollArea->setWidgetResizable(true);
        
        mainLayout->addWidget(scrollArea);

        parent->setLayout(mainLayout);

        parent->setMinimumWidth(250);
    }

    void updateUi(int repoStatus = CPluginManager::CONNECTED)
    {
        statusThumbnail->hide();
        
        for(::PluginThumbnail* widget : pluginThumbnails)
        {
            widget->hide();
        }

        // Removes the items from the layout, but does not delete the
        // widgets themselves.
        QLayoutItem* item;
        while((item = installedLayout->takeAt(0)) != nullptr) 
        {
            delete item;
        }

        while((item = repoLayout->takeAt(0)) != nullptr) 
        {
            delete item;
        }

        bool showInstalled = false;
        bool showRepo = false;

        for(::PluginThumbnail* thumbnail : pluginThumbnails) 
        {
            // If there is only one search result and it's -1, show all plugins
            if(!((searchResults.size() == 1) && (searchResults.count(-1) == 1)))
            {
                // Otherwise skip if the thumbnail ID is not in the search results
                if(searchResults.count(thumbnail->getID()) == 0) 
                {
                    continue; // Skip if not in search results
                }
            }

            if(thumbnail->installed())
            {
                installedLayout->addWidget(thumbnail);
                thumbnail->show();
                showInstalled = true;
            }
            else
            {
                if(repoStatus == CPluginManager::CONNECTED)
                {
                    repoLayout->addWidget(thumbnail);
                    thumbnail->show();
                }
                
                showRepo = true;
            }
        }

        installedPlugins->setVisible(showInstalled);

        if(repoStatus != CPluginManager::CONNECTED)
        {
            statusThumbnail->show();

            if(repoStatus == CPluginManager::ERROR)
            {
                statusThumbnail->SetErrorText("<b>Connection Error</b>");
            }
            else
            {
                statusThumbnail->SetErrorText("<b>Connecting...</b>");
            }

            repoLayout->addWidget(statusThumbnail);
            
            showRepo = true;
        }

        repoPlugins->setVisible(showRepo);

        m_connected = false;
    }

public:
    ::PluginListWidget* m_parent;
    std::unordered_set<int> searchResults; 

    QList<::PluginThumbnail*> pluginThumbnails;

    bool m_connected;
};
    
PluginListWidget::PluginListWidget() : ui(new Ui::PluginListWidget)
{
    ui->setupUi(this);

    ui->searchResults.insert(-1); // Default to showing all plugins
}

void PluginListWidget::UpdateUi(int repoStatus)
{
    ui->updateUi(repoStatus);
}

void PluginListWidget::AddPlugin(const Plugin& plugin)
{
    PluginThumbnail* thumbnail = new PluginThumbnail(&plugin);
    thumbnail->setInstalled(plugin.localCopy);
    connect(thumbnail, &::PluginThumbnail::clicked, this, &PluginListWidget::on_pluginThumbnailClicked);
    ui->pluginThumbnails.append(thumbnail);
}

void PluginListWidget::RemovePlugin(int id)
{
    bool found = false;
    int index;
    for(index = 0; index < ui->pluginThumbnails.size(); index++)
    {
        if(ui->pluginThumbnails[index]->getID() == id)
        {
            found = true;
            break;
        }
    }

    if(found)
    {
        ui->pluginThumbnails[index]->deleteLater();
        ui->pluginThumbnails.remove(index);

        ui->updateUi();
    }
}

void PluginListWidget::Clear()
{
    qDeleteAll(ui->pluginThumbnails);
    ui->pluginThumbnails.clear();

    ui->searchResults.clear();
    ui->searchResults.insert(-1);

    ui->updateUi();
}

void PluginListWidget::SetSearchResults(const std::unordered_set<int>& results)
{
    ui->searchResults = results;

    ui->updateUi();
}

void PluginListWidget::SetPluginInstalled(int id, bool installed)
{
    for(auto plugin : ui->pluginThumbnails)
    {
        if(plugin->getID() == id)
        {
            plugin->setInstalled(installed);
            return;
        }
    }
}

void PluginListWidget::SetPluginStatus(int id, int status)
{
    for(auto plugin : ui->pluginThumbnails)
    {
        if(plugin->getID() == id)
        {
            plugin->SetStatus(status);
            return;
        }
    }
}

void PluginListWidget::SetPluginProgress(int id, float progress)
{
    for(auto plugin : ui->pluginThumbnails)
    {
        if(plugin->getID() == id)
        {
            plugin->SetProgress(progress);
            return;
        }
    }
}

void PluginListWidget::on_pluginThumbnailClicked(int id)
{
    ui->statusThumbnail->setSelected(id==0);
    ui->statusThumbnail->update();

    for(auto plugin : ui->pluginThumbnails)
    {
        if(plugin->getID() == id)
        {
            plugin->setSelected(true);
        }
        else
        {
            plugin->setSelected(false);
        }

        plugin->update();
    }

    emit pluginThumbnailClicked(id);
}
