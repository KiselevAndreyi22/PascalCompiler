#ifndef SINTAX_ANALIZ_H
#define SINTAX_ANALIZ_H
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "Tree.h"
#include "Lexem.h"
#include "variable.h"
#include "MyList.h"
#include "Stack.h"

using namespace std;

class Syntaxx {
public:

    int begin_state = 0; // ��� ��������� begin
    string tek_type = "0";
    MyList <string> result;
    MyList <string> label_setup;
    mylib::Vector <string> brackets;
    mylib::Vector<Lexem> lex_table; // out table of lexemes
    std::map<std::string, Variable> id_map; // our table of identifiers
    Tree* root;
    int label_string;
    int label_set;
    bool cpb_setup = true;
    int brack_o = 0;
    int brack_c = 0;
    int Errors = 0;

    using lex_it = mylib::Vector<Lexem>::Iterator; // alias of vector iterator
    lex_it cursor;
    Stack <string> stack;

    // ����������� ��������� Vector �� rvalue-������ � ���������� ���
    Syntaxx(mylib::Vector<Lexem>&& t_lex_table)
        : lex_table(std::move(t_lex_table)), cursor(lex_table.begin())
    {
        if (lex_table.empty()) {
            std::cout << "Lexemes table is empty!" << std::endl;
        }
        else if (lex_table.at(0).GetToken() == eof_tk) {
            std::cout << "Opened file is empty!" << std::endl;
        }
    }

    // �������� �� �������� ��������� �������
    lex_it getNextLex(lex_it iter) {
        ++iter;
        return iter;
    }

    // ������������� �� ������� ������
    bool checkLexem(const lex_it& t_iter, const tokens& t_tok) {
        if (t_iter == lex_table.end()) {
            return false;
        }
        return t_iter->GetToken() == t_tok;
    }

    //�������� ������������� ����������
    bool isVarExist(const string& t_var_name)
    {
        auto map_iter = id_map.find(t_var_name);
        return !(map_iter == id_map.end());
    }

    //�������� ��� ����������
    string getVarType(const string& t_var_name)
    {
        auto map_iter = id_map.find(t_var_name);
        return map_iter->second.type;
    }

    void printIdMap() const {
        cout << "Identifier Table:" << endl;
        if (id_map.empty()) {
            cout << "  (empty)" << endl;
            return;
        }
        for (const auto& pair : id_map) {
            cout << "  " << pair.first << ": { type: " << pair.second.type
                << " }" << endl;
        }
    }

    void preorderTrav(Tree* root) {
        if (root == nullptr) {
            return;
        }

        // ������������ ������� ����
        cout << root->value << " ";

        // ���������� ������� ����� ���������
        preorderTrav(root->left);

        // ���������� ������� ������ ���������
        preorderTrav(root->right);
    }

    Tree* buildExprTree(MyList<string> result) {
        Tree* rootK = new Tree("op:");
        Tree* cur_k = nullptr;
        Stack<Tree*> stack;

        for (const string& operand : result) {
            if (operand == "+" || operand == "-" || operand == "*" || operand == "div") {
                if (stack.getSize() == 1) { // ���� ������ ���������
                    if (operand == "-") {
                        if (cur_k == nullptr) {
                            Tree* zerO = new Tree("0");
                            Tree* UnOp = new Tree("" + operand); //������� �������
                            UnOp->AddRightTree(stack.top()); // ��������� ����� �� ����� ������
                            UnOp->AddLeftTree(zerO); // ����� ��������� ����
                            cur_k = UnOp;
                            if (cur_k != nullptr) {
                                if (rootK->GetName() == "op:") {
                                    rootK = cur_k;
                                }
                                else {
                                    rootK->AddRightTree(cur_k);
                                }
                            }
                            cur_k = rootK;
                            stack.pop();
                        }
                        else {
                            if (stack.getSize() == 2) {
                                Tree* Op = new Tree("" + operand); //������� ��������
                                Op->AddRightTree(rootK); // ��������� ����� �� ����� ������
                                Op->AddLeftTree(stack.top());
                                cur_k = Op;
                                rootK = cur_k;
                                stack.pop();
                            }
                            else if (stack.getSize() == 1) {
                                cur_k->AddRightTree(stack.top());
                                stack.pop();
                                if (operand == "-") {
                                    Tree* zerO = new Tree("0");
                                    Tree* Op = new Tree("" + operand); //������� �������
                                    Op->AddRightTree(cur_k); // ��������� ����� �� ����� ������
                                    Op->AddLeftTree(zerO);
                                    rootK = Op;
                                }
                            }
                        }
                    }
                    else if (operand == "+") {
                        Tree* Op = new Tree("" + operand); //������� ��������
                        if (stack.getSize() == 1) {
                            Op->AddLeftTree(stack.top());
                            Op->AddRightTree(cur_k);
                            rootK = Op;
                            stack.pop();
                            continue;
                        }
                        else {
                            Op->AddRightTree(rootK); // ��������� ����� �� ����� ������
                            Op->AddLeftTree(stack.top());
                            cur_k = Op;
                            rootK = cur_k;
                            stack.pop();
                        }
                    }
                    else if (operand == "*") {
                        Tree* Op = new Tree("" + operand); //������� ��������
                        Op->AddRightTree(rootK); // ��������� ����� �� ����� ������
                        Op->AddLeftTree(stack.top());
                        cur_k = Op;
                        rootK = cur_k;
                        stack.pop();
                    }
                    else if (operand == "div") {
                        if (cur_k != nullptr) {
                            Tree* Op = new Tree("" + operand); //������� ��������
                            Op->AddRightTree(rootK); // ��������� ����� �� ����� ������
                            Op->AddLeftTree(stack.top());
                            cur_k = Op;
                            rootK = cur_k;
                            stack.pop();
                        }
                        else {
                            Tree* Op = new Tree("" + operand); //������� ��������
                            Op->AddLeftTree(stack.top());
                            cur_k = Op;
                            rootK = cur_k;
                            stack.pop();
                        }
                    }

                }
                else if (stack.getSize() == 2) {
                    if (cur_k == nullptr) {
                        Tree* Op = new Tree("" + operand); //������� ��������
                        Op->AddRightTree(stack.top()); // ��������� ����� �� ����� ������
                        stack.pop();
                        Op->AddLeftTree(stack.top());
                        cur_k = Op;
                        if (cur_k != nullptr) {
                            rootK = cur_k;
                        }
                        stack.pop();
                    }
                    else if (cur_k != nullptr) {
                        Tree* Op = new Tree("" + operand); //������� ��������
                        if (stack.empty()) {
                            Op->AddRightTree(cur_k); // ��������� ����� �� ����� ������
                            Op->AddLeftTree(stack.top());
                            cur_k = Op;
                            if (cur_k != nullptr) {
                                rootK = cur_k;
                            }
                            stack.pop();
                        }
                        else {
                            Op->AddRightTree(cur_k);
                            Op->AddLeftTree(stack.top());
                            stack.pop();
                            if (stack.empty()) {
                                if (cur_k != nullptr) {
                                    rootK = Op;
                                }
                            }
                            rootK = Op;
                        }
                    }
                }
                else if (stack.getSize() > 2) {
                    if (cur_k == nullptr) {
                        Tree* Op = new Tree("" + operand); //������� ��������
                        Op->AddRightTree(stack.top()); // ��������� ����� �� ����� ������
                        stack.pop();
                        Op->AddLeftTree(stack.top());
                        cur_k = Op;
                        rootK = cur_k;
                        stack.pop();
                    }
                    else {
                        Tree* Op = new Tree("" + operand); //������� ��������
                        Op->AddRightTree(stack.top()); // ��������� ����� �� ����� ������
                        stack.pop();
                        Op->AddLeftTree(stack.top());
                        cur_k = Op;
                        if (cur_k != nullptr) {
                            rootK->AddRightTree(cur_k);
                        }
                        stack.pop();
                    }
                }
                else if (stack.empty()) {
                if (operand == "-") { // ������� �����
                    Tree* Op = new Tree("" + operand); //������� �������
                    Tree* zerO = new Tree("0");
                    Op->AddLeftTree(zerO);
                    Op->AddRightTree(cur_k);
                    rootK = Op;
                    }
                else {
                    Tree* Op = new Tree("" + operand); //������� ��������
                    Op->AddRightTree(cur_k); // ��������� ����� �� ����� ������
                    Op->AddLeftTree(rootK);
                    rootK = Op;
                }
                }
            }
            else {
                stack.push(new Tree(operand)); // ������� � ����
                if (result.size() == 1) {
                    return rootK = new Tree("" + operand);
                }
            }
        }

        return rootK;
    }

    Tree* buildExpressTree(MyList<std::string>& postfix) {
        Stack<Tree*> stack;

        for (size_t i = 0; i < postfix.size(); ++i) {
            const std::string& token = postfix[i];

            // ���� ����� - ����� (�������), ������� ����
            if (isdigit(token[0])) {  // ������������, ��� ����� - ������ � ������
                Tree* node = new Tree(token);  // �������� ���� � ���������
                stack.push(node);
            }
            // ���� ����� - ��������
            else if (token == "+" || token == "-") {
                Tree* node = nullptr;

                // ��� �������� ���������� (�������� + ��� -), ������ ���� ��� �������� � �����
                if (stack.getSize() >= 2) {
                    Tree* right = stack.top();  // ��������� ������ �������
                    stack.pop();
                    Tree* left = stack.top();   // ��������� ����� �������
                    stack.pop();

                    node = new Tree(token);  // �������� ���� � ����������
                    node->AddLeftTree(left);  // ��������� ����� �������
                    node->AddRightTree(right);  // ��������� ������ �������
                }
                // ��� �������� ������, � ����� ������ ���� ������ ���� �������
                else if (token == "-" && stack.getSize() >= 1) {
                    Tree* right = stack.top();  // ��������� �������
                    stack.pop();
                    Tree* left = new Tree("0");  // ��� �������� ������ ������� "0" ��� ����� �������

                    node = new Tree(token);  // �������� ���� � ���������� "-"
                    node->AddLeftTree(left);  // ��������� ����� ������� (0)
                    node->AddRightTree(right);  // ��������� ������ �������
                }
                else {
                    std::cerr << "������: ������������ ��������� ��� ��������� " << token << std::endl;
                    return nullptr;
                }

                stack.push(node);  // ��������� ��������� ���� ������� � ����
            }
        }

        // ����� ��������� ���� ���������, � ����� ������ �������� ������ ���� ������� (������ ������)
        if (stack.getSize() != 1) {
            std::cerr << "������: ����� ��������� ��� � ����� ������ �������� ����� 1 �������." << std::endl;
            return nullptr;  // ���������� nullptr, ���� � ����� ������ ������ ��������
        }

        return stack.top();  // ���������� ������ ������
    }

    // ��� ������� - ����� ������ � ���������� ������� (������ �����)
    void printPreorder(Tree* node) {
        if (node) {
            std::cout << node->value << " ";
            printPreorder(node->left);
            printPreorder(node->right);
        }
    }

    void printTree(Tree* root, int level = 0) {
        if (root) {
            // 1. ������� ���������� ������ ���������
            printTree(root->right, level + 1);

            // 2. ���������� ������� ���� � ��������
            cout << string(4 * level, ' ') << "-> " << root->value << endl;

            // 3. ����� � ����� ���������
            printTree(root->left, level + 1);
        }
    }

