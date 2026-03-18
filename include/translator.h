#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cmath>
#include <map>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <locale>

// Structure for the token representation
struct Token {
    enum Type {
        NUMBER,
        OPERATOR,
        VARIABLE,
        FUNCTION,
        PAREN_OPEN,
        PAREN_CLOSE,
        COMMA,
        EQUAL,
        END
    } type;
    std::string value;
    double numValue;
};

// Lexical analysis
std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        if (isspace(c)) continue; // skip the spaces
        if (isdigit(c) || (c == '.' && !tokens.empty() && tokens.back().type == Token::NUMBER)) { // Число
            std::string numStr;
            bool hasDot = (c == '.');
            numStr += c;
            while (i + 1 < input.size() && (isdigit(input[i + 1]) || (input[i + 1] == '.' && !hasDot))) {
                if (input[i + 1] == '.') hasDot = true;
                numStr += input[++i];
            }
            double numVal = std::stod(numStr);
            tokens.push_back({ Token::NUMBER, numStr, numVal });
        }
        else if (isalpha(c) || c == '_') { // variable or function
            std::string varFuncStr;
            varFuncStr += c;
            while (i + 1 < input.size() && (isalnum(input[i + 1]) || input[i + 1] == '_')) {
                varFuncStr += input[++i];
            }

            if (varFuncStr == "sin" || varFuncStr == "cos" || varFuncStr == "tan" || varFuncStr == "atan" ||
                varFuncStr == "asin" || varFuncStr == "acos" || varFuncStr == "atan2" || varFuncStr == "exp" ||
                varFuncStr == "log" || varFuncStr == "log10" || varFuncStr == "max" || varFuncStr == "min")
            {
                tokens.push_back({ Token::FUNCTION, varFuncStr, 0.0 });
            }
            else {
                tokens.push_back({ Token::VARIABLE, varFuncStr, 0.0 });
            }
        }
        else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') { // operator
            tokens.push_back({ Token::OPERATOR, std::string(1, c), 0.0 });
        }
        else if (c == '(') {
            tokens.push_back({ Token::PAREN_OPEN, "(", 0.0 });
        }
        else if (c == ')') {
            tokens.push_back({ Token::PAREN_CLOSE, ")", 0.0 });
        }
        else if (c == ',') {
            tokens.push_back({ Token::COMMA, ",", 0.0 });
        }
        else if (c == '=') {
            tokens.push_back({ Token::EQUAL, "=", 0.0 });
        }
        else {
            throw std::runtime_error("Invalid character: " + std::string(1, c));
        }
    }
    tokens.push_back({ Token::END, "", 0.0 }); // end token
    return tokens;
}

// The structure of the AST node
struct Node {
    Token token;
    std::vector<Node*> children;

    Node(Token t) : token(t) {}
    ~Node() {
        for (Node* child : children) {
            delete child;
        }
    }
};

//// A function for displaying the type of token (for debugging)
//std::string tokenTypeToString(Token::Type type) {
//    switch (type) {
//    case Token::NUMBER: return "NUMBER";
//    case Token::OPERATOR: return "OPERATOR";
//    case Token::VARIABLE: return "VARIABLE";
//    case Token::FUNCTION: return "FUNCTION";
//    case Token::PAREN_OPEN: return "PAREN_OPEN";
//    case Token::PAREN_CLOSE: return "PAREN_CLOSE";
//    case Token::COMMA: return "COMMA";
//    case Token::EQUAL: return "EQUAL";
//    case Token::END: return "END";
//    default: return "UNKNOWN";
//    }
//}


//// Функция для печати AST (для отладки)
//void printAST(const Node* node, int indent = 0) {
//    if (!node) return;
//    for (int i = 0; i < indent; ++i) {
//        std::cout << "  ";
//    }
//    std::cout << "Type: " << tokenTypeToString(node->token.type) << ", Value: " << node->token.value;
//    if (node->token.type == Token::NUMBER)
//    {
//        std::cout << ", Num Value: " << node->token.numValue;
//    }
//    std::cout << std::endl;
//    for (Node* child : node->children) {
//        printAST(child, indent + 1);
//    }
//}

// Prototypes of functions for parsing and computing
Node* parseExpression(std::vector<Token>& tokens, size_t& pos);
Node* parseTerm(std::vector<Token>& tokens, size_t& pos);
Node* parsePower(std::vector<Token>& tokens, size_t& pos);
Node* parseFactor(std::vector<Token>& tokens, size_t& pos);
double evaluate(Node* node, std::map<std::string, double>& variables);

