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
#include "stdafx.h"
#include "CommandWindow.h"
#include "MainWindow.h"
#include "DocManager.h"
#include "ModelDocument.h"
#include "units.h"
#include "Commands.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <FEBioLink/FEBioModule.h>
#include <FEBioLink/FEBioClass.h>
#include <GeomLib/GPrimitive.h>
#include <FECore/MathObject.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <MeshTools/FEMesher.h>
#include <sstream>

class CommandProcessor
{
private:
	struct CCommandDescriptor
	{
		QString name;	// name of command
		QString brief;	// brief description
		bool(CommandProcessor::*f)(QStringList);
	};

	std::vector<CCommandDescriptor> m_cmds;

public:
	CommandProcessor(CMainWindow* wnd) : m_wnd(wnd) 
	{
		m_cmds.push_back({ "addbc"  , "adds a boundary condition to the current model", &CommandProcessor::RunAddBCCmd });
		m_cmds.push_back({ "addbl"  , "adds a body load to the current model", &CommandProcessor::RunAddBodyLoadCmd });
		m_cmds.push_back({ "addmat" , "adds a material to the model", &CommandProcessor::RunAddMaterialCmd });
		m_cmds.push_back({ "addnl"  , "adds a nodal load to the current model", &CommandProcessor::RunAddSurfaceLoadCmd });
		m_cmds.push_back({ "addsl"  , "adds a surface load to the current model", &CommandProcessor::RunAddNodalLoadCmd });
		m_cmds.push_back({ "addstep", "adds a step to the current model", &CommandProcessor::RunAddStepCmd });
		m_cmds.push_back({ "assign" , "assigns the current selection to the active item in the model tree", &CommandProcessor::RunAssignCmd });
		m_cmds.push_back({ "close"  , "closes the current model", &CommandProcessor::RunCloseCmd });
		m_cmds.push_back({ "cmd"    , "run a command script", &CommandProcessor::RunCmdCmd });
		m_cmds.push_back({ "create" , "add a primitive to the current model", &CommandProcessor::RunCreateCmd });
		m_cmds.push_back({ "exit"   , "closes FEBio Studio", &CommandProcessor::RunExitCmd });
		m_cmds.push_back({ "import" , "import a geometry file", &CommandProcessor::RunImportCmd });
		m_cmds.push_back({ "job"    , "run the model in FEBio", &CommandProcessor::RunJobCmd });
		m_cmds.push_back({ "grid"   , "run the model in FEBio", &CommandProcessor::RunGridCmd });
		m_cmds.push_back({ "help"   , "show help", &CommandProcessor::RunHelpCmd });
		m_cmds.push_back({ "mesh"   , "generate mesh for currently selected object.", &CommandProcessor::RunMeshCmd });
		m_cmds.push_back({ "new"    , "create a new model", &CommandProcessor::RunNewCmd });
		m_cmds.push_back({ "open"   , "open a file", &CommandProcessor::RunOpenCmd });
		m_cmds.push_back({ "save"   , "save the current model", &CommandProcessor::RunSaveCmd });
		m_cmds.push_back({ "selsurf", "select a surface", &CommandProcessor::RunSelectSurfCmd });
	}

	QString GetCommandOutput() { return m_output; }

	bool ProcessCommandLine(QString cmdLine)
	{
		QStringList cmdAndOps = ParseCommandLine(cmdLine);
		if (cmdAndOps.empty()) return true;
		QString cmd = cmdAndOps[0];
		QStringList ops = cmdAndOps; ops.pop_front();
		return RunCommand(cmd, ops);
	}

	bool RunCommand(QString cmd, QStringList ops)
	{
		m_output.clear();
		for (auto& entry : m_cmds)
		{
			if (cmd == entry.name)
			{
				return (*this.*(entry.f))(ops);
			}
		}
		return Error(QString("Unknown command: %1").arg(cmd));
	}

public: // command functions
	bool RunAddBCCmd(QStringList ops)
	{
		CModelDocument* doc = m_wnd->GetModelDocument();
		if (doc == nullptr) return Error("No model active");

		if (ops.isEmpty()) m_wnd->on_actionAddNodalBC_triggered();
		else
		{
			FSProject& prj = doc->GetProject();
			FSModel& fem = prj.GetFSModel();

			std::string typeStr = ops[0].toStdString();
			FSBoundaryCondition* pbc = FEBio::CreateBoundaryCondition(typeStr, &fem);
			if (pbc == nullptr) return Error(QString("Can't create bc type: %1").arg(ops[0]));
			FEBio::InitDefaultProps(pbc);
			pbc->SetName(defaultBCName(&fem, pbc));

			ops.pop_front();
			if (SetParameters(pbc, ops) == false)
			{ 
				delete pbc; 
				return false;
			}

			FSStep* step = fem.GetStep(0);
			doc->DoCommand(new CCmdAddBC(step, pbc), pbc->GetNameAndType());
			m_wnd->UpdateModel(pbc);
		}
		return true;
	}