    int programParse(lex_it& t_iter, Tree* parent) {
        // PROGRAMPARSING
        if (checkLexem(t_iter, program_tk)) { // ������� �������� � ������ PROGRAM
            auto iter = getNextLex(t_iter);
            if (!checkLexem(iter, id_tk)) { // �������� �� �������� ���������
                printError(MUST_BE_ID, *iter);
                Errors++;
                return -EXIT_FAILURE;
            }
            Tree* NAMEPROG = Tree::CreateNode(iter->GetName());
            parent->AddLeftTree(NAMEPROG);
            iter++;

            // �������� ���� BLOCK � ���������� ��� ��� ����� ��� Program
            Tree* BLOCK = Tree::CreateNode("BLOCK");
            parent->AddRightTree(BLOCK);

            // ��������� ';' ����� �������� ���������
            if (!checkLexem(iter, semicolon_tk)) {
                printError(MUST_BE_SEMI, *iter);
                Errors++;
                return -EXIT_FAILURE;
            }

            iter++;

            // VARPARSING: ��������� ���������� ����������
            while (!checkLexem(iter, begin_tk)) { // ���� �� ���������� BEGIN
                // ��������� ����������
                while (checkLexem(iter, var_tk)) {
                    iter++; // ��������� ����� 'var'

                    // ���� ���� ���������������
                    MyList<string> var_list;
                    while (checkLexem(iter, id_tk)) {
                        string varName = iter->GetName();

                        // �������� �� ��������� ����������
                        if (isVarExist(varName)) {
                            cout << "Re-declared identifier '" << varName << "'" << endl;
                            Errors++;
                            return -EXIT_FAILURE;
                        }
                        else {
                            // ��������� ������������� ���������� � ����������� �����
                            id_map.emplace(varName, Variable("?", "?", 0));
                            var_list.push_back(varName);
                        }

                        iter++;

                        if (checkLexem(iter, comma_tk)) {
                            iter++; // ���������� ������� � ����������
                            if (!checkLexem(iter, id_tk)) {
                                printError(MUST_BE_ID, *iter);
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        else {
                            break; // ��� �������, ����������� ���� ����������
                        }
                    }

                    // ����� ������ ���������� ������ ���� ':'
                    if (!checkLexem(iter, colon_tk)) {
                        cout << "Must be ':' after variable names." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    iter++; // ��������� ����� ':'

                    // �������� ���� ����������
                    if (!checkLexem(iter, integer_tk) && !checkLexem(iter, boolean_tk)) {
                        cout << "Unknown type." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    string type_iter = iter->GetName();
                    iter++; // ��������� ����� ����

                    // �������� �� ';' ����� ���������� ����������
                    if (!checkLexem(iter, semicolon_tk)) {
                        cout << "Must be ';' after variable declaration." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    iter++; // ��������� ����� ';'

                    // ���������� ����� ���������� � ����� ���������������
                    for (const string& varName : var_list) {
                        id_map.at(varName).type = type_iter;
                    }

                    // �������� ���������� VarDecl ��� ������ ����������
                    for (const string& varName : var_list) {
                        // �������� ���� VarDecl
                        Tree* currentDecl = Tree::CreateNode("VarDecl");

                        // ���������� VarDecl ��� ������� ��� BLOCK
                        if (BLOCK->GetLeftNode() == nullptr) {
                            BLOCK->AddLeftTree(currentDecl);
                        }
                        else {
                            Tree* sibling = BLOCK->GetLeftNode();
                            while (sibling->GetRightNode() != nullptr) {
                                sibling = sibling->GetRightNode();
                            }
                            sibling->AddRightTree(currentDecl);
                        }

                        // ���������� ����� ���������� ��� ����� ���� VarDecl
                        Tree* varNode = Tree::CreateNode(varName);
                        currentDecl->AddLeftTree(varNode);

                        // ���������� ���� ��� ������ ���� VarDecl
                        Tree* typeNode = Tree::CreateNode(type_iter);
                        varNode->AddRightTree(typeNode);
                    }
                }

                // ��������� LABELS
                if (checkLexem(iter, label_tk)) {
                    iter++;
                    // ��������� ����������������
                    if (!checkLexem(iter, id_tk)) { // �������� �� �������������
                        cout << "Must be identifier." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }

                    MyList<string> label_list;

                    // ���� �����
                    while (checkLexem(iter, id_tk)) {
                        string labelName = iter->GetName();

                        // �������� �� ��������� ����������
                        if (isVarExist(labelName)) {
                            cout << "Re-declared identifier '" << labelName << "'" << endl;
                            Errors++;
                            return -EXIT_FAILURE;
                        }
                        else {
                            id_map.emplace(labelName, Variable("label", "?", 0));
                            label_list.push_back(labelName);
                        }

                        iter++;

                        if (checkLexem(iter, comma_tk)) {
                            iter++; // ���������� ������� � ����������
                            if (!checkLexem(iter, id_tk)) {
                                printError(MUST_BE_ID, *iter); 
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        else {
                            break; // ��� �������, ����������� ���� �����
                        }
                    }

                    // �������� �� ';' ����� ���������� �����
                    if (!checkLexem(iter, semicolon_tk)) {
                        cout << "Must be ';' after label declarations." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    iter++; // ��������� ����� ';'

                    // �������� ���������� LabelDecl ��� ������ �����
                    for (const string& labelName : label_list) {
                        // �������� ���� LabelDecl
                        Tree* currentLabelDecl = Tree::CreateNode("LabelDecl");

                        // ���������� LabelDecl ��� ������� ��� BLOCK
                        if (BLOCK->GetLeftNode() == nullptr) {
                            BLOCK->AddLeftTree(currentLabelDecl);
                        }
                        else {
                            Tree* sibling = BLOCK->GetLeftNode();
                            while (sibling->GetRightNode() != nullptr) {
                                sibling = sibling->GetRightNode();
                            }
                            sibling->AddRightTree(currentLabelDecl);
                        }

                        // ���������� ����� ����� ��� ����� ���� LabelDecl
                        Tree* labelNode = Tree::CreateNode(labelName);
                        currentLabelDecl->AddLeftTree(labelNode);
                    }
                }
                // ��������� BEGIN
                else if (!checkLexem(iter, begin_tk)) {
                    cout << "Expected 'begin'." << endl;
                    Errors++;
                    return -EXIT_FAILURE;
                }
            }
            iter++;

            //�������� ������ COMPOUND
            Tree* COMPOUND = Tree::CreateNode("BEGIN");
            BLOCK->AddRightTree(COMPOUND);

            // ������� ������ �����
            lex_it label_iteration;
            label_iteration = iter;

            while (!checkLexem(label_iteration, eof_tk)) {
                if (checkLexem(label_iteration, id_tk)) {
                    if (getVarType(label_iteration->GetName()) == "label") {
                        label_iteration--;
                        if (label_iteration->GetName() != "goto") {
                            label_iteration++;
                            label_setup.push_back(label_iteration->GetName());
                        }
                        else {
                            label_iteration++;
                        }
                    }
                }
                checkLexem(label_iteration, eof_tk);
                label_iteration++;
            }

            // �������������� ���, ��� ��������� ��� BEGIN
            while (!checkLexem(iter, eof_tk)) {

                //label
                // �������� �� ��������������
                if (checkLexem(iter, id_tk)) {
                    if (isVarExist(iter->GetName())) {

                        //���� ���������� �������� ������, �� ����� ��������� �������;
                        if (getVarType(iter->GetName()) == "label") {

                            //��������� ������������� �����
                            label_iteration = iter;
                            for (const string& operand : label_setup) {
                                if (operand == label_iteration->GetName()) {
                                    label_string++;
                                    if (label_string > 1) {
                                        cout << "Repeated label '" << iter->GetName() << "' initialization on line " << iter->GetLine() << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                }
                                label_iteration++;
                            }
                            label_setup.push_back(iter->GetName());
                            //label_setup = iter->GetName();

                            //������ � �������
                            Tree* currentLabel = Tree::CreateNode(iter->GetName());

                            if (COMPOUND->GetLeftNode() == nullptr) {
                                COMPOUND->AddLeftTree(currentLabel);
                            }
                            else {
                                Tree* sibling = COMPOUND->GetLeftNode();
                                while (sibling->GetRightNode() != nullptr) {
                                    sibling = sibling->GetRightNode();
                                }
                                sibling->AddRightTree(currentLabel);
                            }

                            //��������� ������
                            iter++;
                            if (!checkLexem(iter, colon_tk)) {
                                cout << "Must be ':' after label. " << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        //���� ���������� BOOLEAN, �� ����� ��������� �������
                        else if (getVarType(iter->GetName()) == "boolean") {

                            //�������� ���� ����� ����������
                            Tree* currentBool = Tree::CreateNode(iter->GetName());

                            iter--;
                            if(checkLexem(iter,then_tk)){
                                iter++;
                                if (COMPOUND->GetLeftNode() == nullptr) {
                                    COMPOUND->AddLeftTree(currentBool);
                                }
                                else {
                                    Tree* sibling = COMPOUND->GetLeftNode();
                                    Tree* lastLeaf = nullptr;  // ���������� ��������� ���� ��� ��������

                                    while (sibling != nullptr) {
                                        if (sibling->GetLeftNode() == nullptr && sibling->GetRightNode() == nullptr) {
                                            lastLeaf = sibling;  // ��������� ��������� ���� ��� ��������
                                        }

                                        // ��� ������ �� ������� �������, ���� �� ����, ����� �� ������
                                        if (sibling->GetRightNode() != nullptr) {
                                            sibling = sibling->GetRightNode();
                                        }
                                        else if (sibling->GetLeftNode() != nullptr) {
                                            sibling = sibling->GetLeftNode();
                                        }
                                        else {
                                            break;  // ��������� ����, ���� ������ ��� ��������
                                        }
                                    }

                                    // ��������� ����� ���� � ���������� ���������� ���� ��� ��������
                                    if (lastLeaf->GetLeftNode() == nullptr) {
                                        lastLeaf->AddLeftTree(currentBool);
                                        currentBool = lastLeaf;
                                    }
                                    else {
                                        lastLeaf->AddRightTree(currentBool);
                                    }
                                }
                            }

                            else if (checkLexem(iter, else_tk)) {
                                iter++;
                                if (COMPOUND->GetLeftNode() == nullptr) {
                                    COMPOUND->AddLeftTree(currentBool);
                                }
                                else {
                                    Tree* sibling = COMPOUND->GetLeftNode();
                                    Tree* lastLeaf = nullptr;  // ���������� ��������� ���� ��� ��������

                                    while (sibling != nullptr) {
                                        if (sibling->GetLeftNode() == nullptr && sibling->GetRightNode() == nullptr) {
                                            lastLeaf = sibling;  // ��������� ��������� ���� ��� ��������
                                        }

                                        // ��� ������ �� ������� �������, ���� �� ����, ����� �� ������
                                        if (sibling->GetRightNode() != nullptr) {
                                            sibling = sibling->GetRightNode();
                                        }
                                        else if (sibling->GetLeftNode() != nullptr) {
                                            sibling = sibling->GetLeftNode();
                                        }
                                        else {
                                            break;  // ��������� ����, ���� ������ ��� ��������
                                        }
                                    }

                                    // ��������� ����� ���� � ���������� ���������� ���� ��� ��������
                                    if (lastLeaf->GetLeftNode() == nullptr) {
                                        lastLeaf->AddLeftTree(currentBool);
                                        
                                    }
                                    else {
                                        lastLeaf->AddRightTree(currentBool);
                                    }
                                }
                            }

                            else {
                                iter++;
                                //���������� � ������ ����� ����������
                                if (COMPOUND->GetLeftNode() == nullptr) {
                                    COMPOUND->AddLeftTree(currentBool);
                                }
                                else {
                                    Tree* sibling = COMPOUND->GetLeftNode();
                                    while (sibling->GetRightNode() != nullptr) {
                                        sibling = sibling->GetRightNode();
                                    }
                                    sibling->AddRightTree(currentBool);
                                }
                            }

                            //��������� �� ������
                            iter++;
                            if (!checkLexem(iter, assignment_tk)) {
                                cout << "Must be ':=' on line " << iter->GetLine() << "." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }

                            //���������� � ������ :=
                            Tree* ASSIGN = Tree::CreateNode(iter->GetName());
                            currentBool->AddLeftTree(ASSIGN);

                            iter++;
                            if (!checkLexem(iter, bool_true_tk)) {
                                if (!checkLexem(iter, bool_false_tk)) {
                                    if (checkLexem(iter, constant_tk)) {
                                        cout << "Couldn't convert boolean to integer on line " << iter->GetLine() << "." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }

                                    if (isVarExist(iter->GetName())) {
                                        if (getVarType(iter->GetName()) == "integer") {

                                            Tree* BInt = Tree::CreateNode(iter->GetName());
                                            ASSIGN->AddLeftTree(BInt);

                                            iter++;
                                            if(checkLexem(iter, bool_less_tk) || checkLexem(iter, bool_bigger_tk)){

                                                Tree* OpSr = Tree::CreateNode(iter->GetName());
                                                BInt->AddLeftTree(OpSr);

                                                iter++;
                                                if (checkLexem(iter, id_tk) || checkLexem(iter, constant_tk)) {
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {

                                                                Tree* BInt2 = Tree::CreateNode(iter->GetName());
                                                                OpSr->AddLeftTree(BInt2);

                                                                iter++;
                                                                if (checkLexem(iter, semicolon_tk)) {
                                                                    iter++;
                                                                    continue;
                                                                }
                                                                else if (checkLexem(iter, end_tk)) {
                                                                    continue;
                                                                }
                                                                else if (checkLexem(iter, else_tk)) {
                                                                    continue;
                                                                }
                                                                else {
                                                                    cout << "Expected for ',' on line " << iter->GetLine() << "." << endl;
                                                                    Errors++;
                                                                    return -EXIT_FAILURE;
                                                                }
                                                            }
                                                        }
                                                    }
                                                    else if (checkLexem(iter, constant_tk)) {

                                                        Tree* BInt2 = Tree::CreateNode(iter->GetName());
                                                        OpSr->AddLeftTree(BInt2);

                                                        iter++;
                                                        if (checkLexem(iter, semicolon_tk)) {
                                                            continue;
                                                        }
                                                        else if (checkLexem(iter, end_tk)) {
                                                            continue;
                                                        }
                                                        else {
                                                            cout << "Expected for ',' on line " << iter->GetLine() << "." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                }
                                            }
                                            else {
                                                cout << "Couldn't convert boolean to integer." << endl;
                                                Errors++;
                                                return -EXIT_FAILURE;
                                            }
                                        }
                                        else if (getVarType(iter->GetName()) == "label") {
                                            cout << "Expected for var name." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                        else if (isVarExist(iter->GetName())) {
                                            if (getVarType(iter->GetName()) == "boolean") {

                                                Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                ASSIGN->AddLeftTree(VARNAME);

                                                //��������� ���� �� ��������� ���������
                                                iter++;
                                                // <
                                                if (checkLexem(iter, bool_less_tk)) {
                                                    //���� ��� �������, �� ���������� � ������
                                                    Tree* LESS = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(LESS);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '<' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }

                                                            //���� ��� �������, �� ���������� � ������
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            LESS->AddLeftTree(VARNAME);

                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                }
                                                // >
                                                else if (checkLexem(iter, bool_bigger_tk)) {
                                                    //���� ��� �������,�� ���������� � ������
                                                    Tree* BIGGER = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(BIGGER);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '>' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            //���� ��� �������, �� ���������� � ������
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            BIGGER->AddLeftTree(VARNAME);
                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                }
                                                // =
                                                else if (checkLexem(iter, equals_tk)) {
                                                    //���� ��� �������,�� ���������� � ������
                                                    Tree* EQUALS = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(EQUALS);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '=' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            //���� ��� �������, �� ���������� � ������
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            EQUALS->AddLeftTree(VARNAME);
                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                }
                                                // <=
                                                else if (checkLexem(iter, bool_leseqv_tk)) {
                                                    //���� ��� �������,�� ���������� � ������
                                                    Tree* LESEQV = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(LESEQV);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '<=' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            //���� ��� �������, �� ���������� � ������
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            LESEQV->AddLeftTree(VARNAME);
                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                }
                                                // >=
                                                else if (checkLexem(iter, bool_bigeqv_tk)) {
                                                    //���� ��� �������,�� ���������� � ������
                                                    Tree* BIGEQV = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(BIGEQV);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '>=' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            //���� ��� �������, �� ���������� � ������
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            BIGEQV->AddLeftTree(VARNAME);
                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                }
                                                // <>
                                                else if (checkLexem(iter, bool_noneqv_tk)) {
                                                    //���� ��� �������,�� ���������� � ������
                                                    Tree* NONEQV = Tree::CreateNode(iter->GetName());
                                                    VARNAME->AddLeftTree(NONEQV);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {
                                                        if (isVarExist(iter->GetName())) {
                                                            if (getVarType(iter->GetName()) == "integer") {
                                                                cout << "Operation '<>' can't be used to boolean and integer types." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            else if (getVarType(iter->GetName()) == "label") {
                                                                cout << "Expected for variable name." << endl;
                                                                Errors++;
                                                                return -EXIT_FAILURE;
                                                            }
                                                            //���� ��� �������, �� ���������� � ������
                                                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                                            NONEQV->AddLeftTree(VARNAME);
                                                        }
                                                        else {
                                                            cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                        }
                                                    }
                                                    else {
                                                        cout << "Unknown indentifire '" << iter->GetName() << "' ." << endl;
                                                        Errors++;
                                                        return -EXIT_FAILURE;
                                                    }
                                                    //iter--;
                                                }
                                            }
                                        }
                                        else {
                                            cout << "Expected for expression." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else {
                                        cout << "Unknown indentifire '" << iter->GetName() << "'." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                }
                            }
                            //iter--;
                            if (checkLexem(iter, bool_true_tk)) {

                                Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                ASSIGN->AddLeftTree(VARNAME);

                                iter++;
                                if (checkLexem(iter, semicolon_tk)) {
                                    iter++;
                                    continue;
                                }
                                else if (!checkLexem(iter, semicolon_tk)) {

                                    if (!checkLexem(iter, end_tk)) {
                                        cout << "Met '" << iter->GetName() << "' but wait for ',' on line " << iter->GetLine() << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }

                                }

                            }
                            if (checkLexem(iter, bool_false_tk)) {

                                Tree* VARNAME = Tree::CreateNode(iter->GetName());
                                ASSIGN->AddLeftTree(VARNAME);

                                iter++;
                                if (checkLexem(iter, semicolon_tk)) {
                                    iter++;
                                    continue;
                                }
                                else if (!checkLexem(iter, semicolon_tk)) {

                                    if (!checkLexem(iter, end_tk)) {
                                        if (checkLexem(iter, else_tk)) {// ���� � ��� ����� else
                                            //iter++;
                                            continue;
                                        }
                                        cout << "Met '" << iter->GetName() << "' but wait for ',' on line " << iter->GetLine() << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }

                                }

                            }
                            if (checkLexem(iter, end_tk)) {
                                iter++;
                                if (checkLexem(iter, dot_tk)) { // ���� ��� ��������� end
                                    if (begin_state == 1) {
                                        cout << "Unexpected 'end' on line " << iter->GetLine() << "." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                    else {
                                        iter++;
                                        break;
                                    }
                                }
                                else { //���� ���, �� ���������� ������
                                    begin_state--;
                                    continue;
                                }
                            }
                        }
                        //���� ���������� �������� integer, �� ����� ��������� �������
                        //������ :=
                        else if (getVarType(iter->GetName()) == "integer") {

                            Tree* VARNAME = Tree::CreateNode(iter->GetName());
                            //COMPOUND->AddLeftTree(VARNAME);
                            iter--;
                            if (checkLexem(iter, then_tk)) {
                                iter++;
                                if (COMPOUND->GetLeftNode() == nullptr) {
                                    COMPOUND->AddLeftTree(VARNAME);
                                }
                                else {
                                    Tree* sibling = COMPOUND->GetLeftNode();
                                    Tree* lastLeaf = nullptr;  // ���������� ��������� ���� ��� ��������

                                    while (sibling != nullptr) {
                                        if (sibling->GetLeftNode() == nullptr && sibling->GetRightNode() == nullptr) {
                                            lastLeaf = sibling;  // ��������� ��������� ���� ��� ��������
                                        }

                                        // ��� ������ �� ������� �������, ���� �� ����, ����� �� ������
                                        if (sibling->GetRightNode() != nullptr) {
                                            sibling = sibling->GetRightNode();
                                        }
                                        else if (sibling->GetLeftNode() != nullptr) {
                                            sibling = sibling->GetLeftNode();
                                        }
                                        else {
                                            break;  // ��������� ����, ���� ������ ��� ��������
                                        }
                                    }

                                    // ��������� ����� ���� � ���������� ���������� ���� ��� ��������
                                    if (lastLeaf->GetLeftNode() == nullptr) {
                                        lastLeaf->AddLeftTree(VARNAME);
                                        VARNAME = lastLeaf;
                                    }
                                    else {
                                        lastLeaf->AddRightTree(VARNAME);
                                    }
                                }
                            }

                            else if (checkLexem(iter, else_tk)) {
                                iter++;
                                if (COMPOUND->GetLeftNode() == nullptr) {
                                    COMPOUND->AddLeftTree(VARNAME);
                                }
                                else {
                                    Tree* sibling = COMPOUND->GetLeftNode();
                                    Tree* lastLeaf = nullptr;  // ���������� ��������� ���� ��� ��������

                                    while (sibling != nullptr) {
                                        if (sibling->GetLeftNode() == nullptr && sibling->GetRightNode() == nullptr) {
                                            lastLeaf = sibling;  // ��������� ��������� ���� ��� ��������
                                        }

                                        // ��� ������ �� ������� �������, ���� �� ����, ����� �� ������
                                        if (sibling->GetRightNode() != nullptr) {
                                            sibling = sibling->GetRightNode();
                                        }
                                        else if (sibling->GetLeftNode() != nullptr) {
                                            sibling = sibling->GetLeftNode();
                                        }
                                        else {
                                            break;  // ��������� ����, ���� ������ ��� ��������
                                        }
                                    }

                                    // ��������� ����� ���� � ���������� ���������� ���� ��� ��������
                                    if (lastLeaf->GetLeftNode() == nullptr) {
                                        lastLeaf->AddLeftTree(VARNAME);

                                    }
                                    else {
                                        lastLeaf->AddRightTree(VARNAME);
                                    }
                                }
                            }

                            else if (checkLexem(iter, begin_tk)) {
                                iter++;
                                if (COMPOUND->GetLeftNode() == nullptr) {
                                    COMPOUND->AddLeftTree(VARNAME);
                                }
                                else {
                                    Tree* sibling = COMPOUND->GetLeftNode();
                                    Tree* lastLeaf = nullptr;  // ���������� ��������� ���� ��� ��������

                                    while (sibling != nullptr) {
                                        if (sibling->GetLeftNode() == nullptr && sibling->GetRightNode() == nullptr) {
                                            lastLeaf = sibling;  // ��������� ��������� ���� ��� ��������
                                        }

                                        // ��� ������ �� ������� �������, ���� �� ����, ����� �� ������
                                        if (sibling->GetRightNode() != nullptr) {
                                            sibling = sibling->GetRightNode();
                                        }
                                        else if (sibling->GetLeftNode() != nullptr) {
                                            sibling = sibling->GetLeftNode();
                                        }
                                        else {
                                            break;  // ��������� ����, ���� ������ ��� ��������
                                        }
                                    }

                                    // ��������� ����� ���� � ���������� ���������� ���� ��� ��������
                                    if (lastLeaf->GetLeftNode() == nullptr) {
                                        lastLeaf->AddLeftTree(VARNAME);

                                    }
                                    else {
                                        lastLeaf->AddRightTree(VARNAME);
                                    }
                                }
                            }

                            else if (begin_state >= 1) {
                                iter++;

                                if (COMPOUND->GetLeftNode() == nullptr) {
                                    COMPOUND->AddLeftTree(VARNAME);
                                }
                                else {
                                    Tree* sibling = COMPOUND->GetLeftNode();

                                    // ��������� � ���������� ���� �� ����� ������, ����� �������� ������ �������
                                    while (sibling->GetRightNode() != nullptr) {
                                        sibling = sibling->GetRightNode()->GetLeftNode();
                                    }

                                    // ��������� ����� ���� ��� ������� �������
                                    sibling->GetLeftNode()->AddRightTree(VARNAME);
                                }
                            }

                            else {
                                iter++;
                                //���������� � ������ ����� ����������
                                if (COMPOUND->GetLeftNode() == nullptr) {
                                    COMPOUND->AddLeftTree(VARNAME);
                                }
                                else {
                                    Tree* sibling = COMPOUND->GetLeftNode();
                                    while (sibling->GetRightNode() != nullptr) {
                                        sibling = sibling->GetRightNode();
                                    }
                                    sibling->AddRightTree(VARNAME);
                                }
                            }

                            iter++;
                            if (!checkLexem(iter, assignment_tk)) {
                                cout << "Must be ':='." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }

                            Tree* ASSIGN = Tree::CreateNode(iter->GetName());
                            VARNAME->AddLeftTree(ASSIGN);

                            iter++;

                            //��������� ���������
                            while (!checkLexem(iter, end_tk) || !checkLexem(iter, semicolon_tk)) {
                                // ���� ������� ������� � ��������� (����� ��� ����������)
                                if (checkLexem(iter, constant_tk) || checkLexem(iter, id_tk)) {
                                    if (checkLexem(iter, id_tk)) {
                                        if (isVarExist(iter->GetName())){
                                            if (getVarType(iter->GetName()) == "integer") {
                                                result.push_back(iter->GetName()); // ��������� � � ���������
                                                    iter++; // ��������� � ��������� �������

                                                    //������������ ������, ���� ��� ��� ��������� �������������� ��� ���������
                                                    if (checkLexem(iter, const_tk) || checkLexem(iter, id_tk)) {
                                                        cout << "Met '" << iter->GetName() << "' but wait for ',' on line " << iter->GetLine() << endl;
                                                            Errors++;
                                                            return -EXIT_FAILURE;
                                                    }
                                                //���� ����� ���������, �� ������� �� �����
                                                if (checkLexem(iter, semicolon_tk)) {
                                                    break;
                                                }

                                                if (checkLexem(iter, end_tk)) {
                                                    break;
                                                }

                                                // ���� ��������� m * (m)
                                                if (checkLexem(iter, cpb_tk)) { // ����������� ������ ")"
                                                    // ��������� �� ����� �� ��� ���, ���� �� �������� ����������� ������ "("
                                                    while (!stack.empty() && stack.top() != "(") {
                                                        result.push_back(stack.top());
                                                        stack.pop(); // ������� �������� �� �����
                                                    }

                                                    // ������� ���� ����������� ������ �� �����
                                                    if (!stack.empty() && stack.top() == "(") {
                                                        stack.pop();
                                                    }

                                                    //iter++;
                                                    continue;
                                                }
                                                //������������ ������, ���� ������ ������ �������� 
                                                else if (!checkLexem(iter, plus_tk) & !checkLexem(iter, minus_tk) & !checkLexem(iter, mul_tk) & !checkLexem(iter, div_tk)) {
                                                    cout << "Expected procedure name instead of '" << iter->GetName() << "' on line " << iter->GetLine() << " ." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                                else {
                                                    continue;
                                                }

                                            }
                                            else {
                                                cout << "Cannot convert to integer '" << iter->GetName() << "' on line " << iter->GetLine() << endl;
                                                Errors++;
                                                return -EXIT_FAILURE;
                                            }
                                        }
                                        else {
                                            cout << "Uknown indentifire '" << iter->GetName() << "' on line." << iter->GetLine() << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else if (checkLexem(iter, constant_tk)) {
                                        result.push_back(iter->GetName()); // ��������� � � ���������
                                        iter++; // ��������� � ��������� �������
                                        continue;
                                    }
                                }

                                // ���� ������� ������� � ���� ��� �����
                                if (checkLexem(iter, plus_tk) || checkLexem(iter, minus_tk)) {
                                    std::string current_op = iter->GetName();  // �������� ������� �������� (���� ��� �����)

                                    // ��������, �������� �� ����� �������
                                    if (current_op == "-" && (result.empty() || result.back() == "(" || result.back() == "," || result.back() == ":=")) {
                                        // ��� ������� �����, ������ ��������� ��� � ����
                                        stack.push(current_op);
                                    }
                                    else {
                                        // ��� ��������� ������, ������������ ��������� � ����� ������� �����������
                                        while (!stack.empty() && (stack.top() == "*" || stack.top() == "div" || stack.top() == "+" || stack.top() == "-")) {
                                            result.push_back(stack.top());
                                            stack.pop();
                                        }
                                        stack.push(current_op);  // ��������� �������� ����� � ����
                                    }
                                    iter++;  // ������� � ��������� �������
                                    continue;  // ������� � ��������� ��������
                                }

                                if (checkLexem(iter, opb_tk)) { // ����������� ������ "("
                                    //�������� �� ��, ��� ���������� ���� ��������
                                    iter--;
                                    if (!checkLexem(iter, plus_tk) & !checkLexem(iter, minus_tk) & !checkLexem(iter, mul_tk) & !checkLexem(iter, div_tk)) {
                                        if (checkLexem(iter, assignment_tk)) {
                                            iter++;

                                            brackets.push_back(iter->GetName()); // ��������� ������ � ������
                                            stack.push(iter->GetName()); // ������ ��������� � � ����

                                            iter++;
                                            continue;
                                        }
                                        cout << "Met '(' on line " << iter->GetLine() << " instead operator." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                    iter++;
                                    if (!checkLexem(iter, constant_tk) || !checkLexem(iter, id_tk)) {
                                        if (checkLexem(iter, opb_tk)) {
                                            //iter--;
                                            brackets.push_back(iter->GetName()); // ��������� ������ � ������
                                            stack.push(iter->GetName()); // ������ ��������� � � ����
                                            iter++;
                                        }
                                        else {
                                            cout << "Exprected for expression after '(' on " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }

                                    continue;
                                }

                                if (checkLexem(iter, cpb_tk)) { // ����������� ������ ")"
                                    // ��������� �� ����� �� ��� ���, ���� �� �������� ����������� ������ "("
                                    iter--;
                                    if (!checkLexem(iter, constant_tk) & !checkLexem(iter, id_tk)) {
                                        if (checkLexem(iter, cpb_tk)) {

                                        }
                                        else {
                                            cout << "Exprected for expression before ')' on " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    iter++;
                                    while (!stack.empty() && stack.top() != "(") {
                                        result.push_back(stack.top());
                                        stack.pop(); // ������� �������� �� �����
                                    }

                                    // ������� ���� ����������� ������ �� �����
                                    if (!stack.empty() && stack.top() == "(") {
                                        stack.pop();
                                    }

                                    brackets.push_back(iter->GetName());

                                    //��������� ���� �� �� ����� �������� ������
                                    for (size_t i = 0; i < brackets.size(); ++i) {
                                        if (brackets[i] == "(") {
                                            brack_o += 1;
                                            break;
                                        }
                                    }

                                    if (brack_o == 0) {
                                        cout << "Expected for '(' on line " << iter->GetLine() << "." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }

                                    iter++;

                                    if (checkLexem(iter, semicolon_tk)) {
                                        continue;
                                    }
                                    //��������� ������������ ���������� ��������� �������
                                    else if (!checkLexem(iter, plus_tk) & !checkLexem(iter, minus_tk) & !checkLexem(iter, mul_tk) & !checkLexem(iter, div_tk)) {
                                        if (checkLexem(iter, cpb_tk)) {

                                        }
                                        else if (checkLexem(iter, end_tk)) {
                                            break;
                                        }
                                        else {
                                            cout << "Expected for operator instead '" << iter->GetName() << "' on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else {
                                        continue;
                                    }

                                }

                                // ���� ������� ������� - ��������� ��� �������
                                if (checkLexem(iter, mul_tk) || checkLexem(iter, div_tk)) {
                                    std::string current_op = iter->GetName(); // �������� ������� �������� ("*" ��� "div")

                                    // ��������� ������ ��������� � ����� ������� ����������� ("*" � "div")
                                    while (!stack.empty() && (stack.top() == "*" || stack.top() == "div")) {
                                        result.push_back(stack.top());
                                        stack.pop(); // ������� �������� �� �����
                                    }

                                    //��������� ��� �� ������, ��� ����� ���������� ������ ���
                                    iter--;
                                    if (!checkLexem(iter, cpb_tk) && !checkLexem(iter, id_tk) && !checkLexem(iter, constant_tk)){
                                        cout << "Expected for expression instead 'div' on " << iter->GetLine() << "." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                    iter++;

                                    stack.push(current_op); // ��������� ������� �������� � ����
                                    iter++;

                                    // ��������� �� ����������� �� ��������� 2 ���� (a *div (3+4))
                                    if (!checkLexem(iter, constant_tk) & !checkLexem(iter, id_tk) & !checkLexem(iter, opb_tk)) {
                                        if (checkLexem(iter, minus_tk) || checkLexem(iter, plus_tk)) {
                                            continue;
                                        }
                                        else {
                                            cout << "Met '" << iter->GetName() << " instead expression on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else {
                                        continue;
                                    }
                                }

                                //����� �� ����� ���� ����. ����� ';'
                                if (checkLexem(iter, semicolon_tk)) {
                                    break;
                                }
                                if (checkLexem(iter, else_tk)) {
                                    break;
                                }


                                //����� �� ����� ���� ����.����� 'end'
                                if (checkLexem(iter, end_tk)) {
                                    break;
                                }

                                //������ ���� ��������� goto
                                if (checkLexem(iter, goto_tk)) {
                                    cout << "Met '" << iter->GetName() << "' instead ',' on line " << iter->GetLine() << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                            }

                            // ����� ���������� ��������� ������ ��������� ��� ���������� ��������� �� �����
                            while (!stack.empty()) {
                                result.push_back(stack.top());
                                stack.pop();
                            }

                            //��� �������� �������� �������� ������

                            for (const string& operand : result) {
                                // �������� ���� VarDecl
                                cout << operand << endl;
                            }

                            if (!brackets.empty()) {
                                brack_o = 0;
                                brack_c = 0;
                                for (size_t i = 0; i < brackets.size(); ++i) {
                                    if (brackets[i] == "(") {
                                        brack_o++;
                                    }
                                    if (brackets[i] == ")") {
                                        brack_c++;
                                    }
                                }

                                if (brack_o < brack_c) {
                                    cout << "Waited for bracket '(' on " << iter->GetLine() << "." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                                else if (brack_o > brack_c) {
                                    cout << "Waited for bracket ')' on " << iter->GetLine() << "." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }

                                for (size_t i = 0; i < brackets.size(); ++i) {
                                    brackets[i].push_back('0');
                                }
                            }

                            Tree* expr = buildExprTree(result);
                            expr->PrintASCII();
                            result.clear();

                            //��������� ��������� � ������
                            if (ASSIGN->GetLeftNode() == nullptr) {
                                ASSIGN->AddLeftTree(expr);
                            }

                            if (checkLexem(iter, end_tk)) {
                                continue;
                            }

                            if (checkLexem(iter, else_tk)) {
                                continue;
                            }

                        }
                    }

                    else {
                        cout << "Unknown indentifier. '" << iter->GetName() << "'" << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }

                    iter++;

                }

                //��������� ELSE
                else if (checkLexem(iter, else_tk)) {

                //������ � �������
                Tree* currentElse = Tree::CreateNode(iter->GetName());

                if (COMPOUND->GetLeftNode() == nullptr) {
                    COMPOUND->AddLeftTree(currentElse);
                }
                else {
                    Tree* sibling = COMPOUND->GetLeftNode();
                    Tree* lastThenNode = nullptr;       // ��������� ���� � ������ "then"
                    Tree* previousThenNode = nullptr;   // ���������� ���� � ������ "then"

                    while (sibling != nullptr) {
                        if (sibling->GetName() == "then") {
                            previousThenNode = lastThenNode;  // ��������� ���������� ���� "then"
                            lastThenNode = sibling;           // ��������� ��������� ���� "then"
                        }

                        // ��� ������ �� ������: ������� ������, ����� �����
                        if (sibling->GetRightNode() != nullptr) {
                            sibling = sibling->GetRightNode();
                        }
                        else if (sibling->GetLeftNode() != nullptr) {
                            sibling = sibling->GetLeftNode();
                        }
                        else {
                            break;  // ��������� ����, ���� ������ ��� ��������
                        }
                    }

                    // ��������� ������� ��� ���������� ����
                    if (lastThenNode != nullptr) {
                        if (lastThenNode->GetRightNode() != nullptr && lastThenNode->GetRightNode()->GetName() == "else") {
                            // ���� � ���������� "then" ��� ���� "else", ��������� ���� � ����������� "then"
                            if (previousThenNode != nullptr) {
                                previousThenNode->AddRightTree(currentElse);
                            }
                        }
                        else {
                            // ��������� ����� ���� ������ � ���������� ���������� ���� "then"
                            lastThenNode->AddRightTree(currentElse);
                        }
                    }
                }
                    
                    iter--; //��������� ���� �������
                    if (!checkLexem(iter, end_tk)) {
                        iter++;
                        iter++;
                        continue;
                        /*iter++;
                        cout << "Met '" << iter->GetName() << "' but expected operator on line " << iter->GetLine() << "." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                        */
                    }
                    else if (checkLexem(iter, end_tk)){
                        iter++; //������������ �� ������� �������
                        iter++; //���������� �� ��������� �������
                        if (checkLexem(iter, begin_tk)) {
                            begin_state = 1;

                            Tree* Begin = new Tree("begin");
                            iter--;
                            if (!checkLexem(iter, then_tk) && !checkLexem(iter, else_tk)) { //���� if �� ���������
                                iter++;
                                if (COMPOUND->GetLeftNode() == nullptr) {
                                    COMPOUND->AddLeftTree(Begin);
                                }
                                else {
                                    Tree* sibling = COMPOUND->GetLeftNode();
                                    while (sibling->GetRightNode() != nullptr) {
                                        sibling = sibling->GetRightNode();
                                    }
                                    sibling->AddRightTree(Begin);
                                }
                            }
                            else if (checkLexem(iter, then_tk)) { //���� ��������� ����� then �� ��������� �����
                                iter++;
                                if (checkLexem(iter, begin_tk)) {
                                    iter++;
                                    if (checkLexem(iter, if_tk)) {
                                        iter++;
                                        Tree* sibling = COMPOUND->GetLeftNode();
                                        while (sibling != nullptr) {
                                            if (sibling->GetName() == "begin" && sibling->GetLeftNode() == nullptr) {
                                                sibling->AddLeftTree(Begin);  // ��������� ���� currentIf �����
                                                break;  // ��������� ����, ��� ��� ������ ���� ������
                                            }

                                            // ��� ������ �� ������: ������� ������, ����� �����
                                            if (sibling->GetRightNode() != nullptr) {
                                                sibling = sibling->GetRightNode();
                                            }
                                            else if (sibling->GetLeftNode() != nullptr) {
                                                sibling = sibling->GetLeftNode();
                                            }
                                            else {
                                                break;  // ��������� ����, ���� ������ ��� ��������
                                            }
                                        }
                                    }
                                }

                                Tree* sibling = COMPOUND->GetLeftNode();
                                while (sibling != nullptr) {
                                    if (sibling->GetName() == "then" && sibling->GetLeftNode() == nullptr) {
                                        sibling->AddLeftTree(Begin);  // ��������� ���� currentIf �����
                                        break;  // ��������� ����, ��� ��� ������ ���� ������
                                    }

                                    // ��� ������ �� ������: ������� ������, ����� �����
                                    if (sibling->GetRightNode() != nullptr) {
                                        sibling = sibling->GetRightNode();
                                    }
                                    else if (sibling->GetLeftNode() != nullptr) {
                                        sibling = sibling->GetLeftNode();
                                    }
                                    else {
                                        break;  // ��������� ����, ���� ������ ��� ��������
                                    }
                                }

                            }
                            else if (checkLexem(iter, else_tk)) { //���� ��������� ����� else �� ��������� �����
                                iter++;
                                Tree* sibling = COMPOUND->GetLeftNode();
                                while (sibling != nullptr) {
                                    if (sibling->GetName() == "else" && sibling->GetLeftNode() == nullptr) {
                                        sibling->AddLeftTree(Begin);  // ��������� ���� currentIf �����
                                        break;  // ��������� ����, ��� ��� ������ ���� ������
                                    }

                                    // ��� ������ �� ������: ������� ������, ����� �����
                                    if (sibling->GetRightNode() != nullptr) {
                                        sibling = sibling->GetRightNode();
                                    }
                                    else if (sibling->GetLeftNode() != nullptr) {
                                        sibling = sibling->GetLeftNode();
                                    }
                                    else {
                                        break;  // ��������� ����, ���� ������ ��� ��������
                                    }
                                }

                            }

                            iter++;//���� ����� begin, �� ���� �� ����� ����� ��� ���� � �����������
                            continue;
                        }
                        else {
                            //iter--; //���� �� ����� ����� ��� ����� ���������� ������
                            continue; // ���� �� �����, �� ����������� ��� ����� begin
                        }
                    }
                    iter++;
                }

                //��������� BEGIN 
                else if (checkLexem(iter, begin_tk)) {

                //������ � �������
                Tree* currentBegin = Tree::CreateNode(iter->GetName());

                iter--;
                if (!checkLexem(iter, then_tk) && !checkLexem(iter, else_tk)) { //���� if �� ���������
                    iter++;
                    if (COMPOUND->GetLeftNode() == nullptr) {
                        COMPOUND->AddLeftTree(currentBegin);
                    }
                    else {
                        Tree* sibling = COMPOUND->GetLeftNode();
                        while (sibling->GetRightNode() != nullptr) {
                            sibling = sibling->GetRightNode();
                        }
                        sibling->AddRightTree(currentBegin);
                    }
                }
                else if (checkLexem(iter, then_tk)) { //���� ��������� ����� then �� ��������� �����
                    iter++;
                    Tree* sibling = COMPOUND->GetLeftNode();
                    while (sibling != nullptr) {
                        if (sibling->GetName() == "then" && sibling->GetLeftNode() == nullptr) {
                            sibling->AddLeftTree(currentBegin);  // ��������� ���� currentIf �����
                            break;  // ��������� ����, ��� ��� ������ ���� ������
                        }

                        // ��� ������ �� ������: ������� ������, ����� �����
                        if (sibling->GetRightNode() != nullptr) {
                            sibling = sibling->GetRightNode();
                        }
                        else if (sibling->GetLeftNode() != nullptr) {
                            sibling = sibling->GetLeftNode();
                        }
                        else {
                            break;  // ��������� ����, ���� ������ ��� ��������
                        }
                    }
                }
                else if (checkLexem(iter, else_tk)) { //���� ��������� ����� else �� ��������� �����
                    iter++;
                    Tree* sibling = COMPOUND->GetLeftNode();
                    while (sibling != nullptr) {
                        if (sibling->GetName() == "else" && sibling->GetLeftNode() == nullptr) {
                            sibling->AddLeftTree(currentBegin);  // ��������� ���� currentIf �����
                            break;  // ��������� ����, ��� ��� ������ ���� ������
                        }

                        // ��� ������ �� ������: ������� ������, ����� �����
                        if (sibling->GetRightNode() != nullptr) {
                            sibling = sibling->GetRightNode();
                        }
                        else if (sibling->GetLeftNode() != nullptr) {
                            sibling = sibling->GetLeftNode();
                        }
                        else {
                            break;  // ��������� ����, ���� ������ ��� ��������
                        }
                    }

                }

                    begin_state++;
                    iter++;
                    continue;
                }

                //��������� END (� � ����� ��������� end)
                else if (checkLexem(iter, end_tk)){
                    iter++;
                    if (!checkLexem(iter, dot_tk)) {
                        if (!checkLexem(iter, else_tk)) {
                            if (checkLexem(iter, end_tk)) {//���� ����� ��� ���� ��������� end
                                begin_state--;
                                if (begin_state < 0) {
                                    cout << "Unexpected 'end' on line " << iter->GetLine() - 1 << "." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                                continue;
                            }
                            else {
                                cout << "Met '" << iter->GetName() << "' but expected ',' on line " << iter->GetLine() << "." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        else { // ���������� ���� ���� ��������� ELSE
                            begin_state--;
                            if (begin_state < 0) {
                                cout << "Unexpected 'end' on line " << iter->GetLine() - 1 << "." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                            continue;
                        }
                    }
                    else if (checkLexem(iter, dot_tk)) {
                        iter++;
                        if (!checkLexem(iter, eof_tk)){
                            cout << "Met '" << iter->GetName() << "' instead of ',' on line " << iter->GetName() << "." << endl;
                            Errors++;
                            return -EXIT_FAILURE;
                        }
                        //���� ����� ����� ����� EOF, �� ��������� ������ �����
                        else {
                            break;
                        }
                    }
                }

                //��������� IF
                else if (checkLexem(iter, if_tk)){

                //������ � �������
                Tree* currentIf = Tree::CreateNode(iter->GetName());
                iter--;
                if (!checkLexem(iter, then_tk) && !checkLexem(iter, else_tk)){ //���� if �� ���������
                    iter++;
                    if (COMPOUND->GetLeftNode() == nullptr) {
                        COMPOUND->AddLeftTree(currentIf);
                    }
                    else {
                        Tree* sibling = COMPOUND->GetLeftNode();
                            while (sibling->GetRightNode() != nullptr) {
                                sibling = sibling->GetRightNode();
                            }
                        sibling->AddRightTree(currentIf);
                    }
                }
                else if (checkLexem(iter, then_tk)) { //���� ��������� ����� then �� ��������� �����
                    iter++;
                    if (checkLexem(iter, begin_tk)) {
                        iter++;
                        if (checkLexem(iter, if_tk)) {
                            iter++;
                            Tree* sibling = COMPOUND->GetLeftNode();
                            while (sibling != nullptr) {
                                if (sibling->GetName() == "begin" && sibling->GetLeftNode() == nullptr) {
                                    sibling->AddLeftTree(currentIf);  // ��������� ���� currentIf �����
                                    break;  // ��������� ����, ��� ��� ������ ���� ������
                                }

                                // ��� ������ �� ������: ������� ������, ����� �����
                                if (sibling->GetRightNode() != nullptr) {
                                    sibling = sibling->GetRightNode();
                                }
                                else if (sibling->GetLeftNode() != nullptr) {
                                    sibling = sibling->GetLeftNode();
                                }
                                else {
                                    break;  // ��������� ����, ���� ������ ��� ��������
                                }
                            }
                        }
                    }
                    
                        Tree* sibling = COMPOUND->GetLeftNode();
                        while (sibling != nullptr) {
                            if (sibling->GetName() == "then" && sibling->GetLeftNode() == nullptr) {
                                sibling->AddLeftTree(currentIf);  // ��������� ���� currentIf �����
                                break;  // ��������� ����, ��� ��� ������ ���� ������
                            }

                            // ��� ������ �� ������: ������� ������, ����� �����
                            if (sibling->GetRightNode() != nullptr) {
                                sibling = sibling->GetRightNode();
                            }
                            else if (sibling->GetLeftNode() != nullptr) {
                                sibling = sibling->GetLeftNode();
                            }
                            else {
                                break;  // ��������� ����, ���� ������ ��� ��������
                            }
                        }
                    
                }
                else if (checkLexem(iter, else_tk)) { //���� ��������� ����� else �� ��������� �����
                    iter++;
                    Tree* sibling = COMPOUND->GetLeftNode();
                    while (sibling != nullptr) {
                        if (sibling->GetName() == "else" && sibling->GetLeftNode() == nullptr) {
                            sibling->AddLeftTree(currentIf);  // ��������� ���� currentIf �����
                            break;  // ��������� ����, ��� ��� ������ ���� ������
                        }

                        // ��� ������ �� ������: ������� ������, ����� �����
                        if (sibling->GetRightNode() != nullptr) {
                            sibling = sibling->GetRightNode();
                        }
                        else if (sibling->GetLeftNode() != nullptr) {
                            sibling = sibling->GetLeftNode();
                        }
                        else {
                            break;  // ��������� ����, ���� ������ ��� ��������
                        }
                    }

                }


                    iter++;
                    if (checkLexem(iter, id_tk)) {
                        if (isVarExist(iter->GetName())) {
                            if (getVarType(iter->GetName()) != "integer" && getVarType(iter->GetName()) != "boolean") { // ���������� ������ ���� ���� bool ���� integer
                                cout << "Expected for variable name instead indentifire '" << iter->GetName() << "' on line " << iter->GetLine() << "." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                            else {
                                tek_type = getVarType(iter->GetName()); // �������� ��� �������������� ��������������

                                //���������� ���� � ������
                                Tree* varIf1 = Tree::CreateNode(iter->GetName());
                                currentIf->AddLeftTree(varIf1);

                                iter++;
                                if (checkLexem(iter, bool_less_tk)) { // <

                                    //���������� ���� � ������
                                    Tree* opIf = Tree::CreateNode(iter->GetName());
                                    varIf1->AddLeftTree(opIf);

                                    iter++;
                                    if (checkLexem(iter, id_tk)) { // �������� �������������� ����������, ���� ��� ��������������
                                        if (isVarExist(iter->GetName())) {
                                            if (getVarType(iter->GetName()) == tek_type) {

                                                //���������� ���� � ������
                                                Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                                opIf->AddLeftTree(varIf2);

                                                iter++;
                                                if (checkLexem(iter, then_tk)) { // �������� �� then

                                                    //���������� ���� � ������
                                                    Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                    currentIf->AddRightTree(thenIf);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {

                                                        //���������� ���� � ������
                                                        Tree* varThen = Tree::CreateNode(iter->GetName());
                                                        thenIf->AddLeftTree(varThen);

                                                        continue; // ���������� �������� ���� ��� ������
                                                    }
                                                    if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                        continue;
                                                    }
                                                }
                                                else {
                                                    cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                            else {
                                                if (tek_type == "integer") {
                                                    cout << "Cannot use integer to boolean on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                                else if (tek_type == "boolean") {
                                                    cout << "Cannot use boolean to integer on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                        }
                                        else { // ���� �������������� ���������� ���
                                            cout << "Unknown indentifier '" << iter->GetName() << "' on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else if (checkLexem(iter, constant_tk)) {
                                        if (tek_type == "integer") {
                                            //���������� ���� � ������
                                            Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                            opIf->AddLeftTree(varIf2);
                                            iter++;
                                            if (checkLexem(iter, then_tk)) { // �������� �� then
                                                //���������� ���� � ������
                                                Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                currentIf->AddRightTree(thenIf);

                                                iter++;
                                                if (checkLexem(iter, id_tk)) {

                                                    //���������� ���� � ������
                                                    Tree* varThen = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddLeftTree(varThen);

                                                    continue; // ���������� �������� ���� ��� ������
                                                }
                                                if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                    //���������� ���� � ������
                                                    Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddRightTree(beginIf);

                                                    begin_state++; // �������� � ���, ��� ����� begin
                                                    iter++; //���������� ������ ��������� �������
                                                    continue;
                                                }

                                            }
                                            else {
                                                cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                Errors++;
                                                return -EXIT_FAILURE;
                                            }
                                        }
                                        else {
                                            cout << "Another type on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    

                                }
                                else if (checkLexem(iter, bool_bigger_tk)) { // >
                                    //���������� ���� � ������
                                    Tree* opIf = Tree::CreateNode(iter->GetName());
                                    varIf1->AddLeftTree(opIf);

                                    iter++;
                                    if (checkLexem(iter, id_tk)) { // �������� �������������� ����������, ���� ��� ��������������
                                        if (isVarExist(iter->GetName())) {
                                            if (getVarType(iter->GetName()) == tek_type) {

                                                //���������� ���� � ������
                                                Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                                opIf->AddLeftTree(varIf2);

                                                iter++;
                                                if (checkLexem(iter, then_tk)) { // �������� �� then

                                                    //���������� ���� � ������
                                                    Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                    currentIf->AddRightTree(thenIf);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {

                                                        //���������� ���� � ������
                                                        Tree* varThen = Tree::CreateNode(iter->GetName());
                                                        thenIf->AddLeftTree(varThen);

                                                        continue; // ���������� �������� ���� ��� ������
                                                    }
                                                    if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                        //���������� ���� � ������
                                                        Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                        thenIf->AddRightTree(beginIf);

                                                        begin_state++; // �������� � ���, ��� ����� begin
                                                        iter++; //���������� ������ ��������� �������
                                                        continue;
                                                    }
                                                }
                                                else {
                                                    cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                            else {
                                                if (tek_type == "integer") {
                                                    cout << "Cannot use integer to boolean on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                                else if (tek_type == "boolean") {
                                                    cout << "Cannot use boolean to integer on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                        }
                                        else { // ���� �������������� ���������� ���
                                            cout << "Unknown indentifier '" << iter->GetName() << "' on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else if (checkLexem(iter, constant_tk)) {
                                        if (tek_type == "integer") {
                                            //���������� ���� � ������
                                            Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                            opIf->AddLeftTree(varIf2);
                                            iter++;
                                            if (checkLexem(iter, then_tk)) { // �������� �� then
                                                //���������� ���� � ������
                                                Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                currentIf->AddRightTree(thenIf);

                                                iter++;
                                                if (checkLexem(iter, id_tk)) {

                                                    //���������� ���� � ������
                                                    Tree* varThen = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddLeftTree(varThen);

                                                    continue; // ���������� �������� ���� ��� ������
                                                }
                                                if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                    //���������� ���� � ������
                                                    Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddRightTree(beginIf);

                                                    begin_state++; // �������� � ���, ��� ����� begin
                                                    iter++; //���������� ������ ��������� �������
                                                    continue;
                                                }

                                            }
                                            else {
                                                cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                Errors++;
                                                return -EXIT_FAILURE;
                                            }
                                        }
                                        else {
                                            cout << "Another type on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                }
                                else if (checkLexem(iter, bool_bigeqv_tk)) { //>=
                                    //���������� ���� � ������
                                    Tree* opIf = Tree::CreateNode(iter->GetName());
                                    varIf1->AddLeftTree(opIf);

                                    iter++;
                                    if (checkLexem(iter, id_tk)) { // �������� �������������� ����������, ���� ��� ��������������
                                        if (isVarExist(iter->GetName())) {
                                            if (getVarType(iter->GetName()) == tek_type) {

                                                //���������� ���� � ������
                                                Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                                opIf->AddLeftTree(varIf2);

                                                iter++;
                                                if (checkLexem(iter, then_tk)) { // �������� �� then

                                                    //���������� ���� � ������
                                                    Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                    currentIf->AddRightTree(thenIf);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {

                                                        //���������� ���� � ������
                                                        Tree* varThen = Tree::CreateNode(iter->GetName());
                                                        thenIf->AddLeftTree(varThen);

                                                        continue; // ���������� �������� ���� ��� ������
                                                    }
                                                    if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                        //���������� ���� � ������
                                                        Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                        thenIf->AddRightTree(beginIf);

                                                        begin_state++; // �������� � ���, ��� ����� begin
                                                        iter++; //���������� ������ ��������� �������
                                                        continue;
                                                    }
                                                }
                                                else {
                                                    cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                            else {
                                                if (tek_type == "integer") {
                                                    cout << "Cannot use integer to boolean on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                                else if (tek_type == "boolean") {
                                                    cout << "Cannot use boolean to integer on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                        }
                                        else { // ���� �������������� ���������� ���
                                            cout << "Unknown indentifier '" << iter->GetName() << "' on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else if (checkLexem(iter, constant_tk)) {
                                        if (tek_type == "integer") {
                                            //���������� ���� � ������
                                            Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                            opIf->AddLeftTree(varIf2);
                                            iter++;
                                            if (checkLexem(iter, then_tk)) { // �������� �� then
                                                //���������� ���� � ������
                                                Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                currentIf->AddRightTree(thenIf);

                                                iter++;
                                                if (checkLexem(iter, id_tk)) {

                                                    //���������� ���� � ������
                                                    Tree* varThen = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddLeftTree(varThen);

                                                    continue; // ���������� �������� ���� ��� ������
                                                }
                                                if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                    //���������� ���� � ������
                                                    Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddRightTree(beginIf);

                                                    begin_state++; // �������� � ���, ��� ����� begin
                                                    iter++; //���������� ������ ��������� �������
                                                    continue;
                                                }

                                            }
                                            else {
                                                cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                Errors++;
                                                return -EXIT_FAILURE;
                                            }
                                        }
                                        else {
                                            cout << "Another type on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                }
                                else if (checkLexem(iter, bool_leseqv_tk)) { // <=
                                    //���������� ���� � ������
                                    Tree* opIf = Tree::CreateNode(iter->GetName());
                                    varIf1->AddLeftTree(opIf);

                                    iter++;
                                    if (checkLexem(iter, id_tk)) { // �������� �������������� ����������, ���� ��� ��������������
                                        if (isVarExist(iter->GetName())) {
                                            if (getVarType(iter->GetName()) == tek_type) {

                                                //���������� ���� � ������
                                                Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                                opIf->AddLeftTree(varIf2);

                                                iter++;
                                                if (checkLexem(iter, then_tk)) { // �������� �� then

                                                    //���������� ���� � ������
                                                    Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                    currentIf->AddRightTree(thenIf);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {

                                                        //���������� ���� � ������
                                                        Tree* varThen = Tree::CreateNode(iter->GetName());
                                                        thenIf->AddLeftTree(varThen);

                                                        continue; // ���������� �������� ���� ��� ������
                                                    }
                                                    if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                        //���������� ���� � ������
                                                        Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                        thenIf->AddRightTree(beginIf);

                                                        begin_state++; // �������� � ���, ��� ����� begin
                                                        iter++; //���������� ������ ��������� �������
                                                        continue;
                                                    }
                                                }
                                                else {
                                                    cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                            else {
                                                if (tek_type == "integer") {
                                                    cout << "Cannot use integer to boolean on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                                else if (tek_type == "boolean") {
                                                    cout << "Cannot use boolean to integer on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                        }
                                        else { // ���� �������������� ���������� ���
                                            cout << "Unknown indentifier '" << iter->GetName() << "' on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else if (checkLexem(iter, constant_tk)) {
                                        if (tek_type == "integer") {
                                            //���������� ���� � ������
                                            Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                            opIf->AddLeftTree(varIf2);
                                            iter++;
                                            if (checkLexem(iter, then_tk)) { // �������� �� then
                                                //���������� ���� � ������
                                                Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                currentIf->AddRightTree(thenIf);

                                                iter++;
                                                if (checkLexem(iter, id_tk)) {

                                                    //���������� ���� � ������
                                                    Tree* varThen = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddLeftTree(varThen);

                                                    continue; // ���������� �������� ���� ��� ������
                                                }
                                                if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                    //���������� ���� � ������
                                                    Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddRightTree(beginIf);

                                                    begin_state++; // �������� � ���, ��� ����� begin
                                                    iter++; //���������� ������ ��������� �������
                                                    continue;
                                                }

                                            }
                                            else {
                                                cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                Errors++;
                                                return -EXIT_FAILURE;
                                            }
                                        }
                                        else {
                                            cout << "Another type on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                }
                                else if (checkLexem(iter, equals_tk)) { // =
                                    //���������� ���� � ������
                                    Tree* opIf = Tree::CreateNode(iter->GetName());
                                    varIf1->AddLeftTree(opIf);

                                    iter++;
                                    if (checkLexem(iter, id_tk)) { // �������� �������������� ����������, ���� ��� ��������������
                                        if (isVarExist(iter->GetName())) {
                                            if (getVarType(iter->GetName()) == tek_type) {

                                                //���������� ���� � ������
                                                Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                                opIf->AddLeftTree(varIf2);

                                                iter++;
                                                if (checkLexem(iter, then_tk)) { // �������� �� then

                                                    //���������� ���� � ������
                                                    Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                    currentIf->AddRightTree(thenIf);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {

                                                        //���������� ���� � ������
                                                        Tree* varThen = Tree::CreateNode(iter->GetName());
                                                        thenIf->AddLeftTree(varThen);

                                                        continue; // ���������� �������� ���� ��� ������
                                                    }
                                                    if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                        //���������� ���� � ������
                                                        Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                        thenIf->AddRightTree(beginIf);

                                                        begin_state++; // �������� � ���, ��� ����� begin
                                                        iter++; //���������� ������ ��������� �������
                                                        continue;
                                                    }
                                                }
                                                else {
                                                    cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                            else {
                                                if (tek_type == "integer") {
                                                    cout << "Cannot use integer to boolean on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                                else if (tek_type == "boolean") {
                                                    cout << "Cannot use boolean to integer on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                        }


                                        else { // ���� �������������� ���������� ���
                                            cout << "Unknown indentifier '" << iter->GetName() << "' on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else if (checkLexem(iter, constant_tk)) {
                                        if (tek_type == "integer") {
                                            //���������� ���� � ������
                                            Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                            opIf->AddLeftTree(varIf2);
                                            iter++;
                                            if (checkLexem(iter, then_tk)) { // �������� �� then
                                                //���������� ���� � ������
                                                Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                currentIf->AddRightTree(thenIf);

                                                iter++;
                                                if (checkLexem(iter, id_tk)) {

                                                    //���������� ���� � ������
                                                    Tree* varThen = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddLeftTree(varThen);

                                                    continue; // ���������� �������� ���� ��� ������
                                                }
                                                if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                    //���������� ���� � ������
                                                    Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddRightTree(beginIf);

                                                    begin_state++; // �������� � ���, ��� ����� begin
                                                    iter++; //���������� ������ ��������� �������
                                                    continue;
                                                }

                                            }
                                            else {
                                                cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                Errors++;
                                                return -EXIT_FAILURE;
                                            }
                                        }
                                        else {
                                            cout << "Another type on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                }
                                else if (checkLexem(iter, bool_noneqv_tk)) { // <>
                                    //���������� ���� � ������
                                    Tree* opIf = Tree::CreateNode(iter->GetName());
                                    varIf1->AddLeftTree(opIf);

                                    iter++;
                                    if (checkLexem(iter, id_tk)) { // �������� �������������� ����������, ���� ��� ��������������
                                        if (isVarExist(iter->GetName())) {
                                            if (getVarType(iter->GetName()) == tek_type) {

                                                //���������� ���� � ������
                                                Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                                opIf->AddLeftTree(varIf2);

                                                iter++;
                                                if (checkLexem(iter, then_tk)) { // �������� �� then

                                                    //���������� ���� � ������
                                                    Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                    currentIf->AddRightTree(thenIf);

                                                    iter++;
                                                    if (checkLexem(iter, id_tk)) {

                                                        //���������� ���� � ������
                                                        Tree* varThen = Tree::CreateNode(iter->GetName());
                                                        thenIf->AddLeftTree(varThen);

                                                        continue; // ���������� �������� ���� ��� ������
                                                    }
                                                    
                                                }
                                                else {
                                                    cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                            else {
                                                if (tek_type == "integer") {
                                                    cout << "Cannot use integer to boolean on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                                else if (tek_type == "boolean") {
                                                    cout << "Cannot use boolean to integer on line " << iter->GetLine() << "." << endl;
                                                    Errors++;
                                                    return -EXIT_FAILURE;
                                                }
                                            }
                                        }
                                        else { // ���� �������������� ���������� ���
                                            cout << "Unknown indentifier '" << iter->GetName() << "' on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else if (checkLexem(iter, constant_tk)) {
                                        if (tek_type == "integer") {
                                            //���������� ���� � ������
                                            Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                            opIf->AddLeftTree(varIf2);
                                            iter++;
                                            if (checkLexem(iter, then_tk)) { // �������� �� then
                                                //���������� ���� � ������
                                                Tree* thenIf = Tree::CreateNode(iter->GetName());
                                                currentIf->AddRightTree(thenIf);

                                                iter++;
                                                if (checkLexem(iter, id_tk)) {

                                                    //���������� ���� � ������
                                                    Tree* varThen = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddLeftTree(varThen);

                                                    continue; // ���������� �������� ���� ��� ������
                                                }
                                                if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                    //���������� ���� � ������
                                                    Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                    thenIf->AddRightTree(beginIf);

                                                    begin_state++; // �������� � ���, ��� ����� begin
                                                    iter++; //���������� ������ ��������� �������
                                                    continue;
                                                }

                                            }
                                            else {
                                                cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                                Errors++;
                                                return -EXIT_FAILURE;
                                            }
                                        }
                                        else {
                                            cout << "Another type on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }

                                }

                            }
                        }

                        else {
                            cout << "Unknown indentifier '" << iter->GetName() << "' on line " << iter->GetLine() << "." << endl;
                            Errors++;
                            return -EXIT_FAILURE;
                        }
                    }

                    else if (checkLexem(iter, constant_tk)) { // ���� ���������� ���������
                    //���������� ���� � ������
                    Tree* varIf1 = Tree::CreateNode(iter->GetName());
                    currentIf->AddLeftTree(varIf1);
                        iter++;
                        if (checkLexem(iter, bool_less_tk)) { // <
                            //���������� ���� � ������
                            Tree* opIf = Tree::CreateNode(iter->GetName());
                            varIf1->AddLeftTree(opIf);
                            iter++;
                            if (checkLexem(iter, id_tk)) { // �������� �������������� ����������, ���� ��� ��������������
                                if (isVarExist(iter->GetName())) {
                                    if (getVarType(iter->GetName()) == "integer") {
                                        //���������� ���� � ������
                                        Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                        opIf->AddLeftTree(varIf2);
                                        iter++;
                                        if (checkLexem(iter, then_tk)) { // �������� �� then
                                            //���������� ���� � ������
                                            Tree* thenIf = Tree::CreateNode(iter->GetName());
                                            currentIf->AddRightTree(thenIf);

                                            iter++;
                                            if (checkLexem(iter, id_tk)) {

                                                //���������� ���� � ������
                                                Tree* varThen = Tree::CreateNode(iter->GetName());
                                                thenIf->AddLeftTree(varThen);

                                                continue; // ���������� �������� ���� ��� ������
                                            }
                                            if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                //���������� ���� � ������
                                                Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                thenIf->AddRightTree(beginIf);

                                                begin_state++; // �������� � ���, ��� ����� begin
                                                iter++; //���������� ������ ��������� �������
                                                continue;
                                            }
                                            
                                        }
                                        else {
                                            cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else {
                                            cout << "Cannot use integer to boolean on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                    }
                                }
                                else { // ���� �������������� ���������� ���
                                    cout << "Unknown indentifier. '" << iter->GetName() << "'" << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                            }
                            else if (checkLexem(iter, constant_tk)) {
                                //���������� ���� � ������
                                Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                opIf->AddLeftTree(varIf2);
                                iter++;
                                if (checkLexem(iter, then_tk)) { // �������� �� then
                                    //���������� ���� � ������
                                    Tree* thenIf = Tree::CreateNode(iter->GetName());
                                    currentIf->AddRightTree(thenIf);

                                    iter++;
                                    if (checkLexem(iter, id_tk)) {

                                        //���������� ���� � ������
                                        Tree* varThen = Tree::CreateNode(iter->GetName());
                                        thenIf->AddLeftTree(varThen);

                                        continue; // ���������� �������� ���� ��� ������
                                    }
                                    if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                        //���������� ���� � ������
                                        Tree* beginIf = Tree::CreateNode(iter->GetName());
                                        thenIf->AddRightTree(beginIf);

                                        begin_state++; // �������� � ���, ��� ����� begin
                                        iter++; //���������� ������ ��������� �������
                                        continue;
                                    }

                                }
                                else {
                                    cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                            }
                        }
                        else if (checkLexem(iter, bool_bigger_tk)) { // <
                            //���������� ���� � ������
                            Tree* opIf = Tree::CreateNode(iter->GetName());
                            varIf1->AddLeftTree(opIf);
                            iter++;
                            if (checkLexem(iter, id_tk)) { // �������� �������������� ����������, ���� ��� ��������������
                                if (isVarExist(iter->GetName())) {
                                    if (getVarType(iter->GetName()) == "integer") {
                                        //���������� ���� � ������
                                        Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                        opIf->AddLeftTree(varIf2);
                                        iter++;
                                        if (checkLexem(iter, then_tk)) { // �������� �� then
                                            //���������� ���� � ������
                                            Tree* thenIf = Tree::CreateNode(iter->GetName());
                                            currentIf->AddRightTree(thenIf);

                                            iter++;
                                            if (checkLexem(iter, id_tk)) {

                                                //���������� ���� � ������
                                                Tree* varThen = Tree::CreateNode(iter->GetName());
                                                thenIf->AddLeftTree(varThen);

                                                continue; // ���������� �������� ���� ��� ������
                                            }
                                            if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                                //���������� ���� � ������
                                                Tree* beginIf = Tree::CreateNode(iter->GetName());
                                                thenIf->AddRightTree(beginIf);

                                                begin_state++; // �������� � ���, ��� ����� begin
                                                iter++; //���������� ������ ��������� �������
                                                continue;
                                            }

                                        }
                                        else {
                                            cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                            Errors++;
                                            return -EXIT_FAILURE;
                                        }
                                    }
                                    else {
                                        cout << "Cannot use integer to boolean on line " << iter->GetLine() << "." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                }
                                else { // ���� �������������� ���������� ���
                                    cout << "Unknown indentifier. '" << iter->GetName() << "'" << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                            }
                            else if (checkLexem(iter, constant_tk)) {
                                //���������� ���� � ������
                                Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                opIf->AddLeftTree(varIf2);
                                iter++;
                                if (checkLexem(iter, then_tk)) { // �������� �� then
                                    //���������� ���� � ������
                                    Tree* thenIf = Tree::CreateNode(iter->GetName());
                                    currentIf->AddRightTree(thenIf);

                                    iter++;
                                    if (checkLexem(iter, id_tk)) {

                                        //���������� ���� � ������
                                        Tree* varThen = Tree::CreateNode(iter->GetName());
                                        thenIf->AddLeftTree(varThen);

                                        continue; // ���������� �������� ���� ��� ������
                                    }
                                    if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                        //���������� ���� � ������
                                        Tree* beginIf = Tree::CreateNode(iter->GetName());
                                        thenIf->AddRightTree(beginIf);

                                        begin_state++; // �������� � ���, ��� ����� begin
                                        iter++; //���������� ������ ��������� �������
                                        continue;
                                    }

                                }
                                else {
                                    cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                            }
                        }
                        else if (checkLexem(iter, bool_bigeqv_tk)) { // <
                        //���������� ���� � ������
                        Tree* opIf = Tree::CreateNode(iter->GetName());
                        varIf1->AddLeftTree(opIf);
                        iter++;
                        if (checkLexem(iter, id_tk)) { // �������� �������������� ����������, ���� ��� ��������������
                            if (isVarExist(iter->GetName())) {
                                if (getVarType(iter->GetName()) == "integer") {
                                    //���������� ���� � ������
                                    Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                    opIf->AddLeftTree(varIf2);
                                    iter++;
                                    if (checkLexem(iter, then_tk)) { // �������� �� then
                                        //���������� ���� � ������
                                        Tree* thenIf = Tree::CreateNode(iter->GetName());
                                        currentIf->AddRightTree(thenIf);

                                        iter++;
                                        if (checkLexem(iter, id_tk)) {

                                            //���������� ���� � ������
                                            Tree* varThen = Tree::CreateNode(iter->GetName());
                                            thenIf->AddLeftTree(varThen);

                                            continue; // ���������� �������� ���� ��� ������
                                        }
                                        if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                            //���������� ���� � ������
                                            Tree* beginIf = Tree::CreateNode(iter->GetName());
                                            thenIf->AddRightTree(beginIf);

                                            begin_state++; // �������� � ���, ��� ����� begin
                                            iter++; //���������� ������ ��������� �������
                                            continue;
                                        }

                                    }
                                    else {
                                        cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                }
                                else {
                                    cout << "Cannot use integer to boolean on line " << iter->GetLine() << "." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                            }
                            else { // ���� �������������� ���������� ���
                                cout << "Unknown indentifier. '" << iter->GetName() << "'" << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        else if (checkLexem(iter, constant_tk)) {
                            //���������� ���� � ������
                            Tree* varIf2 = Tree::CreateNode(iter->GetName());
                            opIf->AddLeftTree(varIf2);
                            iter++;
                            if (checkLexem(iter, then_tk)) { // �������� �� then
                                //���������� ���� � ������
                                Tree* thenIf = Tree::CreateNode(iter->GetName());
                                currentIf->AddRightTree(thenIf);

                                iter++;
                                if (checkLexem(iter, id_tk)) {

                                    //���������� ���� � ������
                                    Tree* varThen = Tree::CreateNode(iter->GetName());
                                    thenIf->AddLeftTree(varThen);

                                    continue; // ���������� �������� ���� ��� ������
                                }
                                if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                    //���������� ���� � ������
                                    Tree* beginIf = Tree::CreateNode(iter->GetName());
                                    thenIf->AddRightTree(beginIf);

                                    begin_state++; // �������� � ���, ��� ����� begin
                                    iter++; //���������� ������ ��������� �������
                                    continue;
                                }

                            }
                            else {
                                cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        }
                        else if (checkLexem(iter, bool_noneqv_tk)) { // <
                        //���������� ���� � ������
                        Tree* opIf = Tree::CreateNode(iter->GetName());
                        varIf1->AddLeftTree(opIf);
                        iter++;
                        if (checkLexem(iter, id_tk)) { // �������� �������������� ����������, ���� ��� ��������������
                            if (isVarExist(iter->GetName())) {
                                if (getVarType(iter->GetName()) == "integer") {
                                    //���������� ���� � ������
                                    Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                    opIf->AddLeftTree(varIf2);
                                    iter++;
                                    if (checkLexem(iter, then_tk)) { // �������� �� then
                                        //���������� ���� � ������
                                        Tree* thenIf = Tree::CreateNode(iter->GetName());
                                        currentIf->AddRightTree(thenIf);

                                        iter++;
                                        if (checkLexem(iter, id_tk)) {

                                            //���������� ���� � ������
                                            Tree* varThen = Tree::CreateNode(iter->GetName());
                                            thenIf->AddLeftTree(varThen);

                                            continue; // ���������� �������� ���� ��� ������
                                        }
                                        if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                            //���������� ���� � ������
                                            Tree* beginIf = Tree::CreateNode(iter->GetName());
                                            thenIf->AddRightTree(beginIf);

                                            begin_state++; // �������� � ���, ��� ����� begin
                                            iter++; //���������� ������ ��������� �������
                                            continue;
                                        }

                                    }
                                    else {
                                        cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                }
                                else {
                                    cout << "Cannot use integer to boolean on line " << iter->GetLine() << "." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                            }
                            else { // ���� �������������� ���������� ���
                                cout << "Unknown indentifier. '" << iter->GetName() << "'" << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        else if (checkLexem(iter, constant_tk)) {
                            //���������� ���� � ������
                            Tree* varIf2 = Tree::CreateNode(iter->GetName());
                            opIf->AddLeftTree(varIf2);
                            iter++;
                            if (checkLexem(iter, then_tk)) { // �������� �� then
                                //���������� ���� � ������
                                Tree* thenIf = Tree::CreateNode(iter->GetName());
                                currentIf->AddRightTree(thenIf);

                                iter++;
                                if (checkLexem(iter, id_tk)) {

                                    //���������� ���� � ������
                                    Tree* varThen = Tree::CreateNode(iter->GetName());
                                    thenIf->AddLeftTree(varThen);

                                    continue; // ���������� �������� ���� ��� ������
                                }
                                if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                    //���������� ���� � ������
                                    Tree* beginIf = Tree::CreateNode(iter->GetName());
                                    thenIf->AddRightTree(beginIf);

                                    begin_state++; // �������� � ���, ��� ����� begin
                                    iter++; //���������� ������ ��������� �������
                                    continue;
                                }

                            }
                            else {
                                cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        }
                        else if (checkLexem(iter, bool_bigger_tk)) { // <
                        //���������� ���� � ������
                        Tree* opIf = Tree::CreateNode(iter->GetName());
                        varIf1->AddLeftTree(opIf);
                        iter++;
                        if (checkLexem(iter, id_tk)) { // �������� �������������� ����������, ���� ��� ��������������
                            if (isVarExist(iter->GetName())) {
                                if (getVarType(iter->GetName()) == "integer") {
                                    //���������� ���� � ������
                                    Tree* varIf2 = Tree::CreateNode(iter->GetName());
                                    opIf->AddLeftTree(varIf2);
                                    iter++;
                                    if (checkLexem(iter, then_tk)) { // �������� �� then
                                        //���������� ���� � ������
                                        Tree* thenIf = Tree::CreateNode(iter->GetName());
                                        currentIf->AddRightTree(thenIf);

                                        iter++;
                                        if (checkLexem(iter, id_tk)) {

                                            //���������� ���� � ������
                                            Tree* varThen = Tree::CreateNode(iter->GetName());
                                            thenIf->AddLeftTree(varThen);

                                            continue; // ���������� �������� ���� ��� ������
                                        }
                                        if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                            //���������� ���� � ������
                                            Tree* beginIf = Tree::CreateNode(iter->GetName());
                                            thenIf->AddRightTree(beginIf);

                                            begin_state++; // �������� � ���, ��� ����� begin
                                            iter++; //���������� ������ ��������� �������
                                            continue;
                                        }

                                    }
                                    else {
                                        cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                        Errors++;
                                        return -EXIT_FAILURE;
                                    }
                                }
                                else {
                                    cout << "Cannot use integer to boolean on line " << iter->GetLine() << "." << endl;
                                    Errors++;
                                    return -EXIT_FAILURE;
                                }
                            }
                            else { // ���� �������������� ���������� ���
                                cout << "Unknown indentifier. '" << iter->GetName() << "'" << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        else if (checkLexem(iter, constant_tk)) {
                            //���������� ���� � ������
                            Tree* varIf2 = Tree::CreateNode(iter->GetName());
                            opIf->AddLeftTree(varIf2);
                            iter++;
                            if (checkLexem(iter, then_tk)) { // �������� �� then
                                //���������� ���� � ������
                                Tree* thenIf = Tree::CreateNode(iter->GetName());
                                currentIf->AddRightTree(thenIf);

                                iter++;
                                if (checkLexem(iter, id_tk)) {

                                    //���������� ���� � ������
                                    Tree* varThen = Tree::CreateNode(iter->GetName());
                                    thenIf->AddLeftTree(varThen);

                                    continue; // ���������� �������� ���� ��� ������
                                }
                                if (checkLexem(iter, begin_tk)) { // ���� ����� begin tk

                                    //���������� ���� � ������
                                    Tree* beginIf = Tree::CreateNode(iter->GetName());
                                    thenIf->AddRightTree(beginIf);

                                    begin_state++; // �������� � ���, ��� ����� begin
                                    iter++; //���������� ������ ��������� �������
                                    continue;
                                }

                            }
                            else {
                                cout << "Expected 'then' on line " << iter->GetLine() << "." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }
                        }
                        }
                    }
                    else if (checkLexem(iter, if_tk)) {
                        continue;
                    }
                    else {
                        cout << "Unknown indentifier. '" << iter->GetName() << "'" << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                }

                //�������� �� GOTO
                else if (checkLexem(iter, goto_tk)) {

                    //������ � �������
                    Tree* GoTo = Tree::CreateNode(iter->GetName());

                    iter--;
                    if (checkLexem(iter, then_tk)) {
                        iter++;
                        if (COMPOUND->GetLeftNode() == nullptr) {
                            COMPOUND->AddLeftTree(GoTo);
                        }
                        else {
                            Tree* sibling = COMPOUND->GetLeftNode();
                            Tree* lastLeaf = nullptr;  // ���������� ��������� ���� ��� ��������

                            while (sibling != nullptr) {
                                if (sibling->GetLeftNode() == nullptr && sibling->GetRightNode() == nullptr) {
                                    lastLeaf = sibling;  // ��������� ��������� ���� ��� ��������
                                }

                                // ��� ������ �� ������� �������, ���� �� ����, ����� �� ������
                                if (sibling->GetRightNode() != nullptr) {
                                    sibling = sibling->GetRightNode();
                                }
                                else if (sibling->GetLeftNode() != nullptr) {
                                    sibling = sibling->GetLeftNode();
                                }
                                else {
                                    break;  // ��������� ����, ���� ������ ��� ��������
                                }
                            }

                            // ��������� ����� ���� � ���������� ���������� ���� ��� ��������
                            if (lastLeaf->GetLeftNode() == nullptr) {
                                lastLeaf->AddLeftTree(GoTo);
                                GoTo = lastLeaf;
                            }
                            else {
                                lastLeaf->AddRightTree(GoTo);
                            }
                        }
                    }

                    else if (checkLexem(iter, else_tk)) {
                        iter++;
                        if (COMPOUND->GetLeftNode() == nullptr) {
                            COMPOUND->AddLeftTree(GoTo);
                        }
                        else {
                            Tree* sibling = COMPOUND->GetLeftNode();
                            Tree* lastLeaf = nullptr;  // ���������� ��������� ���� ��� ��������

                            while (sibling != nullptr) {
                                if (sibling->GetLeftNode() == nullptr && sibling->GetRightNode() == nullptr) {
                                    lastLeaf = sibling;  // ��������� ��������� ���� ��� ��������
                                }

                                // ��� ������ �� ������� �������, ���� �� ����, ����� �� ������
                                if (sibling->GetRightNode() != nullptr) {
                                    sibling = sibling->GetRightNode();
                                }
                                else if (sibling->GetLeftNode() != nullptr) {
                                    sibling = sibling->GetLeftNode();
                                }
                                else {
                                    break;  // ��������� ����, ���� ������ ��� ��������
                                }
                            }

                            // ��������� ����� ���� � ���������� ���������� ���� ��� ��������
                            if (lastLeaf->GetLeftNode() == nullptr) {
                                lastLeaf->AddLeftTree(GoTo);

                            }
                            else {
                                lastLeaf->AddRightTree(GoTo);
                            }
                        }
                    }

                    else {
                        iter++;
                        //���������� � ������ ����� ����������
                        if (COMPOUND->GetLeftNode() == nullptr) {
                            COMPOUND->AddLeftTree(GoTo);
                        }
                        else {
                            Tree* sibling = COMPOUND->GetLeftNode();
                            while (sibling->GetRightNode() != nullptr) {
                                sibling = sibling->GetRightNode();
                            }
                            sibling->AddRightTree(GoTo);
                        }
                    }

                    //���� ��������� �������
                    iter++;

                    if (!checkLexem(iter, id_tk)) {
                        printError(MUST_BE_ID, *iter);
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    if (isVarExist(iter->GetName())) {
                        if (getVarType(iter->GetName()) == "label") {
                            //���� � ������ ����� ���� �����
                            for (const string& operand : label_setup) {
                                if (operand == iter->GetName()) {
                                    Tree* GoLabel = Tree::CreateNode(iter->GetName());
                                    GoTo->AddLeftTree(GoLabel);
                                    label_set = 1;
                                }
                            }
                            if (label_set != 1) {
                                cout << "Goto error. Label '" << iter->GetName() << "' doesn't setup on line " << iter->GetLine() << "." << endl;
                                Errors++;
                                return -EXIT_FAILURE;
                            }


                        }
                        else {
                            cout << "Indentifire '" << iter->GetName() << "' haven't been found on line " << iter->GetLine() << endl;
                            Errors++;
                            return -EXIT_FAILURE;
                        }
                    }
                    else {
                        cout << "Unknown indentifire '" << iter->GetName() << "' on line " << iter->GetLine() << "." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }

                    if (!checkLexem(iter, end_tk)) {
                        iter++;
                        if (checkLexem(iter, semicolon_tk)) {
                            iter++;
                            continue;
                        }
                        else if (checkLexem(iter, id_tk) || checkLexem(iter, label_tk) || checkLexem(iter, goto_tk)) {
                            cout << "Met '" << iter->GetName() << "' but wait for ',' on line " << iter->GetLine() << endl;
                            Errors++;
                            return -EXIT_FAILURE;
                        }
                    }

                }

                // � ������, ���� ��� �� ����, �� �������
                else if (!checkLexem(iter, end_tk)) {
                    cout << "Expected operator." << endl;
                    Errors++;
                    return -EXIT_FAILURE;
                }

            }


            if (!checkLexem(iter, eof_tk)) {
                cout << "Unexpected symbols after '.'." << endl;
                Errors++;
                return -EXIT_FAILURE;
            }
 
        }

        // ������� �������� ��� PROGRAM
        else {
            while (!checkLexem(t_iter, begin_tk)) {
                // ������� INTEGER � BOOLEAN
                if (checkLexem(t_iter, id_tk)) {
                    //string type_var = getVarType(t)
                    t_iter++;
                    if (!checkLexem(t_iter, colon_tk)) {
                        cout << "Expected ':'." << endl;
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    t_iter++;
                    if (!checkLexem(t_iter, type_tk)) {
                        printError(MUST_BE_TYPE, *t_iter);
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                    t_iter++;
                    if (!checkLexem(t_iter, semicolon_tk)) {
                        printError(MUST_BE_SEMI, *t_iter);
                        Errors++;
                        return -EXIT_FAILURE;
                    }
                }
                t_iter++;
            }
            // �������������� ���, ��� ��������� ��� BEGIN
            while (!checkLexem(t_iter, end_tk)) {
                t_iter++;
            }
            t_iter++;
            if (!checkLexem(t_iter, dot_tk)) { // �������� �� ����� ����� end
                printError(MUST_BE_OP, *t_iter);
                Errors++;
                return -EXIT_FAILURE;
            }

        }
        return EXIT_SUCCESS;
    }

    Tree* Parser() {
        root = Tree::CreateNode("Program");
        lex_it it = cursor;
        while (it != lex_table.end()) {
            programParse(it, root);
            break;
        }
        if (Errors == 0) {
            cout << endl;
            printIdMap();
            cout << endl;
            cout << "Syntax tree: " << endl;
            root->PrintASCII();
            cout << endl;
            cout << "PROGRAMM WAS SUCCESSFULLY PARSEeD!" << endl;
            return root; // ���������� ��������� ��� ��� ������� Tree
        }
        else if (Errors >= 1) {
            cout << "PROGRAMM WAS PARSED WITH ERRORS!" << endl;
            return nullptr;
        }
    }

    void printError(errors t_err, Lexem lex) {
        switch (t_err) {
        case UNKNOWN_LEXEM:
            cout << "SYNTAX ERROR: Get unknown lexem: " << lex.GetName()
                << " on " << lex.GetLine() << " line" << endl;
            break;
        case MUST_BE_ID:
            cout << "SYNTAX ERROR: Must be identifier instead '" << lex.GetName()
                << "' on " << lex.GetLine() << " line" << endl;
            break;
        case EOF_ERR:
            cout << "SYNTAX ERROR: End of file!" << endl;
            break;
        case MUST_BE_SEMI:
            cout << "SYNTAX ERROR: Must be ';' instead " << lex.GetName()
                << " on " << lex.GetLine() << " line" << endl;
            break;
        case MUST_BE_PROG:
            cout << "SYNTAX ERROR: Must be 'program' on " << lex.GetLine() << " line instead of '" << lex.GetName() << "'" << endl;
            break;
        case DUPL_ID_ERR:
            cout << "<E> Syntax: Duplicate identifier '" << lex.GetName()
                << "' on " << lex.GetLine() << " line" << endl;
            break;
        case UNKNOWN_ID:
            cout << "<E> Syntax: Undefined variable '" << lex.GetName()
                << "' on " << lex.GetLine() << " line" << endl;
            break;
        case MUST_BE_BEGIN:
            cout << "SYNTAX ERROR: Waited for 'begin' on " << lex.GetLine() << " line" << endl;
            break;
        case MUST_BE_OP:
            cout << "SYNTAX ERROR: Must be operator instead " << lex.GetName()
                << " on " << lex.GetLine() << " line" << endl;
            break;
        default:
            cout << "<E> Syntax: Undefined type of error" << endl;
            break;
        }
    }

    ~Syntaxx() {

    }
};

#endif // SINTAX_ANALIZ_H