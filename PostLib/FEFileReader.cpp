#include "FEFileReader.h"
#include <stdarg.h>
using namespace std;
using namespace Post;

#ifdef WIN32
#define ftell64(a)     _ftelli64(a)
#define fseek64(a,b,c) _fseeki64(a,b,c)
#endif

#ifdef LINUX // same for Linux and Mac OS X
#define ftell64(a)     ftello(a)
#define fseek64(a,b,c) fseeko(a,b,c)
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
#define ftell64(a)     ftello(a)
#define fseek64(a,b,c) fseeko(a,b,c)
#endif

FEFileReader::FEFileReader(const char* sztype) : m_sztype(sztype)
{
	m_fp = 0;
	m_nfilesize = 0;
	m_cancelled = false;
}

FEFileReader::~FEFileReader()
{
}

void FEFileReader::Cancel()
{
	m_cancelled = true;
}

bool FEFileReader::IsCancelled() const
{
	return m_cancelled;
}

bool FEFileReader::Open(const char* szfile, const char* szmode)
{
	m_cancelled = false;

	if (m_fp) Close();
	m_fp = fopen(szfile, szmode);
	m_fileName = szfile;

	// get the filesize
	if (m_fp)
	{
		fseek(m_fp, 0, SEEK_END);
		m_nfilesize = ftell64(m_fp);
		fseek64(m_fp, 0, SEEK_SET);
	}

	return (m_fp != 0);
}

void FEFileReader::Close()
{
	if (m_fp) fclose(m_fp);
	m_fp = 0;
	m_nfilesize = 0;
}

const std::string& FEFileReader::GetErrorMessage()
{
	return m_err;
}
	
int FEFileReader::Errors()
{
	return m_nerrors;
}

bool FEFileReader::errf(const char* szerr, ...)
{
	// get a pointer to the argument list
	va_list	args;

	// copy to string
	va_start(args, szerr);
	char sz[512] = {0};
#ifdef WIN32
	vsprintf_s(sz, 511, szerr, args);
#endif
#ifdef __APPLE__
    vsnprintf(sz, 511, szerr, args);
#endif
	va_end(args);

	// append to the error string
	if (m_err.empty())
	{
		m_err = string(sz);
	}
	else m_err.append("\n").append(sz);
	
	m_nerrors++;

	// close the file
	Close();

	return false;
}

float FEFileReader::GetFileProgress() const
{
	if (m_fp)
	{
		off_type npos = ftell64(m_fp);
		float pct = (float) npos / (float) m_nfilesize;
		return pct;
	}
	else return 0.0f;
}
