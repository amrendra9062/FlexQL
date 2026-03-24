#include <iostream>
#include "parser/tokenizer.h"
#include "parser/parser.h"

int main() {
    std::string query = "SELECT name FROM student WHERE id = 1;";

    Tokenizer tokenizer(query); 
    auto tokens = tokenizer.tokenize();

    Parser parser(tokens);
    Query q = parser.parse();

    std::cout << "Parsed SELECT on table: " << q.selectQuery.table << std::endl;

    if (q.selectQuery.has_where) {
        std::cout << "WHERE " 
                  << q.selectQuery.where.column << " "
                  << q.selectQuery.where.op << " "
                  << q.selectQuery.where.value << std::endl;
    }

    return 0;
}