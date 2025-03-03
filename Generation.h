#ifndef GENERATION_H
#define GENERATION_H

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <array>
#include <algorithm>
#include "Tree.h"

class GenCode
{
public:
    GenCode(Tree&& t_synt_tree);
    int GenerateAsm();
    ~GenCode();

private:
    std::vector<std::string> label_vector;
    Tree* synt_tree;
    std::ofstream code;
    std::ostringstream test_str;
    std::map<std::string, std::string> id_map;
    int M = 0;

    const std::array<std::string, 2> types = { "integer", "boolean" };

    static constexpr const char* DATA_SECT = ".data";
    static constexpr const char* CODE_SECT = ".code";
    static constexpr const char* MAIN_SECT = "main:";
    static constexpr const char* END_SECT = "end main";
    static constexpr const char* LONG_SIZE = "4";
    static constexpr const char* BYTE_SIZE = "1";
    static constexpr const char* LONG_TYPE = "dd";
    static constexpr const char* BYTE_TYPE = "db";

    void addLine(const std::string& code_line);
    void buildLine(std::string&& code_line);
    int generateInitVars(Tree* var_root);
    void generateCompound(Tree* compound_tree);
    int generateDeclVars(Tree* varDeclRoot);
    int generateDataVar(Tree* node);
    int generateUninitVars(Tree* var_root);
    int generateBssVar(Tree* node);
    void generateTextPart();
    void processBeginBlock(Tree* beginBlock);
    void storeLabel(Tree* labelDeclNode);
    void generateIfStatement(Tree* expr_tree);
    void generateExpressions(Tree* expr_tree);
    void generateLabel(const std::string& name, const std::string& type, const std::string& val);
    void GenerateFromTree(Tree* node);
    std::string getType(Tree* node);
    std::string GetType(const std::string& Var);
    void clearBuffer();
};

#endif // GENCODE_H