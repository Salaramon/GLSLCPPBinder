#include "TOMLHandling.h"



bool shaderFileIsBad(std::ifstream& file, std::string fileName) {
	if (file.fail()) {
		if (CONFIG::GENERATE_LOG) {
			REPORT::LOG << "Failed to open shader file: " << fileName << std::endl;
		}
		if (CONFIG::HALT_ON_ERROR) {
			return 1;
		}
		else if (CONFIG::GENERATE_LOG) {
			REPORT::LOG << "Skipping..." << std::endl;
		}
	}

	if (file.good()) {
		if (CONFIG::GENERATE_LOG) {
			REPORT::LOG << "Succesfully opened file " << fileName << std::endl;
		}
	}

	return 0;
}

template<class ValueType>
std::pair<bool, std::string> readTOMLKey(toml::table& tomlFile, std::string key, ValueType& valueRef) {
	std::ostringstream startupMessage;
	bool warning = true;

	if (!tomlFile[key]) {
		startupMessage << "\nWarning: Could not find \"" << key << "\" key in \"" << FILES::TOMLCONFIG << "\"." << std::endl;
	}
	else {
		if (tomlFile[key].value<ValueType>().has_value()) {
			valueRef = tomlFile[key].value<ValueType>().value();
			warning = false;
		}
		else {
			startupMessage << "Warining: Key \"" << key << "\" has no value! Using default value: " << valueRef << std::endl;
		}
	}

	return std::make_pair(warning, startupMessage.str());
}


bool readTOMLConfig(toml::table& tomlFileReference, std::string tomlConfigPath) {

	std::stringstream startupMessage;

	try {
		tomlFileReference = toml::parse_file(tomlConfigPath);
	}
	catch (const toml::parse_error& error) {
		if (CONFIG::GENERATE_LOG) {
			startupMessage << error << "\nCould not find \"" << FILES::TOMLCONFIG << "\" in directory:\n" << std::filesystem::current_path() << std::endl;
		}
		if (CONFIG::HALT_ON_ERROR) {
			return 1;
		}
	}

	auto genLogKeyResult = readTOMLKey(tomlFileReference, CONFIG_KEYS::GENERATE_LOG, CONFIG::GENERATE_LOG);
	if (genLogKeyResult.first) {
		if (CONFIG::GENERATE_LOG == true) {
			REPORT::LOG.open(FILES::LOG);
		}
	}
	if (CONFIG::GENERATE_LOG) {
		REPORT::LOG << startupMessage.rdbuf();
		REPORT::LOG << genLogKeyResult.second;
	}

	auto errHaltKeyResult = readTOMLKey(tomlFileReference, CONFIG_KEYS::HALT_ON_ERROR, CONFIG::HALT_ON_ERROR);
	if (CONFIG::GENERATE_LOG) {
		REPORT::LOG << errHaltKeyResult.second;
	}

	auto errGLMKeyResult = readTOMLKey(tomlFileReference, CONFIG_KEYS::GLM_INCLUDE, CONFIG::GLM_INCLUDE);
	if (CONFIG::GENERATE_LOG) {
		REPORT::LOG << errGLMKeyResult.second;
	}

	return 0;

}