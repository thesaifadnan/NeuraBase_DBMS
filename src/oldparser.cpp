#include"../include/parser.hpp"
#include<unordered_map>
#include<functional>
#include<iostream>
using namespace std;

TokenType strToTokenType(const std::string& upper) {
    if (upper == "STRING") {
        return TokenType::STRING;
    } else if (upper == "INT" || upper == "INTEGER" || upper == "FLOAT" || upper == "DOUBLE") {
        return TokenType::NUMBER;
    } else if (upper == "BOOL" || upper == "BOOLEAN") {
        return TokenType::BOOL;
    } else {
        throw std::invalid_argument("Unknown type: " + upper);
    }
}

std::string Parser::tokenToStr(TokenType type){
    if(type == TokenType::STRING) return "String";
    if(type == TokenType::NUMBER) return "Int";
    if(type == TokenType::BOOL) return "Bool";
    if(type == TokenType::KEYWORD) return "Keyword";
    if(type == TokenType::SYMBOL) return "Symbol";
    if(type == TokenType::OPERATOR) return "Operator";
    if(type == TokenType::IDENTIFIER) return "Indentifier";

    return "Unknown";
}

FieldType tokenTypeToFieldType(TokenType tokenType) {
    switch (tokenType) {
        case TokenType::NUMBER:
            // we can somehow distinguish INT vs FLOAT here if needed, but NUMBER is general
            return FieldType::NUMBER;
        case TokenType::STRING:
            return FieldType::STRING;
        case TokenType::BOOL:
            return FieldType::BOOL;
        default:
            throw std::invalid_argument("Unkown DataType: " +Parser::tokenToStr(tokenType));
    }
}

Keyword strToKeyword(const std::string& word) {
    static std::unordered_map<std::string, Keyword> keywordMap = {
        {"CREATE", Keyword::CREATE},
        {"TABLE", Keyword::TABLE},
        {"INSERT", Keyword::INSERT},
        {"INTO", Keyword::INTO},
        {"SELECT", Keyword::SELECT},
        {"FROM", Keyword::FROM},
        {"WHERE", Keyword::WHERE},
        {"VALUES", Keyword::VALUES},
        {"UPDATE", Keyword::UPDATE},
        {"SET", Keyword::SET}
    };

    auto it = keywordMap.find(word);
    if (it != keywordMap.end()) {
        return it->second;
    } else {
        throw std::invalid_argument("Invalid keyword: " + word);
    }
}

std::vector<Token> Parser::tokenize(std::string& query) {
    std::vector<Token> tokens;
    size_t i = 0;
    size_t len = query.length();

    while (i < len) {
        if (std::isspace(query[i])) {
            i++;
            continue;
        }

        // String values in quotes
        if (query[i] == '"' || query[i] == '\'') {
            char quote = query[i++];
            std::string word;
            while (i < len && query[i] != quote) {
                word.push_back(query[i++]);
            }
            if (i == len) throw std::runtime_error("Unclosed string literal");
            i++; // skip closing quote
            tokens.push_back({TokenType::STRING, word});
            continue;
        }

        // Numbers
        if (std::isdigit(query[i])) {
            std::string num;
            while (i < len && (std::isdigit(query[i]) || query[i] == '.')) {
                num.push_back(query[i++]);
            }
            tokens.push_back({TokenType::NUMBER, num});
            continue;
        }

        // Symbols
        if (query[i] == '(' || query[i] == ')' || query[i] == ',' || query[i] == ';' || query[i] == '*') {
            tokens.push_back({TokenType::SYMBOL, std::string(1, query[i++])});
            continue;
        }

        // Operators
        if (query[i] == '=' || query[i] == '<' || query[i] == '>' || query[i] == '!') {
            std::string op(1, query[i]);
            if (i + 1 < len && query[i + 1] == '=') {
                op.push_back('=');
                i++;
            }
            tokens.push_back({TokenType::OPERATOR, op});
            i++;
            continue;
        }

        // Identifiers or keywords
        if (std::isalpha(query[i])) {
            std::string word;
            while (i < len && (std::isalnum(query[i]) || query[i] == '_')) {
                word.push_back(query[i++]);
            }

            std::string upper;
            for (char c : word) upper.push_back(std::toupper(c));

            try {
                Keyword k = strToKeyword(upper);
                tokens.push_back({TokenType::KEYWORD, upper});
            } catch (...) {
                try {
                    TokenType type = strToTokenType(upper);
                    tokens.push_back({type, upper});
                } catch (...) {
                    tokens.push_back({TokenType::IDENTIFIER, word});
                }
            }

            continue;
        }

        throw std::runtime_error("Unknown character encountered during tokenization: " + std::string(1, query[i]));
    }

    return tokens;
}

