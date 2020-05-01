#pragma once
#include <stdio.h>
#include <string.h>
#include <string>

#ifdef WIN32
typedef __int64 off_type;
#endif

#ifdef LINUX // same for Linux and Mac OS X
typedef off_t off_type;
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
typedef off_t off_type;
#endif

//-----------------------------------------------------------------------------
// forward declaration of model class
class FEModel;
class FEProject;

//-----------------------------------------------------------------------------
class FileReader
{
public:
	FileReader();
	virtual ~FileReader();

	// This function must be overloaded in derived classes
	virtual bool Load(const char* szfile) = 0;

	// Cancel the file read
	void Cancel();

	// See if the file read was cancelled
	bool IsCancelled() const;

	// get the error string
	const std::string& GetErrorMessage();

	// get the number of errors
	int Errors();

	// get the amount of the file read so far
	// expressed in percentage of total file size
	float GetFileProgress() const;

	// get the file title, i.e. the file name w/o the path
	void FileTitle(char* sz);

	// set the file name
	void SetFileName(const std::string& fileName);

	// get the file name
	std::string GetFileName() const;

protected:
	// open the file
	bool Open(const char* szfile, const char* szmode);

	// close the file
	virtual void Close();

	// helper function that sets the error string
	bool errf(const char* szerr, ...);

	// get the file pointer
	FILE* FilePtr();

protected:
	FILE*			m_fp;

private:
	std::string		m_fileName;	//!< file name
	std::string		m_err;		//!< error messages (separated by \n)
	int				m_nerrors;	//!< number of errors
	off_type		m_nfilesize;	// size of file
	bool			m_cancelled;	//!< file read was cancelled
};

//-----------------------------------------------------------------------------
// class for reading FE file formats
class FEFileImport : public FileReader
{
public:
	FEFileImport(FEProject& prj) : m_prj(prj) {}

	FEProject& GetProject() { return m_prj; }

protected:
	FEProject& m_prj;
};

// helper function to compare strings
inline int szcmp(const char* sz1, const char* sz2)
{
	return strncmp(sz1, sz2, strlen(sz2));
}
