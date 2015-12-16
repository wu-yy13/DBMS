#ifndef FILE_TABLE
#define FILE_TABLE
#include <string>
#include "MyBitMap.h"
#include <map>
#include <vector>
#include <set>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include<cstdlib>
#include"type.h"
#include"baseobj.h"
#include"pagedef.h"
using namespace std;
const int NAME_LEN = 50;

struct Type {
	TYPE type;
	bool null;
	int size;
	char name[NAME_LEN];
	Type() {};
	Type(TYPE t, bool n, int s, char* name_) {
		type = t;
		null = n;
		size = s;
		memcpy(name, name_, NAME_LEN);
	}
};
/**
* 每页的页号 page_id
* 偏移量 offset
* 页是否空闲
*/
struct Info {
	int page_id;
	int offset;
	bool free;
};
const int MaxCol = (PAGE_SIZE - 128) / sizeof(Type);
const int MaxInfo = (PAGE_SIZE - 16) / sizeof(Info);
struct TableDesc {   //描述表的大小和每行存储的记录
	int colSize;
	Type colType[MaxCol];
};
struct HeadPage {  //页式存储 infoHeadPage,pageCount keyname[NAME_LEN] TableDesc desc
	int infoHeadPage;
	int pageCount;
	char keyname[NAME_LEN];
	TableDesc desc;
};

/**
* 下一页nextPage
* 页面大小 size
* 存放页info
*/
struct InfoPage {
	int nextPage;
	int size;
	Info infos[MaxInfo];
};
class FileTable {
public:

	//**
	int keyoffset;
	static const int RowBitmapSize = 2;
	string s;
	string filename;
	unordered_map<void*, Info*> recordInfoMap;
	unordered_set<void*> usedRecords, emptyRecords;
	set<Object> key_object;
	map<void*, int> pageIndex;
	vector<void*> pages;
	set<int> dirtyPages;
	int rowSize;
	HeadPage* head;
	InfoPage* LastInfoPage;
	

