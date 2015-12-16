#ifndef Stmt_H
#define Stmt_H
#include "FileManager.h"

struct Stmt {
    virtual void Run(FileManager& manager) = 0;
    virtual ~Stmt() {}
};
struct InsertStmt : public Stmt {
    string tbl;
    vector<vector<Object>> rows;
    virtual void Run(FileManager& manager);
};
struct DeleteStmt : public Stmt {
    string tbl;
    vector<Condition> conds;
    virtual void Run(FileManager& manager);
};
struct SelectStmt : public Stmt {
    string tbl1, tbl2;
    vector<Condition> conds;
    vector<Expr*>* exprs;
    virtual void Run(FileManager& manager);
};
struct UpdateStmt : public Stmt {
    string tbl;
    vector<Condition> conds;
    ReadExpr lv;
    Object obj;
    UpdateStmt(){};
    virtual void Run(FileManager& manager);
};
struct CreateTableStmt : public Stmt {
    string tbl;
    string key;
    vector<Type> types;
    ~CreateTableStmt() {};
    virtual void Run(FileManager& manager);
};
struct DropTableStmt : public Stmt {
    string tbl;
    virtual void Run(FileManager& manager);
};
struct UseStmt : public Stmt {
    string db;
    virtual void Run(FileManager& manager);
};
struct CreateDBStmt : public Stmt {
    string db;
    virtual void Run(FileManager& manager);
};
struct DropDBStmt : public Stmt {
    string db;
    virtual void Run(FileManager& manager);
};

struct ShowTblStmt : public Stmt {
    virtual void Run(FileManager& manager);
};
struct DescStmt : public Stmt {
    string tbl;
    virtual void Run(FileManager& manager);
};

#endif