	bool RunAddMaterialCmd(QStringList ops)
	{
		CModelDocument* doc = m_wnd->GetModelDocument();
		if (doc == nullptr) return Error("No model active");
		if (ops.empty()) m_wnd->on_actionAddMaterial_triggered();
		else
		{
			FSModel* fem = doc->GetFSModel();
			string type = ops[0].toStdString();
			FSMaterial* pmat = FEBio::CreateMaterial(type, fem);
			if (pmat == nullptr) return Error(QString("Don't know material \"%1\"").arg(ops[0]));

			ops.pop_front();
			SetParameters(pmat, ops);

			GMaterial* gmat = new GMaterial(pmat);
			doc->DoCommand(new CCmdAddMaterial(fem, gmat), gmat->GetNameAndType());
			m_wnd->UpdateModel(gmat);
			return true;
		}
	}

	bool RunAddBodyLoadCmd(QStringList ops)
	{
		CModelDocument* doc = m_wnd->GetModelDocument();
		if (doc == nullptr) return Error("No model active");

		if (ops.isEmpty()) m_wnd->on_actionAddBodyLoad_triggered();
		else
		{
			FSProject& prj = doc->GetProject();
			FSModel& fem = prj.GetFSModel();

			std::string typeStr = ops[0].toStdString();
			FSBodyLoad* pbl = FEBio::CreateBodyLoad(typeStr, &fem);
			if (pbl == nullptr) return Error(QString("Can't create load type: %1").arg(ops[0]));
			FEBio::InitDefaultProps(pbl);
			pbl->SetName(defaultLoadName(&fem, pbl));

			ops.pop_front();
			if (SetParameters(pbl, ops) == false)
			{
				delete pbl;
				return false;
			}

			FSStep* step = fem.GetStep(0);
			doc->DoCommand(new CCmdAddLoad(step, pbl), pbl->GetNameAndType());
			m_wnd->UpdateModel(pbl);
		}
		return true;
	}

	bool RunAddNodalLoadCmd(QStringList ops)
	{
		CModelDocument* doc = m_wnd->GetModelDocument();
		if (doc == nullptr) return Error("No model active");

		if (ops.isEmpty()) m_wnd->on_actionAddNodalLoad_triggered();
		else
		{
			FSProject& prj = doc->GetProject();
			FSModel& fem = prj.GetFSModel();

			std::string typeStr = ops[0].toStdString();
			FSNodalLoad* pbc = FEBio::CreateNodalLoad(typeStr, &fem);
			if (pbc == nullptr) return Error(QString("Can't create load type: %1").arg(ops[0]));
			FEBio::InitDefaultProps(pbc);
			pbc->SetName(defaultLoadName(&fem, pbc));

			ops.pop_front();
			if (SetParameters(pbc, ops) == false)
			{
				delete pbc;
				return false;
			}

			FSStep* step = fem.GetStep(0);
			doc->DoCommand(new CCmdAddLoad(step, pbc), pbc->GetNameAndType());
			m_wnd->UpdateModel(pbc);
		}
		return true;
	}

	bool RunAddStepCmd(QStringList ops)
	{
		CModelDocument* doc = m_wnd->GetModelDocument();
		if (doc == nullptr) return Error("No model active");

		FSModel* fem = doc->GetFSModel();
		FSStep* ps = FEBio::CreateStep(FEBio::GetActiveModuleName(), fem);
		if (ps == nullptr) return Error("Can't create step.");
		std::string name = defaultStepName(fem, ps);
		FEBio::InitDefaultProps(ps);
		ps->SetName(name);
		doc->DoCommand(new CCmdAddStep(fem, ps, -1));
		m_wnd->UpdateModel(ps);
		return true;
	}

