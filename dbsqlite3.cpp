
#include "dbdata.h"
#include "utilfunc.h"
#include <string>

DBResult *pRS; //needed for sqlite-callback

int DBsqlite3::Callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	if (!pRS) return 1;
	if (argc>0)
	{
		DBResult::row_t r;
		for(int i=0; i<argc; i++) pRS->AddColVal(azColName[i], ((argv[i])?argv[i]:"NULL"), r);
		pRS->AddRow(r);
	}
	return 0;
}

DBsqlite3::DBsqlite3() { sDB=""; bDBOK=false; pRS=NULL; sLastError=""; }

DBsqlite3::~DBsqlite3() { Close(); }

bool DBsqlite3::Open(const std::string sDBName)
{
	sDB = sDBName;
	bDBOK = (sqlite3_open(sDB.c_str(), &pDB)==0);
	if (!bDBOK) sLastError = "Cannot open database";
	return bDBOK;
}

void DBsqlite3::Close() { if (bDBOK) sqlite3_close(pDB); bDBOK=false; }
	
size_t DBsqlite3::ExecSQL(DBResult *pRSet, const std::string sSQL)
{
	sLastSQL=sSQL;
	if (!bDBOK) { sLastError = "Database not open"; return 0; }
	char *szErrMsg = 0;
	pRSet->sQuery = sSQL;
	sLastError.clear();
	pRS = pRSet;
	int rc = sqlite3_exec(pDB, sSQL.c_str(), &DBsqlite3::Callback, 0, &szErrMsg);
	if(rc!=SQLITE_OK) { sLastError = szErrMsg; sqlite3_free(szErrMsg); }
	return pRSet->GetRowCount();
}

bool DBsqlite3::MakeTable(const std::string sT, const std::string sF)
{
	DBResult RS;
	std::string sSQL;
	sSQL=spf("CREATE TABLE IF NOT EXISTS ", sT.c_str(), "(", sF.c_str(), ")");
	ExecSQL(&RS, sSQL);
	return (sLastError.empty());
}

const std::string DBsqlite3::GetLastError() { return sLastError; }

