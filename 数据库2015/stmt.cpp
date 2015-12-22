/*
* stmt.cpp
*
*  Created on: 2015Äê10ÔÂ28ÈÕ
*      Author: ÎâÓÀÓî
*/
#include <iostream>
#include <cstdlib>

#include "stmt.h"

void InsertStmt::Run(FileManager& manager) {
    manager.Insert(tbl, rows);
}

void DeleteStmt::Run(FileManager& manager) {
    manager.Delete(tbl, conds);
}

void SelectStmt::Run(FileManager& manager) {
    manager.Select(tbl1, tbl2, conds, exprs);
}

void UpdateStmt::Run(FileManager& manager) {
    manager.Update(tbl, conds, lv, obj);
}

void CreateTableStmt::Run(FileManager& manager) {
    manager.CreateTable(tbl, types, key);
}

void DropTableStmt::Run(FileManager& manager) {
    manager.DropTable(tbl);
}

void UseStmt::Run(FileManager& manager) {
    manager.Use(db);
}

void ShowTblStmt::Run(FileManager& manager) {
    manager.ShowTables();
}

void DescStmt::Run(FileManager& manager) {
    manager.Desc(tbl);
}

void CreateDBStmt::Run(FileManager& manager) {
    manager.CreateDB(db);
}


void DropDBStmt::Run(FileManager& manager) {
    manager.DropDB(db);
}
