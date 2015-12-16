#ifndef FILE_MANAGER
#define FILE_MANAGER
#include <string>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <unordered_map>
#include <vector>
#include "FileTable.h"
#include "object.h"
#include"pagedef.h"
//#include "../MyLinkList.h"
#define MAX_FILE_NUM 128
using namespace std;
struct DBInfo {
	int tableCount;
	char tables[20][NAME_LEN];
};


class FileManager {
public :
	//**
	string dbName;
	unordered_map<string, FileTable*> tables;
	string tblFileName(const string& tbl) {
		return dbName + ":" + tbl + ".db";
	}
	void checkType(Type type, const Object& obj) {
		if (obj.is_null) {
			if (!type.null)
				throw "Type Check Error";
			return;
		}
		if (obj.type != type.type)
			throw "Type Check Error";
	}

	void WriteBinRow(void* buf, const TableDesc& desc, const vector<Object>& objs) {
		unsigned short nullMask = 1;
		unsigned short& nullX = *(unsigned short*)buf;
		nullX = 0;
		void* iter = (char*)buf + 2;
		for (int i = 0; i < desc.colSize; i++) {
			if (objs[i].is_null)
				nullX |= nullMask;
			else
				memcpy(iter, objs[i].loc, desc.colType[i].size);
			nullMask <<= 1;
			(char*&)iter += desc.colType[i].size;
		}
	}

	void WriteRow(void* rec, const TableDesc& desc) {
		unsigned short nullmap = *(unsigned short*)rec;
		(char*&)rec += 2;
		for (int i = 0; i < desc.colSize; i++) {
			const Type& t = desc.colType[i];
			if (nullmap & (1 << i)) {
				cout << "NULL ";
			}
			else {
				switch (t.type) {
				case TYPE_INT:
					cout << *(int*)rec << " ";
					break;
				case TYPE_VARCHAR:
					for (char* p = (char*)rec; *p && p != (char*)rec + t.size; p++)
						cout << *p;
					cout << " ";
					break;
				};
			}
			(char*&)rec += t.size;
		}
	}

