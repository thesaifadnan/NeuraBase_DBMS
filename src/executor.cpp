#include<string>
#include<vector>
#include<fstream>
#include<iostream>
#include<iomanip>
#include<sstream>
#include<cctype>
#include<algorithm>
#include "../include/linenoise.h"
#include"../include/parser.hpp"

using namespace std;

//  -check if the table already exists
//  -creat tableName.schema file spearate for each one
//  -create tableNmae.data file separet fro each. one


void executeCreate(const createQuery& q){
    std::string schemaFile = "../data/catalog/" + q.table.tableName + ".schema";
    std::string dataFile = "../data/tables/" + q.table.tableName + ".data";

    // Check if files already exist
    std::ifstream openSchemaFile(schemaFile);
    std::ifstream openDataFile(dataFile);
    if (openSchemaFile.is_open() || openDataFile.is_open()) {
        std::cerr << "Table already exists: " << q.table.tableName << std::endl;
        openSchemaFile.close();
        openDataFile.close();
        return;
    }

    //create schema file
    std::ofstream curSchemaFile(schemaFile);
    if (!curSchemaFile) {
        std::cerr << "Error creating schema file!" << std::endl;
        return;
    }

    for (const auto& col : q.table.columns) {
        curSchemaFile << col.colName << ":" << Schema::typeToString(col.type) << std::endl;
    }
    curSchemaFile.close();

    std::ofstream curDataFile(dataFile);

    if (!curDataFile) {
        std::cerr << "Error creating data file!" << std::endl;
        return;
    }
    curDataFile.close();

    std::cout << "Table created successfully: " << q.table.tableName << std::endl;
}

void executeInsert(const insertQuery& q){
    std::string schemaFile = "../data/catalog/" + q.tableName + ".schema";
    std::string dataFile = "../data/tables/" + q.tableName + ".data";

    //check if filess exist
    std::ifstream openSchemaFile(schemaFile);
    std::ifstream openDataFile(dataFile);
    if (!openSchemaFile.is_open() || !openDataFile.is_open()) {
        std::cerr << "Table doesn't exists: " << q.tableName << std::endl;
        openSchemaFile.close();
        openDataFile.close();
        return;
    }

    openSchemaFile.close();
    openDataFile.close();

    std::ofstream curDataFile(dataFile, std::ios::app);

    if(!curDataFile){
        std::cerr << "Error writing to data file!" << std::endl;
        return;
    }

    string data;
    for (size_t i = 0; i < q.fields.size(); ++i) {
        data.append(q.fields[i].value);
        if (i < q.fields.size() - 1)
            data.append(",");
    }

    curDataFile << data<< std::endl;

    curDataFile.close();

    std::cout << "Data inserted successfully!" << std::endl;

}

string strToOperator( const string& op){
    return "=";
}

void printTableOnCLI(){
    
}

