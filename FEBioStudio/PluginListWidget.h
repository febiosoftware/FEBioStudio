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
#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QResizeEvent>

class PluginCard : public QWidget {
public:
    PluginCard(const QString& imagePath, const QString& name, const QString& category, const QString& description, QWidget* parent = nullptr)
        : QWidget(parent) {
        QVBoxLayout* layout = new QVBoxLayout(this);
        QLabel* imageLabel = new QLabel(this);
        imageLabel->setPixmap(QPixmap(imagePath).scaled(150, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QLabel* nameLabel = new QLabel("<b>" + name + "</b>", this);
        QLabel* categoryLabel = new QLabel("<i>" + category + "</i>", this);
        QLabel* descriptionLabel = new QLabel(description, this);
        descriptionLabel->setWordWrap(true);

        layout->addWidget(imageLabel);
        layout->addWidget(nameLabel);
        layout->addWidget(categoryLabel);
        layout->addWidget(descriptionLabel);
        setLayout(layout);
        setFixedSize(200, 200);
    }
};

class PluginListWidget : public QWidget {
    Q_OBJECT

public:
    PluginListWidget(QWidget* parent = nullptr) : QWidget(parent) {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // Search Bar
        QHBoxLayout* searchLayout = new QHBoxLayout();
        searchInput = new QLineEdit(this);
        searchInput->setPlaceholderText("Search plugins...");
        QPushButton* searchButton = new QPushButton("Search", this);

        searchLayout->addWidget(searchInput);
        searchLayout->addWidget(searchButton);

        mainLayout->addLayout(searchLayout);

        // Scrollable Plugin Grid
        scrollArea = new QScrollArea(this);
        QWidget* scrollWidget = new QWidget();
        gridLayout = new QGridLayout(scrollWidget);
        scrollWidget->setLayout(gridLayout);

        scrollArea->setWidget(scrollWidget);
        scrollArea->setWidgetResizable(true);
        mainLayout->addWidget(scrollArea);

        setLayout(mainLayout);

        populatePlugins();
    }

protected:
    void resizeEvent(QResizeEvent* event) override {
        QWidget::resizeEvent(event);
        updateGridLayout();
    }

private:
    QLineEdit* searchInput;
    QScrollArea* scrollArea;
    QGridLayout* gridLayout;
    QList<PluginCard*> pluginCards;

    void populatePlugins() {
        // Example plugin data
        struct Plugin {
            QString image;
            QString name;
            QString category;
            QString description;
        };

        QList<Plugin> plugins = {
            {"hydrogel-plugin-logo", "Plugin Name 1", "Category 1", "Short description of the plugin."},
            {"febioheat", "Plugin Name 2", "Category 2", "Short description of the plugin."},
            {"FEBioChemOpt", "Plugin Name 3", "Category 3", "Short description of the plugin."},
            {"fewarp", "Plugin Name 3", "Category 3", "Short description of the plugin."},
            {"datamap_icon", "Plugin Name 3", "Category 3", "Short description of the plugin."},
            {"pl", "Plugin Name 3", "Category 3", "Short description of the plugin."}
        };

        for (const Plugin& plugin : plugins) {
            PluginCard* card = new PluginCard(plugin.image, plugin.name, plugin.category, plugin.description, this);
            pluginCards.append(card);
        }

        updateGridLayout();
    }

    void updateGridLayout() {
        int columns = width() / 220; // Adjust columns based on available width
        if (columns < 1) columns = 1;

        QLayoutItem* item;
        while ((item = gridLayout->takeAt(0)) != nullptr) {
            delete item;
        }

        int row = 0, col = 0;
        for (PluginCard* card : pluginCards) {
            gridLayout->addWidget(card, row, col);
            col++;
            if (col >= columns) {
                col = 0;
                row++;
            }
        }
    }
};
