
#include "dbdata.h"
#include <string>
#include "utilfunc.h"
#include "wtconfig.h"
#include "wtproject.h"


//--------------------------------------------------------------------------------------------------
const std::string DBData::SQLSafeQ(const std::string &sval)
{
	std::string t=sval, r;
	if (!t.empty())
	{
		ReplaceChars(t, "\"", "\"\"");
		ReplaceChars(t, "'", "''");
	}
	r="'"; r+=t; r+="'";
	return r;
}

const std::string DBData::SQLRestore(const std::string &sval)
{
	std::string r=sval;
	if (!r.empty())
	{
		ReplacePhrase(r, "\"\"", "\"");
		ReplacePhrase(r, "''", "'");
	}
	return r;
}

//--------------------------------------------------------------------------------------------------
bool DBData::NoError() { return (GetLastError().empty()); }

//--------------------------------------------------------------------------------------------------IDS-routines
bool DBData::init_ids(const std::string &tname)
{
	DBResult RS;
	std::string sSQL;

	sSQL=spf("SELECT * FROM IDS WHERE name = ", SQLSafeQ(tname));
	if (ExecSQL(&RS, sSQL)>0) return true; //exists
	sSQL=spf("INSERT INTO IDS (name, id, del) VALUES(", SQLSafeQ(tname), ", 0, '')");
	ExecSQL(&RS, sSQL);
	return NoError();
}

uint64_t DBData::new_id(const std::string &tname)
{
	std::string sSQL, s;
	//size_t n;
	DBResult RS;
	uint64_t id=0;
	
	sSQL=spf("SELECT id, del FROM IDS WHERE name = ", SQLSafeQ(tname));
	//n=
	ExecSQL(&RS, sSQL);
	if (NoError())
	{
		std::vector<uint64_t> v;
		s=SQLRestore(RS.GetVal("del", 0));
		if (desv<uint64_t>(s, IDS_DELIM, v, false)>0)
		{
			id=v[0];
			v.erase(v.begin());
			ensv<uint64_t>(v, IDS_DELIM, s, false);
			sSQL=spf("UPDATE IDS SET del = ", SQLSafeQ(s), " WHERE name = ", SQLSafeQ(tname));
			ExecSQL(&RS, sSQL);
			if (!NoError()) id=0;
		}
		else
		{
			id=stot<uint64_t>(RS.GetVal("id",0))+1;
			sSQL=spf("UPDATE IDS SET id = ", id, " WHERE name = ", SQLSafeQ(tname));
			ExecSQL(&RS, sSQL);
			if (!NoError()) id=0;
		}
	}
	return id;
}

bool DBData::del_id(const std::string &tname, uint64_t id)
{
	std::string sSQL, s;
	DBResult RS;
	
	sSQL=spf("SELECT id, del FROM IDS WHERE name = ", SQLSafeQ(tname));
	ExecSQL(&RS, sSQL);
	if (NoError())
	{
		std::vector<uint64_t> v;
		s=SQLRestore(RS.GetVal("del", 0));
		desv<uint64_t>(s, IDS_DELIM, v, false);
		v.push_back(id);
		ensv<uint64_t>(v, IDS_DELIM, s, false);
		sSQL=spf("UPDATE IDS SET del = ", SQLSafeQ(s), " WHERE name = ", SQLSafeQ(tname));
		ExecSQL(&RS, sSQL);
	}
	return NoError();
}

//--------------------------------------------------------------------------------------------------
bool DBData::ImplementSchema()
{
	bool b=bDBOK;
	DBResult RS;
	std::string sSQL;

	if (b)
	{
		sSQL = "CREATE TABLE IF NOT EXISTS IDS (name, id, del)";
		ExecSQL(&RS, sSQL); //create/reuse ids: init_ids(tabelname) once after CREATE TABLE.. and new_id() / del_id() after
		b=NoError();
	}
	if (b&&(b=MakeTable("wtconfig","lock, x, y, w, h, periods, alert")))
	{
		sSQL = "INSERT INTO wtconfig(lock,x,y,w,h,periods,alert) VALUES(0,100,100,800,500,591873,0)";
		ExecSQL(&RS, sSQL); //default config: 591873==0x00090801 (normal workday: 09h00 for 8 x 1 hours)
		b = NoError();
	}
	if (b) b = (MakeTable("projects", "idproject, name, state, purpose, priority")&&init_ids("projects"));
	if (b) b = MakeTable("depends", "idproject, iddep, reason"); //utility(concurrent)/prerequisite(wait)
	if (b) b = MakeTable("locations", "idproject, description, location");
	if (b) b = (MakeTable("todos", "idtodo, idproject, what, info, state")&&init_ids("todos"));

	return b;
}

