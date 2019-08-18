#pragma once
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
