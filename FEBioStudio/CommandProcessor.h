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
#include <QString>
#include <QStringList>
#include <FSCore/color.h>
#include <vector>

class CMainWindow;
class CDocument;
class CModelDocument;
class CPostDocument;

//! The command input class is used to provide a mechanism for the command processor
//! to interact with the user. In GUI mode, this will ofen result in a dialog box
//! being presented. In CLI mode, the user can add the inputs on the command line. 
class CommandInput
{
public:
	CommandInput() {}
	virtual ~CommandInput() {}

	virtual QString GetOpenModelFilename() = 0; // return the filename of the model file to open
	virtual QString GetSaveModelFilename() = 0; // return the filename of the model file to save
	virtual QString GetExportGeometryFilename(QString& formatOption) = 0;
	virtual QString GetExportFEModelFilename(QString& formatOption) = 0;
};

class CommandProcessor
{
private:
	struct CCommandDescriptor
	{
		QString name;	// name of command
		bool(CommandProcessor::* f)(QStringList);
		QString brief;	// brief description
	};

public:
	CommandProcessor(CMainWindow* wnd, CommandInput* cmdinput);
	QString GetCommandOutput() { return m_output; }
	bool ProcessCommandLine(QString cmdLine);
	bool RunCommand(QString cmd, QStringList ops);
	QStringList ParseCommandLine(QString cmd);

public: // command functions
	bool cmd_addbc  (QStringList ops);
	bool cmd_addbl  (QStringList ops);
	bool cmd_addci  (QStringList ops);
	bool cmd_adddata(QStringList ops);
	bool cmd_addmat (QStringList ops);
	bool cmd_addnl  (QStringList ops);
	bool cmd_addsl  (QStringList ops);
	bool cmd_addstep(QStringList ops);
	bool cmd_anim   (QStringList ops);
	bool cmd_assign (QStringList ops);
	bool cmd_bgcol  (QStringList ops);
	bool cmd_bgcol1 (QStringList ops);
	bool cmd_bgcol2 (QStringList ops);
	bool cmd_bgstyle(QStringList ops);
	bool cmd_close  (QStringList ops);
	bool cmd_cmd    (QStringList ops);
	bool cmd_create (QStringList ops);
	bool cmd_exit   (QStringList ops);
	bool cmd_export (QStringList ops);
	bool cmd_expgeo (QStringList ops);
	bool cmd_fgcol  (QStringList ops);
	bool cmd_first  (QStringList ops);
	bool cmd_grid   (QStringList ops);
	bool cmd_help   (QStringList ops);
	bool cmd_import (QStringList ops);
	bool cmd_job    (QStringList ops);
	bool cmd_genmesh(QStringList ops);
	bool cmd_last   (QStringList ops);
	bool cmd_new    (QStringList ops);
	bool cmd_next   (QStringList ops);
	bool cmd_open   (QStringList ops);
	bool cmd_prev   (QStringList ops);
	bool cmd_reset  (QStringList ops);
	bool cmd_save   (QStringList ops);
	bool cmd_stop   (QStringList ops);
	bool cmd_selpart(QStringList ops);
	bool cmd_selsurf(QStringList ops);

private: // error messages
	bool Error(const QString& msg) { m_output = msg; return false; }
	bool Success(const QString& msg) { m_output = msg; return true; }

	bool NoActiveDoc() { return Error("No model active."); }
	bool GLViewIsNull() { return Error("Graphics View not available."); }
	bool InvalidArgsCount() { return Error("Invalid number of arguments."); }
	bool CommandCancelled() { return Error("Command was cancelled."); }

private:
	bool ValidateArgs(const QStringList& ops, int minargs = -1, int maxargs = -1);
	bool ValidateArgs(const QStringList& ops, const std::vector<int>& validSizes);
	bool CmdToColor(QStringList ops, GLColor& c);
	bool RunCommandFile(QString cmdFile, QStringList ops);

private:
	CDocument* GetActiveDocument();
	CModelDocument* GetModelDocument();
	CPostDocument* GetPostDocument();

private:
	CommandInput* m_cmdInput;
	CMainWindow* m_wnd; // TODO: remove
	QString m_output;
	std::vector<CCommandDescriptor> m_cmds;
};
