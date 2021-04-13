/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
/**
 * This file is used for compatibility across windows and mac/linux platforms.
 * This is specific to FLuoRender Code.
 * @author Brig Bagley
 * @version 4 March 2014
 */


#ifndef __COMPATIBILITY_H__
#define __COMPATIBILITY_H__

#ifdef HAS_TEEM

#ifdef _WIN32 //WINDOWS ONLY

// need this because std::byte has been introduced into the standard
// microsoft has yet to come up with an appropriate fix. It would be
// better, to get rid of any window's calls if possible, however the
// java code needs a handler in order for the program to run without
// crashing. Maybe looking into boost will have something similar.
#define byte win_byte_override

#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <string>
#include <cstring> 
#include <locale>
#include <vector>
#include <windows.h>
//#include <ole2.h>
#include <ctime>
#include <sys/types.h>
#include <ctype.h>
#include "tiffio.h"
#include <direct.h>
#include <codecvt>
#include <QString>

#define GETCURRENTDIR _getcwd

#define FSEEK64     _fseeki64
#define SSCANF    sscanf

inline wchar_t GETSLASH() { return L'\\'; }
inline wchar_t GETSLASHALT() { return L'/'; }

inline std::wstring GET_SUFFIX(std::wstring &pathname)
{
	int64_t pos = pathname.find_last_of(L'.');
	if (pos != std::wstring::npos)
		return pathname.substr(pos);
	else
		return L"";
}

inline std::wstring GET_NAME(std::wstring &pathname)
{
	int64_t pos1 = pathname.find_last_of(GETSLASH());
	int64_t pos2 = pathname.find_last_of(GETSLASHALT());
	if (pos1 != std::wstring::npos &&
		pos2 != std::wstring::npos)
		return pathname.substr((pos1 > pos2 ? pos1 : pos2) + 1);
	else if (pos1 != std::wstring::npos)
		return pathname.substr(pos1 + 1);
	else if (pos2 != std::wstring::npos)
		return pathname.substr(pos2 + 1);
	else
		return pathname;
}

inline std::wstring GET_PATH(std::wstring &pathname)
{
	int64_t pos1 = pathname.find_last_of(GETSLASH());
	int64_t pos2 = pathname.find_last_of(GETSLASHALT());
	if (pos1 != std::wstring::npos &&
		pos2 != std::wstring::npos)
		return pathname.substr(0, (pos1 > pos2 ? pos1 : pos2) + 1);
	else if (pos1 != std::wstring::npos)
		return pathname.substr(0, pos1 + 1);
	else if (pos2 != std::wstring::npos)
		return pathname.substr(0, pos2 + 1);
	else
		return pathname;
}

inline bool SEP_PATH_NAME(std::wstring &pathname, std::wstring &path, std::wstring &name)
{
	int64_t pos1 = pathname.find_last_of(GETSLASH());
	int64_t pos2 = pathname.find_last_of(GETSLASHALT());
	if (pos1 != std::wstring::npos &&
		pos2 != std::wstring::npos)
	{
		path = pathname.substr(0, (pos1 > pos2 ? pos1 : pos2) + 1);
		name = pathname.substr((pos1 > pos2 ? pos1 : pos2) + 1);
		return true;
	}
	else if (pos1 != std::wstring::npos)
	{
		path = pathname.substr(0, pos1 + 1);
		name = pathname.substr(pos1 + 1);
		return true;
	}
	else if (pos2 != std::wstring::npos)
	{
		path = pathname.substr(0, pos2 + 1);
		name = pathname.substr(pos2 + 1);
		return true;
	}
	else
		return false;
}

inline std::wstring s2ws(const std::string& utf8) {
	//    return std::wstring( str.begin(), str.end() );
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	return converter.from_bytes(utf8);
}

inline std::string ws2s(const std::wstring& utf16) {
	//    return std::string( str.begin(), str.end() );
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	return converter.to_bytes(utf16);
}

inline TIFF* TIFFOpenW(std::wstring fname, const char* opt) {
	return TIFFOpenW(fname.c_str(), opt);
}

