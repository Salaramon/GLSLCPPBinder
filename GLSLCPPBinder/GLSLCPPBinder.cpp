
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

#include "TOMLHandling.h"

#include "GLSLCPPBinder.h"

//typedef std::vector<std::pair<std::string, std::pair<std::pair<std::string, std::vector<std::string>>, std::pair<std::string, std::vector<std::string>>>>> GLSLBinderDataStructure;

typedef std::array<std::string, 5> GLSLVariable;

/*
struct GLSLVariable {
	GLSLVariable(std::string name, std::string value, std::string size, std::string type, std::string array) : name(name), value(value), size(size), type(type), array(array) {}
	std::string name;
	std::string value;
	std::string size;
	std::string type;
	std::string array;
};
*/


struct GLSLVariables {
public:
	GLSLVariables() {}
	GLSLVariables(const GLSLVariables& other) : typeName(other.typeName), variables(other.variables), name(other.name) {}
	GLSLVariables(const GLSLVariables&& other) : typeName(other.typeName), variables(other.variables), name(other.name) {
		deepCopy(*this, other);
	}
	void deepCopy(GLSLVariables& current, const GLSLVariables& other) {
		for (size_t i = 0; i < other.structure.size(); i++) {
			current.structure.emplace_back(new GLSLVariables(*other.structure[i]));
			deepCopy(*this->structure.back(), *other.structure[i]);
		}
	}
	
	//GLSLVariables& operator=(GLSLVariables& other) {}
	GLSLVariables(std::string typeName) : typeName(typeName) {}
	std::string typeName;
	std::string name;
	std::vector<GLSLVariable> variables;
	std::vector<std::unique_ptr<GLSLVariables>> structure;
};

std::unordered_map<std::string, std::string> constToValue;

const std::unordered_map<std::string, std::string> GLSLTypeToCPP = {
	{"bool", "bool"},
	{"int", "int"},
	{"uint", "unsigned int"},
	{"float", "float"},
	{"double", "double"},

	{"bvec1", "glm::bvec1"},
	{"bvec2", "glm::bvec2"},
	{"bvec3", "glm::bvec3"},
	{"bvec4", "glm::bvec4"},
	{"ivec1", "glm::ivec1"},
	{"ivec2", "glm::ivec2"},
	{"ivec3", "glm::ivec3"},
	{"ivec4", "glm::ivec4"},
	{"uvec1", "glm::uvec1"},
	{"uvec2", "glm::uvec2"},
	{"uvec3", "glm::uvec3"},
	{"uvec4", "glm::uvec4"},
	{"vec1", "glm::vec1"},
	{"vec2", "glm::vec2"},
	{"vec3", "glm::vec3"},
	{"vec4", "glm::vec4"},
	{"dvec1", "glm::dvec1"},
	{"dvec2", "glm::dvec2"},
	{"dvec3", "glm::dvec3"},
	{"dvec4", "glm::dvec4"},

	{"bmat1", "glm::bmat1"},
	{"bmat2", "glm::bmat2"},
	{"bmat3", "glm::bmat3"},
	{"bmat4", "glm::bmat4"},
	{"imat1", "glm::imat1"},
	{"imat2", "glm::imat2"},
	{"imat3", "glm::imat3"},
	{"imat4", "glm::imat4"},
	{"umat1", "glm::umat1"},
	{"umat2", "glm::umat2"},
	{"umat3", "glm::umat3"},
	{"umat4", "glm::umat4"},
	{"mat1", "glm::mat1"},
	{"mat2", "glm::mat2"},
	{"mat3", "glm::mat3"},
	{"mat4", "glm::mat4"},
	{"dmat1", "glm::dmat1"},
	{"dmat2", "glm::dmat2"},
	{"dmat3", "glm::dmat3"},
	{"dmat4", "glm::dmat4"},
};

