#pragma once
#include <FSCore/FileReader.h>
#include <FSCore/FileWriter.h>

class CPostDocument;
class XMLTag;
class XMLWriter;

namespace Post {
	class FEPostModel;
	class GLPlotGroup;
}

class PostSessionFileReader : public FileReader
{
public:
	PostSessionFileReader(CPostDocument* doc);
	~PostSessionFileReader();
	bool Load(const char* szfile) override;

	FileReader* GetOpenFileReader();

	float GetFileProgress() const override;

private:
	bool parse_model(XMLTag& tag);
	bool parse_material(XMLTag& tag);
	bool parse_datafield(XMLTag& tag);
	bool parse_mesh_nodeset(XMLTag& tag);
	bool parse_mesh_edgeset(XMLTag& tag);
	bool parse_mesh_surface(XMLTag& tag);
	bool parse_mesh_elementset(XMLTag& tag);
	bool parse_plot(XMLTag& tag);
	bool parse_view(XMLTag& tag);

private: // obsolete tags (for backward compatibility with 1.0)
	bool parse_open(XMLTag& tag);
	bool parse_kinemat(XMLTag& tag);

protected:
	const char*			m_szfile;
	CPostDocument*		m_doc;
	Post::FEPostModel*	m_fem;
	Post::GLPlotGroup*	m_pg; // group to add plot to
	FileReader*			m_openFile;	// the reader for opening the file
};

class PostSessionFileWriter : public FileWriter
{
public:
	PostSessionFileWriter(CPostDocument* doc);
	~PostSessionFileWriter();

	bool Write(const char* szfile) override;

private:
	void WriteModel();
	void WriteMaterials();
	void WriteDataFields();
	void WriteMeshSelections();
	void WritePlots();
	void WriteView();

private:
	CPostDocument* m_doc;
	XMLWriter* m_xml;
	string	m_fileName;
};
