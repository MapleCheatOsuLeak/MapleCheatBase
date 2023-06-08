#include "Config.h"

#include <filesystem>
#include <fstream>

#include "VirtualizerSDK.h"
#include "xorstr.hpp"

#include "../Storage/Storage.h"
#include "../Storage/StorageConfig.h"
#include "../Utilities/Clipboard/ClipboardUtilities.h"
#include "../Utilities/Crypto/CryptoUtilities.h"
#include "../Utilities/Strings/StringUtilities.h"

ImVec4 Config::parseImVec4(std::string vec)
{
	float result[4];
	int pos = 0;
	int index = 0;
	while ((pos = vec.find(',')) != std::string::npos)
	{
		result[index] = std::stof(vec.substr(0, pos));
		vec.erase(0, pos + 1);

		index++;
	}

	result[3] = std::stof(vec);

	return { result[0], result[1], result[2], result[3] };
}

void Config::loadDefaults()
{
	Example::Test = 100;
}

void Config::refresh()
{
	Storage::EnsureDirectoryExists(Storage::ConfigsDirectory);

	Configs.clear();
	Configs.emplace_back(xorstr_("default"));

	for (const auto& file : std::filesystem::directory_iterator(Storage::ConfigsDirectory))
		if (file.path().extension() == xorstr_(".cfg") && Storage::IsValidFileName(file.path().filename().stem().string()))
			Configs.push_back(file.path().filename().stem().string());
}

void Config::Initialize()
{
	refresh();

	const auto it = std::find(Configs.begin(), Configs.end(), StorageConfig::DefaultConfig);

	if (it != Configs.end())
		CurrentConfig = std::distance(Configs.begin(), it);
	else
		CurrentConfig = 0;

	Load();
}

void Config::Load()
{
	Storage::EnsureDirectoryExists(Storage::ConfigsDirectory);
	loadDefaults(); //load default config first to ensure that old configs are fully initialized

	StorageConfig::DefaultConfig = Configs[CurrentConfig];
	Storage::SaveStorageConfig();

	if (CurrentConfig == 0)
		return;

	std::string configFilePath = Storage::ConfigsDirectory + xorstr_("\\") + Configs[CurrentConfig] + xorstr_(".cfg");

	if (!std::filesystem::exists(configFilePath))
		return;

	std::ifstream file(configFilePath);
	std::string line;

	float configVersion = 0.f;
	while (std::getline(file, line))
	{
		const int delimiterIndex = line.find('=');
		std::string variable = line.substr(0, delimiterIndex);
		std::string value = line.substr(delimiterIndex + 1, std::string::npos);

		if (variable == xorstr_("Version"))
			configVersion = std::stof(value);

		if (variable == xorstr_("Example_Test"))
			Example::Test = value == xorstr_("1");
	}

	file.close();
}

void Config::Save()
{
	if (CurrentConfig == 0)
		return;

	Storage::EnsureDirectoryExists(Storage::ConfigsDirectory);

	const std::string configFilePath = Storage::ConfigsDirectory + xorstr_("\\") + Configs[CurrentConfig] + xorstr_(".cfg");

	std::ofstream ofs;
	ofs.open(configFilePath, std::ofstream::out | std::ofstream::trunc);

	ofs << xorstr_("Version=") << VERSION << std::endl;

	ofs << xorstr_("Example_Test=") << Example::Test << std::endl;

	ofs.close();
}

void Config::Delete()
{
	Storage::EnsureDirectoryExists(Storage::ConfigsDirectory);

	if (CurrentConfig == 0)
		return;

	const std::string configFilePath = Storage::ConfigsDirectory + xorstr_("\\") + Configs[CurrentConfig] + xorstr_(".cfg");

	std::filesystem::remove(configFilePath);

	refresh();

	CurrentConfig = 0;

	Load();
}