const std::unordered_map<std::string, size_t> GLSLTypeSize = {
	{"bool", 4},
	{"int", 4},
	{"uint", 4},
	{"float", 4},
	{"double", 8},

	{"bvec1", 4},
	{"bvec2", 8},
	{"bvec3", 12},
	{"bvec4", 16},
	{"ivec1", 4},
	{"ivec2", 8},
	{"ivec3", 12},
	{"ivec4", 16},
	{"uvec1", 4},
	{"uvec2", 8},
	{"uvec3", 12},
	{"uvec4", 16},
	{"vec1", 4},
	{"vec2", 8},
	{"vec3", 12},
	{"vec4", 16},
	{"dvec1", 8},
	{"dvec2", 16},
	{"dvec3", 24},
	{"dvec4", 32},

	{"bmat1", 4},
	{"bmat2", 16},
	{"bmat3", 36},
	{"bmat4", 64},
	{"imat1", 4},
	{"imat2", 16},
	{"imat3", 36},
	{"imat4", 64},
	{"umat1", 4},
	{"umat2", 16},
	{"umat3", 36},
	{"umat4", 64},
	{"mat1", 4},
	{"mat2", 16},
	{"mat3", 36},
	{"mat4", 64},
	{"dmat1", 8},
	{"dmat2", 32},
	{"dmat3", 72},
	{"dmat4", 128},

};

struct GLSLBinderData {
	GLSLBinderData() : uni("uniform"), loc("location") {}
	GLSLBinderData(const GLSLBinderData&& other) : uni(std::move(other.uni)), loc(std::move(other.loc)), fileName(other.fileName) {}
	std::string fileName;
	GLSLVariables uni;
	GLSLVariables loc;
};

struct GLSLBinderDataStructure : public std::vector<GLSLBinderData> {
};

struct ShaderCodeTypes {
	inline static const char* in = "in";
	inline static const char* uniform = "uniform";

};

namespace ShaderCodeRegex {
	inline const std::regex UNIFORM_STRUCT_VARIABLES("(\\w+)\\s+(\\w+)\\[?([0-9]*?)\\]?\\s*;");
	inline const std::regex UNIFORM_STRUCT("struct\\s+(\\w+)\\s*\\{([\\w\\W]*?(?=\\};))");
	inline const std::regex LOCATION("layout\\s*\\(\\s*location\\s*=\\s*(\\d+)\\s*\\)\\s+in\\s+(\\w+)\\s+(\\w+)\\[?([0-9]*?)\\]?\\s*;");
	inline const std::regex UNIFORM("uniform\\s+(\\w+)\\s+(\\w+)\\[?(\\w+)?\\]?;");
	inline const std::regex SET_VARIABLE("(\\w+)\\s?=\\s?(\\d+);");

};

enum class ATT_INFO {
	LOCATION,
	TYPE,
	NAME,
	ARRAY,
	SIZE
};

enum class UNI_INFO {
	TYPE,
	NAME,
	ARRAY,
	SIZE
};


template<class INFO_TYPE>
std::vector<GLSLVariable> getVariableInfo(std::string shaderCode, std::regex regex) {

	shaderCode = std::regex_replace(shaderCode, std::regex("\\/\\/.+\\n"), "");
	shaderCode = std::regex_replace(shaderCode, std::regex("\\/\\*[\\w\\W]+\\*\\/"), "");

	std::vector<GLSLVariable> variables;
	std::smatch matches;
	while (std::regex_search(shaderCode, matches, regex)) {
		GLSLVariable var;
		for (size_t i = 1; i < matches.size(); i++) {
			 var[i-1] = matches[i].str();
		}
		if (var[(size_t)INFO_TYPE::ARRAY] == "") {
			var[(size_t)INFO_TYPE::ARRAY] = "0";
		}
		else {
			auto it = constToValue.find(var[(size_t)INFO_TYPE::ARRAY]);
			if (it != constToValue.end()) {
				var[(size_t)INFO_TYPE::ARRAY] = it->second;
			}
		}
		bool typeFound = false;
		auto GLSLTypeIt = GLSLTypeSize.find(var[(size_t)INFO_TYPE::TYPE]);
		if (GLSLTypeIt != GLSLTypeSize.end()) {
			typeFound = true;
			var[(size_t)INFO_TYPE::SIZE] = std::to_string(GLSLTypeIt->second);
		}
		else {
			var[(size_t)INFO_TYPE::SIZE] = "0";
		}

		shaderCode = matches.suffix();
		variables.push_back(var);
	}

	return variables;
	
}

void getConstantVariables(std::string shaderCode) {
	std::regex regex = ShaderCodeRegex::SET_VARIABLE;
	shaderCode = std::regex_replace(shaderCode, std::regex("\\/\\/.+\\n"), "");
	shaderCode = std::regex_replace(shaderCode, std::regex("\\/\\*[\\w\\W]+\\*\\/"), "");

	std::smatch matches;
	while (std::regex_search(shaderCode, matches, regex)) {
		constToValue.insert_or_assign(matches[1].str(), matches[2].str());

		shaderCode = matches.suffix();
	}
}


