#include <iostream>
#include <fstream>
#include "ReadWriteFile.h"

ReadWriteFile::ReadWriteFile(const char* file)
	: m_fileName(file)
{
}

ReadWriteFile::~ReadWriteFile()
{
}

void ReadWriteFile::Save() const
{
	std::ofstream file;
	file.open(m_fileName.c_str(), std::ofstream::out);
	for (const std::string& line : m_content)
	{
		file << line.c_str() << std::endl;
	}
	file.close();
}

void ReadWriteFile::Load()
{
	std::ifstream file(m_fileName.c_str(), std::ofstream::in);
	if (file.is_open())
	{
		while(file.eof() == false)
		{
			m_content.push_back(std::string());
			getline(file, *m_content.rbegin());
		}
	}
}

unsigned int ReadWriteFile::GetNumLines() const
{
	return m_content.size();
}

std::string ReadWriteFile::GetLine(unsigned int line) const
{
	if (line < GetNumLines())
		return m_content[line];
	return std::string();
}

std::string ReadWriteFile::GetContent() const
{
	std::string result;
	for (const std::string& line : m_content)
		result += line;

	return result;
}

void ReadWriteFile::PushBackLine(const std::string& newText)
{
	PushBackLine(newText.c_str());
}

void ReadWriteFile::PushBackLine(const char* newText)
{
	m_content.push_back(std::string(newText));
}

void ReadWriteFile::SetLine(unsigned int line, const char* newText)
{
	const unsigned int numLines = GetNumLines();
	if (line < numLines)
		m_content[line] = std::string(newText);
	else if (line == numLines)
		PushBackLine(newText);
}

void ReadWriteFile::SetLine(unsigned int line, const std::string& newText)
{
	SetLine(line, newText.c_str());
}

void ReadWriteFile::AppendToLine(unsigned int line, const char* newText)
{
	const unsigned int numLines = GetNumLines();
	if (line < numLines)
		m_content[line].append(newText);
	else if (line == numLines)
		PushBackLine(newText);
}

void ReadWriteFile::AppendToLine(unsigned int line, const std::string& newText)
{
	AppendToLine(line, newText.c_str());
}

void ReadWriteFile::PopLine()
{
	if(GetNumLines() > 0)
		m_content.pop_back();
}

void ReadWriteFile::Clear()
{
	m_content.clear();
}