//--------------------------------------------------------------------------------------------------
bool DBData::Load(WTConfig *pwtc)
{
	DBResult RS;
	int n;

	n = ExecSQL(&RS, "SELECT * FROM wtconfig");
	if (NoError())
	{
		if (n>0)
		{
			pwtc->blocked = (stot<int>(RS.GetVal("lock", 0))==1);
			pwtc->X = stot<int>(RS.GetVal("x", 0));
			pwtc->Y= stot<int>(RS.GetVal("y", 0));
			pwtc->W = stot<int>(RS.GetVal("w", 0));
			pwtc->H = stot<int>(RS.GetVal("h", 0));
			pwtc->periods = stot<int>(RS.GetVal("periods", 0));
			pwtc->bAlert = (stot<int>(RS.GetVal("alert", 0))==1);
		}
		return true;
	}
	return false;
}


bool DBData::Load(WTProjectList &PL)
{
	DBResult RS;
	size_t i=0,n;

	PL.clear();

	n = ExecSQL(&RS, "SELECT * FROM projects");
	if (NoError())
	{
		while (i<n)
		{
			WTProject P;
			P.id = stot<size_t>(RS.GetVal("idproject", i));
			P.Name = SQLRestore(RS.GetVal("name", i));
			P.nState = stot<int>(RS.GetVal("state", i));
			P.Purpose = SQLRestore(RS.GetVal("purpose", i));
			P.Priority = stot<int>(RS.GetVal("priority", i));
			Load(P.TL, P.id);
			load_deps(P.deps, P.id);
			load_locs(P.locs, P.id);
			PL[P.id]=P;
			i++;
		}
		return true;
	}
	return false;
}

bool DBData::load_deps(Dependencies &Deps, size_t idP)
{
	size_t i,n;
	std::string sSQL;
	DBResult RS;
	
	Deps.clear();
	
	sSQL=spf("SELECT * FROM depends WHERE idproject = ", idP);
	n = ExecSQL(&RS, sSQL);
	if (NoError())
	{
		i=0;
		while (i<n)
		{
			Dependency D;
			D.idP = idP;
			D.idDep = stot<size_t>(RS.GetVal("iddep", i));
			D.reason = SQLRestore(RS.GetVal("reason", i));
			Deps.push_back(D);
			i++;
		}
		return true;
	}
	return false;
}

bool DBData::load_locs(Locations &Locs, size_t idP)
{
	size_t i,n;
	std::string sSQL;
	DBResult RS;
	
	Locs.clear();
	
	sSQL=spf("SELECT * FROM locations WHERE idproject = ", idP);
	n = ExecSQL(&RS, sSQL);
	if (NoError())
	{
		i=0;
		while (i<n)
		{
			Location L;
			L.idP = idP;
			L.description = SQLRestore(RS.GetVal("description", i));
			L.location = SQLRestore(RS.GetVal("location", i));
			Locs.push_back(L);
			i++;
		}
		return true;
	}
	return false;
}

bool DBData::Load(WTTodoList &TL, size_t idP)
{
	size_t i,n;
	std::string sSQL;
	DBResult RS;
	
	TL.clear();
	
	sSQL=spf("SELECT * FROM todos WHERE idproject = ", idP);
	n = ExecSQL(&RS, sSQL);
	if (NoError())
	{
		i=0;
		while (i<n)
		{
			WTTodo T;
			T.idP = idP;
			T.id = stot<size_t>(RS.GetVal("idtodo", i));
			T.What = SQLRestore(RS.GetVal("what", i));
			T.Info = SQLRestore(RS.GetVal("info", i));
			T.nState = stot<int>(RS.GetVal("state", i));
			TL[T.id]=T;
			i++;
		}
		return true;
	}
	return false;
}

bool DBData::Save(WTConfig *pwtc)
{
	std::string sSQL;
	DBResult RS;
	//zap & replace
	ExecSQL(&RS, "DELETE FROM wtconfig");
	sSQL=spf("INSERT INTO wtconfig(lock,x,y,w,h,periods,alert) VALUES(",
				(pwtc->blocked?1:0),
				", ", pwtc->X,
				", ", pwtc->Y,
				", ", pwtc->W,
				", ", pwtc->H,
				", ", pwtc->periods,
				", ", ((pwtc->bAlert)?1:0), ")");
	ExecSQL(&RS, sSQL);
	return NoError();
}

bool DBData::Save(WTProject &P)
{
	std::string sSQL;
	DBResult RS;
	bool b=false;
	if (P.id)
	{
		sSQL=spf("UPDATE projects SET",
					" name = ", SQLSafeQ(P.Name),
					", state = ", P.nState,
					", purpose = ", SQLSafeQ(P.Purpose),
					", priority = ", P.Priority,
					" WHERE idproject = ", P.id);
		ExecSQL(&RS, sSQL);
	}
	else
	{
		P.id=new_id("projects");
		sSQL=spf("INSERT INTO projects(idproject, name, state, purpose, priority) VALUES(",
					P.id,
					", ", SQLSafeQ(P.Name),
					", ", P.nState,
					", ", SQLSafeQ(P.Purpose),
					", ", P.Priority, ")");
		ExecSQL(&RS, sSQL);
	}
	if (NoError())  b=(save_deps(P.deps, P.id)&&save_locs(P.locs, P.id));
	return b;
}

