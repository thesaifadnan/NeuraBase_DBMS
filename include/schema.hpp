#ifndef SCHEMA_HPP
#define SCHEMA_HPP
#include<vector>


enum class FieldType { NUMBER, STRING, BOOL};
enum class Operator { EQUAL, NOT_EQUAL,GREAT, GREATER_EQUAL, LESS, LESS_EQUAL};
enum class Keywords { CREATE, SELECT};

struct Field {
    FieldType type;
    std::string value;
};

struct ColumnDef {
    std::string colName;
    FieldType type;
};

struct TableSchema {
    std::string tableName;
    std::vector<ColumnDef> columns;
};

struct Condition {
    std::string column;
    std::string op;
    std::string value;
};


class Schema{
    public:
        static std::string typeToString(FieldType t);
        static FieldType strToType(const std::string& s);
};

#endif