// Syntactic analysis (recursive descent)
Node* parse(std::vector<Token> tokens) {
    size_t pos = 0;
    // Checking for assignment
    if (tokens.size() >= 3 && tokens[1].type == Token::EQUAL && tokens[0].type == Token::VARIABLE)
    {
        if (tokens[0].value == "pi" || tokens[0].value == "e")
        {
            throw std::runtime_error("Error: cannot reassign constant value");
        }
        Node* assignmentNode = new Node(tokens[1]);
        Node* varNode = new Node(tokens[0]);
        pos = 2;
        Node* expressionNode = parseExpression(tokens, pos);

        assignmentNode->children.push_back(varNode);
        assignmentNode->children.push_back(expressionNode);
        if (tokens[pos].type != Token::END)
        {
            delete assignmentNode;
            throw std::runtime_error("Error: extraneous characters at end of expression.");
        }
        return assignmentNode;
    }
    else
    {
        Node* root = parseExpression(tokens, pos);
        if (tokens[pos].type != Token::END)
        {
            delete root;
            throw std::runtime_error("Error: extraneous characters at end of expression.");
        }
        return root;
    }

}


// Parsing an expression
Node* parseExpression(std::vector<Token>& tokens, size_t& pos) {
    Node* left = parseTerm(tokens, pos);
    while (tokens[pos].type == Token::OPERATOR && (tokens[pos].value == "+" || tokens[pos].value == "-")) {
        Token op = tokens[pos++];
        Node* right = parseTerm(tokens, pos);
        Node* node = new Node(op);
        node->children.push_back(left);
        node->children.push_back(right);
        left = node;
    }
    return left;
}


// Term Parsing
Node* parseTerm(std::vector<Token>& tokens, size_t& pos) {
    Node* left = parsePower(tokens, pos);
    while (tokens[pos].type == Token::OPERATOR && (tokens[pos].value == "*" || tokens[pos].value == "/")) {
        Token op = tokens[pos++];
        Node* right = parsePower(tokens, pos);
        Node* node = new Node(op);
        node->children.push_back(left);
        node->children.push_back(right);
        left = node;
    }
    return left;
}


// Degree Parsing
Node* parsePower(std::vector<Token>& tokens, size_t& pos)
{
    Node* left = parseFactor(tokens, pos);
    while (tokens[pos].type == Token::OPERATOR && tokens[pos].value == "^")
    {
        Token op = tokens[pos++];
        Node* right = parseFactor(tokens, pos);
        Node* node = new Node(op);
        node->children.push_back(left);
        node->children.push_back(right);
        left = node;
    }
    return left;
}

// Factor Parsing
Node* parseFactor(std::vector<Token>& tokens, size_t& pos) {
    bool unaryMinus = false;
    while (tokens[pos].type == Token::OPERATOR && tokens[pos].value == "-")
    {
        unaryMinus = !unaryMinus;
        pos++;
    }

    Node* node;
    if (tokens[pos].type == Token::NUMBER) {
        node = new Node(tokens[pos++]);
    }
    else if (tokens[pos].type == Token::VARIABLE) {
        node = new Node(tokens[pos++]);
    }
    else if (tokens[pos].type == Token::FUNCTION)
    {
        Token funcToken = tokens[pos++];
        if (tokens[pos].type != Token::PAREN_OPEN)
        {
            throw std::runtime_error("Error: expected opening parenthesis after function " + funcToken.value);
        }
        pos++;
        std::vector<Node*> funcArgs;
        if (tokens[pos].type != Token::PAREN_CLOSE)
        {

            funcArgs.push_back(parseExpression(tokens, pos));
            while (tokens[pos].type == Token::COMMA)
            {
                pos++;
                funcArgs.push_back(parseExpression(tokens, pos));
            }

        }

        if (tokens[pos].type != Token::PAREN_CLOSE)
        {
            throw std::runtime_error("Error: expected closing parenthesis after function " + funcToken.value);
        }
        pos++;
        node = new Node(funcToken);
        node->children = funcArgs;
    }
    else if (tokens[pos].type == Token::PAREN_OPEN) {
        pos++;
        node = parseExpression(tokens, pos);
        if (tokens[pos].type != Token::PAREN_CLOSE) {
            delete node;
            throw std::runtime_error("Error: expected closing parenthesis");
        }
        pos++;
    }
    else {
        throw std::runtime_error("Error: expected factor");
    }
    if (unaryMinus)
    {
        Node* minusNode = new Node({ Token::OPERATOR, "-", 0.0 });
        minusNode->children.push_back(node);
        return minusNode;
    }
    return node;
}