	void WriteObj(Object obj) {
		if (obj.is_null) {
			cout << "NULL ";
		}
		else {
			switch (obj.type) {
			case TYPE_INT:
				cout << *(int*)obj.loc << " ";
				break;
			case TYPE_VARCHAR:
				for (char* p = (char*)obj.loc; *p && p != (char*)obj.loc + obj.size; p++)
					cout << *p;
				cout << " ";
				break;
			};
		}
	}
	FileTable* getTable(const string& tbl, bool init) {
		string t_tbl = tblFileName(tbl);
		if (init) {
			auto& ptbl = tables[t_tbl];
			if (ptbl == nullptr) {
				ptbl = new FileTable(t_tbl, true);
				return ptbl;
			}
			throw "Table Already Exist";
		}
		else {
			auto& ptbl = tables[t_tbl];
			if (ptbl == nullptr) {
				ptbl = new FileTable(t_tbl, false);
			}
			return ptbl;
		}
	}
	vector<void*> filterOne(const string& tbl, const vector<Condition>& conds) {
		vector<void*> filtered;
		FileTable* table = getTable(tbl, false);
		for (int k = 0; k < conds.size(); k++) {
			conds[k].l->Use(tbl, "", &table->head->desc);
			conds[k].r->Use(tbl, "", &table->head->desc);
		}
		for (auto record : table->usedRecords) {
			bool OK = true;
			for (int k = 0; k<conds.size(); k++) {
				Expr *l = conds[k].l;
				Expr *r = conds[k].r;
				if (!conds[k].op(l->getObj(record, nullptr), r->getObj(record, nullptr))) {
					OK = false;
					break;
				}
			}
			if (OK)
				filtered.push_back(record);
		}
		return move(filtered);
	}
	vector<pair<void*, void*> > filterTwo(const string& tbl1, const string& tbl2, const vector<Condition>& conds) {
		vector<pair<void*, void*>> filtered;
		FileTable* table1 = getTable(tbl1, false);
		FileTable* table2 = getTable(tbl2, false);
		for (int k = 0; k < conds.size(); k++) {
			conds[k].l->Use(tbl1, tbl2, &table1->head->desc, &table2->head->desc);
			conds[k].r->Use(tbl1, tbl2, &table1->head->desc, &table2->head->desc);
		}
		for (auto record1 : table1->usedRecords) {
			for (auto record2 : table2->usedRecords) {
				bool OK = true;
				for (int k = 0; k<conds.size(); k++) {
					Expr *l = conds[k].l;
					Expr *r = conds[k].r;
					if (!conds[k].op(l->getObj(record1, record2),
						r->getObj(record1, record2))) {
						OK = false;
						break;
					}
				}
				if (OK)
					filtered.push_back(make_pair(record1, record2));
			}
		}
		return move(filtered);
	}
	void Insert(const string& tbl, const vector<vector<Object>>& rows) {
		FileTable* tblX = getTable(tbl, false);


		for (auto row : rows) {
			if (row.size() != tblX->head->desc.colSize) {
				throw "Column Size Error;";
			}
			for (int i = 0; i < row.size(); i++) {
				checkType(tblX->head->desc.colType[i], row[i]);
			}
			//check primary and insert
			if (tblX->keyoffset != -1) {
				Object keyobj = row[tblX->keyoffset];
				if (auto iter = tblX->key_object.find(keyobj) != tblX->key_object.end())
					throw "primary key clustered";
				tblX->key_object.insert(keyobj);
			}
		}

		for (auto row : rows) {
			void* rec = (char*)tblX->genNewRecord();
			tblX->setDirty(rec);
			WriteBinRow(rec, tblX->head->desc, row);
		}
		tblX->writeback();
	}
	void Delete(const string& tbl, const vector<Condition>& conds) {
		FileTable* table = getTable(tbl, false);
		vector<void*> filtered = filterOne(tbl, conds);

		if (table->keyoffset != -1) {
			ReadExpr* lv = new ReadExpr(tbl, table->head->keyname);
			lv->Use(tbl, "", &table->head->desc);
			for (auto record : filtered) {
				Object keyobj = lv->getObj(record);
				auto iter = table->key_object.find(keyobj);
				if (iter != table->key_object.end())
					table->key_object.erase(iter);
			}
		}

		for (const auto& record : filtered) {
			table->removeRecord(record);
			WriteRow(record, table->head->desc);
			cout << endl;

		}
		table->writeback();
	}
	void Select(const string& tbl1, const string& tbl2, const vector<Condition>& conds, vector<Expr*>* sel = nullptr) {
		if (tbl2 == "") {

			auto filtered = filterOne(tbl1, conds);
			auto table = getTable(tbl1, false);
			if (sel != nullptr)
				for (auto& expr : *sel)
					expr->Use(tbl1,"", &table->head->desc, nullptr);
			for (auto row : filtered) {
				if (sel == nullptr) {

					WriteRow(row, table->head->desc);
				}
				else {
					cout << "obj is nullptr" << endl;
					for (auto& expr : *sel)
						WriteObj(expr->getObj(row, nullptr));
				}
				cout << endl;
			}
		}
		else {
			auto filtered = filterTwo(tbl1, tbl2, conds);
			auto table1 = getTable(tbl1, false);
			auto table2 = getTable(tbl2, false);
			if (sel != nullptr)
				for (auto& expr : *sel)
					expr->Use(tbl1, tbl2, &table1->head->desc, &table2->head->desc);
			for (auto row : filtered) {
				if (sel == nullptr) {
					WriteRow(row.first, table1->head->desc);
					WriteRow(row.second, table2->head->desc);
				}
				else {
					for (auto& expr : *sel)
						WriteObj(expr->getObj(row.first, row.second));
				}
				cout << endl;
			}
		}
	}
	void Update(const string& tbl, const vector<Condition>& conds, ReadExpr& lv, const Object& rv) {
		vector<void*> filtered = filterOne(tbl, conds);
		if (filtered.size() == 0)
		{
			return;
		}
		FileTable* table = getTable(tbl, false);
		lv.Use(tbl, "", &table->head->desc);

		if (lv.type != rv.type) {
			throw "Type Check Error";
		}
		//check whether the update is primary key : rv is primary key and is not ""
		// realize : if it exists in key_obj but not in filtered

		if (rv.is_null) {
			for (void* record : filtered) {
				*(unsigned short*)record |= lv.nullMask;
				table->setDirty(record);
			}
			table->writeback();
			return;
		}

		if (strcmp(lv.name.c_str(), table->head->keyname) == 0
			&& table->keyoffset != -1) {
			// construct set of filtered key objs;

			if (filtered.size() > 1)
				throw "primary key clustered";

			Object obj = lv.getObj(filtered[0]);

			auto iter = table->key_object.find(rv);
			if (iter != table->key_object.end() && rv != obj)
				throw "primary key clustered";

			table->key_object.erase(table->key_object.find(obj));
			table->key_object.insert(rv);
		}

		for (void* record : filtered) {
			*(unsigned short*)record &= ~lv.nullMask;
			Object obj = lv.getObj(record);
			memcpy(obj.loc, rv.loc, lv.size);
			table->setDirty(record);
		}
		table->writeback();
	}
	void CreateTable(const string& tbl, const vector<Type>& types, const string& keyname) {
		FileTable* table = getTable(tbl, true);
		table->head->desc.colSize = types.size();
		table->rowSize = FileTable::RowBitmapSize;
		strcpy(table->head->keyname, keyname.c_str());
		for (int i = 0; i<types.size(); i++) {
			table->head->desc.colType[i] = types[i];
			table->rowSize += types[i].size;
		}
		table->setDirty(0);
		table->writeback();
		table->initKey();
		fstream dbf(dbName + ".dbx", ios::in | ios::out | ios::binary);
		DBInfo info;
		dbf.read((char*)(void*)&info, sizeof(info));
		strcpy(info.tables[info.tableCount++], tbl.c_str());
		dbf.seekp(0);
		dbf.write((char*)(void*)&info, sizeof(info));
	}
	void DropTable(const string& tbl) {
		tables.erase(tblFileName(tbl));
		remove(tblFileName(tbl).c_str());
		fstream dbf(dbName + ".dbx", ios::in | ios::out | ios::binary);
		DBInfo info;
		dbf.read((char*)(void*)&info, sizeof(info));
		for (int i = 0; i < info.tableCount; i++)
			if (info.tables[i] == tbl)
				memcpy(info.tables[i], info.tables[--info.tableCount], 20);
		dbf.seekp(0);
		dbf.write((char*)(void*)&info, sizeof(info));
	}
	void Use(const string& db) {
		dbName = db;
	}
	void CreateDB(const string& db) {
		fstream out(dbName + ".dbx", ios::out | ios::binary | ios::trunc);
		DBInfo info;
		info.tableCount = 0;
		out.write((char*)(void*)&info, sizeof(info));
		out.close();
	}
	void DropDB(const string& db) {
		remove((dbName + ".dbx").c_str());
	}
	void Desc(const string& tbl) {
		FileTable* table = getTable(tbl, false);
		for (int i = 0; i < table->head->desc.colSize; i++) {
			cout << table->head->desc.colType[i].name;
			if (table->head->desc.colType[i].type == TYPE_INT)
				cout << " INT";
			else
				cout << " VARCHAR";
			if (table->head->desc.colType[i].null)
				cout << " NULL";
			else
				cout << " NOT NULL";
			cout << endl;
		}
	}
	void ShowTables() {
		fstream dbf(dbName + ".dbx", ios::in | ios::out | ios::binary);
		DBInfo info;
		dbf.read((char*)(void*)&info, sizeof(info));
		for (int i = 0; i < info.tableCount; i++)
			cout << info.tables[i] << endl;
	}
	//**

private:
	//FileTable* ftable;
	int fd[MAX_FILE_NUM];
	MyBitMap* fm;
	MyBitMap* tm;
	int _createFile(const char* name) {
		FILE* f = fopen(name, "a+");
		if (f == NULL) {
			cout << "fail" << endl;
			return -1;
		}
		fclose(f);
		return 0;
	}
	int _openFile(const char* name, int fileID) {
		int f = open(name, O_RDWR);
		if (f == -1) {
			return -1;
		}
		fd[fileID] = f;
		return 0;
	}
public:
	/*
	 * FilManager构造函数
	 */
	FileManager() {
		fm = new MyBitMap(MAX_FILE_NUM, 1);
		tm = new MyBitMap(MAX_TYPE_NUM, 1);
		dbName = "test";
		DBInfo info;
		info.tableCount = 0;
		fstream out("test.dbx", ios::in | ios::binary);
		if (!out) {
			out.close();
			out.open("test.dbx", ios::out | ios::binary | ios::trunc);
			out.write((char*)(void*)&info, sizeof(info));
		}
		out.close();
	}
	/*
	 * @函数名writePage
	 * @参数fileID:文件id，用于区别已经打开的文件
	 * @参数pageID:文件的页号
	 * @参数buf:存储信息的缓存(4字节无符号整数数组)
	 * @参数off:偏移量
	 * 功能:将buf+off开始的2048个四字节整数(8kb信息)写入fileID和pageID指定的文件页中
	 * 返回:成功操作返回0
	 */
	int writePage(int fileID, int pageID, BufType buf, int off) {
		int f = fd[fileID];
		off_t offset = pageID;
		offset = (offset << PAGE_SIZE_IDX);
		off_t error = lseek(f, offset, SEEK_SET);
		if (error != offset) {
			return -1;
		}
		BufType b = buf + off;
		error = write(f, (void*) b, PAGE_SIZE);
		return 0;
	}
	/*
	 * @函数名readPage
	 * @参数fileID:文件id，用于区别已经打开的文件
	 * @参数pageID:文件页号
	 * @参数buf:存储信息的缓存(4字节无符号整数数组)
	 * @参数off:偏移量
	 * 功能:将fileID和pageID指定的文件页中2048个四字节整数(8kb)读入到buf+off开始的内存中
	 * 返回:成功操作返回0
	 */
	int readPage(int fileID, int pageID, BufType buf, int off) {
		//int f = fd[fID[type]];
		int f = fd[fileID];
		off_t offset = pageID;
		offset = (offset << PAGE_SIZE_IDX);
		off_t error = lseek(f, offset, SEEK_SET);
		if (error != offset) {
			return -1;
		}
		BufType b = buf + off;
		error = read(f, (void*) b, PAGE_SIZE);
		return 0;
	}
	/*
	 * @函数名closeFile
	 * @参数fileID:用于区别已经打开的文件
	 * 功能:关闭文件
	 * 返回:操作成功，返回0
	 */
	int closeFile(int fileID) {
		fm->setBit(fileID, 1);
		int f = fd[fileID];
		close(f);
		return 0;
	}
	/*
	 * @函数名createFile
	 * @参数name:文件名
	 * 功能:新建name指定的文件名
	 * 返回:操作成功，返回true
	 */
	bool createFile(const char* name) {
		_createFile(name);
		return true;
	}
	/*
	 * @函数名openFile
	 * @参数name:文件名
	 * @参数fileID:函数返回时，如果成功打开文件，那么为该文件分配一个id，记录在fileID中
	 * 功能:打开文件
	 * 返回:如果成功打开，在fileID中存储为该文件分配的id，返回true，否则返回false
	 */
	bool openFile(const char* name, int& fileID) {
		fileID = fm->findLeftOne();
		fm->setBit(fileID, 0);
		_openFile(name, fileID);
		return true;
	}
	int newType() {
		int t = tm->findLeftOne();
		tm->setBit(t, 0);
		return t;
	}
	void closeType(int typeID) {
		tm->setBit(typeID, 1);
	}
	void shutdown() {
		delete tm;
		delete fm;
	}
	~FileManager() {
		this->shutdown();
	}
};
//bool isCorrect(Object& obj1, Object& obj2, Oper op);
#endif