inline FILE* FOPEN(FILE** fp, const char *fname, const char* mode) {
	fopen_s(fp, fname, mode);
	return *fp;
}

inline FILE* WFOPEN(FILE** fp, const wchar_t* fname, const wchar_t* mode) {
	_wfopen_s(fp, fname, mode);
	return *fp;
}

inline errno_t STRCPY(char* d, size_t n, const char* s) { return strcpy_s(d, n, s); }

inline errno_t STRNCPY(char* d, size_t n, const char* s, size_t x) {
	return strncpy_s(d, n, s, x);
}

inline errno_t STRCAT(char * d, size_t n, const char* s) {
	return strcat_s(d, n, s);
}

inline char* STRDUP(const char* s) { return _strdup(s); }

inline int SPRINTF(char* buf, size_t n, const char * fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int r = vsprintf_s(buf, n, fmt, args);
	va_end(args);
	return r;
}

inline int WSTOI(std::wstring s) { return _wtoi(s.c_str()); }

inline double WSTOD(std::wstring s) { return _wtof(s.c_str()); }

inline int STOI(const char * s) { return (s ? atoi(s) : 0); }
inline int STOI(const QString * s) { return (s ? s->toInt() : 0); }
inline int STOI(QString * s) { return (s ? s->toInt() : 0); }

inline double STOD(const char * s) { return (s ? atof(s) : 0.0); }
inline double STOD(QString * s) { return (s ? s->toFloat() : 0.0); }
inline double STOD(const QString * s) { return (s ? s->toFloat() : 0.0); }

inline time_t TIME(time_t* n) { return _time32((__time32_t*)n); }

inline uint32_t GET_TICK_COUNT() { return GetTickCount(); }

inline bool FIND_FILES_4D(std::wstring path_name,
	std::wstring id, std::vector<std::wstring> &batch_list,
	int &cur_batch)
{
	int64_t begin = path_name.find(id);
	size_t id_len = id.length();
	if (begin == -1)
		return false;
	else
	{
		std::wstring searchstr = path_name.substr(0, begin);
		searchstr.push_back(L'*');
		std::wstring t_num;
		size_t k;
		bool end_digits = false;
		for (k = begin+id_len; k < path_name.length(); ++k)
		{
			wchar_t c = path_name[k];
			if (iswdigit(c))
			{
				if (end_digits)
					searchstr.push_back(c);
				else
					t_num.push_back(c);
			}
			else if (k == begin + id_len)
				return false;
			else
			{
				end_digits = true;
				searchstr.push_back(c);
			}
		}
		if (t_num.length() == 0)
			return false;
		
		std::wstring search_path = path_name.substr(0,
			path_name.find_last_of(L'\\')) + L'\\';
		WIN32_FIND_DATAW FindFileData;
		HANDLE hFind;
		hFind = FindFirstFileW(searchstr.c_str(), &FindFileData);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			int cnt = 0;
			batch_list.clear();
			std::wstring name = search_path + FindFileData.cFileName;
			batch_list.push_back(name);
			if (name == path_name)
				cur_batch = cnt;
			cnt++;

			while (FindNextFileW(hFind, &FindFileData) != 0)
			{
				name = search_path + FindFileData.cFileName;
				batch_list.push_back(name);
				if (name == path_name)
					cur_batch = cnt;
				cnt++;
			}
		}
		FindClose(hFind);

		return true;
	}
}

inline void FIND_FILES(std::wstring m_path_name,
	std::wstring search_ext,
	std::vector<std::wstring> &m_batch_list,
	int &m_cur_batch, std::wstring regex = L"") {
	std::wstring search_path = m_path_name.substr(0,
		m_path_name.find_last_of(L'\\')) + L'\\';
	std::wstring search_str = regex + L"*" + search_ext;
	if (std::string::npos == search_str.find(m_path_name))
		search_str = m_path_name + search_str;
	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind;
	hFind = FindFirstFileW(search_str.c_str(), &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		int cnt = 0;
		m_batch_list.clear();
		std::wstring name = search_path + FindFileData.cFileName;
		m_batch_list.push_back(name);
		if (name == m_path_name)
			m_cur_batch = cnt;
		cnt++;

		while (FindNextFileW(hFind, &FindFileData) != 0)
		{
			name = search_path + FindFileData.cFileName;
			m_batch_list.push_back(name);
			if (name == m_path_name)
				m_cur_batch = cnt;
			cnt++;
		}
	}
	FindClose(hFind);
}