std::vector<GLSLVariables> extractStructs(std::string& shaderCode) {
	
	std::vector<GLSLVariables> result;
	std::string code = shaderCode;
	std::vector<std::string> structCode;
	std::smatch matches;
	while (std::regex_search(code, matches, ShaderCodeRegex::UNIFORM_STRUCT)) {
		GLSLVariables test(matches[1].str());
		result.push_back(test);
		structCode.push_back(matches[2].str());
		code = matches.suffix();
	}

	for (size_t i = 0; i < structCode.size(); i++) {
		result[i].variables = getVariableInfo<UNI_INFO>(structCode[i], ShaderCodeRegex::UNIFORM_STRUCT_VARIABLES);
	}
	shaderCode = std::regex_replace(shaderCode, ShaderCodeRegex::UNIFORM_STRUCT, "");

	
	return result;
}


void expandHeaderStructs(std::fstream& GLSLBinderHeader, GLSLVariables& vars, size_t depth) {
	
	GLSLBinderHeader << std::string(depth, '\t') << "struct " << vars.typeName << "{\n";
	for (auto var : vars.variables) {
		GLSLBinderHeader << std::string(depth + 1, '\t') << "inline static Uniform<" + GLSLTypeToCPP.find(var[(size_t)UNI_INFO::TYPE])->second + "> " + var[(size_t)UNI_INFO::NAME] + "{\"" + var[(size_t)UNI_INFO::TYPE] << "\", \"" << vars.name + "." + var[(size_t)UNI_INFO::NAME] << "\", " << var[(size_t)UNI_INFO::ARRAY] << ", " << var[(size_t)UNI_INFO::SIZE] << "};\n";
	}
	for (auto& s : vars.structure) {
		expandHeaderStructs(GLSLBinderHeader, (*s), depth + 1);
	}
	GLSLBinderHeader << std::string(depth, '\t') << "};\n";
}

std::string strings_equal = "\tconstexpr bool strings_equal(char const* a, char const* b) {\n\treturn *a == *b && (*a == '\\0' || strings_equal(a + 1, b + 1));\n}";
std::string type_list = "\ttemplate<class... Args>\n\tstruct type_list\n{\n\ttemplate <char const* str>\n\tusing type = typename std::tuple_element<get_map_value(str), std::tuple<Args...>>::type;\n\t};";

std::string removeExtension(std::string fileName) {
	size_t extPos = fileName.find(".");
	std::string noExtFile;
	if (extPos != std::string::npos) {
		noExtFile = fileName.substr(0, extPos);
	}
	return noExtFile;
}

bool isUniform(std::vector<GLSLVariables> structs, GLSLVariable var) {
	bool isUniform = true;
	for (auto& sTests : structs) {
		if (var[(size_t)UNI_INFO::TYPE] == sTests.typeName) {
			isUniform = false;
		}
	}
	return isUniform;
}

std::string getVariableName(GLSLVariable var) {
	std::string varName = var[(size_t)UNI_INFO::NAME];
	if (var[(size_t)UNI_INFO::ARRAY] != "0") {
		varName += "[" + var[(size_t)UNI_INFO::ARRAY] + "]";
	}

	return varName;
}