	bool RunAddSurfaceLoadCmd(QStringList ops)
	{
		CModelDocument* doc = m_wnd->GetModelDocument();
		if (doc == nullptr) return Error("No model active");

		if (ops.isEmpty()) m_wnd->on_actionAddSurfLoad_triggered();
		else
		{
			FSProject& prj = doc->GetProject();
			FSModel& fem = prj.GetFSModel();

			std::string typeStr = ops[0].toStdString();
			FSSurfaceLoad* pbc = FEBio::CreateSurfaceLoad(typeStr, &fem);
			if (pbc == nullptr) return Error(QString("Can't create load type: %1").arg(ops[0]));
			FEBio::InitDefaultProps(pbc);
			pbc->SetName(defaultLoadName(&fem, pbc));

			ops.pop_front();
			if (SetParameters(pbc, ops) == false)
			{
				delete pbc;
				return false;
			}

			FSStep* step = fem.GetStep(0);
			doc->DoCommand(new CCmdAddLoad(step, pbc), pbc->GetNameAndType());
			m_wnd->UpdateModel(pbc);
		}
		return true;
	}

	bool RunAssignCmd(QStringList ops)
	{
		if (!ValidateArgs(ops, 0, 0)) return false;
		m_wnd->on_actionAssignSelection_triggered();
		return true;
	}

	bool RunGridCmd(QStringList ops)
	{
		if (!ValidateArgs(ops, 1, 1)) return false;
		if (ops[0] == "on")
			m_wnd->on_actionShowGrid_toggled(true);
		else if (ops[0] == "off")
			m_wnd->on_actionShowGrid_toggled(false);
		else Error("Invalid command option");
		return true;
	}

	bool RunJobCmd(QStringList ops)
	{
		if (ops.empty())
		{
			m_wnd->on_actionFEBioRun_triggered();
			return true;
		}
		return Error("Invalid number of arguments.");
	}

	bool RunCloseCmd(QStringList ops)
	{
		if (ValidateArgs(ops, 0, 0) == false) return false;
		m_wnd->CloseView(m_wnd->GetDocument());
		return true;
	}

	bool RunHelpCmd(QStringList ops)
	{
		if (ValidateArgs(ops, 0, 1) == false) return false;
		if (ops.empty())
		{
			m_output = "available commands:\n";
			for (auto& entry : m_cmds)
			{
				m_output += QString("  %1 - %2\n").arg(entry.name, -8).arg(entry.brief);
			}
		}
		else
		{
			QString cmd = ops[0];
			for (auto& entry : m_cmds)
			{
				if (entry.name == cmd)
				{
					m_output = QString("%1 - %2\n").arg(entry.name).arg(entry.brief);
					return true;
				}
			}
			return Error(QString("unknown command: %1").arg(cmd));
		}
	}

	bool RunMeshCmd(QStringList ops)
	{
		GObject* po = m_wnd->GetActiveObject();
		if (po == nullptr) return Error("No active object.");
		FEMesher* pm = po->GetFEMesher();
		if (pm == nullptr) return Error("Active object cannot be meshed.");
		SetParameters(pm, ops);
		po->BuildMesh();
		return true;
	}

	bool RunNewCmd(QStringList ops)
	{
		if (ValidateArgs(ops, 0, 1) == false) return false;
		if (ops.empty())
			m_wnd->on_actionNewModel_triggered();
		else
		{
			CDocManager* dm = m_wnd->GetDocManager();
			int moduleID = FEBio::GetModuleId(ops[0].toStdString());
			if (moduleID != -1)
			{
				CModelDocument* doc = dm->CreateNewDocument(moduleID);
				if (doc)
				{
					int units = doc->GetUnitSystem();
					Units::SetUnitSystem(units);
					m_wnd->AddDocument(doc);
				}
				else return Error("Failed creating new model.");
			}
			else return Error(QString("Don't know module \"%1\"").arg(ops[0]));
		}
		return true;
	}

	bool RunOpenCmd(QStringList ops)
	{
		if (ValidateArgs(ops, 0, 1) == false) return false;
		if (ops.empty())
			m_wnd->on_actionOpen_triggered();
		else
		{
			m_wnd->OpenFile(ops[0]);
		}
		return true;
	}

