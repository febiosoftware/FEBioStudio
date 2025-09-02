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

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QAction>
#include <QToolButton>
#include <QMouseEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileDialog>
#include <QJsonDocument>
#include <QMessageBox>
#include <QStackedWidget>
#include <QProgressBar>
#include "TagWidget.h"
#include "DlgSubmitPlugin.h"
#include "PublicationWidgetView.h"
#include "PluginManager.h"

class ImageDisplayWidget : public QFrame
{
public:
    ImageDisplayWidget()
    {
        QVBoxLayout* layout = new QVBoxLayout;

        instructions = new QLabel("Click or drag and drop to select an image for your plugin.");
        instructions->setAlignment(Qt::AlignCenter);
        instructions->setWordWrap(true);
        layout->addWidget(instructions);

        layout->addWidget(imageLabel = new QLabel);
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setVisible(false);

        setLayout(layout);

        setFixedSize(200, 200);
        setFrameStyle(QFrame::StyledPanel);

        setAcceptDrops(true);
    }

    QString GetFilename()
    {
        return filename;
    }

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        if(event->button() == Qt::LeftButton) 
        {
            QString fname = QFileDialog::getOpenFileName(nullptr, "Choose Plugin Thumbnail", QString(), "Images (*.png *.jpg)");

            if(fname.isEmpty()) return;

            QPixmap pixmap;
            if(pixmap.load(fname))
            {
                filename = fname;

                imageLabel->setPixmap(pixmap.scaledToHeight(200, Qt::SmoothTransformation));

                instructions->hide();
                imageLabel->show();
            }
            
        }
    }
    
    void dragEnterEvent(QDragEnterEvent *event)
    {
        if(event->mimeData()->hasUrls()) 
        { 
            event->acceptProposedAction();
        } 
        else 
        {
            event->ignore();
        }
    }

    void dragMoveEvent(QDragMoveEvent *event)
    {
        if(event->mimeData()->hasUrls()) 
        { 
            event->acceptProposedAction();
        } 
        else 
        {
            event->ignore();
        }
    }

    void dropEvent(QDropEvent* event) override
    {
        auto mimeData = event->mimeData();

        if(mimeData->hasUrls())
        {
            QPixmap pixmap;
            if(pixmap.load(mimeData->urls().first().toLocalFile()))
            {
                imageLabel->setPixmap(pixmap.scaledToHeight(200, Qt::SmoothTransformation));

                filename = mimeData->urls().first().toLocalFile();

                instructions->hide();
                imageLabel->show();
            }
        }
    }

private:
    QLabel* instructions;
    QLabel* imageLabel;
    QString filename;

};

class Ui::DlgSubmitPlugin
{
public:
    QStackedWidget* stack;

    QLabel* progressLabel;
    QProgressBar* progressBar;

    QLineEdit* nameEdit;
    QLineEdit* repoURLEdit;
    QLineEdit* userNameEdit;    
    QLineEdit* emailEdit;
    QTextEdit* descriptionEdit;

	::TagWidget* tags;

	::CPublicationWidgetView* pubs;
    ImageDisplayWidget* image;

