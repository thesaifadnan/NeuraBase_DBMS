#ifndef PARSER_HPP
#define PARSER_HPP
#include"schema.hpp"
#include<vector>
#include<variant>
#include<type_traits>
using namespace std;

enum class TokenType {  STRING, SYMBOL, IDENTIFIER, KEYWORD, OPERATOR, BOOL, NUMBER };

enum class Keyword {
    CREATE, TABLE, INSERT, UPDATE, SHOW, INTO,  SELECT, FROM, WHERE, VALUES, SET, TABLES, AND
};

struct Token {
    TokenType type;
    std::string value;
};

struct createQuery {
    std::string operation;
    TableSchema table;
};

struct insertQuery {
    std::string operation;
    std::string tableName;
    std::vector<Field> fields;
};

struct selectQuery {
    std::string operation;
    std::string tableName;
    std::vector<Token> columns;
    std::vector<Condition> whereClause;
};
// update 
struct updateQuery {
    std::string operation;
    std::string table;
    std::vector<Condition> assignments;
    std::vector<Condition> whereClause;
};
// show tables;
struct showQuery{
    std::string operation;
};

using AST = std::variant<createQuery, insertQuery, selectQuery, updateQuery, showQuery>;

//ignore the return types of the parser functions
class Parser{
    public:
        static std::vector<Token> tokenize(std::string& query);
        static AST parser(std::string& query);
        static createQuery parseCreate(std::vector<Token>& tokens);
        static selectQuery parseSelect(const std::vector<Token>& tokens);
        static insertQuery parseInsert(std::vector<Token>& tokens);
        static updateQuery parseUpdate(const std::vector<Token>& tokens);
        static std::string tokenToStr(TokenType type);
        static showQuery parseShow(const std::vector<Token>& tokens);
};

#endif

// SELECT
// - columns: ["name", "age"]
// - table: "users"
// - where:
//      - column: "age"
//      - operator: ">"
//      - value: 18