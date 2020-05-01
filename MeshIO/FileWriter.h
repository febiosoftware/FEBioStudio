#pragma once

class FileWriter
{
public:
	FileWriter() {}
	virtual ~FileWriter() {}

	virtual bool Write(const char* szfile) = 0;
};
