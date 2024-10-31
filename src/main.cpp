#include <iostream>
#include <vector>
#include "BanzaiXML.h"
/* #include "simple.h" */


int main(){

    std::string token_typenames[9] = {
        "BEGIN_TAG",
        "END_TAG",
        "CLOSE_TAG",
        "END_AND_CLOSE_TAG",
        "ELEMENT",
        "CONTENT",
        "ATTRIBUTE_NAME",
        "ATTRIBUTE_VALUE",
        "END_PARSING"
    };

    std::string filename = "../test/lukesmith.xml";

    Lexer l;

    l.openFile(filename);
    l.readBuffer(1500);
    Token t;
    std::vector<Token> result;

    while((t = l.getToken()).type != END_PARSING){
        result.push_back(t);
    }
    l.closeFile();

    /* std::vector<Token> result; */

    /* Tokenize(result, filename); */

    for(auto t : result)
        std::cout << token_typenames[t.type] << " " << t.lexeme << '\n';
}
