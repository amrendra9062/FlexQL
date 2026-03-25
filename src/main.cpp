#include "parser/tokenizer.h"
#include "parser/parser.h"
#include "query/executor.h"

#include <iostream>

int main() {
    Database db;
    Executor executor(db);    
    std::vector<std::string> queries = {
    "CREATE TABLE student (id INT, name VARCHAR)",

    "INSERT INTO student VALUES (1, 'Alice')",
    "INSERT INTO student VALUES (2, 'Bob')",
    "INSERT INTO student VALUES (3, 'Charlie')",

    "SELECT * FROM student WHERE id = 2"
    "SELECT * FROM student WHERE id = 2",
    "SELECT * FROM student WHERE id = 2"
};

    for (auto& sql : queries) {
        Tokenizer tokenizer(sql);
        auto tokens = tokenizer.tokenize();

        Parser parser(tokens);
        Query query = parser.parse();

        executor.execute(query);
    }

    return 0;
}