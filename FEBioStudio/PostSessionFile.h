#pragma once
#include <MeshIO/FileReader.h>
#include <MeshIO/FileWriter.h>

class CPostDocument;
class XMLTag;

namespace Post {
	class FEPostModel;
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

private: // obsolete tags (for backward compatibility with 1.0)
	bool parse_open(XMLTag& tag);
	bool parse_kinemat(XMLTag& tag);

protected:
	const char*			m_szfile;
	CPostDocument*		m_doc;
	Post::FEPostModel*	m_fem;
	FileReader*			m_openFile;	// the reader for opening the file
};

class PostSessionFileWriter : public FileWriter
{
public:
	PostSessionFileWriter(CPostDocument* doc);

	bool Write(const char* szfile) override;

private:
	CPostDocument* m_doc;
};
