// Serializable.h: interface for the CSerializable class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERIALIZABLE_H__A95E0A20_CEA8_4CBC_89C6_B6A9736E98B3__INCLUDED_)
#define AFX_SERIALIZABLE_H__A95E0A20_CEA8_4CBC_89C6_B6A9736E98B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Archive.h"
#include "enum.h"
#include "CallTracer.h"

class InvalidVersion {};

class ReadError
{
public: 
	ReadError(const char* szmsg = 0) : m_szmsg(szmsg) { CCallStack::FlagError(); }
public:
	const char* m_szmsg;
};

class CSerializable  
{
public:
	CSerializable();
	virtual ~CSerializable();

	virtual void Load(IArchive& ar) {}
	virtual void Save(OArchive& ar) {}
};

#endif // !defined(AFX_SERIALIZABLE_H__A95E0A20_CEA8_4CBC_89C6_B6A9736E98B3__INCLUDED_)