    QDialogButtonBox* buttonBox;

public:
    void SetupUi(::DlgSubmitPlugin* parent, QStringList completerList)
    {
        QVBoxLayout* layout = new QVBoxLayout;

        stack = new QStackedWidget;

        QWidget* page1 = new QWidget;
        QVBoxLayout* page1Layout = new QVBoxLayout;
        page1Layout->setContentsMargins(0,0,0,0);

        QHBoxLayout* hLayout = new QHBoxLayout;
        hLayout->setContentsMargins(0,0,0,0);

        QVBoxLayout* leftLayout = new QVBoxLayout;

        QFormLayout* topForm = new QFormLayout;
        topForm->setContentsMargins(0,0,0,0);

        topForm->addRow("Plugin name:", nameEdit = new QLineEdit);
        topForm->addRow("GitHub URL:", repoURLEdit = new QLineEdit);

        leftLayout->addLayout(topForm);

        leftLayout->addWidget(new QLabel("Plugin Thumbnail:"));
        leftLayout->addWidget(image = new ImageDisplayWidget, 0, Qt::AlignHCenter);

        QFormLayout* bottomForm = new QFormLayout;
        bottomForm->setContentsMargins(0,0,0,0);

        bottomForm->addRow("User name:", userNameEdit = new QLineEdit);
        bottomForm->addRow("User email:", emailEdit = new QLineEdit);

        leftLayout->addLayout(bottomForm);
        leftLayout->addStretch();

        hLayout->addLayout(leftLayout);

        QVBoxLayout* middleLayout = new QVBoxLayout;

        middleLayout->addWidget(new QLabel("Description:"));
        middleLayout->addWidget(descriptionEdit = new QTextEdit);

        hLayout->addLayout(middleLayout);

        QVBoxLayout* rightLayout = new QVBoxLayout;
        rightLayout->addWidget(tags = new ::TagWidget);
        tags->SetTagCompleter(completerList);

        rightLayout->addWidget(new QLabel("Publications:"));

		pubs = new ::CPublicationWidgetView(::CPublicationWidgetView::EDITABLE, true, true);
		rightLayout->addWidget(pubs);

        hLayout->addLayout(rightLayout);

        page1Layout->addLayout(hLayout);
        page1->setLayout(page1Layout);
        stack->addWidget(page1);

        QWidget* page2 = new QWidget;
        QHBoxLayout* page2Layout = new QHBoxLayout;
        page2Layout->setContentsMargins(0,0,0,0);
        page2Layout->setAlignment(Qt::AlignCenter);
        
        page2Layout->addStretch(1);
        
        QVBoxLayout* progressLayout = new QVBoxLayout;
        progressLayout->setContentsMargins(0,0,0,0);
        progressLayout->addWidget(progressLabel = new QLabel);
        progressLayout->addWidget(progressBar = new QProgressBar);
        
        page2Layout->addLayout(progressLayout, 2);
        
        page2Layout->addStretch(1);

        page2->setLayout(page2Layout);
        stack->addWidget(page2);

        layout->addWidget(stack);

        layout->addWidget(buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel));

        parent->setLayout(layout);

        uploadSucceeded = false;
        uploadInProgress = false;

        QObject::connect(buttonBox, &QDialogButtonBox::accepted, parent, &::DlgSubmitPlugin::accept);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, parent, &::DlgSubmitPlugin::reject);
    }

public:
    bool uploadInProgress;
    bool uploadSucceeded;

};

DlgSubmitPlugin::DlgSubmitPlugin(CPluginManager* manager) 
    : m_manager(manager), ui(new Ui::DlgSubmitPlugin)
{
    QStringList completerList;
    m_manager->GetTags(completerList);

    ui->SetupUi(this, completerList);

    connect(m_manager, &CPluginManager::UploadFinished, this, &DlgSubmitPlugin::OnUploadFinished);
    connect(m_manager, &CPluginManager::ReadyForImageUpload, this, &DlgSubmitPlugin::OnReadyForImageUpload);
    connect(m_manager, &CPluginManager::HTMLError, this, &DlgSubmitPlugin::OnHTMLError);
    connect(m_manager, &CPluginManager::uploadProgress, this, &DlgSubmitPlugin::uploadProgress);
}

