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

namespace Ui {

	class CMainWindow;	// the ui class of the CMainWindow (see ui_mainwindow.h)

	enum Config {
		EMPTY_CONFIG,		// no document open
		HTML_CONFIG,		// html documument (i.e. welcome page)
		MODEL_CONFIG,		// model document	(i.e. fsm file)
		POST_CONFIG,		// post document	(i.e. xplt file)
		TEXT_CONFIG,		// text document	(i.e. raw feb file)
		XML_CONFIG,			// text document	(i.e. feb file)
		MONITOR_CONFIG,		// febio monitor document
		FEBREPORT_CONFIG,	// febio report document
	};

	// Protected base class for configurations
	// To create a new configuration, create a derived class
	// and implement the Apply member. (Make sure to call the base class Apply as well!)
	class CUIConfig
	{
	protected:
		CUIConfig(Ui::CMainWindow* ui, Ui::Config config) : ui(ui), m_config(config) {}
		virtual ~CUIConfig() {}

	public:
		Ui::Config GetConfig() const { return m_config; }

		virtual void Apply();

	protected:
		Ui::CMainWindow* ui;
		Ui::Config m_config;
	};

	// The empty configuration for when no document is open. 
	class CEmptyConfig : public CUIConfig
	{
	public:
		CEmptyConfig(Ui::CMainWindow* ui) : CUIConfig(ui, Ui::Config::EMPTY_CONFIG) {}
		void Apply() override;
	};

	// Configuration for showing a html page (used for Welcome page)
	class CHTMLConfig : public CUIConfig
	{
	public:
		CHTMLConfig(Ui::CMainWindow* ui) : CUIConfig(ui, Ui::Config::HTML_CONFIG) {}
		void Apply() override;
	};

	// Configuration for build mode
	class CModelConfig : public CUIConfig
	{
	public:
		CModelConfig(Ui::CMainWindow* ui) : CUIConfig(ui, Ui::Config::MODEL_CONFIG) {}
		void Apply() override;
	};

	// Configuration for post mode
	class CPostConfig : public CUIConfig
	{
	public:
		CPostConfig(Ui::CMainWindow* ui) : CUIConfig(ui, Ui::Config::POST_CONFIG) {}
		void Apply() override;
	};

	// Configuration for showing text file. (Used for showing febio input file text.)
	class CTextConfig : public CUIConfig
	{
	public:
		CTextConfig(Ui::CMainWindow* ui) : CUIConfig(ui, Ui::Config::TEXT_CONFIG) {}
		void Apply() override;
	};

	// Configuration for showing an xml file in model tree. 
	class CXMLConfig : public CUIConfig
	{
	public:
		CXMLConfig(Ui::CMainWindow* ui) : CUIConfig(ui, Ui::Config::XML_CONFIG) {}
		void Apply() override;
	};

	// Configuration for running the febio monitor
	class CMonitorConfig : public CUIConfig
	{
	public:
		CMonitorConfig(Ui::CMainWindow* ui) : CUIConfig(ui, Ui::Config::MONITOR_CONFIG) {}
		void Apply() override;
	};

	// Configuration for viewing an febio report
	class CFEBReportConfig : public CUIConfig
	{
	public:
		CFEBReportConfig(Ui::CMainWindow* ui) : CUIConfig(ui, Ui::Config::FEBREPORT_CONFIG) {}
		void Apply() override;
	};

} // namespace Ui
