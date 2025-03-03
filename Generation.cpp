#ifndef GEN_CODE_CPP
#define GEN_CODE_CPP
#include "Generation.h"
#include <iostream>

GenCode::GenCode(Tree&& t_synt_tree)
{
    try {
        synt_tree = &t_synt_tree;

        // Получаем имя программы из дерева
        Tree* titleNode = synt_tree->GetLeftNode();
        std::string fileName = "output";

        if (titleNode) {
            fileName = titleNode->GetValue();
        }
        else {
            std::cerr << "<W> GenCode: No title node found under Program. Using default file name.\n";
        }

        // Создаём файл с ассемблерным кодом
        code.open(fileName + ".txt", std::ios::out | std::ios::trunc);
        if (!code.is_open()) {
            throw std::runtime_error("Cannot open output file for code. \n");
        }

        std::cout << "<I> Assembly file will be saved as: " << fileName << ".txt\n";
    }
    catch (const std::exception& exp)
    {
        std::string what = "<E> GenCode: Exception in constructor: ";
        throw std::runtime_error(what + std::string(exp.what()));
    }
}

GenCode::~GenCode()
{
    code.close();
    clearBuffer();
    
}



int GenCode::GenerateAsm()
{
    std::cout << "Starting ASM generation.\n" << std::endl;

    // 1. Сначала выводим заголовочные директивы `.386` и `model small`
    addLine(".386");
    addLine(".model small");
    addLine("");

    // 2. Генерация секции `.data`
    addLine(".data");
    addLine("output_message db 'Result: $'");
    addLine("num_buffer db 10 dup(0)");
    addLine("result dd 0");
    addLine("");
    std::cout << "Generating data section for variables...\n" << std::endl;

    // Обходим дерево, чтобы сгенерировать переменные/метки
    GenerateFromTree(synt_tree);

    // Выгружаем оставшиеся данные из буфера, если таковые есть
    if (!test_str.str().empty())
    {
        addLine(test_str.str());
        clearBuffer();
    }
    addLine("");

    // 3. Генерация секции `.text` и основной части программы
    std::cout << "Generating TextPart (Begin block)...\n" << std::endl;
    generateTextPart();

    // 4. Закрываем программу
    std::cout << "Generating end section.\n" << std::endl;

    return EXIT_SUCCESS;
}


void GenCode::GenerateFromTree(Tree* node)
{
    if (node == nullptr) return;

    std::string nodeValue = node->GetValue();
    std::cout << "Entering GenerateFromTree with node: " << nodeValue << std::endl;

    if (node->name == "VarDecl")
    {
        std::cout << "Processing VarDecl node." << std::endl;
        generateDeclVars(node);
    }
    else if (nodeValue == "LabelDecl")
    {
        std::cout << "Processing LabelDecl node." << std::endl;
        storeLabel(node);
    }
    else if (nodeValue == "Begin")
    {
        std::cout << "Skipping Begin node, handled separately.\n";
        return;
    }
    else if (isdigit(nodeValue[0]) || isalpha(nodeValue[0]))
    {
        // Узлы с числами или переменными
        std::cout << "Processing variable/value node: " << nodeValue << std::endl;
    }
    else
    {
        std::cout << "<W> Unrecognized node: " << nodeValue << " skipping...\n";
    }

    // Рекурсивный обход дерева
    GenerateFromTree(node->GetLeftNode());
    GenerateFromTree(node->GetRightNode());
}






int GenCode::generateDeclVars(Tree* varDeclRoot)
{
    if (!varDeclRoot) {
        std::cerr << "<E> GenCode: Root of VarDecl is null" << std::endl;
        return -EXIT_FAILURE;
    }

    Tree* currentVarDecl = varDeclRoot;

    while (currentVarDecl != nullptr)
    {
        // Проверяем, что текущий узел — VarDecl
        if (currentVarDecl->GetValue() != "VarDecl") {
            std::cout << "<W> Skipping non-VarDecl node: " << currentVarDecl->GetValue() << std::endl;
            break;
        }

        // Генерация данных переменной
        Tree* varNameNode = currentVarDecl->GetLeftNode();
        if (!varNameNode) {
            std::cerr << "<E> GenCode: VarDecl has no variable name \n" << std::endl;
            return -EXIT_FAILURE;
        }

        // Передаваем в `generateDataVar`, которая уже включает проверку на дубликаты
        int result = generateDataVar(varNameNode);
        if (result != EXIT_SUCCESS) {
            // Лог пропуска переменной
            std::cerr << "<W> Duplicate definition detected during decl generation: "
                << varNameNode->GetValue() << std::endl;
        }

        // Переход к следующему узлу
        currentVarDecl = currentVarDecl->GetRightNode();
    }

    return EXIT_SUCCESS;
}