#else // MAC OSX or LINUX

#include <string>
#include <cstring> 
#include <locale>
#include <unistd.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <vector>
#include <iostream>
#include <tiffio.h>
#include <codecvt>

#define GETCURRENTDIR getcwd

#define FSEEK64     fseek

inline wchar_t GETSLASH() { return L'/'; }

inline std::wstring GET_SUFFIX(std::wstring &pathname)
{
	int64_t pos = pathname.find_last_of(L'.');
	if (pos != std::wstring::npos)
		return pathname.substr(pos);
	else
		return L"";
}

inline std::wstring GET_NAME(std::wstring &pathname)
{
	int64_t pos = pathname.find_last_of(GETSLASH());
	if (pos != std::wstring::npos)
		return pathname.substr(pos + 1);
	else
		return pathname;
}

inline std::wstring GET_PATH(std::wstring &pathname)
{
	int64_t pos = pathname.find_last_of(GETSLASH());
	if (pos != std::wstring::npos)
		return pathname.substr(0, pos + 1);
	else
		return pathname;
}

inline bool SEP_PATH_NAME(std::wstring &pathname, std::wstring &path, std::wstring &name)
{
	int64_t pos = pathname.find_last_of(GETSLASH());
	if (pos != std::wstring::npos)
	{
		path = pathname.substr(0, pos + 1);
		name = pathname.substr(pos + 1);
		return true;
	}
	else
		return false;
}

inline std::wstring s2ws(const std::string& utf8) {
	//    return std::wstring( str.begin(), str.end() );
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	return converter.from_bytes(utf8);
}

inline std::string ws2s(const std::wstring& utf16) {
	//    return std::string( str.begin(), str.end() );
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	return converter.to_bytes(utf16);
}

inline int SSCANF(const char* buf, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int r = vsscanf(buf, fmt, args);
	va_end(args);
	return r;
}

inline int swprintf_s(wchar_t *buf, size_t n, const wchar_t* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int r = vswprintf(buf, n, fmt, args);
	va_end(args);
	return r;
}

inline void SetDoubleBuffered(bool) {};

inline char* STRCPY(char* d, size_t n, const char* s) { return strncpy(d, s, n - 1); }

inline char* STRNCPY(char* d, size_t n, const char* s, size_t x) {
	return strncpy(d, s, n - 1);
}

inline char* STRCAT(char * d, size_t n, const char* s) {
	return strncat(d, s, n - strlen(d) - 1);
}

inline char* STRDUP(const char* s) { return strdup(s); }

inline TIFF* TIFFOpenW(std::wstring fname, const char* opt) {
	return TIFFOpen(ws2s(fname).c_str(), opt);
}

inline int SPRINTF(char* buf, size_t n, const char * fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int r = vsprintf(buf, fmt, args);
	va_end(args);
	return r;
}

inline int WSTOI(std::wstring s) { return atoi(ws2s(s).c_str()); }

inline double WSTOD(std::wstring s) { return atof(ws2s(s).c_str()); }

inline int STOI(const char * s) { return (s ? atoi(s) : 0); }

inline double STOD(const char * s) { return (s ? atof(s) : 0.0); }

inline time_t TIME(time_t* n) { return time(n); }

