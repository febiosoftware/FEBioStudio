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
#include "CommandProcessor.h"
#include "MainWindow.h"
#include "ModelDocument.h"
#include "PostDocument.h"
#include "ModelViewer.h"
#include "GLView.h"
#include <FEBio/FEBioExport4.h>
#include <PostLib/FEFEBioExport.h>
#include <PostLib/FEPostModel.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <MeshTools/FEMesher.h>
#include <FEBioLink/FEBioModule.h>
#include <FEBioLink/FEBioClass.h>
#include <GeomLib/GPrimitive.h>
#include "DocManager.h"
#include "Commands.h"
#include "units.h"
#include <QFileDialog>
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

static void SetParameter(Param& p, const QString& v)
{
	switch (p.GetParamType())
	{
	case Param_INT  : p.SetIntValue(v.toInt()); break;
	case Param_FLOAT: p.SetFloatValue(v.toDouble()); break;
	case Param_BOOL : p.SetBoolValue(v.toInt() == 1); break;
	}
}

static bool SetParameters(FSObject* pc, const QStringList& ops)
{
	int N = ops.size();
	if (N > pc->Parameters()) return false;
	for (int i = 0; i < ops.size(); ++i)
	{
		Param& pp = pc->GetParam(i);
		SetParameter(pp, ops[i]);
	}
	return true;
}

CommandProcessor::CommandProcessor(CMainWindow* wnd, CommandInput* cmdinput) : m_wnd(wnd), m_cmdInput(cmdinput)
{
	m_cmds.push_back({ "addbc"  , &CommandProcessor::cmd_addbc  , "adds a boundary condition to the current model" });
	m_cmds.push_back({ "addbl"  , &CommandProcessor::cmd_addbl  , "adds a body load to the current model" });
	m_cmds.push_back({ "addci"  , &CommandProcessor::cmd_addci  , "adds a contact interface to the current model" });
	m_cmds.push_back({ "adddata", &CommandProcessor::cmd_adddata, "add a standard datafield to the post model." });
	m_cmds.push_back({ "addmat" , &CommandProcessor::cmd_addmat , "adds a material to the model" });
	m_cmds.push_back({ "addnl"  , &CommandProcessor::cmd_addnl  , "adds a nodal load to the current model" });
	m_cmds.push_back({ "addsl"  , &CommandProcessor::cmd_addsl  , "adds a surface load to the current model" });
	m_cmds.push_back({ "addstep", &CommandProcessor::cmd_addstep, "adds a step to the current model" });
	m_cmds.push_back({ "anim"   , &CommandProcessor::cmd_anim   , "animate the model" });
	m_cmds.push_back({ "assign" , &CommandProcessor::cmd_assign , "assigns the current selection to the active item in the model tree" });
	m_cmds.push_back({ "bgcol"  , &CommandProcessor::cmd_bgcol  , "set both background colors" });
	m_cmds.push_back({ "bgcol1" , &CommandProcessor::cmd_bgcol1 , "set background color 1" });
	m_cmds.push_back({ "bgcol2" , &CommandProcessor::cmd_bgcol2 , "set background color 2" });
	m_cmds.push_back({ "bgstyle", &CommandProcessor::cmd_bgstyle, "set background style" });
	m_cmds.push_back({ "close"  , &CommandProcessor::cmd_close  , "closes the current model" });
	m_cmds.push_back({ "cmd"    , &CommandProcessor::cmd_cmd    , "run a command script" });
	m_cmds.push_back({ "create" , &CommandProcessor::cmd_create , "add a primitive to the current model" });
	m_cmds.push_back({ "exit"   , &CommandProcessor::cmd_exit   , "closes FEBio Studio" });
	m_cmds.push_back({ "export" , &CommandProcessor::cmd_export , "Export model to file" });
	m_cmds.push_back({ "expgeo" , &CommandProcessor::cmd_expgeo , "Export selected geometry model to file" });
	m_cmds.push_back({ "fgcol"  , &CommandProcessor::cmd_fgcol  , "sets the foreground color" });
	m_cmds.push_back({ "first"  , &CommandProcessor::cmd_first  , "Display the first timestep" });
	m_cmds.push_back({ "grid"   , &CommandProcessor::cmd_grid   , "turn the grid in the Graphics View on or off" });
	m_cmds.push_back({ "help"   , &CommandProcessor::cmd_help   , "show help" });
	m_cmds.push_back({ "import" , &CommandProcessor::cmd_import , "import a geometry file" });
	m_cmds.push_back({ "job"    , &CommandProcessor::cmd_job    , "run the model in FEBio" });
	m_cmds.push_back({ "genmesh", &CommandProcessor::cmd_genmesh, "generate mesh for currently selected object." });
	m_cmds.push_back({ "last"   , &CommandProcessor::cmd_last   , "Display the last timestep" });
	m_cmds.push_back({ "new"    , &CommandProcessor::cmd_new    , "create a new model" });
	m_cmds.push_back({ "next"   , &CommandProcessor::cmd_next   , "Display the next timestep" });
	m_cmds.push_back({ "open"   , &CommandProcessor::cmd_open   , "open a file" });
	m_cmds.push_back({ "prev"   , &CommandProcessor::cmd_prev   , "Display the previous timestep" });
	m_cmds.push_back({ "reset"  , &CommandProcessor::cmd_reset  , "reset all options to their defaults." });
	m_cmds.push_back({ "save"   , &CommandProcessor::cmd_save   , "save the current model" });
	m_cmds.push_back({ "sel"    , &CommandProcessor::cmd_sel    , "select an item of the active mesh"});
	m_cmds.push_back({ "selpart", &CommandProcessor::cmd_selpart, "select a part",  });
	m_cmds.push_back({ "selsurf", &CommandProcessor::cmd_selsurf, "select a surface" });
	m_cmds.push_back({ "stop"   , &CommandProcessor::cmd_stop   , "Stops the animation." });
}

