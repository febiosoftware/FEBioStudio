#pragma once
#include <vector>
#include <string>
using namespace std;

class DocTemplate
{
public:
	DocTemplate();
	DocTemplate(const DocTemplate& doc);
	void operator = (const DocTemplate& doc);

public:
	string	title;
	string	description;
	string	fileName;
	int		module;		// list of active modules
};

class TemplateManager
{
public:
	static void Init();

	static bool LoadTemplate(const char* sztmp);

	static int Templates();

	static void AddTemplate(DocTemplate& tmp);

	static const DocTemplate& GetTemplate(int i);

	static string TemplatePath();

private:
	TemplateManager();

	static vector<DocTemplate>	m_doc;
	static string m_path;
};