void executeSelect(const selectQuery& q) {
    TableSchema curTableSchema;

    //load schema file
    std::string schemaFile = "../data/catalog/" + q.tableName + ".schema";
    std::ifstream schemaIn(schemaFile);
    if (!schemaIn) {
        cout << "Error: Table '" << q.tableName << "' does not exist." << endl;
        return;
    }

    string line;
    while (getline(schemaIn, line)) {
        if (line.empty()) continue;
        size_t pos = line.find(":");
        string colName = line.substr(0, pos);
        string colType = line.substr(pos + 1);
        FieldType type;
        if (colType == "NUMBER") type = FieldType::NUMBER;
        else if (colType == "STRING") type = FieldType::STRING;
        else if (colType == "BOOL") type = FieldType::BOOL;
        else type = FieldType::STRING;
        curTableSchema.columns.push_back({colName, type});
    }
    schemaIn.close();

    // ;oad data file
    std::string dataFile = "../data/tables/" + q.tableName + ".data";
    std::ifstream dataIn(dataFile);
    if (!dataIn) {
        cout << "No data found in table '" << q.tableName << "'." << endl;
        return;
    }

    vector<vector<Field>> tableData;
    while (getline(dataIn, line)) {
        if (line.empty()) continue;
        vector<Field> row;
        string value;
        stringstream ss(line);
        int colIndex = 0;
        while (getline(ss, value, ',')) {
            if (value.empty()) continue;
            if (colIndex >= curTableSchema.columns.size()) {
                std::cerr << "[ERROR] Data row has more fields than schema defines.\n";
                break;
            }

            FieldType type = curTableSchema.columns[colIndex].type;
            row.push_back({type, value});
            colIndex++;
        }

        tableData.push_back(row);
    }
    dataIn.close();

    //apply WHERE clause ( if any)
    vector<vector<Field>> filteredData;
    for (auto& row : tableData) {
        bool include = true;

        if (!q.whereClause.empty()) {
            Condition cond = q.whereClause[0];
            int colIndex = -1;

            for (int i = 0; i < curTableSchema.columns.size(); i++) {
                if (curTableSchema.columns[i].colName == cond.column) {
                    colIndex = i;
                    break;
                }
            }

            if (colIndex == -1) {
                std::cerr << "Error: Column '" << cond.column << "' not found." << std::endl;
                return;
            }

            string rowVal = row[colIndex].value;
            string condVal = cond.value;

            if (curTableSchema.columns[colIndex].type == FieldType::NUMBER) {
                int rowNum = stoi(rowVal);
                int condNum = stoi(condVal);
                include = (rowNum == condNum); // TODO: handle more operators
            } else {
                include = (rowVal == condVal);
            }
        }

        if (include) filteredData.push_back(row);
    }


    // ddetermine columns to show (* or columns name like id, name,etc)
    vector<int> colIndexes;
    if (q.columns.size() == 1 && q.columns[0].value == "*") {
        for (int i = 0; i < curTableSchema.columns.size(); i++) {
            colIndexes.push_back(i);
        }
    } else {
        for (auto& col : q.columns) {
            int found = -1;
            for (int i = 0; i < curTableSchema.columns.size(); i++) {
                if (curTableSchema.columns[i].colName == col.value) {
                    found = i; break;
                }
            }
            if (found == -1) {
                cout << "Error: Column '" << col.value << "' not found." << endl;
                return;
            }
            colIndexes.push_back(found);
        }
    }

    //print output in sql like table format

    std::vector<size_t>colWidths(curTableSchema.columns.size(),0);

    for(size_t i =0; i < curTableSchema.columns.size(); i++){
        colWidths[i] = curTableSchema.columns[i].colName.length();
    }

    for(auto &row: filteredData){
        for(size_t i =0; i< row.size(); i++){
            colWidths[i] = std::max(colWidths[i], row[i].value.length());
        }
    }

    //printing the rows/lines
    // +--------+--------+---+
    auto printline = [&](){
        cout<<"+";
        for(size_t i = 0; i< colIndexes.size();i++){
            cout<< string(colWidths[colIndexes[i]]+2, '-') <<"+";
        }
        cout<<endl;
    };
    // | adnan sheikh | hello | 232 |
    auto printrow = [&](const vector<std::string>& values ){
        cout<<"|";
        for(size_t i= 0; i< colIndexes.size();i++){
            cout<<" " << std::left << setw(colWidths[colIndexes[i]])<< values[i] << " |";
        }
        cout<<endl;
    };

    //printing the actual data
    cout << "\nTable: " << q.tableName << endl;
    printline();

    vector<string> tableSchemaHeader;
    for(size_t i : colIndexes){
        tableSchemaHeader.push_back(curTableSchema.columns[i].colName);
    }
    printrow(tableSchemaHeader);
    printline();

    for(auto& row: filteredData){
        vector<string> rowValues;
        for(int idx: colIndexes){
            rowValues.push_back(row[idx].value);
        }
        printrow(rowValues);
    }
    printline();

}

void executeShow(const showQuery& q){
    std::vector<std::string> tables;
    std::string catalogPath = "../data/catalog";

    for (const auto& entry : std::filesystem::directory_iterator(catalogPath)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();

            //check if it's a .schema file
            if (filename.size() >= 7 && filename.substr(filename.size() - 7) == ".schema") {
                std::string tableName = filename.substr(0, filename.size() - 7);
                tables.push_back(tableName);
            }
        }
    }

    if (tables.empty()) {
        std::cout << "No tables found.\n";
        return;
    }

    std::cout << "Tables:\n";
    for (const auto& table : tables) {
        std::cout << "- " << table << "\n";
    }

}

