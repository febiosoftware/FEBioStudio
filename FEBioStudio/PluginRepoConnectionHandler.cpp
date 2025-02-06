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

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include "PluginRepoConnectionHandler.h"
#include "MainWindow.h"
#include "ServerSettings.h"
#include "LocalDatabaseHandler.h"

#include <iostream>

#define API_URL "/pluginRepo/api/v1.00/"

class CPluginRepoConnectionHandler::Imp
{
public:
	Imp(CRepositoryPanel* dbPanel, CPluginRepoConnectionHandler* handler)
		: dbPanel(dbPanel), uploadPermission(0), sizeLimit(0), authenticated(false), uploadReady(false)
	{
	}

	void loggedOut()
	{
		username = "";
		token = "";
		uploadPermission = 0;
		sizeLimit = 0;
		authenticated = false;

		fileToken = "";
		uploadReady = false;

		// dbPanel->LoginTimeout();
	}

	CRepositoryPanel* dbPanel;

	QString username;
	QString token;
	int uploadPermission;
	qint64 sizeLimit;
	bool authenticated;

	QString fileToken;
	bool uploadReady;
};


CPluginRepoConnectionHandler::CPluginRepoConnectionHandler(CRepositoryPanel* dbPanel, CLocalDatabaseHandler* dbHandler, CMainWindow* wnd)
    : CRepoConnectionHandler(API_URL, dbHandler, wnd), imp(new Imp(dbPanel, this))
{

}

CPluginRepoConnectionHandler::~CPluginRepoConnectionHandler()
{
	delete imp;
}

void CPluginRepoConnectionHandler::handleReply(QNetworkReply *r)
{

}

void CPluginRepoConnectionHandler::tablesReady()
{

}

void CPluginRepoConnectionHandler::outOfDate()
{
    
}