std::string getVariableParameters(std::vector<GLSLVariables> structs, GLSLVariables s, GLSLVariable var, bool insideStruct = false){
	std::stringstream text;
	bool isUniformVariable = isUniform(structs, var);
	std::string varName = getVariableName(var);
	
	std::string type = "";
	auto typeIt = GLSLTypeToCPP.find(var[(size_t)UNI_INFO::TYPE]);
	if (typeIt != GLSLTypeToCPP.end()) {
		type = typeIt->second;
	}

	int arraySize = std::atoi(var[(size_t)UNI_INFO::ARRAY].c_str());
	if (arraySize != 0) {
		for (int i = 0; i < arraySize; i++) {
			if (!isUniformVariable) {
				if (i != arraySize - 1) {
					text << var[(size_t)UNI_INFO::TYPE] << "(String(" << (insideStruct ? "name + \".\" + " : "") << "\"" << var[(size_t)UNI_INFO::NAME] << "[" << i << "]" << "\")), ";
				}
				else {
					text << var[(size_t)UNI_INFO::TYPE] << "(String(" << (insideStruct ? "name + \".\" + " : "") << "\"" << var[(size_t)UNI_INFO::NAME] << "[" << i << "]" << "\"))";
				}
			}
			else {
				if (i != arraySize - 1) {
					text << "Uniform<" << type << ">(\"" << var[(size_t)UNI_INFO::TYPE] << "\", String(" << (insideStruct ? "name + \".\" + " : "") << "\"" << var[(size_t)UNI_INFO::NAME] << "[" << i << "]" << "\"), " << var[(size_t)UNI_INFO::ARRAY] << ", " << var[(size_t)UNI_INFO::SIZE] << "), ";
				}
				else {
					text << "Uniform<" << type << ">(\"" << var[(size_t)UNI_INFO::TYPE] << "\", String(" << (insideStruct ? "name + \".\" + " : "") << "\"" << var[(size_t)UNI_INFO::NAME] << "[" << i << "]" << "\"), " << var[(size_t)UNI_INFO::ARRAY] << ", " << var[(size_t)UNI_INFO::SIZE] << ")";
				}
			}
		}
	}
	else {
		if (!isUniformVariable) {
			text << var[(size_t)UNI_INFO::TYPE] << "(String(" << (insideStruct ? "name + \".\" + " : "") << "\"" << var[(size_t)UNI_INFO::NAME] << "\"))";
		}
		else {
			text << "Uniform<" << type << ">(\"" << var[(size_t)UNI_INFO::TYPE] << "\", String(" << (insideStruct ? "name + \".\" + " : "") << "\"" << var[(size_t)UNI_INFO::NAME] << "\"), " << var[(size_t)UNI_INFO::ARRAY] << ", " << var[(size_t)UNI_INFO::SIZE] << ")";
		}
	}

	return text.str();
}

std::string getVariableLine(std::vector<GLSLVariables> structs, GLSLVariables s, GLSLVariable var) {
	std::stringstream text;
	std::string varName = getVariableName(var);

	bool isUniformVariable = isUniform(structs, var);
	text << (isUniformVariable ? "Uniform<" : "");

	auto it = GLSLTypeToCPP.find(var[(size_t)UNI_INFO::TYPE]);
	if (it != GLSLTypeToCPP.end()) {
		text << GLSLTypeToCPP.find(var[(size_t)UNI_INFO::TYPE])->second;
	}
	else {
		if (isUniformVariable) {
			text << "";
		}
		else {
			text << var[(size_t)UNI_INFO::TYPE];
		}
	}
	
	text << (isUniformVariable ? "> " : " ") << varName;

	return text.str();
}