void executeUpdate(const updateQuery& q) {
    std::string schemaFile = "../data/catalog/" + q.table + ".schema";
    std::string dataFile = "../data/tables/" + q.table + ".data";

    // Check if schema and data files exist
    std::ifstream schemaIn(schemaFile);
    std::ifstream dataIn(dataFile);
    if (!schemaIn.is_open() || !dataIn.is_open()) {
        std::cerr << "Error: Table '" << q.table << "' does not exist." << std::endl;
        return;
    }

    // Step 1: Load schema
    TableSchema schema;
    schema.tableName = q.table;
    std::string line;
    while (getline(schemaIn, line)) {
        if (line.empty()) continue;
        size_t pos = line.find(":");
        std::string colName = line.substr(0, pos);
        std::string colType = line.substr(pos + 1);
        FieldType type;
        if (colType == "NUMBER") type = FieldType::NUMBER;
        else if (colType == "STRING") type = FieldType::STRING;
        else if (colType == "BOOL") type = FieldType::BOOL;
        else type = FieldType::STRING;
        schema.columns.push_back({colName, type});
    }
    schemaIn.close();

    // Step 2: Load all rows into memory
    std::vector<std::vector<Field>> tableData;
    while (getline(dataIn, line)) {
        if (line.empty()) continue;
        std::vector<Field> row;
        std::stringstream ss(line);
        std::string value;
        int colIndex = 0;
        while (getline(ss, value, ',')) {
            if (colIndex >= schema.columns.size()) break;
            FieldType type = schema.columns[colIndex].type;
            row.push_back({type, value});
            colIndex++;
        }
        tableData.push_back(row);
    }
    dataIn.close();

    // Step 3: Determine WHERE clause (optional)
    int updatedCount = 0;
    for (auto& row : tableData) {
        bool shouldUpdate = true;

        if (!q.whereClause.empty()) {
            for (const auto& cond : q.whereClause) {
                int colIndex = -1;
                for (size_t i = 0; i < schema.columns.size(); ++i) {
                    if (schema.columns[i].colName == cond.column) {
                        colIndex = i;
                        break;
                    }
                }
                if (colIndex == -1) {
                    std::cerr << "Error: Column '" << cond.column << "' not found." << std::endl;
                    return;
                }

                std::string rowVal = row[colIndex].value;
                std::string condVal = cond.value;
                bool match = false;

                if (schema.columns[colIndex].type == FieldType::NUMBER) {
                    int rowNum = stoi(rowVal);
                    int condNum = stoi(condVal);
                    if (cond.op == "=") match = (rowNum == condNum);
                    else if (cond.op == "!=") match = (rowNum != condNum);
                    else if (cond.op == ">") match = (rowNum > condNum);
                    else if (cond.op == "<") match = (rowNum < condNum);
                    else if (cond.op == ">=") match = (rowNum >= condNum);
                    else if (cond.op == "<=") match = (rowNum <= condNum);
                } else {
                    if (cond.op == "=") match = (rowVal == condVal);
                    else if (cond.op == "!=") match = (rowVal != condVal);
                }

                if (!match) {
                    shouldUpdate = false;
                    break;
                }
            }
        }

        // Step 4: Apply updates if WHERE matches (or no WHERE)
        if (shouldUpdate) {
            for (const auto& assign : q.assignments) {
                int colIndex = -1;
                for (size_t i = 0; i < schema.columns.size(); ++i) {
                    if (schema.columns[i].colName == assign.column) {
                        colIndex = i;
                        break;
                    }
                }
                if (colIndex == -1) {
                    std::cerr << "Error: Column '" << assign.column << "' not found." << std::endl;
                    return;
                }

                // Basic validation
                if (schema.columns[colIndex].type == FieldType::NUMBER) {
                    try { std::stoi(assign.value); } 
                    catch (...) { std::cerr << "Invalid numeric value: " << assign.value << std::endl; return; }
                }

                row[colIndex].value = assign.value;
            }
            updatedCount++;
        }
    }

    // Step 5: Rewrite data file
    std::ofstream outFile(dataFile, std::ios::trunc);
    if (!outFile) {
        std::cerr << "Error writing to data file!" << std::endl;
        return;
    }

    for (const auto& row : tableData) {
        for (size_t i = 0; i < row.size(); ++i) {
            outFile << row[i].value;
            if (i < row.size() - 1) outFile << ",";
        }
        outFile << std::endl;
    }

    outFile.close();

    std::cout << updatedCount << " row(s) updated successfully." << std::endl;
}

