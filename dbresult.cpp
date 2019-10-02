
#include "dbdata.h"
#include "utilfunc.h"

#include <string>


//=============================================================================
DBResult::DBResult() { Clear(); }
DBResult::~DBResult() { Clear(); }

void DBResult::Clear()	{ Rows.clear(); sQuery.clear(); }
void DBResult::AddColVal(const std::string scol, const std::string sval, row_t &r) { r.push_back(ColVal(scol,sval)); }
void DBResult::AddRow(row_t &r) { Rows.push_back(r); }

const std::string DBResult::GetQuery() { return sQuery; }

int DBResult::GetColCount() { return ((Rows.size()>0)?Rows.at(0).size():0); }
int DBResult::GetRowCount() { return Rows.size(); }

const std::string DBResult::GetName(int nCol, int nRow)
{
	return (((nRow<(int)Rows.size())&&(nCol<GetColCount()))?Rows.at(nRow).at(nCol).col:"");
}

const std::string DBResult::GetVal(int nCol, int nRow)
{
	return (((nRow<(int)Rows.size())&&(nCol<GetColCount()))?Rows.at(nRow).at(nCol).val:"");
}

const std::string DBResult::GetVal(const std::string sCol, int nRow)
{
	if (nRow<(int)Rows.size())
	{
		auto it = Rows.at(nRow).begin();
		while (it != Rows.at(nRow).end()) { if (sieqs((*it).col, sCol)) return ((*it).val); it++; }
	}
	return "";
}
