#include "translator.h"
#include <gtest.h>
#include <cmath>
#include <sstream>

// Helper function for testing evaluation
double evaluateExpression(const std::string& expression, std::map<std::string, double>& variables) {
    std::vector<Token> tokens = tokenize(expression);
    Node* ast = parse(tokens);
    double result = evaluate(ast, variables);
    delete ast;
    return result;
}

// Helper function for testing evaluation without variables
double evaluateExpression(const std::string& expression) {
    std::map<std::string, double> variables;
    return evaluateExpression(expression, variables);
}


TEST(TokenizerTest, ValidTokens) {
    std::string input = "123 + 4.56 * sin(x) - 7 / (2 + pi)";
    std::vector<Token> tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 17);
    ASSERT_EQ(tokens[0].type, Token::NUMBER);
    ASSERT_EQ(tokens[0].value, "123");
    ASSERT_EQ(tokens[0].numValue, 123.0);
    ASSERT_EQ(tokens[1].type, Token::OPERATOR);
    ASSERT_EQ(tokens[1].value, "+");
    ASSERT_EQ(tokens[2].type, Token::NUMBER);
    ASSERT_EQ(tokens[2].value, "4.56");
    ASSERT_EQ(tokens[2].numValue, 4.56);
    ASSERT_EQ(tokens[3].type, Token::OPERATOR);
    ASSERT_EQ(tokens[3].value, "*");
    ASSERT_EQ(tokens[4].type, Token::FUNCTION);
    ASSERT_EQ(tokens[4].value, "sin");
    ASSERT_EQ(tokens[5].type, Token::PAREN_OPEN);
    ASSERT_EQ(tokens[6].type, Token::VARIABLE);
    ASSERT_EQ(tokens[6].value, "x");
    ASSERT_EQ(tokens[7].type, Token::PAREN_CLOSE);
    ASSERT_EQ(tokens[8].type, Token::OPERATOR);
    ASSERT_EQ(tokens[8].value, "-");
    ASSERT_EQ(tokens[9].type, Token::NUMBER);
    ASSERT_EQ(tokens[9].value, "7");
    ASSERT_EQ(tokens[9].numValue, 7.0);
    ASSERT_EQ(tokens[10].type, Token::OPERATOR);
    ASSERT_EQ(tokens[10].value, "/");
    ASSERT_EQ(tokens[11].type, Token::PAREN_OPEN);
    ASSERT_EQ(tokens[12].type, Token::NUMBER);
    ASSERT_EQ(tokens[12].value, "2");
    ASSERT_EQ(tokens[12].numValue, 2.0);
    ASSERT_EQ(tokens[13].type, Token::OPERATOR);
    ASSERT_EQ(tokens[13].value, "+");
    ASSERT_EQ(tokens[14].type, Token::VARIABLE);
    ASSERT_EQ(tokens[14].value, "pi");
    ASSERT_EQ(tokens[15].type, Token::PAREN_CLOSE);
    ASSERT_EQ(tokens[16].type, Token::END);
}

TEST(TokenizerTest, InvalidCharacter) {
    ASSERT_THROW(tokenize("123$"), std::runtime_error);
}

TEST(ParserTest, SimpleAddition) {
    std::vector<Token> tokens = tokenize("1 + 2");
    Node* ast = parse(tokens);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->token.type, Token::OPERATOR);
    ASSERT_EQ(ast->token.value, "+");
    ASSERT_EQ(ast->children.size(), 2);
    delete ast;
}


TEST(ParserTest, ComplexExpression) {
    std::vector<Token> tokens = tokenize("2 * (3 + 4) - 1");
    Node* ast = parse(tokens);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->token.type, Token::OPERATOR);
    ASSERT_EQ(ast->token.value, "-");
    ASSERT_EQ(ast->children.size(), 2);
    delete ast;
}

TEST(ParserTest, FunctionCall) {
    std::vector<Token> tokens = tokenize("sin(x)");
    Node* ast = parse(tokens);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->token.type, Token::FUNCTION);
    ASSERT_EQ(ast->token.value, "sin");
    ASSERT_EQ(ast->children.size(), 1);
    ASSERT_EQ(ast->children[0]->token.type, Token::VARIABLE);
    delete ast;
}
TEST(ParserTest, FunctionWithMultipleArguments)
{
    std::vector<Token> tokens = tokenize("atan2(y,x)");
    Node* ast = parse(tokens);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->token.type, Token::FUNCTION);
    ASSERT_EQ(ast->token.value, "atan2");
    ASSERT_EQ(ast->children.size(), 2);
    delete ast;
}

TEST(ParserTest, Assignment) {
    std::vector<Token> tokens = tokenize("x = 10");
    Node* ast = parse(tokens);
    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->token.type, Token::EQUAL);
    ASSERT_EQ(ast->children.size(), 2);
    ASSERT_EQ(ast->children[0]->token.type, Token::VARIABLE);
    ASSERT_EQ(ast->children[1]->token.type, Token::NUMBER);
    delete ast;
}
TEST(ParserTest, ExtraneousCharacters) {
    std::vector<Token> tokens = tokenize("1 + 2 abc");
    ASSERT_THROW(parse(tokens), std::runtime_error);
}
TEST(ParserTest, UnmatchedParenthesis) {
    std::vector<Token> tokens = tokenize("(1 + 2");
    ASSERT_THROW(parse(tokens), std::runtime_error);
}
TEST(ParserTest, MissingParenthesis) {
    std::vector<Token> tokens = tokenize("sin x");
    ASSERT_THROW(parse(tokens), std::runtime_error);
}