	//**************************************************
private:
	multiset<string> isExist;
	multiset<string> isOpen;
	vector<string> fname;
	vector<string> format;
	map<string, int> nameToID;
	string* idToName;
	MyBitMap* ft, *ff;
	int n;
	void load() {
		ifstream fin("filenames");
		fin >> n;
		for (int i = 0; i < n; ++ i) {
			string s, a;
			fin >> s;
			isExist.insert(s);
			fname.push_back(s);
			fin >> a;
			format.push_back(a);
		}
		fin.close();
	}
	void save() {
		ofstream fout("filenames");
		fout << fname.size() << endl;
		for (uint i = 0; i < fname.size(); ++ i) {
			fout << fname[i] << endl;
			fout << format[i] << endl;
		}
		fout.close();
	}
public:
	FileTable(const string& _filename, bool init = false) {
		ifstream in(filename, ios::in | ios::binary);
		if (init) {
			if (in)
			{
				throw "Table Already Exist!";
				cout << "文件名字：" << _filename << endl;

			}
			in.close();
			fstream out(filename, ios::out | ios::binary);
			head = (HeadPage*)new char[PAGE_SIZE];
			LastInfoPage = (InfoPage*)new char[PAGE_SIZE];
			memset(head, 0, PAGE_SIZE);
			memset(LastInfoPage, 0, PAGE_SIZE);
			head->infoHeadPage = 1;
			head->pageCount = 2;
			out.write((char*)(void*)head, PAGE_SIZE);
			out.seekp(PAGE_SIZE);
			out.write((char*)(void*)LastInfoPage, PAGE_SIZE);
			out.close();
			rowSize = 0;
			pages.push_back(head);
			pages.push_back(LastInfoPage);
			pageIndex[head] = 0;
			pageIndex[LastInfoPage] = 1;
		}
		else {
			if (!in)
				throw "Table Not Found!"+_filename;
			head = (HeadPage*)new char[PAGE_SIZE];
			in.read((char*)(void*)head, PAGE_SIZE);
			pages.push_back(head);
			pageIndex[head] = 0;
			//Read Pages
			for (int i = 1; i < head->pageCount; i++) {
				char* buf = new char[PAGE_SIZE];
				in.read(buf, PAGE_SIZE);
				pages.push_back(buf);
				pageIndex[buf] = i;
			}

			// get key object;
			int offset = 2;
			initKey();

			for (int i = 0; i<keyoffset; i++)
				offset += head->desc.colType[i].size;
			int nullMask = 1 << keyoffset;
			TYPE type = head->desc.colType[keyoffset].type;
			int size = head->desc.colType[keyoffset].type;


			int infoIdx = head->infoHeadPage;
			while (infoIdx != 0) {
				InfoPage* infos = (InfoPage*)(void*)pages[infoIdx];
				infoIdx = infos->nextPage;
				for (int i = 0; i < infos->size; i++) {
					Info* info = infos->infos + i;
					void* rec = (char*)pages[info->page_id] + info->offset;
					recordInfoMap[rec] = info;
					if (info->free)
						emptyRecords.insert(rec);
					else {
						Object ret;
						ret.size = size;
						ret.is_null = nullMask & *(unsigned short*)rec;
						ret.loc = (char*)rec + offset;
						ret.type = type;
						key_object.insert(ret);
						usedRecords.insert(rec);
					}
				}
				LastInfoPage = infos;
			}
			rowSize = FileTable::RowBitmapSize;
			for (int i = 0; i < head->desc.colSize; i++)
				rowSize += head->desc.colType[i].size;
		}

	}
	void setDirty(void* dst) {
		auto it = pageIndex.upper_bound(dst);
		it--;
		dirtyPages.insert(it->second);
	}
	void setDirty(int page_id)
	{
		dirtyPages.insert(page_id);
	}
	void writeback() {
		ofstream out(filename, ios::binary | ios::out | ios::in);
		for (auto page_id : dirtyPages) {
			out.seekp(page_id * PAGE_SIZE);
			out.write((char*)pages[page_id], PAGE_SIZE);
		}
		out.close();
	}
	void* getPage(int page_id) {
		return pages[page_id];
	}
	void* genNewRecord() {
		if (emptyRecords.empty()) {
			int new_page = newPage();
			char* page = (char*)getPage(new_page);
			for (int i = 0; i + rowSize <= PAGE_SIZE; i += rowSize) {
				if (LastInfoPage->size == MaxInfo) {
					setDirty(LastInfoPage);
					int new_info_page = newPage();
					LastInfoPage->nextPage = new_info_page;
					LastInfoPage = (InfoPage*)getPage(new_info_page);
				}
				Info* info = LastInfoPage->infos + LastInfoPage->size++;
				info->page_id = new_page;
				info->offset = i;
				info->free = true;
				void* rec = page + info->offset;
				emptyRecords.insert(rec);
				recordInfoMap[rec] = info;
			}
			setDirty(LastInfoPage);
		}
		Info* info = recordInfoMap[*emptyRecords.begin()];
		void* rec = (char*)getPage(info->page_id) + info->offset;
		info->free = false;
		emptyRecords.erase(rec);
		usedRecords.insert(rec);
		setDirty(info);
		return rec;
	}
	void removeRecord(void* rec) {
		Info* info = recordInfoMap[rec];
		info->free = true;
		emptyRecords.insert(rec);
		usedRecords.erase(rec);

	}
	void initKey() {
		if (strcmp(head->keyname, "") == 0) {
			keyoffset = -1;
			return;
		}
		keyoffset = 0;
		for (keyoffset = 0; keyoffset<head->desc.colSize; keyoffset++) {
			if (strcmp(head->desc.colType[keyoffset].name, head->keyname) == 0)
				break;
		}

	}
	int newPage() {
		char* buf = new char[PAGE_SIZE];
		memset(buf, 0, PAGE_SIZE);
		pageIndex[buf] = head->pageCount;
		pages.push_back(buf);
		head->pageCount++;
		setDirty(0);
		setDirty(head->pageCount - 1);
		return head->pageCount - 1;
	}
	//**********************
	int newTypeID() {
		int k = ft->findLeftOne();
		ft->setBit(k, 0);
		return k;
	}
	int newFileID(const string& name) {
		int k = ff->findLeftOne();
		ff->setBit(k, 0);
		nameToID[name] = k;
		isOpen.insert(name);
		idToName[k] = name;
		return k;
	}
	bool ifexist(const string& name) {
		return (isExist.find(name) != isExist.end());
	}
	void addFile(const string& name, const string& fm) {
		isExist.insert(name);
		fname.push_back(name);
		format.push_back(fm);
	}
	int getFileID(const string& name) {
		if (isOpen.find(name) == isOpen.end()) {
			return -1;
		}
		return nameToID[name];
	}
	void freeTypeID(int typeID) {
		ft->setBit(typeID, 1);
	}
	void freeFileID(int fileID) {
		ff->setBit(fileID, 1);
		string name = idToName[fileID];
		isOpen.erase(name);
	}
	string getFormat(string name) {
		for (uint i = 0; i < fname.size(); ++ i) {
			if (name == fname[i]) {
				return format[i];
			}
		}
		return "-";
	}
	FileTable(int fn, int tn) {
		load();
		ft = new MyBitMap(tn, 1);
		ff = new MyBitMap(fn, 1);
		idToName = new string[fn];
		isOpen.clear();
	}
	~FileTable() {
		save();
	}
};
#endif
