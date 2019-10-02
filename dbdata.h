#ifndef _DBDATA_H_
#define _DBDATA_H_


#include <string>
#include <vector>

#include <sqlite3.h>

//--------------------------------------------------------------------------------------------------
class DBsqlite3;
class DBResult
{
	struct ColVal
	{
		ColVal(const std::string sc,const std::string sv):col(sc),val(sv) {}
		std::string col, val;
	};

public:
	typedef std::vector<ColVal> row_t;

private:
	std::vector<row_t> Rows;
	std::string sQuery;

	void AddColVal(const std::string colname, const std::string value, row_t &r);
	void AddRow(row_t &r);

public:
	
	DBResult();
	~DBResult();
	void Clear();
	const std::string GetQuery();
	int GetColCount();
	int GetRowCount();
	const std::string GetName(int nCol, int nRow);
	const std::string GetVal(int nCol, int nRow);
	const std::string GetVal(const std::string sCol, int nRow);
friend class DBsqlite3;
};

//--------------------------------------------------------------------------------------------------
struct DBsqlite3
{
	sqlite3 *pDB;
	std::string sDB, sLastError, sLastSQL;
	
	static int Callback(void *NotUsed, int argc, char **argv, char **azColName);

	bool bDBOK;

	DBsqlite3();
	~DBsqlite3();
	
	bool Open(const std::string sDBName);
	void Close();
	size_t ExecSQL(DBResult *pRSet, const std::string sSQL);
	bool MakeTable(const std::string sT, const std::string sF);
	const std::string GetLastError();
};


//--------------------------------------------------------------------------------------------------
struct WTConfig;
struct WTProject;
struct WTProjectList;
struct WTTodo;
struct WTTodoList;
struct Dependencies;
struct Locations;

struct DBData : public DBsqlite3
{
	virtual ~DBData() {}
	
	const std::string SQLSafeQ(const std::string &sval); //..Q=>returned string wrapped in single-quotes
	const std::string SQLRestore(const std::string &sval);
	bool NoError();

	const char IDS_DELIM=(const char)',';
	bool init_ids(const std::string &tname);
	uint64_t new_id(const std::string &tablename);
	bool del_id(const std::string &tablename, uint64_t id);

	bool ImplementSchema();

	bool Load(WTConfig *pwtc);
	bool Load(WTProjectList &PL);
	bool load_deps(Dependencies &Deps, size_t idP);
	bool load_locs(Locations &Locs, size_t idP);
	bool Load(WTTodoList &TL, size_t idP);
	
	bool Save(WTConfig *pwtc);
	bool Save(WTProject &P);
	bool save_deps(Dependencies &Deps, size_t idP);
	bool save_locs(Locations &Locs, size_t idP);
	bool Save(WTTodo &T);
	
	bool Delete(WTProject &P);
	bool remove_dep(size_t iddep);
	bool Delete(WTTodo &T, bool bInTrans=false);
};





#endif //_DBDATA_H_

