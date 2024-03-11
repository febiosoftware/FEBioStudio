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
#include "ModelViewer.h"
#include "units.h"
#include "Commands.h"
#include "GLView.h"
#include <QBoxLayout>
#include <QLineEdit>
#include <QFileDialog>
#include <QPlainTextEdit>
#include <QToolButton>
#include <QMessageBox>
#include <FEBioLink/FEBioModule.h>
#include <FEBioLink/FEBioClass.h>
#include <GeomLib/GPrimitive.h>
#include <FECore/MathObject.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <MeshTools/FEMesher.h>
#include "version.h"
#include <sstream>
#include <map>

// These are the W3C names and corresponding color values (see https://www.w3schools.com/colors/colors_hex.asp)
std::map<QString, GLColor> colorTable = { \
	{"black", GLColor(0X000000FF)},
	{ "navy", GLColor(0X000080FF) },
	{ "darkblue", GLColor(0X00008BFF) },
	{ "mediumblue", GLColor(0X0000CDFF) },
	{ "blue", GLColor(0X0000FFFF) },
	{ "darkgreen", GLColor(0X006400FF) },
	{ "green", GLColor(0X008000FF) },
	{ "teal", GLColor(0X008080FF) },
	{ "darkcyan", GLColor(0X008B8BFF) },
	{ "deepskyblue", GLColor(0X00BFFFFF) },
	{ "darkturquoise", GLColor(0X00CED1FF) },
	{ "mediumspringgreen", GLColor(0X00FA9AFF) },
	{ "lime", GLColor(0X00FF00FF) },
	{ "springgreen", GLColor(0X00FF7FFF) },
	{ "aqua", GLColor(0X00FFFFFF) },
	{ "cyan", GLColor(0X00FFFFFF) },
	{ "midnightblue", GLColor(0X191970FF) },
	{ "dodgerblue", GLColor(0X1E90FFFF) },
	{ "lightseagreen", GLColor(0X20B2AAFF) },
	{ "forestgreen", GLColor(0X228B22FF) },
	{ "seagreen", GLColor(0X2E8B57FF) },
	{ "darkslategray", GLColor(0X2F4F4FFF) },
	{ "darkslategrey", GLColor(0X2F4F4FFF) },
	{ "limegreen", GLColor(0X32CD32FF) },
	{ "mediumseagreen", GLColor(0X3CB371FF) },
	{ "turquoise", GLColor(0X40E0D0FF) },
	{ "royalblue", GLColor(0X4169E1FF) },
	{ "steelblue", GLColor(0X4682B4FF) },
	{ "darkslateblue", GLColor(0X483D8BFF) },
	{ "mediumturquoise", GLColor(0X48D1CCFF) },
	{ "indigo", GLColor(0X4B0082FF) },
	{ "darkolivegreen", GLColor(0X556B2FFF) },
	{ "cadetblue", GLColor(0X5F9EA0FF) },
	{ "cornflowerblue", GLColor(0X6495EDFF) },
	{ "rebeccapurple", GLColor(0X663399FF) },
	{ "mediumaquamarine", GLColor(0X66CDAAFF) },
	{ "dimgray", GLColor(0X696969FF) },
	{ "dimgrey", GLColor(0X696969FF) },
	{ "slateblue", GLColor(0X6A5ACDFF) },
	{ "olivedrab", GLColor(0X6B8E23FF) },
	{ "slategray", GLColor(0X708090FF) },
	{ "slategrey", GLColor(0X708090FF) },
	{ "lightslategray", GLColor(0X778899FF) },
	{ "lightslategrey", GLColor(0X778899FF) },
	{ "mediumslateblue", GLColor(0X7B68EEFF) },
	{ "lawngreen", GLColor(0X7CFC00FF) },
	{ "chartreuse", GLColor(0X7FFF00FF) },
	{ "aquamarine", GLColor(0X7FFFD4FF) },
	{ "maroon", GLColor(0X800000FF) },
	{ "purple", GLColor(0X800080FF) },
	{ "olive", GLColor(0X808000FF) },
	{ "gray", GLColor(0X808080FF) },
	{ "grey", GLColor(0X808080FF) },
	{ "skyblue", GLColor(0X87CEEBFF) },
	{ "lightskyblue", GLColor(0X87CEFAFF) },
	{ "blueviolet", GLColor(0X8A2BE2FF) },
	{ "darkred", GLColor(0X8B0000FF) },
	{ "darkmagenta", GLColor(0X8B008BFF) },
	{ "saddlebrown", GLColor(0X8B4513FF) },
	{ "darkseagreen", GLColor(0X8FBC8FFF) },
	{ "lightgreen", GLColor(0X90EE90FF) },
	{ "mediumpurple", GLColor(0X9370DBFF) },
	{ "darkviolet", GLColor(0X9400D3FF) },
	{ "palegreen", GLColor(0X98FB98FF) },
	{ "darkorchid", GLColor(0X9932CCFF) },
	{ "yellowgreen", GLColor(0X9ACD32FF) },
	{ "sienna", GLColor(0XA0522DFF) },
	{ "brown", GLColor(0XA52A2AFF) },
	{ "darkgray", GLColor(0XA9A9A9FF) },
	{ "darkgrey", GLColor(0XA9A9A9FF) },
	{ "lightblue", GLColor(0XADD8E6FF) },
	{ "greenyellow", GLColor(0XADFF2FFF) },
	{ "paleturquoise", GLColor(0XAFEEEEFF) },
	{ "lightsteelblue", GLColor(0XB0C4DEFF) },
	{ "powderblue", GLColor(0XB0E0E6FF) },
	{ "firebrick", GLColor(0XB22222FF) },
	{ "darkgoldenrod", GLColor(0XB8860BFF) },
	{ "mediumorchid", GLColor(0XBA55D3FF) },
	{ "rosybrown", GLColor(0XBC8F8FFF) },
	{ "darkkhaki", GLColor(0XBDB76BFF) },
	{ "silver", GLColor(0XC0C0C0FF) },
	{ "mediumvioletred", GLColor(0XC71585FF) },
	{ "indianred", GLColor(0XCD5C5CFF) },
	{ "peru", GLColor(0XCD853FFF) },
	{ "chocolate", GLColor(0XD2691EFF) },
	{ "tan", GLColor(0XD2B48CFF) },
	{ "lightgray", GLColor(0XD3D3D3FF) },
	{ "lightgrey", GLColor(0XD3D3D3FF) },
	{ "thistle", GLColor(0XD8BFD8FF) },
	{ "orchid", GLColor(0XDA70D6FF) },
	{ "goldenrod", GLColor(0XDAA520FF) },
	{ "palevioletred", GLColor(0XDB7093FF) },
	{ "crimson", GLColor(0XDC143CFF) },
	{ "gainsboro", GLColor(0XDCDCDCFF) },
	{ "plum", GLColor(0XDDA0DDFF) },
	{ "burlywood", GLColor(0XDEB887FF) },
	{ "lightcyan", GLColor(0XE0FFFFFF) },
	{ "lavender", GLColor(0XE6E6FAFF) },
	{ "darksalmon", GLColor(0XE9967AFF) },
	{ "violet", GLColor(0XEE82EEFF) },
	{ "palegoldenrod", GLColor(0XEEE8AAFF) },
	{ "lightcoral", GLColor(0XF08080FF) },
	{ "khaki", GLColor(0XF0E68CFF) },
	{ "aliceblue", GLColor(0XF0F8FFFF) },
	{ "honeydew", GLColor(0XF0FFF0FF) },
	{ "azure", GLColor(0XF0FFFFFF) },
	{ "sandybrown", GLColor(0XF4A460FF) },
	{ "wheat", GLColor(0XF5DEB3FF) },
	{ "beige", GLColor(0XF5F5DCFF) },
	{ "whitesmoke", GLColor(0XF5F5F5FF) },
	{ "mintcream", GLColor(0XF5FFFAFF) },
	{ "ghostwhite", GLColor(0XF8F8FFFF) },
	{ "salmon", GLColor(0XFA8072FF) },
	{ "antiquewhite", GLColor(0XFAEBD7FF) },
	{ "linen", GLColor(0XFAF0E6FF) },
	{ "lightgoldenrodyellow", GLColor(0XFAFAD2FF) },
	{ "oldlace", GLColor(0XFDF5E6FF) },
	{ "red", GLColor(0XFF0000FF) },
	{ "fuchsia", GLColor(0XFF00FFFF) },
	{ "magenta", GLColor(0XFF00FFFF) },
	{ "deeppink", GLColor(0XFF1493FF) },
	{ "orangered", GLColor(0XFF4500FF) },
	{ "tomato", GLColor(0XFF6347FF) },
	{ "hotpink", GLColor(0XFF69B4FF) },
	{ "coral", GLColor(0XFF7F50FF) },
	{ "darkorange", GLColor(0XFF8C00FF) },
	{ "lightsalmon", GLColor(0XFFA07AFF) },
	{ "orange", GLColor(0XFFA500FF) },
	{ "lightpink", GLColor(0XFFB6C1FF) },
	{ "pink", GLColor(0XFFC0CBFF) },
	{ "gold", GLColor(0XFFD700FF) },
	{ "peachpuff", GLColor(0XFFDAB9FF) },
	{ "navajowhite", GLColor(0XFFDEADFF) },
	{ "moccasin", GLColor(0XFFE4B5FF) },
	{ "bisque", GLColor(0XFFE4C4FF) },
	{ "mistyrose", GLColor(0XFFE4E1FF) },
	{ "blanchedalmond", GLColor(0XFFEBCDFF) },
	{ "papayawhip", GLColor(0XFFEFD5FF) },
	{ "lavenderblush", GLColor(0XFFF0F5FF) },
	{ "seashell", GLColor(0XFFF5EEFF) },
	{ "cornsilk", GLColor(0XFFF8DCFF) },
	{ "lemonchiffon", GLColor(0XFFFACDFF) },
	{ "floralwhite", GLColor(0XFFFAF0FF) },
	{ "snow", GLColor(0XFFFAFAFF) },
	{ "yellow", GLColor(0XFFFF00FF) },
	{ "lightyellow", GLColor(0XFFFFE0FF) },
	{ "ivory", GLColor(0XFFFFF0FF) },
	{ "white", GLColor(0XFFFFFFFF) }
};

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
		m_cmds.push_back({ "addci"  , "adds a contact interface to the current model", &CommandProcessor::RunAddContactCmd });
		m_cmds.push_back({ "addmat" , "adds a material to the model", &CommandProcessor::RunAddMaterialCmd });
		m_cmds.push_back({ "addnl"  , "adds a nodal load to the current model", &CommandProcessor::RunAddSurfaceLoadCmd });
		m_cmds.push_back({ "addsl"  , "adds a surface load to the current model", &CommandProcessor::RunAddNodalLoadCmd });
		m_cmds.push_back({ "addstep", "adds a step to the current model", &CommandProcessor::RunAddStepCmd });
		m_cmds.push_back({ "anim"   , "animate the model", &CommandProcessor::RunAnimCmd });
		m_cmds.push_back({ "assign" , "assigns the current selection to the active item in the model tree", &CommandProcessor::RunAssignCmd });
		m_cmds.push_back({ "bgcol"  , "set both background colors", &CommandProcessor::RunBgcolCmd });
		m_cmds.push_back({ "bgcol1" , "set background color 1", &CommandProcessor::RunBgcol1Cmd });
		m_cmds.push_back({ "bgcol2" , "set background color 2", &CommandProcessor::RunBgcol2Cmd });
		m_cmds.push_back({ "bgstyle", "set background style", &CommandProcessor::RunBgstyleCmd });
		m_cmds.push_back({ "close"  , "closes the current model", &CommandProcessor::RunCloseCmd });
		m_cmds.push_back({ "cmd"    , "run a command script", &CommandProcessor::RunCmdCmd });
		m_cmds.push_back({ "create" , "add a primitive to the current model", &CommandProcessor::RunCreateCmd });
		m_cmds.push_back({ "exit"   , "closes FEBio Studio", &CommandProcessor::RunExitCmd });
		m_cmds.push_back({ "fgcol"  , "sets the foreground color", &CommandProcessor::RunFgcolCmd });
		m_cmds.push_back({ "grid"   , "turn the grid in the Graphics View on or off", &CommandProcessor::RunGridCmd });
		m_cmds.push_back({ "help"   , "show help", &CommandProcessor::RunHelpCmd });
		m_cmds.push_back({ "import" , "import a geometry file", &CommandProcessor::RunImportCmd });
		m_cmds.push_back({ "job"    , "run the model in FEBio", &CommandProcessor::RunJobCmd });
		m_cmds.push_back({ "mesh"   , "generate mesh for currently selected object.", &CommandProcessor::RunMeshCmd });
		m_cmds.push_back({ "new"    , "create a new model", &CommandProcessor::RunNewCmd });
		m_cmds.push_back({ "open"   , "open a file", &CommandProcessor::RunOpenCmd });
		m_cmds.push_back({ "reset"  , "reset all options to their defaults.", &CommandProcessor::RunResetCmd });
		m_cmds.push_back({ "save"   , "save the current model", &CommandProcessor::RunSaveCmd });
		m_cmds.push_back({ "selpart", "select a part", &CommandProcessor::RunSelectPartCmd });
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
		}
		return true;
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

	bool RunAddContactCmd(QStringList ops)
	{
		CModelDocument* doc = m_wnd->GetModelDocument();
		if (doc == nullptr) return Error("No model active");

		if (ops.isEmpty()) m_wnd->on_actionAddContact_triggered();
		else
		{
			FSProject& prj = doc->GetProject();
			FSModel& fem = prj.GetFSModel();

			std::string typeStr = ops[0].toStdString();
			FSInterface* pci = FEBio::CreatePairedInterface(typeStr, &fem);
			if (pci == nullptr) return Error(QString("Can't create contact interface type: %1").arg(ops[0]));
			FEBio::InitDefaultProps(pci);
			pci->SetName(defaultInterfaceName(&fem, pci));

			ops.pop_front();
			if (SetParameters(pci, ops) == false)
			{
				delete pci;
				return false;
			}

			FSStep* step = fem.GetStep(0);
			doc->DoCommand(new CCmdAddInterface(step, pci), pci->GetNameAndType());
			m_wnd->UpdateModel(pci);
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

	bool RunAnimCmd(QStringList ops)
	{
		if (!ValidateArgs(ops, 0, 0)) return false;
		m_wnd->on_actionPlay_toggled(true);
		return true;
	}

	bool RunAssignCmd(QStringList ops)
	{
		if (!ValidateArgs(ops, 0, 1)) return false;
		if (ops.empty())
			m_wnd->on_actionAssignSelection_triggered();
		else
		{
			int n = -1;
			if      (ops[0] == "0") n = 0;
			else if (ops[0] == "1") n = 1;
			else return Error(QString("Can't assign to target %1").arg(ops[0]));
			m_wnd->GetModelViewer()->AssignCurrentSelection(n);
		}
		return true;
	}

	bool RunBgcolCmd(QStringList ops)
	{
		if (m_wnd->GetGLView() == nullptr) return Error("Graphics View not available.");
		GLViewSettings& vs = m_wnd->GetGLView()->GetViewSettings();
		if (!ValidateArgs(ops, { 1, 3 })) return false;
		GLColor newCol;
		if (!CmdToColor(ops, newCol)) return false;
		vs.m_col1 = vs.m_col2 = newCol;
		m_wnd->RedrawGL();
		return true;
	}

	bool RunBgcol1Cmd(QStringList ops)
	{
		if (m_wnd->GetGLView() == nullptr) return Error("Graphics View not available.");
		GLViewSettings& vs = m_wnd->GetGLView()->GetViewSettings();
		if (!ValidateArgs(ops, { 1, 3 })) return false;
		GLColor newCol;
		if (!CmdToColor(ops, newCol)) return false;
		vs.m_col1 = newCol;
		m_wnd->RedrawGL();
		return true;
	}

	bool RunBgcol2Cmd(QStringList ops)
	{
		if (m_wnd->GetGLView() == nullptr) return Error("Graphics View not available.");
		GLViewSettings& vs = m_wnd->GetGLView()->GetViewSettings();
		if (!ValidateArgs(ops, { 1, 3 })) return false;
		GLColor newCol;
		if (!CmdToColor(ops, newCol)) return false;
		vs.m_col2 = newCol;
		m_wnd->RedrawGL();
		return true;
	}

	bool RunBgstyleCmd(QStringList ops)
	{
		if (m_wnd->GetGLView() == nullptr) return Error("Graphics View not available.");
		GLViewSettings& vs = m_wnd->GetGLView()->GetViewSettings();
		if (!ValidateArgs(ops, 1, 1)) return false;
		QString style = ops[0];
		QStringList bgops; bgops << "color1" << "color2" << "horizontal" << "vertical";
		int n = bgops.indexOf(style);
		if (n == -1) return Error(QString("invalid argument: %1").arg(ops[0]));
		vs.m_nbgstyle = n;
		m_wnd->RedrawGL();
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
			QString flt = ops[0];
			m_output = QString("available commands (filtered by %1):\n").arg(flt);
			for (auto& entry : m_cmds)
			{
				if (entry.name.contains(flt, Qt::CaseInsensitive))
				{
					m_output += QString("  %1 - %2\n").arg(entry.name, -8).arg(entry.brief);
				}
			}
		}
		return true;
	}

	bool RunMeshCmd(QStringList ops)
	{
		GObject* po = m_wnd->GetActiveObject();
		if (po == nullptr) return Error("No active object.");
		FEMesher* pm = po->GetFEMesher();
		if (pm == nullptr) return Error("Active object cannot be meshed.");
		SetParameters(pm, ops);
		po->BuildMesh();
		m_wnd->RedrawGL();
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
		if (ValidateArgs(ops, 0, 2) == false) return false;
		if (ops.empty())
			m_wnd->on_actionOpen_triggered();
		else 
		{
			QString sub = ops[0];
			if (sub == "recent")
			{
				int n = 0;
				if (ops.size() > 1) n = ops[1].toInt() - 1;
				QStringList recentFiles = m_wnd->GetRecentFileList();
				if ((n >= 0) || (n < recentFiles.size()))
				{
					QString fileName = recentFiles.at(n);
					m_wnd->OpenFile(fileName);
					m_output = "open \"" + fileName + "\"";
				}
				else return Error(QString("Can't open recent file %d").arg(n + 1));
			}
			else if (ops.size() == 1)
			{
				QString fileName = ops[0];
				m_wnd->OpenFile(fileName);
			}
			else return Error("Invalid number of arguments.");
		}
		return true;
	}

	bool RunResetCmd(QStringList ops)
	{
		CGLView* glview = m_wnd->GetGLView();
		if (glview == nullptr) return Error("Graphics view not available.");
		GLViewSettings& view = glview->GetViewSettings();
		int ntheme = m_wnd->currentTheme();
		view.Defaults(ntheme);
		m_wnd->RedrawGL();
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
		}
		return true;
	}

	bool RunSelectPartCmd(QStringList ops)
	{
		CModelDocument* doc = m_wnd->GetModelDocument();
		if (doc == nullptr) return Error("No model active");
		if (!ValidateArgs(ops, 1, 1)) return false;

		GModel* gm = doc->GetGModel();
		if (gm == nullptr) return Error("No model active.");

		string name = ops[0].toStdString();
		GPart* pg = gm->FindPart(name);
		if (pg == nullptr) Error(QString("Cannot find part \"%1\"").arg(ops[0]));

		int index = pg->GetID();
		m_wnd->on_actionSelectParts_toggled(true);
		doc->DoCommand(new CCmdSelectPart(gm, &index, 1, false), name);
		return true;
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
		return true;
	}

	bool RunExitCmd(QStringList ops)
	{
		if (ValidateArgs(ops, 0, 0) == false) return false;
		m_wnd->on_actionExit_triggered();
		return true;
	}

	bool RunFgcolCmd(QStringList ops)
	{
		if (m_wnd->GetGLView() == nullptr) return Error("Graphics View not available.");
		GLViewSettings& vs = m_wnd->GetGLView()->GetViewSettings();
		if (!ValidateArgs(ops, { 1, 3 })) return false;
		GLColor newCol;
		if (!CmdToColor(ops, newCol)) return false;
		vs.m_fgcol = newCol;
		m_wnd->RedrawGL();
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

	bool ValidateArgs(const QStringList& ops, const std::vector<int>& validSizes)
	{
		int N = ops.size();
		for (int i : validSizes)
		{
			if (i == N) return true;
		}
		return Error("Incorrect number of arguments.");
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

	bool CmdToColor(QStringList ops, GLColor& c)
	{
		if (ops.size() == 1)
		{
			QString& colName = ops[0];
			if (colorTable.find(colName) != colorTable.cend()) c = colorTable[colName];
			else return Error(QString("Don't know color %1").arg(colName));
		}
		else if (ops.size() == 3)
		{
			uint8_t r = (uint8_t)ops[0].toInt();
			uint8_t g = (uint8_t)ops[1].toInt();
			uint8_t b = (uint8_t)ops[2].toInt();
			c = GLColor(r, g, b);
		}
		return true;
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
	QPlainTextEdit* log = nullptr;

	CommandProcessor* cmd = nullptr;

public:
	void setup(::CCommandWindow* w)
	{
		QHBoxLayout* h = new QHBoxLayout;

		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(input = new QLineEdit);

		out = new QPlainTextEdit;
		out->setReadOnly(true);
		out->setFont(QFont("Courier", 11));
		out->setWordWrapMode(QTextOption::NoWrap);
		l->addWidget(out);
		h->addLayout(l);

		QHBoxLayout* tl = new QHBoxLayout;
		QToolButton* b1 = new QToolButton; b1->setIcon(QIcon(":/icons/save.png")); b1->setAutoRaise(true); b1->setObjectName("cmdlogSave"); b1->setToolTip("<font color=\"black\">Save log");
		QToolButton* b2 = new QToolButton; b2->setIcon(QIcon(":/icons/clear.png")); b2->setAutoRaise(true); b2->setObjectName("cmdlogClear"); b2->setToolTip("<font color=\"black\">Clear log");
		tl->addWidget(b1);
		tl->addWidget(b2);
		tl->addStretch();

		log = new QPlainTextEdit;
		log->setReadOnly(true);
		log->setFont(QFont("Courier", 11));
		log->setWordWrapMode(QTextOption::NoWrap);

		QVBoxLayout* rl = new QVBoxLayout;
		rl->addLayout(tl);
		rl->addWidget(log);

		h->addLayout(rl);

		w->setLayout(h);

		QObject::connect(input, &QLineEdit::returnPressed, w, &::CCommandWindow::OnEnter);
		QObject::connect(b1, &QToolButton::clicked, w, &::CCommandWindow::OnSave);
		QObject::connect(b2, &QToolButton::clicked, w, &::CCommandWindow::OnClear);
	}

	QString getCommand() { return input->text(); }

	void Output(QString msg, int level = 0)
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

	void Log(QString msg)
	{
		QTextDocument* document = log->document();
		QTextCursor cursor(document);
		cursor.movePosition(QTextCursor::End);
		cursor.insertText(msg + "\n");
	}

	void RunCalculator(QString str)
	{
		MSimpleExpression m;
		std::string sstr = str.toStdString();
		if (m.Create(sstr) == false)
		{
			Output("syntax error", 1);
		}
		else
		{
			double v = m.value();
			QString ans = QString("%1 = %2").arg(QString::fromStdString(sstr)).arg(v, 0, 'g', 15);
			Output(ans);
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

	QStringList cmdAndOps = ui->cmd->ParseCommandLine(str);
	if (!cmdAndOps.empty())
	{
		QString cmd = cmdAndOps[0];
		QStringList ops = cmdAndOps; ops.pop_front();

		bool b = ui->cmd->RunCommand(cmd, ops);
		QString msg = ui->cmd->GetCommandOutput();
		if (b)
		{
			if (msg.isEmpty()) msg = str;
			ui->Output(msg);
			ui->input->clear();
			// don't log help command
			if (cmd != "help")
				ui->Log(str);
		}
		else
		{
			msg = "ERROR: " + msg;
			ui->Output(msg, 1);
		}
	}
}

void CCommandWindow::OnSave()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Save", "", "FEBio Studio Command Files (*.fsc)");
	if (fileName.isEmpty() == false)
	{
		// convert to const char*
		std::string sfile = fileName.toStdString();
		const char* szfile = sfile.c_str();

		// open the file
		FILE* fp = fopen(szfile, "wb");
		if (fp == 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed saving command file");
			return;
		}

		// add a header comment
		fprintf(fp, "# Created by FEBio Studio %d.%d.%d\n", FBS_VERSION, FBS_SUBVERSION, FBS_SUBSUBVERSION);

		// convert data to string
		QString txt = ui->log->toPlainText();
		std::string s = txt.toStdString();
		size_t len = s.length();
		size_t nwritten = fwrite(s.c_str(), sizeof(char), len, fp);

		// close the file
		fclose(fp);
	}
}

void CCommandWindow::OnClear()
{
	if (QMessageBox::question(this, "Command Window", "Are you sure you want to clear the command history?"))
	{
		ui->log->clear();
	}
}