	bool RunSaveCmd(QStringList ops)
	{
		if (ValidateArgs(ops, 0, 1) == false) return false;
		if (ops.empty())
			m_wnd->on_actionSave_triggered();
		else
		{
			CDocument* doc = m_wnd->GetDocument();
			if (doc == nullptr) return Error("No document open.");
			string filename = ops[0].toStdString();
			if (!doc->SaveDocument(filename)) return Error("Failed to save document.");
			m_wnd->UpdateTab(doc);
			return true;
		}
	}

	bool RunSelectSurfCmd(QStringList ops)
	{
		CModelDocument* doc = m_wnd->GetModelDocument();
		if (doc == nullptr) return Error("No model active");
		if (!ValidateArgs(ops, 1, 1)) return false;

		GModel* gm = doc->GetGModel();
		if (gm == nullptr) return Error("No model active.");

		string name = ops[0].toStdString();
		GFace* pf = gm->FindSurfaceFromName(name);
		if (pf == nullptr) Error(QString("Cannot find surface \"%1\"").arg(ops[0]));

		int index = pf->GetID();
		m_wnd->on_actionSelectSurfaces_toggled(true);
		doc->DoCommand(new CCmdSelectSurface(gm, &index, 1, false), name);
		return true;
	}

	bool RunImportCmd(QStringList ops)
	{
		if (ValidateArgs(ops, 0, 1) == false) return false;
		if (ops.isEmpty())
			m_wnd->on_actionImportGeometry_triggered();
		else 
			m_wnd->ImportFiles(QStringList() << ops[0]);
	}

	bool RunExitCmd(QStringList ops)
	{
		if (ValidateArgs(ops, 0, 0) == false) return false;
		m_wnd->on_actionExit_triggered();
		return true;
	}

	bool RunCmdCmd(QStringList ops)
	{
		if (ValidateArgs(ops, 0, 1) == false) return false;
		QString cmdFile;
		if (ops.empty())
		{
			QStringList filters; filters << "FEBio Studio Command File (*.fsc)";

			QFileDialog dlg(m_wnd, "Open");
			dlg.setFileMode(QFileDialog::ExistingFile);
			dlg.setAcceptMode(QFileDialog::AcceptOpen);
			dlg.setDirectory(m_wnd->CurrentWorkingDirectory());
			dlg.setNameFilters(filters);
			if (dlg.exec())
			{
				// get the file name
				QStringList files = dlg.selectedFiles();
				cmdFile = files.first();
			}
			else return true;
		}
		else cmdFile = ops[0];
		if (!cmdFile.isEmpty())
		{
			return RunCommandFile(cmdFile);
		}
		return Error("Failed to run command file.");
	}

	bool RunCreateCmd(QStringList ops)
	{
		if (ValidateArgs(ops, 1, -1) == false) return false;

		CModelDocument* doc = m_wnd->GetModelDocument();
		if (doc == nullptr) return Error("No active model.");

		QString type = ops[0];
		GObject* po = nullptr;
		if (type == "box") po = new GBox(); 
		if (po == nullptr) return Error(QString("Can't create %1").arg(type));

		// set default name
		std::stringstream ss;
		ss << "Object" << po->GetID();
		po->SetName(ss.str());

		// apply parameters
		ops.pop_front();
		if (SetParameters(po, ops) == false)
		{
			delete po;
			return false;
		}
		po->Update();

		doc->DoCommand(new CCmdAddAndSelectObject(doc->GetGModel(), po), po->GetNameAndType());
		m_wnd->on_actionZoomExtents_triggered();
		m_wnd->UpdateModel(po);
		return true;
	}

private:
	bool RunCommandFile(QString cmdFile)
	{
		QFile file(cmdFile);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
			return Error(QString("Failed to open command file: %1").arg(cmdFile));

		int lineCount = 0;
		while (!file.atEnd())
		{
			QByteArray line = file.readLine();
			lineCount++;
			string s = line.toStdString();
			QString cmdLine = QString::fromStdString(s);
			if (ProcessCommandLine(cmdLine) == false)
			{
				QString msg = QString("Error at line %1:\n%2").arg(lineCount).arg(m_output);
				return Error(msg);
			}
		}
		return Success(QString("cmd %1").arg(cmdFile));
	}

	bool Error(const QString& msg) { m_output = msg; return false; }
	bool Success(const QString& msg) { m_output = msg; return true; }

	bool ValidateArgs(const QStringList& ops, int minargs = -1, int maxargs = -1)
	{
		int N = ops.size();
		if ((minargs >= 0) && (N < minargs)) return Error("Insufficient number of arguments.");
		if ((maxargs >= 0) && (N > maxargs)) return Error("Too many arguments.");
		return true;
	}