bool DBData::save_deps(Dependencies &Deps, size_t idP)
{
	std::string sSQL;
	DBResult RS;
	
	sSQL=spf("DELETE FROM depends WHERE idproject = ", idP);
	ExecSQL(&RS, sSQL);
	
	if (!Deps.empty())
	{
		for (auto& d:Deps)
		{
			d.idP=idP;
			sSQL=spf("INSERT INTO depends(idproject, iddep, reason) VALUES(",
						d.idP,
						", ", d.idDep,
						", ", SQLSafeQ(d.reason), ")");
			ExecSQL(&RS, sSQL);
			if (!NoError()) return false;
		}
	}
	return true;
}

bool DBData::save_locs(Locations &Locs, size_t idP)
{
	std::string sSQL;
	DBResult RS;
	
	sSQL=spf("DELETE FROM locations WHERE idproject = ", idP);
	ExecSQL(&RS, sSQL);
	
	if (!Locs.empty())
	{
		for (auto& l:Locs)
		{
			l.idP=idP;
			sSQL=spf("INSERT INTO locations(idproject, description, location) VALUES(",
						l.idP,
						", ", SQLSafeQ(l.description),
						", ", SQLSafeQ(l.location), ")");
			ExecSQL(&RS, sSQL);
			if (!NoError()) return false; //else continue with next l
		}
	}
	return true;
}

bool DBData::Save(WTTodo &T)
{
	std::string sSQL;
	DBResult RS;

	if (T.id)
	{
		sSQL=spf("UPDATE todos SET what = ", SQLSafeQ(T.What),
								", info = ", SQLSafeQ(T.Info),
								", state = ", T.nState,
								" WHERE idtodo = ", T.id);
		ExecSQL(&RS, sSQL);
	}
	else
	{
		T.id=new_id("todos");
		sSQL=spf("INSERT INTO todos(idtodo, idproject, what, info, state) VALUES(",
					T.id,
					", ",	T.idP,
					", ",	SQLSafeQ(T.What),
					", ",	SQLSafeQ(T.Info),
					", ",	T.nState, ")");
		ExecSQL(&RS, sSQL);
	}
	return NoError();
}

bool DBData::Delete(WTProject &P)
{
	std::string sSQL;
	DBResult RS;
	auto del_todos=[&]()->bool
		{
			WTTodoList TL;
			if (Load(TL, P.id))
			{
				for (auto t:TL)
				{
					if (!Delete(t.second, true)) return false;
				}
				return true;
			}
			return false;
		};
	ExecSQL(&RS, "BEGIN TRANSACTION");
		sSQL=spf("DELETE FROM projects WHERE idproject = ", P.id);
		ExecSQL(&RS, sSQL);
		if (NoError())
		{
			if (del_todos())
			{
				sSQL=spf("DELETE FROM depends WHERE idproject = ", P.id, " OR iddep = ", P.id);
				ExecSQL(&RS, sSQL);
				if (NoError())
				{
					sSQL=spf("DELETE FROM locations WHERE idproject = ", P.id);
					ExecSQL(&RS, sSQL);
					if (NoError()&&del_id("projects", P.id))
					{
						ExecSQL(&RS, "COMMIT");
						return true;
					}
				}
			}
		}
	ExecSQL(&RS, "ROLLBACK");
	return false;
}

bool DBData::remove_dep(size_t iddep)
{
	std::string sSQL;
	DBResult RS;
	sSQL=spf("DELETE FROM depends WHERE iddep = ", iddep);
	ExecSQL(&RS, sSQL);
	return NoError();
}

bool DBData::Delete(WTTodo &T, bool bInTrans)
{
	std::string sSQL;
	DBResult RS;
	if (bInTrans)
	{
		sSQL=spf("DELETE FROM todos WHERE idtodo = ", T.id);
		ExecSQL(&RS, sSQL);
		if (NoError()) return del_id("todos", T.id);
	}
	else
	{
		ExecSQL(&RS, "BEGIN TRANSACTION");
			sSQL=spf("DELETE FROM todos WHERE idtodo = ", T.id);
			ExecSQL(&RS, sSQL);
			if (NoError())
			{
				if (del_id("todos", T.id))
				{
					ExecSQL(&RS, "COMMIT");
					return true;
				}
			}
		ExecSQL(&RS, "ROLLBACK");
	}
	return false;

}