CDocument* CommandProcessor::GetActiveDocument()
{
	return m_wnd->GetDocument();
}

CModelDocument* CommandProcessor::GetModelDocument()
{
	return dynamic_cast<CModelDocument*>(GetActiveDocument());
}

CPostDocument* CommandProcessor::GetPostDocument()
{
	return dynamic_cast<CPostDocument*>(GetActiveDocument());
}

FSMesh* CommandProcessor::GetActiveMesh()
{
	CGLDocument* doc = dynamic_cast<CGLDocument*>(GetActiveDocument());
	if (doc == nullptr) return nullptr;
	GObject* po = doc->GetActiveObject();
	if (po == nullptr) return nullptr;
	return po->GetFEMesh();
}

CMD_RETURN_CODE CommandProcessor::ProcessCommandLine(QString cmdLine)
{
	QStringList cmdAndOps = ParseCommandLine(cmdLine);
	if (cmdAndOps.empty()) return (m_output.isEmpty() ? CMD_RETURN_CODE::CMD_SUCCESS : CMD_RETURN_CODE::CMD_ERROR);
	QString cmd = cmdAndOps[0];
	QStringList ops = cmdAndOps; ops.pop_front();
	return RunCommand(cmd, ops);
}

CMD_RETURN_CODE CommandProcessor::RunCommand(QString cmd, QStringList ops)
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

