/*
 * testfilesystem.cpp
 *
 *  Created on: 2015年11月9日
 *      Author: 吴永宇
 */
#include "parser.h"
#include"FileManager.h"
#include <string>
#include <iostream>
#include <fstream>
using namespace std;
void RunStmt(const string& sql, Parser& p, FileManager& manager) {
	try {
		auto X = p.parse(sql);
		X->Run(manager);
	}
	catch (const char* t) {
		cout << t << endl;
	}
}

int main(int argc, char** argv) {
	//remove("test.dbx");
	FileManager manager;
	Parser parser;
	
	if (argc == 2) {
		ifstream input(argv[1]);
		if (!input) {
			cout << "文件没有找到" << endl;
			return 0;
		}
		vector<char> sql;
		char c = '\0';
		while (input) {
			int x = input.get();
			if (x == -1)
				break;
			if (c == '\0' && x == ';') {
				RunStmt(string(sql.begin(), sql.end()), parser, manager);

				sql.clear();
			}
			else {
				if (c == '\0' && x == '\'')
					c = '\'';
				else if (c == '\0' && x == '"')
					c = '"';
				else if (c == '\'' && x == '\'')
					c = '\0';
				else if (c == '"' && x == '"')
					c = '\0';
				sql.push_back(x);
			}
		}
	}
	else {
		while (true) {
			string sql;
			cout << ">> ";
			getline(cin, sql);
			if (!cin)
				break;
			if (sql.substr(0, 4) == "exit")
			{
				//remove("test.dbx");
				break;
				
			}
				
			if (!sql.empty())
			{
				RunStmt(sql, parser, manager); 
			}
				
		}
	}
	//remove("test.dbx");
}