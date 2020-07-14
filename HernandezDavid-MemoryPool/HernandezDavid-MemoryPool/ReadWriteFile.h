#ifndef __READWRITEFILE
#define __READWRITEFILE

#include <string>
#include <vector>

class ReadWriteFile
{
public:
	ReadWriteFile() = delete;
	ReadWriteFile(ReadWriteFile&) = delete;
	ReadWriteFile(const char* file);
	ReadWriteFile(const std::string& file);
	~ReadWriteFile();

	void Save() const;
	void Load(bool append = true);

	unsigned int GetNumLines() const;

	std::string GetLine(unsigned int line) const;
	std::string GetContent() const;

	void PushBackLine(const std::string& newText);
	void PushBackLine(const char* newText);
	void SetLine(unsigned int line, const char* newText);
	void SetLine(unsigned int line, const std::string& newText);
	void AppendToLine(unsigned int line, const std::string& newText);
	void AppendToLine(unsigned int line, const char* newText);
	void PopLine();
	void Clear();

private:
	std::string m_fileName;
	std::vector<std::string> m_content;
};

#endif // !__READWRITEFILE