void DlgSubmitPlugin::accept()
{
    if(ui->stack->currentIndex() == 1)
    {
        if(ui->uploadSucceeded)
        {
            QDialog::accept();
            return;
        }

        ui->stack->setCurrentIndex(0);
        return;
    }


    QString username = ui->userNameEdit->text();
    QString email = ui->emailEdit->text();
    QString name = ui->nameEdit->text();
    QString url = ui->repoURLEdit->text();
    QString description = ui->descriptionEdit->toPlainText();
    QString imageFilename = ui->image->GetFilename();

    // Check if fields are empty.
    if(username.isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please enter your name.");
		return;
	}

    if(email.isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please enter a contact email. This will not be made public and will only be used by our team.");
		return;
	}
    
    if(name.isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please enter a name for your plugin.");
		return;
	}

    if(url.isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please enter a GitHub URL pointing to your plugin's source code.");
		return;
	}

    if(description.isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please enter a description for your plugin.");
		return;
	}

    if(imageFilename.isEmpty())
	{
		QMessageBox::critical(this, "Upload", "Please provide a valid image file for your plugin.");
		return;
	}

    // Check fields for invalid characters
    std::string dissallowed = "<>:\"\\/|?*.";
    QString fieldName = "";
	for(auto c : dissallowed)
	{
		if(username.contains(c))
		{
			fieldName = "User name";
            break;
		}
        if(name.contains(c))
		{
			fieldName = "Plugin name";
            break;
		}
	}

    if(!fieldName.isEmpty())
    {
        QString message = QString("The \"%1\" field cannot contain any of the following symbols:\n\n. < > : \" \\ / | ? *").arg(fieldName);

        QMessageBox::critical(this, "Upload", message);
        return;
    }

    // Check validity of filename and URL
    QFileInfo info(imageFilename);
    if(!info.exists())
    {
        QString message = QString("The image file:\n\n%1\n\ndoes not exist").arg(imageFilename);
        QMessageBox::critical(this, "Upload", message);
		return;
    }

    QUrl repoUrl(url);
    if(!repoUrl.isValid())
    {
        QString message = QString("The given GitHub URL:\n\n%1\n\nis not a valid URL").arg(url);
        QMessageBox::critical(this, "Upload", message);
		return;
    }

    if(m_manager->IsPluginNameInUse(name))
    {
        QMessageBox::critical(this, "Upload", "A plugin with this name already exists in the repository"
				"n\nPlease choose a different name.");
        return;
    }
    
	if(ui->tags->Count() == 0)
	{
		QMessageBox::critical(this, "Upload", "Please add at least one tag to your plugin.");

		return;
	}

    if(ui->pubs->count() == 0)
	{
		QMessageBox::StandardButton reply = QMessageBox::question(this, "Upload", "You have not associated any publications with your plugin."
				"\n\nWould you like to upload anyway?");

		if(reply != QMessageBox::Yes)
		{
			return;
		}
	}

    SubmitPlugin();
}

void DlgSubmitPlugin::reject()
{
    if(ui->stack->currentIndex() == 1)
    {
        if(ui->uploadInProgress)
        {
            m_manager->CancelUpload();
        }

        ui->stack->setCurrentIndex(0);
        return;
    }

    QDialog::reject();
}

QByteArray DlgSubmitPlugin::GetPluginJSON()
{
    QVariantMap pluginInfo;

    pluginInfo["username"] = ui->userNameEdit->text();
    pluginInfo["userEmail"] = ui->emailEdit->text();
    pluginInfo["name"] = ui->nameEdit->text();
	pluginInfo["repoURL"] = ui->repoURLEdit->text();
	pluginInfo["description"] = ui->descriptionEdit->toPlainText();
	pluginInfo["type"] = 1; // This means it's a FEBio plugin as opposed to a Python plugin. Need to update later.
	pluginInfo["tags"] = ui->tags->GetTags();
	pluginInfo["publications"] = ui->pubs->getPublicationInfo();

	return QJsonDocument::fromVariant(pluginInfo).toJson();
}

void DlgSubmitPlugin::SubmitPlugin()
{
    ui->progressLabel->setText("Uploading Plugin Metadata...");
    ui->progressBar->show();
    ui->stack->setCurrentIndex(1);
    ui->uploadInProgress = true;

    QByteArray pluginInfo = GetPluginJSON();
    m_manager->SumbitPlugin(pluginInfo);
}

void DlgSubmitPlugin::OnReadyForImageUpload(QByteArray& token)
{
    ui->progressLabel->setText("Uploading Image File...");
    ui->progressBar->show();

    QString filename = ui->image->GetFilename();

    m_manager->UploadImage(token, filename);
}

void DlgSubmitPlugin::OnUploadFinished(QString& message)
{
    ui->progressBar->hide();
    ui->progressLabel->setText(message);
    ui->uploadSucceeded = true;
    ui->uploadInProgress = false;
}

void DlgSubmitPlugin::OnHTMLError(QString& message)
{
    ui->progressBar->hide();
    ui->progressLabel->setText(message);
    ui->uploadSucceeded = false;
    ui->uploadInProgress = false;
}

void DlgSubmitPlugin::uploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    ui->progressBar->setRange(0, bytesTotal);
    ui->progressBar->setValue(bytesSent);
}