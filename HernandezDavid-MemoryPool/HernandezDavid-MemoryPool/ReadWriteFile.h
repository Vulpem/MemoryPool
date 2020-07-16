#ifndef __READWRITEFILE
#define __READWRITEFILE

#include <string>
#include <vector>

class ReadWriteFile
{
public:
	ReadWriteFile();
	~ReadWriteFile();

	void Save(const std::string& fileName, bool overwriteFile = true) const;
	void Load(const std::string& fileName, bool clearContent = false);

	unsigned int GetNumLines() const;

	std::string GetLine(unsigned int line) const;
	std::string GetContent() const;

	void PushBackLine(const std::string& newText);
	void PushBackLine(const char* newText);
	void SetLine(unsigned int line, const char* newText);
	void SetLine(unsigned int line, const std::string& newText);
	void AppendToLine(unsigned int line, const std::string& newText);
	void AppendToLine(unsigned int line, const char* newText);
	void AppendToEndLine(const std::string& newText);
	void AppendToEndLine(const char* newText);
	void PopLine();
	void Clear();

private:
	std::vector<std::string> m_content;
};

#endif // !__READWRITEFILE