void executeDelete(const deleteQuery& q) {
    std::string schemaFile = "../data/catalog/" + q.tableName + ".schema";
    std::string dataFile = "../data/tables/" + q.tableName + ".data";

    // Check if schema and data files exist
    std::ifstream schemaIn(schemaFile);
    std::ifstream dataIn(dataFile);
    if (!schemaIn.is_open() || !dataIn.is_open()) {
        std::cerr << "Error: Table '" << q.tableName << "' does not exist." << std::endl;
        return;
    }

    // Load schema
    TableSchema tableSchema;
    std::string line;
    while (getline(schemaIn, line)) {
        if (line.empty()) continue;
        size_t pos = line.find(":");
        std::string colName = line.substr(0, pos);
        std::string colType = line.substr(pos + 1);
        FieldType type;
        if (colType == "NUMBER") type = FieldType::NUMBER;
        else if (colType == "STRING") type = FieldType::STRING;
        else if (colType == "BOOL") type = FieldType::BOOL;
        else type = FieldType::STRING;
        tableSchema.columns.push_back({colName, type});
    }
    schemaIn.close();

    // Load data
    std::vector<std::vector<Field>> tableData;
    while (getline(dataIn, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::vector<Field> row;
        std::string value;
        int idx = 0;
        while (getline(ss, value, ',')) {
            if (idx >= tableSchema.columns.size()) break;
            row.push_back({tableSchema.columns[idx].type, value});
            idx++;
        }
        tableData.push_back(row);
    }
    dataIn.close();

    // Track how many were deleted
    size_t beforeCount = tableData.size();

    // Apply WHERE (delete matching rows)
    if (!q.whereClause.empty()) {
        tableData.erase(std::remove_if(tableData.begin(), tableData.end(),
            [&](const std::vector<Field>& row) {
                for (const auto& cond : q.whereClause) {
                    int colIndex = -1;
                    for (size_t i = 0; i < tableSchema.columns.size(); ++i) {
                        if (tableSchema.columns[i].colName == cond.column) {
                            colIndex = i;
                            break;
                        }
                    }

                    if (colIndex == -1) {
                        std::cerr << "Error: Column '" << cond.column << "' not found." << std::endl;
                        return false;
                    }

                    std::string rowVal = row[colIndex].value;
                    std::string condVal = cond.value;
                    bool match = false;

                    if (tableSchema.columns[colIndex].type == FieldType::NUMBER) {
                        int rowNum = std::stoi(rowVal);
                        int condNum = std::stoi(condVal);
                        if (cond.op == "=") match = (rowNum == condNum);
                        else if (cond.op == "!=") match = (rowNum != condNum);
                        else if (cond.op == ">") match = (rowNum > condNum);
                        else if (cond.op == "<") match = (rowNum < condNum);
                        else if (cond.op == ">=") match = (rowNum >= condNum);
                        else if (cond.op == "<=") match = (rowNum <= condNum);
                    } else {
                        if (cond.op == "=") match = (rowVal == condVal);
                        else if (cond.op == "!=") match = (rowVal != condVal);
                    }

                    // 🔥 FIX: if the row matches condition, we return true (delete it)
                    if (match) return true;
                }
                return false; // keep row
            }), tableData.end());
    } else {
        // No WHERE → delete all rows
        tableData.clear();
    }

    // Save file
    std::ofstream out(dataFile, std::ios::trunc);
    if (!out) {
        std::cerr << "Error writing to data file!" << std::endl;
        return;
    }

    for (const auto& row : tableData) {
        for (size_t i = 0; i < row.size(); ++i) {
            out << row[i].value;
            if (i < row.size() - 1) out << ",";
        }
        out << "\n";
    }

    out.close();

    size_t deletedCount = beforeCount - tableData.size();
    std::cout << deletedCount << " row(s) deleted successfully." << std::endl;
}


void executor(std::string& query) {
    AST ast = Parser::parser(query);

    std::visit([](auto&& q) {
        using T = std::decay_t<decltype(q)>;

        if constexpr (std::is_same_v<T, createQuery>) {
            executeCreate(q);
        } else if constexpr (std::is_same_v<T, insertQuery>) {
            executeInsert(q);
        } else if constexpr (std::is_same_v<T, selectQuery>) {
            executeSelect(q);
        } else if constexpr (std::is_same_v<T, showQuery>) {
            executeShow(q);
        } else if constexpr (std::is_same_v<T, deleteQuery>) {
            executeDelete(q);
        } else {
            std::cerr << "Unknown query type in executor.\n";
        }
    }, ast);
}

static inline std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\n\r");
    auto end = s.find_last_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

int main() {
    while (true) {
        char* input = linenoise("NeuraBase> ");
        if (!input) break;
        std::string query(input);
        free(input);

        if (query.empty()) continue;

        linenoiseHistoryAdd(query.c_str()); // save history like to scroll through the previous typed commands

        std::string lower = query;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == "exit") break;

        executor(query);
    }
    return 0;
}