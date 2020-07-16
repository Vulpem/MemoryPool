#ifndef __LOG
#define __LOG

#include <string>
#include "ReadWriteFile.h"

class Log
{
private:
	Log(Log const&) = delete;
	Log operator=(Log const&) = delete;
	Log();
public:
	~Log();

	static void StoreLogsForSaving(bool store);
	static bool StoringLogsForSaving();
	static void SetOutputFile(const std::string& fileName);

	static void NewLine(const std::string& newText);
	static void Append(const std::string& newText);

private:
	static Log& Instance();

	bool m_storeLogsForSaving;
	ReadWriteFile m_outputFile;
	std::string m_outputFileName;
};

#endif // !__LOG