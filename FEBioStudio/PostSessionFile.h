#pragma once
#include <MeshIO/FileReader.h>
#include <MeshIO/FileWriter.h>

class CPostDocument;

class PostSessionFileReader : public FileReader
{
public:
	PostSessionFileReader(CPostDocument* doc);
	~PostSessionFileReader();
	bool Load(const char* szfile) override;

	FileReader* GetOpenFileReader();

	float GetFileProgress() const override;

protected:
	CPostDocument* m_doc;
	FileReader*		m_openFile;	// the reader for opening the file
};

class PostSessionFileWriter : public FileWriter
{
public:
	PostSessionFileWriter(CPostDocument* doc);

	bool Write(const char* szfile) override;

private:
	CPostDocument* m_doc;
};
