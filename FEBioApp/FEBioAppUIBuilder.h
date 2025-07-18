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
#pragma once
#include <QWidget>

class QBoxLayout;
class XMLTag;
class FEBioApp;
class FEBioAppWidget;

class FEBioAppUIBuilder
{
public:
	FEBioAppUIBuilder();

	FEBioAppWidget* BuildUIFromFile(QString filePath, FEBioApp* app);

private:
	FEBioAppWidget* error();

	bool parseModel(XMLTag& tag);
	bool parseGUI  (XMLTag& tag);

	bool parseGUITags  (XMLTag& tag, QBoxLayout* layout);
	void parseLabel    (XMLTag& tag, QBoxLayout* layout);
	void parseButton   (XMLTag& tag, QBoxLayout* layout);
	void parseGraph    (XMLTag& tag, QBoxLayout* layout);
	void parseInputList(XMLTag& tag, QBoxLayout* layout);
	void parseVGroup   (XMLTag& tag, QBoxLayout* playout);
	void parseHGroup   (XMLTag& tag, QBoxLayout* playout);
	void parseStretch  (XMLTag& tag, QBoxLayout* playout);
	void parseInput    (XMLTag& tag, QBoxLayout* playout);
	void parseTabGroup (XMLTag& tag, QBoxLayout* playout);
	void parsePlot3d   (XMLTag& tag, QBoxLayout* playout);
	void parseOutput   (XMLTag& tag, QBoxLayout* playout);

private:
	FEBioAppWidget* ui;
	FEBioApp* app;
	QString	m_appFolder;
};