// Обработка инициализированных переменных
int GenCode::generateInitVars(Tree* var_root)
{
    if (var_root == nullptr) return EXIT_SUCCESS;

    std::cout << "Processing initialized variable node: " << var_root->GetValue() << std::endl;

    if (var_root->GetLeftNode() != nullptr) {
        int result = generateDataVar(var_root->GetLeftNode());
        if (result != EXIT_SUCCESS) return result;
    }

    if (var_root->GetRightNode() != nullptr) {
        return generateInitVars(var_root->GetRightNode());
    }

    if (!test_str.str().empty()) {
        addLine(".data");
        addLine(test_str.str());
        clearBuffer();
    }

    return EXIT_SUCCESS;
}


void GenCode::generateExpressions(Tree* expr_tree)
{
    if (expr_tree == nullptr) {
        std::cerr << "<E> Expression tree is null!" << std::endl;
        return;
    }

    std::string op = expr_tree->GetValue();
    std::cout << "Processing expression node: " << op << std::endl;

    // === Присваивание (:=) ===
    if (op == ":=")
    {
        Tree* leftVar = expr_tree->GetParentNode(); // Узел переменной (a)
        Tree* rightExpr = expr_tree->GetLeftNode(); // Узел выражения (оператор +, -, и т.д.)

        if (!leftVar || !rightExpr) {
            std::cerr << "<E> Invalid ':=' operator: missing variable or expression!" << std::endl;
            return;
        }

        // Генерируем правую часть выражения
        generateExpressions(rightExpr);

        // Сохранение результата в переменную
        std::string variable = leftVar->GetValue();
        // Сохранение результата в переменную:
        addLine("pop eax"); // Забираем результат из стека
        addLine("mov " + variable + ", eax"); // Сохраняем в переменную (например, a)

        // Дополнительно сохраняем значение в result, если целевая переменная - не result
        if (variable != "result")
        {
            addLine("mov result, eax");
        }
    }
    // === Арифметические операции ===
    else if (op == "+" || op == "-" || op == "*" || op == "/")
    {
        Tree* leftOperand = expr_tree->GetLeftNode(); // Левый операнд
        Tree* rightOperand = expr_tree->GetRightNode(); // Правый операнд

        if (!leftOperand || !rightOperand) {
            std::cerr << "<E> '" << op << "' operator is missing operands!" << std::endl;
            return;
        }

        // Генерация операндов
        generateExpressions(leftOperand);
        generateExpressions(rightOperand);

        // Выполнение операции
        addLine("pop ebx"); // Забираем из стека правый операнд
        addLine("pop eax"); // Забираем из стека левый операнд

        if (op == "+") {
            addLine("add eax, ebx");
        }
        else if (op == "-") {
            addLine("sub eax, ebx");
        }
        else if (op == "*") {
            addLine("imul eax, ebx");
        }
        else if (op == "/") {
            addLine("cdq"); // Расширение знака для деления
            addLine("idiv ebx");
        }

        addLine("push eax"); // Возвращаем результат на стек
    }
    // === Числовые литералы ===
    else if (isdigit(op[0]) || (op[0] == '-' && isdigit(op[1]))) // Отрицательные числа
    {
        addLine("push " + op); // Число помещаем в стек
    }
    // === Логические операторы (and, or, not) ===
    else if (op == "and" || op == "or")
    {
        Tree* leftOperand = expr_tree->GetLeftNode();
        Tree* rightOperand = expr_tree->GetRightNode();

        if (!leftOperand || !rightOperand) {
            std::cerr << "<E> Logical operator '" << op << "' is missing operands!" << std::endl;
            return;
        }

        generateExpressions(leftOperand);
        generateExpressions(rightOperand);

        addLine("pop ebx");
        addLine("pop eax");

        if (op == "and") {
            addLine("and eax, ebx");
        }
        else if (op == "or") {
            addLine("or eax, ebx");
        }

        addLine("push eax"); // Сохраняем результат логической операции
    }
    else if (op == "not")
    {
        Tree* operand = expr_tree->GetLeftNode();

        if (!operand) {
            std::cerr << "<E> Logical NOT is missing an operand!" << std::endl;
            return;
        }

        generateExpressions(operand);

        addLine("pop eax");
        addLine("not eax");
        addLine("push eax"); // Результат сохраняем обратно в стек
    }
    // === Условные операторы (IF) ===
    else if (op == "if")
    {
        Tree* condition = expr_tree->GetLeftNode(); // Узел условия
        Tree* thenElseNodes = expr_tree->GetRightNode(); // THEN и/или ELSE

        if (!condition || !thenElseNodes) {
            std::cerr << "<E> IF statement is missing condition or branches!" << std::endl;
            return;
        }

        Tree* thenBranch = thenElseNodes->GetLeftNode(); // Ветка THEN
        Tree* elseBranch = thenElseNodes->GetRightNode(); // Ветка ELSE

        std::string labelElse = "label_else_" + std::to_string(M++);
        std::string labelEnd = "label_end_" + std::to_string(M++);

        generateExpressions(condition); // Генерируем условие

        // Логика условия
        addLine("pop eax"); // Результат условия в eax
        addLine("cmp eax, 0");
        addLine("je " + labelElse); // Если false — перейти в ELSE

        // Генерация THEN
        if (thenBranch) {
            generateExpressions(thenBranch);
        }

        addLine("jmp " + labelEnd); // Переход после THEN

        addLine(labelElse + ":"); // Метка ELSE
        if (elseBranch) {
            generateExpressions(elseBranch);
        }

        addLine(labelEnd + ":"); // Метка конца IF
    }
    // === Оператор GOTO ===
    else if (op == "goto")
    {
        Tree* labelNode = expr_tree->GetLeftNode();

        if (!labelNode) {
            std::cerr << "<E> GOTO operation is missing a label!" << std::endl;
            return;
        }

        std::string label = labelNode->GetValue();

        if (std::find(label_vector.begin(), label_vector.end(), label) != label_vector.end()) {
            addLine("jmp " + label); // Переход к метке
        }
        else {
            std::cerr << "<E> Unknown label: " << label << std::endl;
        }
    }
    // === Некорректные узлы ===
    else
    {
        std::cerr << "<W> Unrecognized node in expression: " << op << std::endl;
    }
}