// [KEYWORD: CREATE]
// [KEYWORD: TABLE]
// [IDENTIFIER: emp]
// [SYMBOL: (]
// [IDENTIFIER: id]
// [NUMBER: INT]
// [SYMBOL: ,]
// [IDENTIFIER: name]
// [STRING: STRING]
// [SYMBOL: )]
// [SYMBOL: ;]
createQuery Parser::parseCreate(vector<Token>& tokens){
    createQuery q;
    size_t i = 0;

    if(tokens[i].type == TokenType::KEYWORD && tokens[i].value == "CREATE"){
        q.operation = tokens[i].value;
        i++;
    }else{
        throw runtime_error("Expected 'CREATE' keyword");
    }

    if(i < tokens.size() && tokens[i].type == TokenType::KEYWORD && tokens[i].value == "TABLE"){
        i++;
    }else{
        throw runtime_error("Expected 'TABLE' keyword");
    }

    if(i < tokens.size() && tokens[i].type == TokenType::IDENTIFIER){
        q.table.tableName = tokens[i].value;
        i++;
    }else{
        throw runtime_error("Expected table name after TABLE");
    }

    if(i < tokens.size() && tokens[i].type == TokenType::SYMBOL && tokens[i].value == "("){
        i++;
    }else{
        throw runtime_error("Expected '(' after table name");
    }



    while(i < tokens.size()){
        if(tokens[i].type == TokenType::IDENTIFIER && i+1 < tokens.size()){
            q.table.columns.push_back({tokens[i].value,tokenTypeToFieldType(tokens[++i].type)});
            i++;
        }

        if(i < tokens.size() && tokens[i].type == TokenType::SYMBOL && tokens[i].value == ","){
            i++;
            continue;
        }

        if(i < tokens.size() && tokens[i].type == TokenType::SYMBOL && tokens[i].value == ")"){
            i++;
            break;
        }

        throw runtime_error("Unexpected token in column list");

    }

    if(i < tokens.size() && tokens[i].type == TokenType::SYMBOL && tokens[i].value == ";"){
        i++;
    }else{
        throw runtime_error("Expected ';' after table name");
    }
    return q;
}

//create table emp (id int, name string);

//INSERT INTO emp (id, name) values (101, "adnan");
//INSERT INTO emp (101, "Adnan");
//emp
//  - 101, adnan

insertQuery Parser::parseInsert(std::vector<Token>& tokens) {
    insertQuery q;
    size_t i = 0;

    //check for 'INSERT'
    if (i >= tokens.size() || tokens[i].value != "INSERT" || tokens[i].type != TokenType::KEYWORD) {
        throw std::runtime_error("Expected 'INSERT' keyword");
    }
    q.operation = tokens[i++].value;

    // Check for 'INTO'
    if (i >= tokens.size() || tokens[i].value != "INTO" || tokens[i].type != TokenType::KEYWORD) {
        throw std::runtime_error("Expected 'INTO' keyword");
    }
    i++;

    //table  name
    if (i >= tokens.size() || tokens[i].type != TokenType::IDENTIFIER) {
        throw std::runtime_error("Expected table name");
    }
    q.tableName = tokens[i++].value;

    // Check for 'VALUES'
    if (i >= tokens.size() || tokens[i].value != "VALUES" || tokens[i].type != TokenType::KEYWORD) {
        throw std::runtime_error("Expected 'VALUES' keyword");
    }
    i++;

    //Open parenthesis
    if (i >= tokens.size() || tokens[i].value != "(" || tokens[i].type != TokenType::SYMBOL) {
        throw std::runtime_error("Expected '(' after 'VALUES'");
    }
    i++;

    // Parse values
    while (i < tokens.size()) {
        if (tokens[i].type == TokenType::NUMBER ||
            tokens[i].type == TokenType::STRING ||
            tokens[i].type == TokenType::BOOL) {

            FieldType type = tokenTypeToFieldType(tokens[i].type);
            std::string value = tokens[i].value;
            q.fields.push_back({type, value});
            i++;

        } else {
            throw std::runtime_error("Expected a literal value (number, string, or bool)");
        }

        if (i >= tokens.size()) {
            throw std::runtime_error("Unexpected end of tokens while parsing VALUES");
        }

        if (tokens[i].value == "," && tokens[i].type == TokenType::SYMBOL) {
            i++;
            continue;
        } else if (tokens[i].value == ")" && tokens[i].type == TokenType::SYMBOL) {
            i++;
            break;
        } else {
            throw std::runtime_error("Expected ',' or ')' after value");
        }
    }

    // Check for semicolon
    if (i >= tokens.size() || tokens[i].value != ";" || tokens[i].type != TokenType::SYMBOL) {
        throw std::runtime_error("Expected ';' at the end of INSERT statement");
    }

    return q;
}


