#pragma once

#include <iostream>
#include <vector>
#include <fstream>

enum TOKEN_TYPE {
    BEGIN_TAG,          // '<...'
    END_TAG,            // '...>'
    CLOSE_TAG,          // '</...'
    END_AND_CLOSE_TAG,  // '.../>'
    ELEMENT,            // <ELEMENT>, <ELEMENT/>, </ELEMENT>,
                        // <ELEMENT ATTRIBUTE_NAME="ATTRIBUTE_VALUE">
    CONTENT,            // text between END_TAG and CLOSE_TAG
    ATTRIBUTE_NAME,     // 'ATTRIBUTE_NAME = "ATTRIBUTE_VALUE'
    ATTRIBUTE_VALUE,     // 'ATTRIBUTE_NAME = "ATTRIBUTE_VALUE'

    END_PARSING
};

struct Token {
    TOKEN_TYPE type;
    std::string lexeme;

    Token(TOKEN_TYPE t) : type(t), lexeme() {};
    Token(TOKEN_TYPE t, std::string l) : type(t), lexeme(l) {};
};

enum LEXER_STATE {
    DEFAULT,
    IN_TAG
};

LEXER_STATE current_state = DEFAULT;


int skip_whitespace(char **ptr){
    while(**ptr == ' ' || **ptr == '\n' || **ptr == '\t' || **ptr == '\r'){
        if(**ptr == '\0') return -1;
        (*ptr)++;
    }
    return 0;
}

void read_buffer(FILE *file, char *buffer, int size){
    fread(buffer, sizeof(char), size, file);
}

void Tokenize(std::vector<Token> &tokens, std::string filename){
    FILE *file = fopen(filename.c_str(), "r");

    char *ptr = new char[1001];
    read_buffer(file, ptr, 1000);
    ptr[1000]='\0';
    fclose(file);

    while(*ptr != '\0'){
        switch(current_state){
            case DEFAULT: {
                if(skip_whitespace(&ptr) == -1) return;
                /* std::cout << "DEFAULT state\n"; */

                switch(*ptr){
                    case '<': {
                        /* std::cout << "\tOpening brace\n"; */
                        ptr++;
                        if(*ptr == '\0') return;
                        else if(*ptr == '/'){
                            current_state = IN_TAG;
                            tokens.push_back(Token(CLOSE_TAG));
                            ptr++;
                        }
                        else {
                            current_state = IN_TAG;
                            tokens.push_back(Token(BEGIN_TAG));
                        }
                        break;
                    }
                    default: {
                        /* std::cout << "\tLetters\n"; */
                        std::string buffer;
                        while(*ptr != '\0' && *ptr != '<'){
                            buffer += *ptr;
                            ptr++;
                        }
                        tokens.push_back(Token(CONTENT, buffer));
                        break;
                    }
                }
                break;
            }

            case IN_TAG: {
                if(skip_whitespace(&ptr) == -1) return;
                /* std::cout << "IN_TAG state, current char ->" << *ptr << '\n'; */
                std::string buffer;
                switch(*ptr){
                    case '>': {
                        /* std::cout << "\tClosing tag\n"; */
                        tokens.push_back(Token(END_TAG));
                        current_state = DEFAULT;
                        ptr++;
                        break;
                    }
                    case '/': {
                        std::cout << "\tSlash\n";
                        /* if(*ptr == '>') tokens.push_back(Token(END_AND_CLOSE_TAG)); */
                        tokens.push_back(Token(END_AND_CLOSE_TAG));
                        ptr+=2;
                        current_state = DEFAULT;
                        break;
                    }
                    case '"': {
                        /* std::cout << "\tQuotations\n"; */
                        std::string buffer;
                        ptr++;
                        while(*ptr != '"'){
                            buffer += *ptr;
                            ptr++;
                        }
                        ptr++;
                        tokens.push_back(Token(ATTRIBUTE_VALUE, buffer));
                        break;
                    }
                    default: {
                        /* std::cout << "\tLetters\n"; */
                        std::string buffer;
                        while(*ptr != ' ' && *ptr != '=' && *ptr != '>'){
                            buffer += *ptr;
                            ptr++;
                        }
                        if(skip_whitespace(&ptr) == -1) return;
                        switch(*ptr){
                            case '=': {
                                tokens.push_back(Token(ATTRIBUTE_NAME, buffer));
                                while(*ptr != '"') ptr++;
                                break;
                            }
                            default: {
                                tokens.push_back(Token(ELEMENT, buffer));
                                break;
                            }
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
    /* std::cout << "Finished function\n"; */
    return;
}