int GenCode::generateDataVar(Tree* varNameNode)
{
    if (varNameNode == nullptr) {
        std::cerr << "<E> GenCode: Variable node is null" << std::endl;
        return -EXIT_FAILURE;
    }

    // Получаем имя и тип переменной
    std::string varName = varNameNode->GetValue();
    Tree* typeNode = varNameNode->GetLeftNode();
    if (typeNode == nullptr) {
        std::cerr << "<E> GenCode: Missing type for variable " << varName << std::endl;
        return -EXIT_FAILURE;
    }

    // Проверяем, уже ли сгенерирована такая переменная (используем `id_map`)
    if (id_map.find(varName) != id_map.end()) {
        std::cerr << "<W> GenCode: Variable " << varName << " is already generated, skipping." << std::endl;
        return EXIT_FAILURE; // Выходим без генерации
    }

    // Определяем тип переменной для ASM
    std::string varType = typeNode->GetValue();
    std::string asmType = (varType == "integer") ? LONG_TYPE : BYTE_TYPE;
    std::string defaultValue = "0"; // Значение по умолчанию

    // Лог и генерация кода для переменной
    std::cout << "Generating data for variable: " << varName << " of type: " << varType << std::endl;
    addLine(varName + " " + asmType + " " + defaultValue);

    // Сохраняем переменную в таблицу идентификаторов
    id_map.emplace(varName, asmType);

    return EXIT_SUCCESS;
}

int GenCode::generateUninitVars(Tree* var_root)
{
    if (var_root == nullptr) return EXIT_SUCCESS;

    if (var_root->GetLeftNode() == nullptr)
    {
        std::cerr << "<E> GenCode: Can't find any variables" << std::endl;
        return -EXIT_FAILURE;
    }

    generateBssVar(var_root->GetLeftNode());
    if (var_root->GetRightNode() != nullptr)
        generateUninitVars(var_root->GetRightNode());

    return EXIT_SUCCESS;
}

int GenCode::generateBssVar(Tree* node)
{
    if (node == nullptr || node->GetLeftNode() == nullptr)
    {
        std::cerr << "<E> GenCode: Invalid BSS node or missing type information." << std::endl;
        return -EXIT_FAILURE;
    }

    std::string varName = node->GetValue();
    std::string varType = getType(node);

    if (id_map.find(varName) != id_map.end())
    {
        std::cerr << "<W> GenCode: Variable " << varName << " already exists. Skipping." << std::endl;
        return EXIT_FAILURE;
    }

    std::string asmType = (varType == "integer") ? LONG_SIZE : BYTE_SIZE;

    id_map.emplace(varName, asmType);
    addLine(varName + " resb " + asmType);  // Генерация памяти для переменной

    return EXIT_SUCCESS;
}


void GenCode::generateCompound(Tree* compound_tree)
{
    if (!compound_tree) return;

    generateExpressions(compound_tree->GetLeftNode());
    generateExpressions(compound_tree->GetRightNode());
}