void generateHeader(GLSLBinderDataStructure variables, std::vector<GLSLVariables> structs) {
	std::fstream GLSLBinderHeader("GLSLCPPBinder.h", std::fstream::in | std::fstream::out | std::ofstream::trunc);

	std::stringstream sstream;
	sstream << GLSLBinderHeader.rdbuf();

	GLSLBinderHeader.seekg(0);

	GLSLBinderHeader << "#pragma once\n\n";
	GLSLBinderHeader << "#include <" << CONFIG::GLM_INCLUDE << "glm.hpp>\n";
	GLSLBinderHeader << "#include <glad/glad.h>\n";
	GLSLBinderHeader << "#include <string>\n";
	GLSLBinderHeader << "#include <tuple>\n";

	GLSLBinderHeader << "#include <array>\n\n";

	GLSLBinderHeader << "namespace Binder {\n\n";

	std::vector<GLSLVariable> vertexVariables;
	for (auto& file : variables) {
		if (file.loc.variables.size() == 3) {
			vertexVariables = file.loc.variables;
			break;
		}
	}

	//String is probably a useless helper class now that uniform initializors are used
	GLSLBinderHeader << "\tstruct String : public std::string {\n";
	GLSLBinderHeader << "\t\tusing std::string::basic_string;\n";
	GLSLBinderHeader << "\t\toperator const char*() const{\n";
	GLSLBinderHeader << "\t\t\treturn c_str();\n";
	GLSLBinderHeader << "\t\t}\n";
	GLSLBinderHeader << "\t};\n\n";

	GLSLBinderHeader << "\tstruct LocationInfo {\n";
	GLSLBinderHeader << "\t\tLocationInfo() : loc(0), type(\"\"), name(\"\"), array(0), size(0) {}\n";
	GLSLBinderHeader << "\t\tLocationInfo(const GLuint loc, String type, String name, const size_t array, const GLint size) : loc(loc), type(type), name(name), array(array), size(size) {}\n";
	GLSLBinderHeader << "\t\tconst GLuint loc;\n";
	GLSLBinderHeader << "\t\tString type;\n";
	GLSLBinderHeader << "\t\tString name;\n";
	GLSLBinderHeader << "\t\tconst GLint size;\n";
	GLSLBinderHeader << "\t\tconst size_t array;\n";
	GLSLBinderHeader << "\t\tsize_t flag;\n";
	GLSLBinderHeader << "\t};\n\n";

	GLSLBinderHeader << "\ttemplate<class T = void>\n";
	GLSLBinderHeader << "\tstruct Location : public LocationInfo {\n";
	GLSLBinderHeader << "\t\tLocation(const GLuint loc, String type, String name, const size_t array, const GLint size) : LocationInfo(loc, type, name, array, size) {}\n";
	GLSLBinderHeader << "\t\tusing type = T;";
	GLSLBinderHeader << "\t};\n\n";

	GLSLBinderHeader << "\tstruct UniformInfo {\n";
	GLSLBinderHeader << "\t\tUniformInfo() : type(\"\"), name(\"\"), array(0), size(0) {}\n";
	GLSLBinderHeader << "\t\tUniformInfo(String type, String name, const size_t array, const GLint size) : type(type), name(name), array(array), size(size) {}\n";
	GLSLBinderHeader << "\t\tString type;\n";
	GLSLBinderHeader << "\t\tString name;\n";
	GLSLBinderHeader << "\t\tconst size_t array;\n";
	GLSLBinderHeader << "\t\tconst GLint size;\n";
	GLSLBinderHeader << "\t\tsize_t flag;\n";
	GLSLBinderHeader << "\t};\n\n";

	GLSLBinderHeader << "\ttemplate<class T = void>\n";
	GLSLBinderHeader << "\tstruct Uniform : public UniformInfo {\n";
	GLSLBinderHeader << "\t\tUniform(String type, String name, const size_t array, const GLint size) : UniformInfo(type, name, array, size) {}\n";
	GLSLBinderHeader << "\t\tusing type = T;";
	GLSLBinderHeader << "\t};\n\n";

	GLSLBinderHeader << strings_equal << "\n";

	GLSLBinderHeader << "\tconstexpr unsigned int get_map_value(char const* a){\n";
	size_t returnValue = 0;
	for (auto& file : variables) {
		if (file.loc.variables.size() != 0) {
			GLSLBinderHeader << "\t\tif (strings_equal(\"" << file.fileName << "\", a)) { return " << returnValue << "; }\n";
			returnValue++;
		}
	}
	GLSLBinderHeader << "\t};\n\n";
	
	GLSLBinderHeader << type_list << "\n";

	for (auto& file : variables) {
		if (file.loc.variables.size() != 0) {
			std::string noExtFile = removeExtension(file.fileName);
			GLSLBinderHeader << "\tstruct AttributeStructure_" << noExtFile << "{\n";
			std::vector<size_t> varLocations;
			size_t varCurrent = 0;
			for (auto& var : file.loc.variables) {
				GLSLBinderHeader << "\t\t" << GLSLTypeToCPP.find(var[(size_t)ATT_INFO::TYPE])->second << " " << var[(size_t)ATT_INFO::NAME] << ";\n";
				varLocations.push_back(varCurrent);
				varCurrent += std::atoi(var[(size_t)ATT_INFO::SIZE].c_str());
			}

			
			GLSLBinderHeader << "\t\tinline static const std::array<size_t, " << file.loc.variables.size() << "> offsets = {";
			for (size_t i = 0; i < varLocations.size(); i++) {
				GLSLBinderHeader << varLocations[i];
				if (i + 1 < varLocations.size()) {
					GLSLBinderHeader << ",";
				}
			}
			GLSLBinderHeader << "};\n";
			
			GLSLBinderHeader << "\t\tinline static const std::array<LocationInfo, " << file.loc.variables.size() << "> locations = {\n";
			for (size_t i = 0; i < file.loc.variables.size(); i++) {
				GLSLBinderHeader << "\t\tLocationInfo(" << file.loc.variables[i][(size_t)ATT_INFO::LOCATION] << ", \"" << file.loc.variables[i][(size_t)ATT_INFO::TYPE] << "\", \"" << file.loc.variables[i][(size_t)ATT_INFO::NAME] << "\", " << file.loc.variables[i][(size_t)ATT_INFO::ARRAY] << ", " << file.loc.variables[i][(size_t)ATT_INFO::SIZE] << ")";
				if (i + 1 < file.loc.variables.size()) {
					GLSLBinderHeader << ",\n";
				}
			}
			GLSLBinderHeader << "};\n";

			GLSLBinderHeader << "\t};\n";
		}
	}
	GLSLBinderHeader << "\n";

	GLSLBinderHeader << "\tstruct AttributeObject {\n";
	GLSLBinderHeader << "\t\ttemplate<char const* str>\n";
	GLSLBinderHeader << "\t\tusing Get = typename type_list<";
	std::vector<std::string> attributes;
	for (size_t i = 0; i < variables.size(); i++) {
		if (variables[i].loc.variables.size() != 0) {
			std::string noExtFile = removeExtension(variables[i].fileName);
			attributes.push_back("AttributeStructure_" + noExtFile);
		}
	}
	for (size_t i = 0; i < attributes.size(); i++) {
		GLSLBinderHeader << attributes[i];
		if (i != attributes.size() - 1) {
			GLSLBinderHeader << ",";
		}
	}
	GLSLBinderHeader << ">::type<str>;\n";
	GLSLBinderHeader << "\t};\n\n";

	GLSLBinderHeader << "\tnamespace file_names{\n";
	for (auto& file : variables) {
		std::string noExtFile = removeExtension(file.fileName);
		GLSLBinderHeader << "\t\tconstexpr char " << noExtFile << "[] = " << "\"" << file.fileName << "\";\n";
	}
	GLSLBinderHeader << "\t}\n\n";

	for (auto& s : structs) {
		GLSLBinderHeader << "\tstruct " << s.typeName << "{\n";
		GLSLBinderHeader << "\t\t" << s.typeName << "(String name) ";
		std::stringstream memberInit;
		bool isFirstMember = true;

		for (size_t i = 0; i < s.variables.size(); i++) {
			if (isFirstMember) {
				isFirstMember = false;
				memberInit << " :\n";
			}
			if(isUniform(structs, s.variables[i])){
				memberInit << "\t\t\t" << s.variables[i][(size_t)UNI_INFO::NAME] << "{" << getVariableParameters(structs, s, s.variables[i], true) << "}";
			}
			else {
				memberInit << "\t\t\t" << s.variables[i][(size_t)UNI_INFO::NAME] << "{" << getVariableParameters(structs, s, s.variables[i], true) << "}";
			}
			if(i != s.variables.size() - 1){
				memberInit << ",\n";
			}
		}
		GLSLBinderHeader << memberInit.str() << "\n\t\t{}\n\n";
		for (auto& var : s.variables) {
			GLSLBinderHeader << "\t\t\t" << getVariableLine(structs, s, var);
			GLSLBinderHeader << ";\n";
		}
		GLSLBinderHeader << "\t};\n\n";
	}

	std::string uniformList;
	std::string inList;
	for (auto& file : variables) {

		std::string noExtFile = removeExtension(file.fileName);

		GLSLBinderHeader << "\struct " << noExtFile << " {\n";
		GLSLBinderHeader << "\t\struct " << file.loc.typeName << "s" << "{\n";
		for (auto var : file.loc.variables) {
			GLSLBinderHeader << "\t\t\tinline static Location<" + GLSLTypeToCPP.find(var[(size_t)ATT_INFO::TYPE])->second + "> " + var[(size_t)ATT_INFO::NAME] + "{" + "Location <" + GLSLTypeToCPP.find(var[(size_t)ATT_INFO::TYPE])->second + ">(" + var[(size_t)ATT_INFO::LOCATION] + ", \"" + var[(size_t)ATT_INFO::TYPE] + "\", \"" + var[(size_t)ATT_INFO::NAME] + "\", " + var[(size_t)ATT_INFO::ARRAY] + ", " + var[(size_t)ATT_INFO::SIZE] + ")};\n";
		}//Fix order how uniforms and locs are passed<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		GLSLBinderHeader << "\t\t};\n";

		GLSLBinderHeader << "\t\tstruct " << file.uni.typeName << "s" << "{\n";
		size_t type = (size_t)UNI_INFO::TYPE;
		for (auto var : file.uni.variables) {
			GLSLBinderHeader << "\t\t\t inline static " << getVariableLine(structs, file.uni, var);
			GLSLBinderHeader << "{";
			GLSLBinderHeader << getVariableParameters(structs, file.uni, var);
			GLSLBinderHeader << "};\n";
		}
		for (auto& vars : file.uni.structure) {
			if (vars != nullptr) {
				expandHeaderStructs(GLSLBinderHeader, (*vars), 3);
			}
		}

		GLSLBinderHeader << "\t\t};\n";
		GLSLBinderHeader << "\t};\n\n";
	}
	GLSLBinderHeader << "};";
};

