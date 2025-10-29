// #include<iostream>
// #include<vector>
// #include<sstream>
// using namespace std;

// //STRUCTURES AND DEFINITIONS

// enum class FieldType {INT, FLOAT, STRING, BOOL};

// struct Field {
//     FieldType type;
//     string value;
// };

// struct ColumnDef{
//     string colName;
//     FieldType type;
// };

// struct TableSchema{
//     string tableName;
//     vector<ColumnDef> columns;
// };

// // HELPING FUNCTIONS

// void parser(string query){
//     TableSchema curSchema;
//     stringstream ss(query);
//     vector<string> words;
//     using Row = vector<Field>;
//     vector<Row> tableData;
//     string word;

//     while(ss >> word){
//         words.push_back(word);
//     }

//     if(words[0] == "CREATE"){
//         curSchema.tableName = words[2];
//         int i = 4;
//         string str = words[i];
//         while(str != ")" || str != ");"){
//             ColumnDef col;
//             col.colName = str;
//             col.type = FieldType::words[i+1];
//             curSchema.columns.push_back(col);
//             i+=2;
//             string str = words[i];
//         }
//     }

// // INSERT INTO emp ( 101 , "Adnan" );

//     if(words[0] == "INSERT"){
//         int len = curSchema.columns.size();
        
//         int i = 4, j=0;
//         Row r;

//         while(j != len){
//             r.push_back({FieldType::{curSchema.columns.type},words[i+1]});
//             tableData.push_back(r);
//             j++;
//         }
//     }

// //SELECT * FROM emp;
// // 101 Adnan
// // 102 Anika

//     if(words[0] == "SELECT"){
//         int len = tableData.size(); int j=0;
//         while(j !=len){
//             for(int i=0;i<=curSchema.columns.size();i++){
//                 cout<<tableData[i][j].value<<" ";
//             }
//             cout<<endl;
//             j++;
//         }
//     }
    
    
// }

// int main(){
//     string query;
//     cout<<"NeuraBase: ";
//     cin>>query;
//     parser(query);
//     return 0;
    
// }

// // CREATE TABLE emp ( id INT, name STRING );

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
using namespace std;

enum class FieldType { INT, FLOAT, STRING, BOOL };

struct Field {
    FieldType type;
    string value;
};

struct ColumnDef {
    string colName;
    FieldType type;
};

struct TableSchema {
    string tableName;
    vector<ColumnDef> columns;
};

using Row = vector<Field>;
vector<Row> tableData;
TableSchema curSchema;

// Helper: convert string to FieldType
FieldType strToType(const string& s) {
    if (s == "INT") return FieldType::INT;
    if (s =="FLOAT") return FieldType::FLOAT;
    if (s== "STRING") return FieldType::STRING;
    if (s == "BOOL") return FieldType::BOOL;
    throw runtime_error("Unknown type: " + s);
}

// Helper: print value properly
string typeToString(FieldType t) {
    switch(t) {
        case FieldType::INT: return "INT";
        case FieldType::FLOAT: return "FLOAT";
        case FieldType::STRING: return "STRING";
        case FieldType::BOOL: return "BOOL";
    }
    return "UNKNOWN";
}

void parser(string query) {
    stringstream ss(query);
    vector<string> words;
    string word;

    while (ss >> word) words.push_back(word);

    if (words.size() ==0) return;

    // CREATE TABLE emp ( id INT name STRING )
    if (words[0] == "CREATE") {
        curSchema.tableName = words[2];
        for (size_t i = 4; i <words.size(); i += 2) {
            if (words[i] == ")" || words[i] == ");") break;
            ColumnDef col;
            col.colName = words[i];
            col.type = strToType(words[i + 1]);
            curSchema.columns.push_back(col);
        }
        cout <<"Table "<< curSchema.tableName <<" created with "
             << curSchema.columns.size() << " columns.\n";
    }

    // INSERT INTO emp ( 101 "Adnan" )
    else if (words[0] == "INSERT") {
        Row r;
        size_t i =4;
        for (auto &col : curSchema.columns) {
            Field f;
            f.type = col.type;
            f.value = words[i++];
            r.push_back(f);
        }
        tableData.push_back(r);
        cout << "1 row inserted.\n";
    }

    // SELECT * FROM emp
    else if (words[0] == "SELECT") {
        cout << "TABLE: " <<curSchema.tableName << "\n";
        for (auto &col : curSchema.columns) {
            cout << col.colName << "(" << typeToString(col.type) << ") ";
        }
        cout << "\n----------------\n";
        for (auto &row : tableData) {
            for (auto &field : row) {
                cout << field.value << " ";
            }
            cout << "\n";
        }
    }
}

int main() {
    string query;
    while(query != "EXIT"){
        cout << "NeuraBase> ";
        getline(cin, query);
        parser(query);
    }

    return 0;
}