void GenCode::generateIfStatement(Tree* expr_tree)
{
    if (expr_tree == nullptr) return;

    Tree* condition = expr_tree->GetLeftNode();  // Узел условия
    Tree* thenNode = expr_tree->GetRightNode()->GetLeftNode(); // Узел ветви "then"
    Tree* elseNode = expr_tree->GetRightNode()->GetRightNode(); // Узел ветви "else"

    std::string labelElse = "label_else_" + std::to_string(M++);
    std::string labelEnd = "label_end_" + std::to_string(M++);

    // Генерация кода условия
    generateExpressions(condition);
    addLine("pop eax");
    addLine("cmp eax, 0");
    addLine("je " + labelElse);

    // Генерация кода для ветви "then"
    if (thenNode)
    {
        processBeginBlock(thenNode);
    }

    addLine("jmp " + labelEnd);

    // Генерация кода для ветви "else"
    addLine(labelElse + ":");
    if (elseNode)
    {
        processBeginBlock(elseNode);
    }

    addLine(labelEnd + ":");
}


void GenCode::storeLabel(Tree* labelDeclNode)
{
    if (labelDeclNode == nullptr) return;

    Tree* nameNode = labelDeclNode->GetLeftNode();

    if (nameNode != nullptr)
    {
        std::string labelName = nameNode->GetValue();
        if (std::find(label_vector.begin(), label_vector.end(), labelName) == label_vector.end()) {
            label_vector.push_back(labelName);
            std::cout << "Stored label: " << labelName << std::endl;
        }
    }
}

void GenCode::addLine(const std::string& code_line)
{
    code << code_line << std::endl;
}

void GenCode::buildLine(std::string&& code_line)
{
    test_str << code_line << std::endl;
}


void GenCode::generateTextPart()
{
    addLine(CODE_SECT);

    addLine("");
    addLine("print_result proc");
    addLine("mov ebx, 10");
    addLine("xor ecx, ecx");
    addLine("lea edi, num_buffer");
    addLine("convert_number_loop:");
    addLine("xor edx, edx");
    addLine("div ebx");
    addLine("add dl, '0'");
    addLine("dec edi");
    addLine("mov [edi], dl");
    addLine("inc ecx");
    addLine("test eax, eax");
    addLine("jnz convert_number_loop");
    addLine("mov ah, 02h");
    addLine("mov esi, edi");
    addLine("output_number_loop:");
    addLine("lodsb");
    addLine("int 21h");
    addLine("loop output_number_loop");
    addLine("ret");
    addLine("print_result endp");
    addLine("");

    addLine(MAIN_SECT);
    addLine("mov ax, @data");
    addLine("mov ds, ax");

    Tree* blockNode = synt_tree->GetRightNode();
    if (blockNode != nullptr && blockNode->GetRightNode() != nullptr) {
        Tree* beginBlock = blockNode->GetRightNode();
        if (beginBlock != nullptr && beginBlock->GetValue() == "Begin") {
            processBeginBlock(beginBlock);
        }
    }

    // Вывод результата
    addLine("mov eax, result");
    addLine("call print_result");

    addLine("mov ah, 4Ch");
    addLine("int 21h");
    addLine(END_SECT);
    addLine("");
}



void GenCode::processBeginBlock(Tree* beginBlock)
{
    if (!beginBlock) return;

    Tree* current = beginBlock->GetLeftNode(); // Первый узел в блоке Begin

    while (current != nullptr)
    {
        std::cout << "Processing node in Begin block: " << current->GetValue() << std::endl;

        // Если это узел выражения с присваиванием ":="
        if (current->GetLeftNode() != nullptr && current->GetLeftNode()->GetValue() == ":=")
        {
            generateExpressions(current->GetLeftNode());  // Генерация выражения
        }
        else
        {
            std::cerr << "<W> Unsupported operation in Begin block: " << current->GetValue() << std::endl;
        }

        current = current->GetRightNode(); // Переход к следующему узлу в блоке Begin
    }
}






void GenCode::generateLabel(const std::string& name, const std::string& type, const std::string& val)
{
    buildLine(name + ": " + type + " " + val);
}


std::string GenCode::getType(Tree* node)
{
    if (node == nullptr || node->GetRightNode() == nullptr)
    {
        std::cerr << "<E> GenCode: Type node is missing for variable " << node->GetValue() << std::endl;
        return "";
    }
    return node->GetRightNode()->GetValue();
}

std::string GenCode::GetType(const std::string& Var)
{
    auto map_iter = id_map.find(Var);
    if (map_iter != id_map.end())
    {
        return map_iter->second;
    }
    else {
        return "";
    }
}


void GenCode::clearBuffer()
{
    test_str.str("");
    test_str.clear();
}


#endif