#pragma once

#include <sstream>
#include <vector>
#include <filesystem>
#include <toml++/toml.h>
#include <fstream>
#include <memory>
#include <string>
#include <regex>
#include <iostream>
#include <unordered_map>
#include <array>
#include <memory>

struct CONFIG {
	inline static bool GENERATE_LOG = true;
	inline static bool HALT_ON_ERROR = true;
	inline static std::string GLM_INCLUDE = "";
};

struct FILES {
	inline static const char* TOMLCONFIG = "GLSLCPPBinder.toml";
	inline static std::string LOG = "GLSLCPPBinder.log";
};

struct REPORT {
	inline static std::ofstream LOG;
};

struct CONFIG_KEYS {
	inline static const char* GENERATE_LOG = "Generate_Log";
	inline static const char* HALT_ON_ERROR = "Halt_On_Error";
	inline static const char* SHADER_FILES = "Shader_Files";
	inline static const char* GLM_INCLUDE = "GLM_Include";

};

bool shaderFileIsBad(std::ifstream& file, std::string fileName);

template<class ValueType>
std::pair<bool, std::string> readTOMLKey(toml::table& tomlFile, std::string key, ValueType& valueRef);

bool readTOMLConfig(toml::table& tomlFileReference, std::string tomlConfigPath);