// Calculation
double evaluate(Node* node, std::map<std::string, double>& variables) {
    if (!node) return 0.0;

    if (node->token.type == Token::NUMBER) {
        return node->token.numValue;
    }
    else if (node->token.type == Token::VARIABLE) {
        if (node->token.value == "pi")
            return acos(-1.0);
        else if (node->token.value == "e")
            return exp(1.0);
        else if (variables.count(node->token.value))
            return variables[node->token.value];
        else
            throw std::runtime_error("Error: variable '" + node->token.value + "' is not defined");
    }
    else if (node->token.type == Token::OPERATOR) {
        if (node->token.value == "+") {
            return evaluate(node->children[0], variables) + evaluate(node->children[1], variables);
        }
        else if (node->token.value == "-") {
            if (node->children.size() == 1)
            {
                return -evaluate(node->children[0], variables); // unary minus
            }
            return evaluate(node->children[0], variables) - evaluate(node->children[1], variables);
        }
        else if (node->token.value == "*") {
            return evaluate(node->children[0], variables) * evaluate(node->children[1], variables);
        }
        else if (node->token.value == "/") {
            double divisor = evaluate(node->children[1], variables);
            if (divisor == 0)
            {
                throw std::runtime_error("Error: division by zero");
            }
            return evaluate(node->children[0], variables) / divisor;
        }
        else if (node->token.value == "^") {
            return pow(evaluate(node->children[0], variables), evaluate(node->children[1], variables));
        }
    }
    else if (node->token.type == Token::FUNCTION) {
        if (node->token.value == "sin") {
            if (node->children.size() != 1) {
                throw std::runtime_error("Error: function sin expects 1 argument");
            }
            return sin(evaluate(node->children[0], variables));
        }
        else if (node->token.value == "cos") {
            if (node->children.size() != 1) {
                throw std::runtime_error("Error: function cos expects 1 argument");
            }
            return cos(evaluate(node->children[0], variables));
        }
        else if (node->token.value == "tan") {
            if (node->children.size() != 1) {
                throw std::runtime_error("Error: function tan expects 1 argument");
            }
            return tan(evaluate(node->children[0], variables));
        }
        else if (node->token.value == "asin") {
            if (node->children.size() != 1) {
                throw std::runtime_error("Error: function asin expects 1 argument");
            }
            return asin(evaluate(node->children[0], variables));
        }
        else if (node->token.value == "acos") {
            if (node->children.size() != 1) {
                throw std::runtime_error("Error: function acos expects 1 argument");
            }
            return acos(evaluate(node->children[0], variables));
        }
        else if (node->token.value == "atan") {
            if (node->children.size() != 1) {
                throw std::runtime_error("Error: function atan expects 1 argument");
            }
            return atan(evaluate(node->children[0], variables));
        }
        else if (node->token.value == "atan2") {
            if (node->children.size() != 2) {
                throw std::runtime_error("Error: function atan2 expects 2 arguments");
            }
            return atan2(evaluate(node->children[0], variables), evaluate(node->children[1], variables));
        }
        else if (node->token.value == "exp") {
            if (node->children.size() != 1) {
                throw std::runtime_error("Error: function exp expects 1 argument");
            }
            return exp(evaluate(node->children[0], variables));
        }
        else if (node->token.value == "log") {
            if (node->children.size() != 1) {
                throw std::runtime_error("Error: function log expects 1 argument");
            }
            return log(evaluate(node->children[0], variables));
        }
        else if (node->token.value == "log10") {
            if (node->children.size() != 1) {
                throw std::runtime_error("Error: function log10 expects 1 argument");
            }
            return log10(evaluate(node->children[0], variables));
        }
        else if (node->token.value == "max") {
            if (node->children.size() != 2) {
                throw std::runtime_error("Error: function max expects 2 arguments");
            }
            return std::max(evaluate(node->children[0], variables), evaluate(node->children[1], variables));
        }
        else if (node->token.value == "min") {
            if (node->children.size() != 2) {
                throw std::runtime_error("Error: function min expects 2 arguments");
            }
            return std::min(evaluate(node->children[0], variables), evaluate(node->children[1], variables));
        }
    }
    else if (node->token.type == Token::EQUAL)
    {
        if (node->children.size() == 2 && node->children[0]->token.type == Token::VARIABLE)
        {
            variables[node->children[0]->token.value] = evaluate(node->children[1], variables);
            return variables[node->children[0]->token.value];
        }
        else
        {
            throw std::runtime_error("Error: invalid assignment");
        }
    }

    throw std::runtime_error("Error: unknown AST node");
}

// Formatting the output
std::string formatOutput(double value) {
    if (std::abs(value - static_cast<int>(value)) < 1e-10) {
        return std::to_string(static_cast<int>(value));
    }
    else {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(10) << value;
        return ss.str();
    }
}


int run() {
    std::map<std::string, double> variables;
    std::string input;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        if (input == "exit") break;

        try {
            std::vector<Token> tokens = tokenize(input);
            Node* ast = parse(tokens);

            if (ast->token.type == Token::EQUAL && ast->children.size() == 2 && ast->children[0]->token.type == Token::VARIABLE) {
                double result = evaluate(ast->children[1], variables);
                variables[ast->children[0]->token.value] = result;
                std::cout << ast->children[0]->token.value << " = " << formatOutput(result) << std::endl;
            }
            else
            {
                double result = evaluate(ast, variables);
                std::cout << formatOutput(result) << std::endl;
            }

            delete ast;
        }
        catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}