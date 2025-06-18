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

#include <QWidget>

class QLineEdit;
class QScrollArea;
class QGridLayout;
struct Plugin;

namespace Ui {
    class PluginListWidget;
}

class PluginThumbnail : public QWidget 
{
    Q_OBJECT

public:
    PluginThumbnail(const Plugin& plugin);

    void SetStatus(int status);

    int getID();

signals:
    void clicked(int id);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    Ui::PluginListWidget* ui;
};

class PluginListWidget : public QWidget 
{
    Q_OBJECT

public:
    PluginListWidget();

    void AddPlugin(const Plugin& plugin);

    void Clear();

signals:
    void pluginThumbnailClicked(int id);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QLineEdit* searchInput;
    QScrollArea* scrollArea;
    QGridLayout* gridLayout;
    QList<PluginThumbnail*> pluginCards;

    void updateGridLayout();
};
