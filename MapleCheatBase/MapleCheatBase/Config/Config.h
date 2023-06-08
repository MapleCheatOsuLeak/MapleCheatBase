#pragma once

#include <comdef.h>
#include <string>
#include <vector>

#include "imgui.h"

class Config
{
	static ImVec4 parseImVec4(std::string vec);
	static void loadDefaults();
	static void refresh();
public:
	static inline std::vector<std::string> Configs;
	static inline int CurrentConfig = 0;
	static inline char RenamedConfigName[24] = { };
	static inline char NewConfigName[24] = { };

	static void Initialize();
	static void Load();
	static void Save();
	static void Delete();
	static void Import();
	static void Export();
	static void Rename();
	static void Create();

	//Version 0.0 - first version
	//Version 1.0 - breaks aim assist config compatibility (removed aav1, added aav3 (aqn))
	static inline constexpr float VERSION = 1.f;

	struct Example
	{
		static inline int Test;
	};
};