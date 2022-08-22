
#include <string>
#include <fstream>
#include <vector>
#include <algorithm> 
class IniReader
{
public:
	bool OpenFile(const char* Filepath);
	bool SaveFile(const char* Filepath);
	int GetGroupCount();
	bool Close();
	bool WriteStrings(std::string AppName, std::vector<std::string>& strings);
	std::vector<std::string> GetStrings(std::string AppName);
	std::string GetString(std::string AppName, std::string KeyName, std::string DefaultValue = "");
    int GetInt(std::string AppName, std::string KeyName, int DefaultValue = 0);
    float GetFloat(std::string AppName, std::string KeyName, float DefaultValue = 0.0f);
    bool GetBool(std::string AppName, std::string KeyName, bool DefaultValue = false);
	bool WriteString(std::string AppName, std::string KeyName, std::string Value);
	struct AppIndex { std::string Name; int Index = -1; AppIndex(int ind, std::string name) { Name = name, Index = ind; }; };
	std::vector<AppIndex> Applines;
	std::vector<std::string> Lines;
private:
	std::fstream f;
};