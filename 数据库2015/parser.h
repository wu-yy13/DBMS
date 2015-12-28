/*
* parser.h
*
*  Created on: 2015ƒÍ10‘¬28»’
*      Author: Œ‚”¿”Ó
*/
#ifndef PARSER_H
#define PARSER_H
#include "stmt.h"
struct Token {
    enum Type {
        USE,
        DROP,
        DATABASE,
        TABLE,
        SET,
        CREATE,
        SHOW,
        DESC,
        DELETE,
        UPDATE,
        INSERT,
        VARCHAR_LIT,
        INT_LIT,
        OPER,
        ID,
        SELECT,
        WHERE,
        FROM,
        AND,
        INTO,
        VALUES,
        PRIMARY,
        INT,
        VARCHAR,
		SUM,
		AVG,
		MAX,
		MIN,
        NULL_LIT,
    } token;
    string raw;
};
struct Parser {
    LiteralManager litManager;
    typedef vector<Token> TokenList;
    typedef TokenList::iterator TokenIter;
    void clear();
    Stmt* parse(const string& sql);
    TokenList tokenize(const string& sql);
    Stmt* parseSQL(TokenIter beg, TokenIter end);
    SelectStmt* parseSelect(TokenIter beg, TokenIter end);
    DeleteStmt* parseDelete(TokenIter beg, TokenIter end);
    UpdateStmt* parseUpdate(TokenIter beg, TokenIter end);
    InsertStmt* parseInsert(TokenIter beg, TokenIter end);
    ShowTblStmt* parseShow(TokenIter beg, TokenIter end);
    DescStmt* parseDesc(TokenIter beg, TokenIter end);
    CreateTableStmt* parseCreateTable(TokenIter beg, TokenIter end);
    DropTableStmt* parseDropTable(TokenIter beg, TokenIter end);
    CreateDBStmt* parseCreateDB(TokenIter beg, TokenIter end);
    DropDBStmt* parseDropDB(TokenIter beg, TokenIter end);
    UseStmt* parseUse(TokenIter beg, TokenIter end);
    pair<string, string> parseFrom(TokenIter beg, TokenIter end);
    vector<Condition> parseWhere(TokenIter beg, TokenIter end);
    string parseTableName(TokenIter beg, TokenIter end);
    pair<ReadExpr*, Object> parseSet(TokenIter beg, TokenIter end);
    Condition parseCond(TokenIter beg, TokenIter end);
    vector<Expr*>* parseExprs(TokenIter beg, TokenIter end);
    Expr* parseExpr(TokenIter beg, TokenIter end);
    vector<vector<Object>> parseRows(TokenIter beg, TokenIter end);
    vector<Object> parseRow(TokenIter beg, TokenIter end);
    pair<vector<Type>, string> parseTypes(TokenIter beg, TokenIter end);
    Type parseType(TokenIter beg, TokenIter end);
    TokenIter findToken(TokenIter beg, TokenIter end, Token::Type token, const string& raw = "");
};
#endif