QStringList CommandProcessor::ParseCommandLine(QString cmd)
{
	m_output.clear();
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
		else if (c == '%')
		{
			m_output = QString("Unexpected character (%) at position %1").arg(i);
			return QStringList();
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

CMD_RETURN_CODE CommandProcessor::cmd_addbc(QStringList ops)
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return NoActiveDoc();

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
			return InvalidArgsCount();
		}

		FSStep* step = fem.GetStep(0);
		doc->DoCommand(new CCmdAddBC(step, pbc), pbc->GetNameAndType());
		m_wnd->UpdateModel(pbc);
	}
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_addbl(QStringList ops)
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return NoActiveDoc();

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
			return InvalidArgsCount();
		}

		FSStep* step = fem.GetStep(0);
		doc->DoCommand(new CCmdAddLoad(step, pbl), pbl->GetNameAndType());
		m_wnd->UpdateModel(pbl);
	}
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_addci(QStringList ops)
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return NoActiveDoc();

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
			return InvalidArgsCount();
		}

		FSStep* step = fem.GetStep(0);
		doc->DoCommand(new CCmdAddInterface(step, pci), pci->GetNameAndType());
		m_wnd->UpdateModel(pci);
	}
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_adddata(QStringList ops)
{
	if (!ValidateArgs(ops, 0, 1)) return CMD_RETURN_CODE::CMD_ERROR;
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return NoActiveDoc();
	Post::FEPostModel* fem = doc->GetFSModel();
	if (fem == nullptr) return NoActiveDoc();

	std::string datafieldName = ops[0].toStdString();

	// TODO: Can I do this elsewhere or somehow avoid?
	Post::InitStandardDataFields();

	if (Post::AddStandardDataField(*fem, datafieldName) == false)
	{
		return Error(QString("Unknown datafield name \"%1\"").arg(ops[0]));
	}
	m_wnd->Update(nullptr, true);
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_addmat(QStringList ops)
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return NoActiveDoc();
	if (ops.empty()) m_wnd->on_actionAddMaterial_triggered();
	else
	{
		FSModel* fem = doc->GetFSModel();
		string type = ops[0].toStdString();
		FSMaterial* pmat = FEBio::CreateMaterial(type, fem);
		if (pmat == nullptr) return Error(QString("Don't know material \"%1\"").arg(ops[0]));

		ops.pop_front();
		if (SetParameters(pmat, ops) == false)
		{
			delete pmat;
			return InvalidArgsCount();
		}

		GMaterial* gmat = new GMaterial(pmat);
		doc->DoCommand(new CCmdAddMaterial(fem, gmat), gmat->GetNameAndType());
		m_wnd->UpdateModel(gmat);
	}
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_addnl(QStringList ops)
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return NoActiveDoc();

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
			return InvalidArgsCount();
		}

		FSStep* step = fem.GetStep(0);
		doc->DoCommand(new CCmdAddLoad(step, pbc), pbc->GetNameAndType());
		m_wnd->UpdateModel(pbc);
	}
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_addsl(QStringList ops)
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return NoActiveDoc();

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
			return InvalidArgsCount();
		}

		FSStep* step = fem.GetStep(0);
		doc->DoCommand(new CCmdAddLoad(step, pbc), pbc->GetNameAndType());
		m_wnd->UpdateModel(pbc);
	}
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_addstep(QStringList ops)
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return NoActiveDoc();

	FSModel* fem = doc->GetFSModel();
	FSStep* ps = FEBio::CreateStep(FEBio::GetActiveModuleName(), fem);
	if (ps == nullptr) return Error("Can't create step.");
	std::string name = defaultStepName(fem, ps);
	FEBio::InitDefaultProps(ps);
	ps->SetName(name);
	doc->DoCommand(new CCmdAddStep(fem, ps, -1));
	m_wnd->UpdateModel(ps);
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_anim(QStringList ops)
{
	if (!ValidateArgs(ops, 0, 0)) return CMD_RETURN_CODE::CMD_ERROR;
	m_wnd->on_actionPlay_toggled(true);
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_assign(QStringList ops)
{
	if (!ValidateArgs(ops, 0, 1)) return CMD_RETURN_CODE::CMD_ERROR;
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
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_bgcol(QStringList ops)
{
	if (m_wnd->GetGLView() == nullptr) return GLViewIsNull();
	GLViewSettings& vs = m_wnd->GetGLView()->GetViewSettings();
	if (!ValidateArgs(ops, { 1, 3 })) return CMD_RETURN_CODE::CMD_ERROR;
	GLColor newCol;
	if (!CmdToColor(ops, newCol)) return CMD_RETURN_CODE::CMD_ERROR;
	vs.m_col1 = vs.m_col2 = newCol;
	m_wnd->RedrawGL();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_bgcol1(QStringList ops)
{
	if (m_wnd->GetGLView() == nullptr) return GLViewIsNull();
	GLViewSettings& vs = m_wnd->GetGLView()->GetViewSettings();
	if (!ValidateArgs(ops, { 1, 3 })) return CMD_RETURN_CODE::CMD_ERROR;
	GLColor newCol;
	if (!CmdToColor(ops, newCol)) return CMD_RETURN_CODE::CMD_ERROR;
	vs.m_col1 = newCol;
	m_wnd->RedrawGL();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_bgcol2(QStringList ops)
{
	if (m_wnd->GetGLView() == nullptr) return GLViewIsNull();
	GLViewSettings& vs = m_wnd->GetGLView()->GetViewSettings();
	if (!ValidateArgs(ops, { 1, 3 })) return CMD_RETURN_CODE::CMD_ERROR;
	GLColor newCol;
	if (!CmdToColor(ops, newCol)) return CMD_RETURN_CODE::CMD_ERROR;
	vs.m_col2 = newCol;
	m_wnd->RedrawGL();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_bgstyle(QStringList ops)
{
	if (m_wnd->GetGLView() == nullptr) return GLViewIsNull();
	GLViewSettings& vs = m_wnd->GetGLView()->GetViewSettings();
	if (!ValidateArgs(ops, 1, 1)) return CMD_RETURN_CODE::CMD_ERROR;
	QString style = ops[0];
	QStringList bgops; bgops << "color1" << "color2" << "horizontal" << "vertical";
	int n = bgops.indexOf(style);
	if (n == -1) return Error(QString("invalid argument: %1").arg(ops[0]));
	vs.m_nbgstyle = n;
	m_wnd->RedrawGL();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_close(QStringList ops)
{
	if (ValidateArgs(ops, 0, 0) == false) return CMD_RETURN_CODE::CMD_ERROR;
	m_wnd->CloseView(GetActiveDocument());
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_cmd(QStringList ops)
{
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
		else return CMD_RETURN_CODE::CMD_SUCCESS;
	}
	else cmdFile = ops[0];
	if (!cmdFile.isEmpty())
	{
		if (!ops.empty()) ops.pop_front();
		return RunCommandFile(cmdFile, ops);
	}
	return Error("Failed to run command file.");
}

CMD_RETURN_CODE CommandProcessor::cmd_create(QStringList ops)
{
	if (!ValidateArgs(ops, 1, -1)) return CMD_RETURN_CODE::CMD_ERROR;

	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return NoActiveDoc();

	std::string type = ops[0].toStdString();
	GObject* po = FSCore::CreateClass<GObject>(CLASS_OBJECT, type.c_str());
	if (po == nullptr) return Error(QString("Can't create %1").arg(ops[0]));

	// set default name
	std::stringstream ss;
	ss << "Object" << po->GetID();
	po->SetName(ss.str());

	// apply parameters
	ops.pop_front();
	if (SetParameters(po, ops) == false)
	{
		delete po;
		return InvalidArgsCount();
	}
	po->Update();

	doc->DoCommand(new CCmdAddAndSelectObject(doc->GetGModel(), po), po->GetNameAndType());
	m_wnd->on_actionZoomExtents_triggered();
	m_wnd->UpdateModel(po);
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_exit(QStringList ops)
{
	if (!ValidateArgs(ops, 0, 0)) return CMD_RETURN_CODE::CMD_ERROR;
	m_wnd->on_actionExit_triggered();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_export(QStringList ops)
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return NoActiveDoc();
	if (!ValidateArgs(ops, { 0, 2 })) return CMD_RETURN_CODE::CMD_ERROR;

	QString fileName;
	QString format = "feb";

	if (ops.empty())
	{
		fileName = m_cmdInput->GetExportFEModelFilename(format);
		if (fileName.isEmpty()) return CommandCancelled();
	}
	else
	{
		format = ops[0];
		fileName = ops[1];
	}

	if (format == "feb")
	{
		FSProject& prj = doc->GetProject();
		string filename = fileName.toStdString();
		FEBioExport4 writer(prj);
		bool bsuccess = writer.Write(filename.c_str());
		if (!bsuccess) return Error(QString::fromStdString(writer.GetErrorMessage()));

		m_output = "export feb \"" + fileName + "\"";
	}
	else return Error("Unrecognized format.");
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_expgeo(QStringList ops)
{
	if (!ValidateArgs(ops, { 0, 2 })) return CMD_RETURN_CODE::CMD_ERROR;
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return NoActiveDoc();

	QString fileName;
	QString fmt = "feb";
	if (ops.empty())
	{
		fileName = m_cmdInput->GetExportGeometryFilename(fmt);
		if (fileName.isEmpty()) return CommandCancelled();
	}
	else
	{
		fmt = ops[0];
		fileName = ops[1];
	}

	if (fmt == "feb")
	{
		Post::FEPostModel& fem = *doc->GetFSModel();
		string filename = fileName.toStdString();
		Post::FEFEBioExport4 fr;
		bool bsuccess = fr.Save(fem, filename.c_str());
		if (!bsuccess) return Error("Failed to export geometry to feb4 format.");
		m_output = "expgeo feb \"" + fileName + "\"";
	}
	else return Error("Unrecognized format.");

	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_fgcol(QStringList ops)
{
	if (m_wnd->GetGLView() == nullptr) return GLViewIsNull();
	GLViewSettings& vs = m_wnd->GetGLView()->GetViewSettings();
	if (!ValidateArgs(ops, { 1, 3 })) return CMD_RETURN_CODE::CMD_ERROR;
	GLColor newCol;
	if (!CmdToColor(ops, newCol)) return CMD_RETURN_CODE::CMD_ERROR;
	vs.m_fgcol = newCol;
	m_wnd->RedrawGL();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_first(QStringList ops)
{
	if (!ValidateArgs(ops, 0, 0)) return CMD_RETURN_CODE::CMD_ERROR;
	m_wnd->on_actionFirst_triggered();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_grid(QStringList ops)
{
	if (!ValidateArgs(ops, 1, 1)) return CMD_RETURN_CODE::CMD_ERROR;
	if (ops[0] == "on")
		m_wnd->on_actionShowGrid_toggled(true);
	else if (ops[0] == "off")
		m_wnd->on_actionShowGrid_toggled(false);
	else Error("Invalid command option");
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_help(QStringList ops)
{
	if (!ValidateArgs(ops, 0, 1)) return CMD_RETURN_CODE::CMD_ERROR;
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
	return CMD_RETURN_CODE::CMD_IGNORE;
}

CMD_RETURN_CODE CommandProcessor::cmd_import(QStringList ops)
{
	if (ValidateArgs(ops, 0, 1) == false) return CMD_RETURN_CODE::CMD_ERROR;
	if (ops.isEmpty())
		m_wnd->on_actionImportGeometry_triggered();
	else
		m_wnd->ImportFiles(QStringList() << ops[0]);
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_job(QStringList ops)
{
	if (ops.empty())
	{
		m_wnd->on_actionFEBioRun_triggered();
		return CMD_RETURN_CODE::CMD_SUCCESS;
	}
	return InvalidArgsCount();
}

CMD_RETURN_CODE CommandProcessor::cmd_genmesh(QStringList ops)
{
	GObject* po = m_wnd->GetActiveObject();
	if (po == nullptr) return Error("No active object.");
	FEMesher* pm = po->GetFEMesher();
	if (pm == nullptr) return Error("Active object cannot be meshed.");
	if (SetParameters(pm, ops) == false)
	{
		return InvalidArgsCount();
	}
	po->BuildMesh();
	m_wnd->RedrawGL();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_last(QStringList ops)
{
	if (!ValidateArgs(ops, 0, 0)) return CMD_RETURN_CODE::CMD_ERROR;
	m_wnd->on_actionLast_triggered();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_new(QStringList ops)
{
	if (ValidateArgs(ops, 0, 1) == false) return CMD_RETURN_CODE::CMD_ERROR;
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
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_next(QStringList ops)
{
	if (!ValidateArgs(ops, 0, 0)) return CMD_RETURN_CODE::CMD_ERROR;
	m_wnd->on_actionNext_triggered();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_open(QStringList ops)
{
	if (!ValidateArgs(ops, 0, 2)) return CMD_RETURN_CODE::CMD_ERROR;

	QString fileName;
	if (ops.empty())
	{
		fileName = m_cmdInput->GetOpenModelFilename();
		if (fileName.isEmpty()) return CommandCancelled();
	}
	else fileName = ops[0];

	if (fileName == "recent")
	{
		int n = 0;
		if (ops.size() > 1) n = ops[1].toInt() - 1;
		QStringList recentFiles = m_wnd->GetRecentFileList();
		if ((n >= 0) || (n < recentFiles.size()))
		{
			fileName = recentFiles.at(n);
		}
		else return Error(QString("Can't open recent file %d").arg(n + 1));
	}
	else if (ops.size() > 1) return InvalidArgsCount();

	m_output = "open \"" + fileName + "\"";
	m_wnd->OpenFile(fileName, false, false, false);
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_prev(QStringList ops)
{
	if (!ValidateArgs(ops, 0, 0)) return CMD_RETURN_CODE::CMD_ERROR;
	m_wnd->on_actionPrev_triggered();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_reset(QStringList ops)
{
	CGLView* glview = m_wnd->GetGLView();
	if (glview == nullptr) return GLViewIsNull();
	GLViewSettings& view = glview->GetViewSettings();
	int ntheme = m_wnd->currentTheme();
	view.Defaults(ntheme);
	m_wnd->RedrawGL();
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_save(QStringList ops)
{
	if (ValidateArgs(ops, 0, 1) == false) return CMD_RETURN_CODE::CMD_ERROR;
	QString fileName;
	if (ops.empty())
	{
		fileName = m_cmdInput->GetSaveModelFilename();
		if (fileName.isEmpty()) return CommandCancelled();
	}
	else
		fileName = ops[0];

	CDocument* doc = GetActiveDocument();
	if (doc == nullptr) return NoActiveDoc();
	string filename = fileName.toStdString();
	if (!doc->SaveDocument(filename)) return Error("Failed to save document.");
	m_wnd->UpdateTab(doc);
	m_output = "save \"" + fileName + "\"";

	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_sel(QStringList ops)
{
	CUndoDocument* doc = dynamic_cast<CUndoDocument*>(GetActiveDocument());
	if (doc == nullptr) return NoActiveDoc();

	if (ValidateArgs(ops, 0, 1) == false) return CMD_RETURN_CODE::CMD_ERROR;
	FSMesh* pm = GetActiveMesh();
	if (pm == nullptr) return NoActiveMesh();

	std::string id = ops[0].toStdString();
	int mode = 0;
	if      (id[0] == 'E') mode = ITEM_ELEM;
	else if (id[0] == 'F') mode = ITEM_FACE;
	else if (id[0] == 'N') mode = ITEM_NODE;
	else if (id[0] == 'L') mode = ITEM_EDGE;
	else return Error("Unrecognized item identifier.");

	int nid = atoi(id.c_str() + 1);

	if (mode == ITEM_ELEM)
	{
		int index = pm->ElementIndexFromID(nid);
		if (index < 0) return Error("Invalid element ID.");
		doc->DoCommand(new CCmdSelectElements(pm, &index, 1, false));
	}
	else if (mode == ITEM_FACE)
	{
		int index = nid - 1;
		if (index < 0) return Error("Invalid face ID.");
		doc->DoCommand(new CCmdSelectFaces(pm, &index, 1, false));
	}
	else if (mode == ITEM_NODE)
	{
		int index = pm->NodeIndexFromID(nid);
		if (index < 0) return Error("Invalid node ID.");
		doc->DoCommand(new CCmdSelectFENodes(pm, &index, 1, false));
	}
	else if (mode == ITEM_EDGE)
	{
		int index = nid - 1;
		if (index < 0) return Error("Invalid edge ID.");
		doc->DoCommand(new CCmdSelectFEEdges(pm, &index, 1, false));
	}

	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_selpart(QStringList ops)
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return NoActiveDoc();
	if (!ValidateArgs(ops, 1, 1)) return CMD_RETURN_CODE::CMD_ERROR;

	GModel* gm = doc->GetGModel();
	if (gm == nullptr) return Error("No model active.");

	string name = ops[0].toStdString();
	GPart* pg = gm->FindPart(name);
	if (pg == nullptr) Error(QString("Cannot find part \"%1\"").arg(ops[0]));

	int index = pg->GetID();
	m_wnd->on_actionSelectParts_toggled(true);
	doc->DoCommand(new CCmdSelectPart(gm, &index, 1, false), name);
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_selsurf(QStringList ops)
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr) return NoActiveDoc();
	if (!ValidateArgs(ops, 1, 1)) return CMD_RETURN_CODE::CMD_ERROR;

	GModel* gm = doc->GetGModel();
	if (gm == nullptr) return Error("No model active.");

	string name = ops[0].toStdString();
	GFace* pf = gm->FindSurfaceFromName(name);
	if (pf == nullptr) Error(QString("Cannot find surface \"%1\"").arg(ops[0]));

	int index = pf->GetID();
	m_wnd->on_actionSelectSurfaces_toggled(true);
	doc->DoCommand(new CCmdSelectSurface(gm, &index, 1, false), name);
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::cmd_stop(QStringList ops)
{
	if (!ValidateArgs(ops, 0, 0)) return CMD_RETURN_CODE::CMD_ERROR;
	m_wnd->on_actionPlay_toggled(false);
	return CMD_RETURN_CODE::CMD_SUCCESS;
}

CMD_RETURN_CODE CommandProcessor::RunCommandFile(QString cmdFile, QStringList ops)
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
		int n = 1;
		for (QString& arg : ops)
		{
			QString s = "\"" + arg + "\"";
			QString r = "%" + QString("%1").arg(n++);
			cmdLine.replace(r, s);
		}
		if (ProcessCommandLine(cmdLine) == CMD_RETURN_CODE::CMD_ERROR)
		{
			QString msg = QString("Error at line %1:\n%2").arg(lineCount).arg(m_output);
			return Error(msg);
		}
	}
	return Success(QString("cmd %1").arg(cmdFile));
}

bool CommandProcessor::ValidateArgs(const QStringList& ops, int minargs, int maxargs)
{
	int N = ops.size();
	if ((minargs >= 0) && (N < minargs)) { Error("Insufficient number of arguments."); return false; }
	if ((maxargs >= 0) && (N > maxargs)) { Error("Too many arguments."); return false; }
	return true;
}

bool CommandProcessor::ValidateArgs(const QStringList& ops, const std::vector<int>& validSizes)
{
	int N = ops.size();
	for (int i : validSizes)
	{
		if (i == N) return true;
	}
	Error("Incorrect number of arguments.");
	return false;
}

bool CommandProcessor::CmdToColor(QStringList ops, GLColor& c)
{
	if (ops.size() == 1)
	{
		QString& colName = ops[0];
		if (colorTable.find(colName) != colorTable.cend()) c = colorTable[colName];
		else
		{
			Error(QString("Don't know color %1").arg(colName));
			return false;
		}
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
