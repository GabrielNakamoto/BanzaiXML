#pragma once

#include <iostream>
#include <vector>
#include <map>

/** TODO:
 *
 * https://www.w3schools.com/xml/xml_syntax.asp
 * - escape characters: &
 * - XML comments
 * - namespaces
 * - input buffering
 *
 */

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

/* std::map<std::string, char> ENTITY_REF = { */
/*     { "&lt", '<' }, */
/*     { "&gt", '>' }, */
/*     { "&amp", '&' } */
/*     /1* { "&apos", char(39)} *1/ */
/* }; */

struct Token {
    TOKEN_TYPE type;
    std::string lexeme;

    Token() {};
    Token(TOKEN_TYPE t) : type(t), lexeme() {};
    Token(TOKEN_TYPE t, std::string l) : type(t), lexeme(l) {};
};


class Lexer;

class LexerState {
public:
    virtual ~LexerState() {};
    virtual Token getToken(Lexer *l) = 0;
};


class DefaultState : public LexerState {
public:
    ~DefaultState() {};
    Token getToken(Lexer *l);
    static LexerState& getInstance();
private:
    DefaultState() {};
};

class TagState : public LexerState {
public:
    ~TagState() {};
    Token getToken(Lexer *l);
    static LexerState& getInstance();
private:
    TagState() {};
};



class Lexer {
public:
    Lexer() : currentState(&DefaultState::getInstance()), fileHandle(nullptr), ptr(nullptr) {};
    Token getToken();
    void setState(LexerState& newState);
    int skipWhitespace();
    void openFile(std::string filename);
    void closeFile();
    void readBuffer(int size);
    char *ptr;

private:
    LexerState *currentState;
    FILE *fileHandle;
};


Token Lexer::getToken(){
    return this->currentState->getToken(this);
}

void Lexer::setState(LexerState& newState){
    this->currentState = &newState;
}

void Lexer::openFile(std::string filename){
    this->fileHandle = fopen(filename.c_str(), "r");
}

void Lexer::closeFile(){
    fclose(fileHandle);
}

void Lexer::readBuffer(int size){
    // error handling, check bytes read
    delete[] ptr;
    ptr = new char[size+1];
    fread(ptr, sizeof(char), size, fileHandle);
    ptr[size]='\0';
}

int Lexer::skipWhitespace(){
    while(*ptr == ' ' || *ptr == '\n' || *ptr == '\t' || *ptr == '\r'){
        ptr++;
    }
    if(*ptr == '\0') return -1;
    return 0;
}


Token DefaultState::getToken(Lexer *l){
    if(l->skipWhitespace() == -1) return Token(END_PARSING);
    // new tag
    if(*l->ptr == '<'){
        l->ptr++;
        if(*l->ptr == '/'){
            l->ptr++;
            l->setState(TagState::getInstance());
            return Token(CLOSE_TAG);
        } else {
            l->setState(TagState::getInstance());
            return Token(BEGIN_TAG);
        }

    } else {
        // text content
        std::string buffer;
        while(*l->ptr != '\0' && *l->ptr != '<'){
            /* if(*l->ptr == '&'){ */
            /*     std::string entity_r; */
            /*     while(isalpha(*l->ptr)){ */
            /*         entity_r += *l->ptr; */
            /*         l->ptr++; */
            /*     } */
            /* } */
            buffer += *l->ptr;
            l->ptr++;
        }
        return Token(CONTENT, buffer);
    }
}

LexerState& DefaultState::getInstance(){
    static DefaultState singleton;
    return singleton;
}


Token TagState::getToken(Lexer *l){
    if(l->skipWhitespace() == -1) return Token(END_PARSING);
    switch(*l->ptr){
        case '>': {
            l->setState(DefaultState::getInstance());
            l->ptr++;
            return Token(END_TAG);
        }
        case '/': {
            l->setState(DefaultState::getInstance());
            l->ptr+=2;
            return Token(END_AND_CLOSE_TAG);
        }
        case '"': {
            std::string buffer;
            l->ptr++;
            while(*l->ptr != '"'){
                buffer += *l->ptr;
                l->ptr++;
            }
            l->ptr++;
            return Token(ATTRIBUTE_VALUE, buffer);
        }
        default: {
            std::string buffer;
            while(*l->ptr != ' ' && *l->ptr != '=' && *l->ptr != '>'){
                buffer += *l->ptr;
                l->ptr++;
            }
            if(l->skipWhitespace() == -1) return Token(END_PARSING);
            switch(*l->ptr){
                case '=': {
                    while(*l->ptr != '"') l->ptr++;
                    return Token(ATTRIBUTE_NAME, buffer);
                }
                default: {
                    return Token(ELEMENT, buffer);
                }
            }
        }
    }
}

LexerState& TagState::getInstance(){
    static TagState singleton;
    return singleton;
}
