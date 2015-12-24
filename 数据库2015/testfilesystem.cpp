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
vector <string>split(const string &x, const char sp)
{
	vector<string>ans;
	bool show = true;
	int last = 0;
	for (int i = 0; i < x.size(); i++)
	{
		if (x[i] == sp&&show)
		{
			if (i - last > 0)
			{

				ans.push_back(string(x, last, i - last));
			}
			last = i + 1;
		}

	}
	if (last != x.size())
	{
		ans.push_back(string(x, last, x.size() - last));
	}
	return ans;
}
string  ToLower(string sql)
{
	string ret;
	ret.resize(sql.size());
	for (int i = 0; i < sql.size(); i++) {
		char ch = sql[i];
		if (ch >= 'A' && ch <= 'Z')
			ch ^= 0x20;
		ret[i] = ch;
	}
	return ret;
}
void RunStmt(const string& sql, Parser& p, FileManager& manager) {
	try {
		string ret=ToLower(sql);		
		vector<string> list = split(ret,' ');
		string newSql = "";
		if (list.at(0) == "create"&&list.at(1)=="table")
		{
			
			for (int i = 0; i < list.size(); i++)
			{
				if (list.at(i) == "date"&&list.at(i+1)!="date")
				{
					newSql += " varchar(20) ";
				}
				else if (list.at(i) == "date,")
				{
					newSql += " varchar(20), ";
				}
				else
				{
					newSql += " "+list.at(i)+" ";
				}
			}
		}
		else
		{
			newSql = sql;
		}
		
		auto X = p.parse(newSql);
		X->Run(manager);
	}
	catch (const char* t) {
		cout << t << endl;
	}
}

int main(int argc, char** argv) {
	FileManager manager;
	Parser parser;
	
	if (argc == 2) {
		ifstream input(argv[1]);
		if (!input) {
			cout << "文件没有找到" << endl;
			return 0;
		}
		cout << "FileName : " << argv[1]  << endl;
		vector<char> sql;
		char c = '\0';
		while (input) {
			int x = input.get();
			if (x == -1)
				break;
			if (c == '\0' && x == ';') {
				//cout << "Ins :"<<string(sql.begin(), sql.end()) << endl;
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
				break;
				
			}
				
			if (!sql.empty())
			{
				RunStmt(sql, parser, manager); 
			}
				
		}
	}
}