	void SetParameter(Param& p, const QString& v)
	{
		switch (p.GetParamType())
		{
		case Param_INT  : p.SetIntValue  (v.toInt()); break;
		case Param_FLOAT: p.SetFloatValue(v.toDouble()); break;
		case Param_BOOL : p.SetBoolValue (v.toInt() == 1); break;
		}
	}

	bool SetParameters(FSObject* pc, const QStringList& ops)
	{
		int N = ops.size();
		if (N > pc->Parameters()) return Error("Invalid number of arguments.");
		for (int i = 0; i < ops.size(); ++i)
		{
			Param& pp = pc->GetParam(i);
			SetParameter(pp, ops[i]);
		}
		return true;
	}

	QStringList ParseCommandLine(QString cmd)
	{
		QStringList out;
		QString tmp;
		bool insideQuotes = false;
		for (int i = 0; i < cmd.size(); ++i)
		{
			QChar c = cmd[i];
			if (c == ' ')
			{
				if (insideQuotes) tmp += c;
				else {
					if (!tmp.isEmpty())
					{
						out.push_back(tmp);
						tmp.clear();
					}
				}
			}
			else if (c == '\"')
			{
				insideQuotes = !insideQuotes;
			}
			else if ((c == '#') || (c == '\n') || (c == '\r'))
			{
				break;
			}
			else tmp += c;
		}
		if (insideQuotes)
		{
			m_output = "Missing end quote.";
			return QStringList();
		}
		if (!tmp.isEmpty()) out.push_back(tmp);
		return out;
	}

private:
	CMainWindow* m_wnd;
	QString m_output;
};

class Ui::CCommandWindow
{
public:
	::CMainWindow* m_wnd;

	QLineEdit* input = nullptr;
	QPlainTextEdit* out = nullptr;

	CommandProcessor* cmd = nullptr;

public:
	void setup(::CCommandWindow* w)
	{
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(input = new QLineEdit);

		out = new QPlainTextEdit;
		out->setReadOnly(true);
		out->setFont(QFont("Courier", 11));
		out->setWordWrapMode(QTextOption::NoWrap);
		l->addWidget(out);
		w->setLayout(l);

		QObject::connect(input, &QLineEdit::returnPressed, w, &::CCommandWindow::OnEnter);
	}

	QString getCommand() { return input->text(); }

	void Log(QString msg, int level = 0)
	{
		out->clear();
		QTextDocument* document = out->document();
		QTextCursor cursor(document);
		QTextCharFormat fmt = cursor.charFormat();
		switch (level)
		{
		case 0: fmt.setForeground(Qt::black); break;
		case 1: fmt.setForeground(Qt::red); break;
		}
		cursor.movePosition(QTextCursor::End);
		cursor.insertText(msg, fmt);
	}

	void RunCalculator(QString str)
	{
		MSimpleExpression m;
		std::string sstr = str.toStdString();
		if (m.Create(sstr) == false)
		{
			Log("syntax error", 1);
		}
		else
		{
			double v = m.value();
			QString ans = QString("%1 = %2").arg(QString::fromStdString(sstr)).arg(v, 0, 'g', 15);
			Log(ans);
			input->clear();
		}
	}
};

CCommandWindow::CCommandWindow(CMainWindow* wnd, QWidget* parent) : QWidget(parent), ui(new Ui::CCommandWindow)
{
	ui->m_wnd = wnd;
	ui->cmd = new CommandProcessor(wnd);
	ui->setup(this);
}

void CCommandWindow::Show()
{
	parentWidget()->show();
	parentWidget()->raise();
	ui->input->setFocus();
}

void CCommandWindow::showEvent(QShowEvent* ev)
{
	ui->input->setFocus();
}

void CCommandWindow::OnEnter()
{
	QString str = ui->getCommand();
	if (str.isEmpty()) return;

	if (str[0] == '=')
	{
		str.remove(0, 1); // remove '='
		ui->RunCalculator(str);
		return;
	}

	bool b = ui->cmd->ProcessCommandLine(str);
	QString msg = ui->cmd->GetCommandOutput();
	if (b)
	{
		if (msg.isEmpty()) msg = str;
		ui->Log(msg);
		ui->input->clear();
	}
	else
	{
		msg = "ERROR: " + msg;
		ui->Log(msg, 1);
	}
}