void deepCopy(GLSLVariables& vars, const GLSLVariables& otherVars) {
	vars.structure.emplace_back(std::unique_ptr<GLSLVariables>(new GLSLVariables(otherVars)));
	for (size_t i = 0; i < otherVars.structure.size(); i++) {
		deepCopy(*vars.structure.back(), *otherVars.structure[i]);
	}
}


template<class INFO_TYPE>
void insertStructsTypes(std::vector<GLSLVariables> structs, GLSLVariables& variables) {
	bool skipIncrement = false;
	for (std::vector<GLSLVariable>::iterator it = variables.variables.begin(); it != variables.variables.end(); ) {
		for (GLSLVariables s : structs) {
			if (it->at((size_t)INFO_TYPE::TYPE) == s.typeName) {
				variables.structure.emplace_back(std::unique_ptr<GLSLVariables>(new GLSLVariables(s)));
				//variables.structure.back()->typeName = it->at((size_t)INFO_TYPE::NAME);
				variables.structure.back()->name = /*(variables.name == "" ? "" : variables.name + ".") +*/ it->at((size_t)INFO_TYPE::NAME);
				insertStructsTypes<INFO_TYPE>(structs, *variables.structure.back());
				it = variables.variables.erase(it);
				skipIncrement = true;
				break;
			}
		}
		if (!skipIncrement) {
			it++;
		}
		skipIncrement = false;
	}
}
/*
constexpr unsigned int get_map_value(char const* a){
	if (strings_equal("float", a)) { return 0; }
	if (strings_equal("best", a)) { return 2; }
}
*/
/*
template<class... Args>
struct type_list
{
	template <char const* str>
	using type = typename std::tuple_element<get_map_value(str), std::tuple<Args...>>::type;
};
*/
/*
struct AttributeObject {
	template<char const* str>
	using Get = typename type_list<float, int, char>::type<str>;
};
*/

