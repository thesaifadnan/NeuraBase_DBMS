#include"../include/schema.hpp"
#include<string>
using namespace std;

// Helper: convert string to FieldType
FieldType Schema::strToType(const std::string& s) {
    if (s == "NUMBER") return FieldType::NUMBER;
    if (s== "STRING") return FieldType::STRING;
    if (s == "BOOL") return FieldType::BOOL;
    throw runtime_error("Unknown type: " + s);
}

// Helper: print value properly
std::string Schema::typeToString(FieldType t) {
    switch(t) {
        case FieldType::NUMBER: return "NUMBER";
        case FieldType::STRING: return "STRING";
        case FieldType::BOOL: return "BOOL";
    }
    return "UNKNOWN";
}