//just for tthe testing
ostream& operator<<(ostream& os, TokenType type) {
    switch (type) {
        case TokenType::NUMBER: return os << "NUMBER";
        case TokenType::STRING: return os << "STRING";
        case TokenType::SYMBOL: return os << "SYMBOL";
        case TokenType::IDENTIFIER: return os << "IDENTIFIER";
        case TokenType::KEYWORD: return os << "KEYWORD";
        case TokenType::OPERATOR: return os << "OPERATOR";
        default: return os << "UNKNOWN";
    }
}

AST Parser::parser(string& query){
    // string query = " CREATE table emp( id , name );";
    vector<Token> tokens = tokenize(query);

    //just for now later we will use dispatch table
    // if(tokens[0].value == "CREATE"){
    //     createQuery q = parseCreate(tokens);
    // }


//FOR printing the return values from tokenizer

    int i = 0;
    int len = tokens.size();
    while(i< len){
        cout<<tokens[i].type<<" : "<<tokens[i++].value<<endl;
    }

    // createQuery q = parseCreate(tokens);
    // cout<<q.operation<<endl;
    // cout<<q.table.tableName<<endl;
    // i =0;
    // while(i < q.table.columns.size()){
    //     cout<<q.table.columns[i].colName<<endl;
    //     cout<<Schema::typeToString(q.table.columns[i++].type)<<endl;
    // }

    // unordered_map<string, function<void(const vector<Token>&)>> dispatch = {
    //     {"CREATE", parseCreate},
    //     {"SELECT", parseSelect}
    // };

    // auto it = dispatch.find(tokens[0].value);
    // if(it != dispatch.end()){
    //     it->second(tokens);
    // }else{
    //     // Raise ERROR
    //     // cerr << "Unknown command: " << tokens[0].value <<endl;
    // }

    if (tokens.empty()) throw std::runtime_error("Empty query!");

    std::string command = tokens[0].value;

    try {
        if (command == "CREATE") {
            createQuery ast = parseCreate(tokens);
            std::cout << "[CREATE] Table: " << ast.table.tableName << "\n";
            for (const auto& col : ast.table.columns) {
                std::cout << "  - " << col.colName << ": " << Schema::typeToString(col.type) << "\n";
            }
            // return ast;

        } else if (command == "INSERT") {
            insertQuery ast = parseInsert(tokens);
            std::cout << "[INSERT] Into Table: " << ast.tableName << "\nValues: ";
            for (const auto& val : ast.fields) std::cout << val.value << " ";
            std::cout << "\n";

            return ast;

        } 
        else {
            throw std::runtime_error("Unsupported command: " + command);
        }
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] Parsing failed: " << ex.what() << std::endl;
    }
}


// int main(){
//     std::string query = " insert into emp values( 101, 'Adnan');";
//     Parser::parser(query);
//     return 0;
// }



//CREATE 
//     - create table 
//     - wrong datatype (only datatypes allowed from enum list)
//     - check wrong order of tokens 
//     - able to make primary key
//     - invalid attribute name (no space btw,  no symbols, missing datatype of attributes)

// INSERT
//     - insert values 
//     - check primary key is unique
//     - allowed to insert empty values if not primary
//     - dont insert in wrong order (match with the schema of table)

// SELECT
//     - check the table and attributes exists
//     - load asked data from db to memory 
//     - consider the where clause
//     - print the table data in a table format 

// UPDATE 
//     - check the table and attributes exists
//     - check if the update is allowed
//     - update the data in both memory and db