constexpr char vertex[] = "float";

int WinMain() {

	//Initialize variables
	int error = 0;
	toml::table tomlFile;
	std::vector<std::string> fileNames;

	error = readTOMLConfig(tomlFile, "GLSLSourceFiles.toml");
	if (error) {
		return error;
	}
	else {
		for (size_t i = 0; i < tomlFile[CONFIG_KEYS::SHADER_FILES].as_array()->size(); i++) {
			fileNames.push_back(tomlFile[CONFIG_KEYS::SHADER_FILES][i].value<std::string>().value());
		}
	}
	
	GLSLBinderDataStructure variables;
	std::vector<GLSLVariables> structs;
	
	for (std::string file : fileNames) {
		std::ifstream shaderFile(file);
		error = shaderFileIsBad(shaderFile, file);
		if (error) {
			return error;
		}
		else {
			GLSLBinderData data;
			data.fileName = file;

			std::stringstream converter;
			converter << shaderFile.rdbuf();
			std::string shaderCode = converter.str();
			
			getConstantVariables(shaderCode);
			auto extractedStructs = extractStructs(shaderCode);
			//structs.insert(structs.end(), extractedStructs.begin(), extractedStructs.end());
			std::copy(extractedStructs.begin(), extractedStructs.end(), std::back_inserter(structs));
			data.loc.variables = getVariableInfo<ATT_INFO>(shaderCode, ShaderCodeRegex::LOCATION);
			data.uni.variables = getVariableInfo<UNI_INFO>(shaderCode, ShaderCodeRegex::UNIFORM);
			//insertStructsTypes<UNI_INFO>(structs, data.uni);
			variables.push_back(std::move(data));
			
		}

		
	}
	
	generateHeader(std::move(variables), structs);
	REPORT::LOG.close();

	return 0;
}