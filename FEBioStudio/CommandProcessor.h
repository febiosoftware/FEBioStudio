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
class FSMesh;

enum class CMD_RETURN_CODE {
	CMD_ERROR,
	CMD_SUCCESS,
	CMD_IGNORE
};

class CommandProcessor
{
private:
	struct CCommandDescriptor
	{
		QString name;	// name of command
		CMD_RETURN_CODE (CommandProcessor::* f)(QStringList);
		QString brief;	// brief description
	};

public:
	CommandProcessor(CMainWindow* wnd);
	QString GetCommandOutput() { return m_output; }
	CMD_RETURN_CODE ProcessCommandLine(QString cmdLine);
	CMD_RETURN_CODE RunCommand(QString cmd, QStringList ops);
	QStringList ParseCommandLine(QString cmd);

public: // command functions
	CMD_RETURN_CODE cmd_addbc     (QStringList ops);
	CMD_RETURN_CODE cmd_addbl     (QStringList ops);
	CMD_RETURN_CODE cmd_addci     (QStringList ops);
	CMD_RETURN_CODE cmd_adddata   (QStringList ops);
	CMD_RETURN_CODE cmd_addic     (QStringList ops);
	CMD_RETURN_CODE cmd_addmat    (QStringList ops);
	CMD_RETURN_CODE cmd_addnl     (QStringList ops);
	CMD_RETURN_CODE cmd_addnlc    (QStringList ops);
	CMD_RETURN_CODE cmd_addsl     (QStringList ops);
	CMD_RETURN_CODE cmd_addstep   (QStringList ops);
	CMD_RETURN_CODE cmd_anim      (QStringList ops);
	CMD_RETURN_CODE cmd_assign    (QStringList ops);
	CMD_RETURN_CODE cmd_bgcol     (QStringList ops);
	CMD_RETURN_CODE cmd_bgcol1    (QStringList ops);
	CMD_RETURN_CODE cmd_bgcol2    (QStringList ops);
	CMD_RETURN_CODE cmd_bgstyle   (QStringList ops);
	CMD_RETURN_CODE cmd_close     (QStringList ops);
	CMD_RETURN_CODE cmd_cmd       (QStringList ops);
	CMD_RETURN_CODE cmd_create    (QStringList ops);
	CMD_RETURN_CODE cmd_exit      (QStringList ops);
	CMD_RETURN_CODE cmd_export    (QStringList ops);
	CMD_RETURN_CODE cmd_expgeo    (QStringList ops);
	CMD_RETURN_CODE cmd_fgcol     (QStringList ops);
	CMD_RETURN_CODE cmd_first     (QStringList ops);
	CMD_RETURN_CODE cmd_grid      (QStringList ops);
	CMD_RETURN_CODE cmd_help      (QStringList ops);
	CMD_RETURN_CODE cmd_import    (QStringList ops);
	CMD_RETURN_CODE cmd_job       (QStringList ops);
	CMD_RETURN_CODE cmd_genmesh   (QStringList ops);
	CMD_RETURN_CODE cmd_last      (QStringList ops);
	CMD_RETURN_CODE cmd_new       (QStringList ops);
	CMD_RETURN_CODE cmd_next      (QStringList ops);
	CMD_RETURN_CODE cmd_open      (QStringList ops);
	CMD_RETURN_CODE cmd_prev      (QStringList ops);
	CMD_RETURN_CODE cmd_reset     (QStringList ops);
	CMD_RETURN_CODE cmd_save      (QStringList ops);
	CMD_RETURN_CODE cmd_sel       (QStringList ops);
	CMD_RETURN_CODE cmd_selconnect(QStringList ops);
	CMD_RETURN_CODE cmd_selpart   (QStringList ops);
	CMD_RETURN_CODE cmd_selsurf   (QStringList ops);
	CMD_RETURN_CODE cmd_stop      (QStringList ops);

private: // error messages
	CMD_RETURN_CODE Error(const QString& msg) { m_output = msg; return CMD_RETURN_CODE::CMD_ERROR; }
	CMD_RETURN_CODE Success(const QString& msg) { m_output = msg; return CMD_RETURN_CODE::CMD_SUCCESS; }

	CMD_RETURN_CODE NoActiveDoc() { return Error("No model active."); }
	CMD_RETURN_CODE NoActiveMesh() { return Error("No active mesh."); }
	CMD_RETURN_CODE GLViewIsNull() { return Error("Graphics View not available."); }
	CMD_RETURN_CODE InvalidArgsCount() { return Error("Invalid number of arguments."); }
	CMD_RETURN_CODE InvalidArgument() { return Error("Invalid argument."); }
	CMD_RETURN_CODE CommandCancelled() { return Error("Command was cancelled."); }

private:
	bool ValidateArgs(const QStringList& ops, int minargs = -1, int maxargs = -1);
	bool ValidateArgs(const QStringList& ops, const std::vector<int>& validSizes);
	bool CmdToColor(QStringList ops, GLColor& c);
	CMD_RETURN_CODE RunCommandFile(QString cmdFile, QStringList ops);

private:
	CDocument* GetActiveDocument();
	CModelDocument* GetModelDocument();
	CPostDocument* GetPostDocument();
	FSMesh* GetActiveMesh();

private:
	CMainWindow* m_wnd; // TODO: remove
	QString m_output;
	std::vector<CCommandDescriptor> m_cmds;
};