void Config::Import()
{
	Storage::EnsureDirectoryExists(Storage::ConfigsDirectory);

	const std::string encodedConfigData = ClipboardUtilities::Read();

	if (encodedConfigData.empty())
		return;

	const std::string decodedConfigData = CryptoUtilities::MapleXOR(CryptoUtilities::Base64Decode(encodedConfigData), xorstr_("xbb9tuvQCGJRhN8z"));
	const std::vector<std::string> decodedConfigDataSplit = StringUtilities::Split(decodedConfigData, "|");

	if (decodedConfigDataSplit.size() < 2 || decodedConfigDataSplit.size() > 2)
		return;

	std::string configName = CryptoUtilities::Base64Decode(decodedConfigDataSplit[0]);
	const std::string configData = CryptoUtilities::Base64Decode(decodedConfigDataSplit[1]);

	std::string configFilePath = Storage::ConfigsDirectory + xorstr_("\\") + configName + xorstr_(".cfg");

	if (!Storage::IsValidFileName(configName))
		return;

	if (std::filesystem::exists(configFilePath))
	{
		unsigned int i = 2;
		while (true)
		{
			const std::string newConfigName = configName + xorstr_("_") + std::to_string(i);
			const std::string newConfigFilePath = Storage::ConfigsDirectory + xorstr_("\\") + newConfigName + xorstr_(".cfg");
			if (!std::filesystem::exists(newConfigFilePath))
			{
				configName = newConfigName;
				configFilePath = newConfigFilePath;

				break;
			}

			i++;
		}
	}

	std::ofstream ofs(configFilePath);
	ofs << configData << std::endl;
	ofs.close();

	refresh();

	const auto it = std::find(Configs.begin(), Configs.end(), configName);

	if (it != Configs.end())
		CurrentConfig = std::distance(Configs.begin(), it);

	Load();
}

void Config::Export()
{
	Storage::EnsureDirectoryExists(Storage::ConfigsDirectory);

	if (CurrentConfig == 0)
		return;

	const std::string configFilePath = Storage::ConfigsDirectory + xorstr_("\\") + Configs[CurrentConfig] + xorstr_(".cfg");

	std::ifstream ifs(configFilePath);
	const std::string configData((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	ifs.close();

	const std::string encodedConfigData = CryptoUtilities::Base64Encode(CryptoUtilities::MapleXOR(CryptoUtilities::Base64Encode(Configs[CurrentConfig]) + xorstr_("|") + CryptoUtilities::Base64Encode(configData), xorstr_("xbb9tuvQCGJRhN8z")));

	ClipboardUtilities::Write(encodedConfigData);
}

void Config::Rename()
{
	Storage::EnsureDirectoryExists(Storage::ConfigsDirectory);

	if (CurrentConfig == 0)
		return;

	const std::string configFilePath = Storage::ConfigsDirectory + xorstr_("\\") + Configs[CurrentConfig] + xorstr_(".cfg");

	if (!Storage::IsValidFileName(RenamedConfigName) || Storage::IsSameFileName(RenamedConfigName, Configs[CurrentConfig]))
		return;

	std::string renamedConfigName = RenamedConfigName;
	std::string renamedConfigFilePath = Storage::ConfigsDirectory + xorstr_("\\") + renamedConfigName + xorstr_(".cfg");

	if (std::filesystem::exists(renamedConfigFilePath))
	{
		unsigned int i = 2;
		while (true)
		{
			const std::string newConfigName = renamedConfigName + xorstr_("_") + std::to_string(i);
			const std::string newConfigFilePath = Storage::ConfigsDirectory + xorstr_("\\") + newConfigName + xorstr_(".cfg");
			if (!std::filesystem::exists(newConfigFilePath))
			{
				renamedConfigName = newConfigName;
				renamedConfigFilePath = newConfigFilePath;

				break;
			}

			i++;
		}
	}

	std::rename(configFilePath.c_str(), renamedConfigFilePath.c_str());

	refresh();

	const auto it = std::find(Configs.begin(), Configs.end(), renamedConfigName);

	if (it != Configs.end())
		CurrentConfig = std::distance(Configs.begin(), it);
}

void Config::Create()
{
	Storage::EnsureDirectoryExists(Storage::ConfigsDirectory);

	std::string configName = NewConfigName;
	std::string configFilePath = Storage::ConfigsDirectory + xorstr_("\\") + NewConfigName + xorstr_(".cfg");

	if (!Storage::IsValidFileName(configName))
		return;

	if (std::filesystem::exists(configFilePath))
	{
		unsigned int i = 2;
		while (true)
		{
			const std::string newConfigName = configName + xorstr_("_") + std::to_string(i);
			const std::string newConfigFilePath = Storage::ConfigsDirectory + xorstr_("\\") + newConfigName + xorstr_(".cfg");
			if (!std::filesystem::exists(newConfigFilePath))
			{
				configName = newConfigName;
				configFilePath = newConfigFilePath;

				break;
			}

			i++;
		}
	}

	std::ofstream ofs(configFilePath);
	ofs.close();

	refresh();

	const auto it = std::find(Configs.begin(), Configs.end(), configName);

	if (it != Configs.end())
		CurrentConfig = std::distance(Configs.begin(), it);

	loadDefaults();
}