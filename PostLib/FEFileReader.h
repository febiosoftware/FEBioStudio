#pragma once
#include <stdio.h>
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
namespace Post {

class FEModel;

//-----------------------------------------------------------------------------
class FEFileReader
{
public:
	FEFileReader(const char* sztype);
	virtual ~FEFileReader();

	// This function must be overloaded in derived classes
	virtual bool Load(FEModel& fem, const char* szfile) = 0;

public:
	// get the error string
	const std::string& GetErrorMessage();
	
	// get the number of errors
	int Errors();

	// get the amount of the file read so far
	// expressed in percentage of total file size
	float GetFileProgress() const;

	// Cancel the file read
	void Cancel();

	// See if the file read was cancelled
	bool IsCancelled() const;

protected:
	// open the file
	bool Open(const char* szfile, const char* szmode);

	// close the file
	void Close();

public:
	// helper function that sets the error string
	bool errf(const char* szerr, ...);

protected:
	FILE*			m_fp;
	std::string		m_fileName;	//!< file name

private:
	const char*		m_sztype;	//!< type identifier
	std::string		m_err;		//!< error messages (separated by \n)
	int				m_nerrors;	//!< number of errors
	off_type		m_nfilesize;	//!< size of file
	bool			m_cancelled;	//!< file read was cancelled
};

}