TEST(EvaluatorTest, SimpleEvaluation) {
    std::map<std::string, double> variables;
    ASSERT_EQ(evaluateExpression("2 + 3", variables), 5.0);
    ASSERT_EQ(evaluateExpression("10 - 5", variables), 5.0);
    ASSERT_EQ(evaluateExpression("4 * 6", variables), 24.0);
    ASSERT_EQ(evaluateExpression("10 / 2", variables), 5.0);
}


TEST(EvaluatorTest, ParenthesesEvaluation) {
    std::map<std::string, double> variables;
    ASSERT_EQ(evaluateExpression("(2 + 3) * 4", variables), 20.0);
    ASSERT_EQ(evaluateExpression("2 * (3 + 4)", variables), 14.0);
}

TEST(EvaluatorTest, VariableEvaluation) {
    std::map<std::string, double> variables;
    variables["x"] = 5;
    ASSERT_EQ(evaluateExpression("x + 2", variables), 7.0);
}

TEST(EvaluatorTest, AssignmentEvaluation)
{
    std::map<std::string, double> variables;
    // Corrected test: evaluate the assignment, then check the variable
    evaluateExpression("x = 5", variables);
    ASSERT_EQ(variables["x"], 5.0);
    ASSERT_EQ(evaluateExpression("x + 2", variables), 7.0);
}
TEST(EvaluatorTest, PiEvaluation) {
    std::map<std::string, double> variables;
    double pi = acos(-1.0);
    ASSERT_NEAR(evaluateExpression("pi", variables), pi, 1e-10);
}
TEST(EvaluatorTest, EEvaluation) {
    std::map<std::string, double> variables;
    double e = exp(1.0);
    ASSERT_NEAR(evaluateExpression("e", variables), e, 1e-10);
}
TEST(EvaluatorTest, FunctionEvaluation) {
    std::map<std::string, double> variables;
    ASSERT_NEAR(evaluateExpression("sin(0)", variables), 0.0, 1e-10);
    ASSERT_NEAR(evaluateExpression("cos(0)", variables), 1.0, 1e-10);
    ASSERT_NEAR(evaluateExpression("tan(0)", variables), 0.0, 1e-10);
    ASSERT_NEAR(evaluateExpression("asin(0)", variables), 0.0, 1e-10);
    ASSERT_NEAR(evaluateExpression("acos(1)", variables), 0.0, 1e-10);
    ASSERT_NEAR(evaluateExpression("atan(0)", variables), 0.0, 1e-10);
    ASSERT_NEAR(evaluateExpression("atan2(1,1)", variables), acos(-1.0) / 4, 1e-10);
    ASSERT_NEAR(evaluateExpression("exp(1)", variables), exp(1.0), 1e-10);
    ASSERT_NEAR(evaluateExpression("log(1)", variables), 0.0, 1e-10);
    ASSERT_NEAR(evaluateExpression("log10(10)", variables), 1.0, 1e-10);
    ASSERT_EQ(evaluateExpression("max(1, 2)", variables), 2.0);
    ASSERT_EQ(evaluateExpression("min(1, 2)", variables), 1.0);
}
TEST(EvaluatorTest, DivisionByZero)
{
    std::map<std::string, double> variables;
    ASSERT_THROW(evaluateExpression("10 / 0", variables), std::runtime_error);
}

TEST(EvaluatorTest, UndefinedVariable) {
    std::map<std::string, double> variables;
    ASSERT_THROW(evaluateExpression("x + 2", variables), std::runtime_error);
}
TEST(EvaluatorTest, UnaryMinus)
{
    std::map<std::string, double> variables;
    ASSERT_EQ(evaluateExpression("-5", variables), -5.0);
    ASSERT_EQ(evaluateExpression("5 - -5", variables), 10.0);
    ASSERT_EQ(evaluateExpression("-(2 + 3)", variables), -5.0);
}
TEST(EvaluatorTest, PowerEvaluation)
{
    std::map<std::string, double> variables;
    ASSERT_EQ(evaluateExpression("2^3", variables), 8.0);
}
TEST(EvaluatorTest, AssignConstant)
{
    std::map<std::string, double> variables;
    ASSERT_THROW(evaluateExpression("pi=5", variables), std::runtime_error);
}
TEST(FormatOutputTest, IntegerOutput) {
    ASSERT_EQ(formatOutput(5), "5");
}

TEST(FormatOutputTest, DecimalOutput) {
    ASSERT_EQ(formatOutput(5.12345), "5.1234500000");
}

TEST(FormatOutputTest, NearIntegerOutput) {
    ASSERT_EQ(formatOutput(5.00000000001), "5");
}

TEST(RunTest, SimpleRun) {
    std::stringstream input_stream;
    input_stream << "x = 5\n";
    input_stream << "x + 5\n";
    input_stream << "exit\n";
    std::streambuf* oldCout = std::cout.rdbuf();
    std::stringstream output_stream;
    std::cout.rdbuf(output_stream.rdbuf());
    std::streambuf* oldCin = std::cin.rdbuf();
    std::cin.rdbuf(input_stream.rdbuf());

    run();
    std::cout.rdbuf(oldCout);
    std::cin.rdbuf(oldCin);
    std::string expectedOutput = "> x = 5\n> 10\n";
    std::string actualOutput = output_stream.str();

    size_t found = actualOutput.find(expectedOutput);
    ASSERT_NE(found, std::string::npos);
}