typedef union _LARGE_INTEGER {
	struct {
		unsigned int LowPart;
		long HighPart;
	} v;
	struct {
		unsigned int LowPart;
		long HighPart;
	} u;
	long long QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

inline bool FIND_FILES_4D(std::wstring path_name,
	std::wstring id, std::vector<std::wstring> &batch_list,
	int &cur_batch)
{
	int64_t begin = path_name.find(id);
	size_t id_len = id.length();
	if (begin == -1)
		return false;
	else
	{
		std::wstring searchstr = path_name.substr(0, begin);
		std::wstring searchstr2;
		std::wstring t_num;
		size_t k;
		bool end_digits = false;
		for (k = begin + id_len; k < path_name.length(); ++k)
		{
			wchar_t c = path_name[k];
			if (iswdigit(c))
			{
				if (end_digits)
					searchstr.push_back(c);
				else
					t_num.push_back(c);
			}
			else if (k == begin + id_len)
				return false;
			else
			{
				end_digits = true;
				searchstr2.push_back(c);
			}
		}
		if (t_num.length() == 0)
			return false;

		std::wstring search_path = path_name.substr(0,
			path_name.find_last_of(L'/')) + L'/';
		DIR* dir;
		struct dirent *ent;
		if ((dir = opendir(ws2s(search_path).c_str())) != NULL)
		{
			int cnt = 0;
			batch_list.clear();

			while ((ent = readdir(dir)) != NULL)
			{
				std::string file(ent->d_name);
				std::wstring wfile = search_path + s2ws(file);
				//check if it contains the string.
				if (ent->d_name[0] != '.' &&
					wfile.find(searchstr) != std::string::npos &&
					wfile.find(searchstr2) != std::string::npos) {
					std::string ss = ent->d_name;
					std::wstring f = s2ws(ss);
					std::wstring name;
					if (f.find(search_path) == std::string::npos)
						name = search_path + f;
					else
						name = f;
					batch_list.push_back(name);
					if (name == path_name)
						cur_batch = cnt;
					cnt++;
				}
			}
		}
		return true;
	}
}

inline void FIND_FILES(std::wstring m_path_name,
	std::wstring search_ext,
	std::vector<std::wstring> &m_batch_list,
	int &m_cur_batch, std::wstring regex = L"") {
	std::wstring search_path = m_path_name.substr(0, m_path_name.find_last_of(L'/')) + L'/';
	std::wstring regex_min;
	if (regex.find(search_path) != std::string::npos)
		regex_min = regex.substr(search_path.length(), regex.length() - search_path.length());
	else
		regex_min = regex;
	DIR* dir;
	struct dirent *ent;
	if ((dir = opendir(ws2s(search_path).c_str())) != NULL) {
		int cnt = 0;
		m_batch_list.clear();
		while ((ent = readdir(dir)) != NULL) {
			std::string file(ent->d_name);
			std::wstring wfile = s2ws(file);
			//check if it contains the string.
			if (ent->d_name[0] != '.' &&
				wfile.find(search_ext) != std::string::npos &&
				wfile.find(regex_min) != std::string::npos) {
				std::string ss = ent->d_name;
				std::wstring f = s2ws(ss);
				std::wstring name;
				if (f.find(search_path) == std::string::npos)
					name = search_path + f;
				else
					name = f;
				m_batch_list.push_back(name);
				if (name == m_path_name)
					m_cur_batch = cnt;
				cnt++;
			}
		}
	}
}

inline FILE* WFOPEN(FILE ** fp, const wchar_t* filename, const wchar_t* mode) {
	*fp = fopen(ws2s(std::wstring(filename)).c_str(),
		ws2s(std::wstring(mode)).c_str());
	return *fp;
}

inline FILE* FOPEN(FILE ** fp, const char* filename, const char* mode) {
	*fp = fopen(filename, mode);
	return *fp;
}

inline uint32_t GET_TICK_COUNT() {
	struct timeval ts;
	gettimeofday(&ts, NULL);
	return ts.tv_sec * 1000 + ts.tv_usec / 1000;
}

//LINUX SPECIFIC
#ifdef _LINUX
#endif
//MAC OSX SPECIFIC
#ifdef _DARWIN
#include <dlfcn.h>
#endif

#endif //END_IF_DEF__WIN32__
#endif
#endif //END__COMPATIBILITY_H__
