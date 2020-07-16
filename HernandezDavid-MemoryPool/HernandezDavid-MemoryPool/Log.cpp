#include "Log.h"
#include <iostream>

Log::Log()
	: m_storeLogsForSaving(false)
	, m_outputFile()
	, m_outputFileName("log.txt")
{}

Log::~Log()
{
	if(m_outputFileName.size() > 1 && m_storeLogsForSaving)
		m_outputFile.Save(m_outputFileName);
}

void Log::StoreLogsForSaving(bool store)
{
	Instance().m_storeLogsForSaving = store;
}

bool Log::StoringLogsForSaving()
{
	return Instance().m_storeLogsForSaving;
}

void Log::SetOutputFile(const std::string& fileName)
{
	if (Instance().m_outputFile.GetNumLines() > 0)
		Instance().m_outputFileName;
}

void Log::NewLine(const std::string& newText)
{
	if (Instance().m_storeLogsForSaving)
		Instance().m_outputFile.PushBackLine(newText);
	std::cout << std::endl << newText;
}

void Log::Append(const std::string& newText)
{
	if (Instance().m_storeLogsForSaving)
		Instance().m_outputFile.AppendToEndLine(newText);
	std::cout << newText;
}

Log& Log::Instance()
{
	static Log instance;
	return instance;
}
