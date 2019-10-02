
#include "wtproject.h"
#include "wtconfig.h"
#include "dbdata.h"
#include "wt.h"
#include "utilfunc.h"
#include "uiproject.h"


//--------------------------------------------------------------------------------------------------
WTProjectList glob_PL;


//--------------------------------------------------------------------------------------------------
bool WTTodo::Save()
{
	if (DB.Save(*this)) return true;
	Message(uspf("Error: WTTodo::Save()\n", DB.GetLastError().c_str(), "\nSQL:\n", DB.sLastSQL.c_str()));
	return false;
}

bool WTTodo::Edit()
{
	UITodo uiT;
	uiT.ebWhat.SetData(What.c_str());
	uiT.ebInfo.SetData(Info.c_str());
	uiT.SetState(nState);
	uiT.Execute();
	if (uiT.bOK)
	{
		What = (const char*)uiT.ebWhat.GetData().ToString();
		nState = uiT.GetState();
		Info = (const char*)uiT.ebInfo.GetData().ToString();
		return Save();
	}
	return false;
}


//--------------------------------------------------------------------------------------------------
Dependency::Dependency()											{ idP=idDep=0; reason.clear(); }
Dependency::Dependency(const Dependency &D)							{ (*this)=D; }
Dependency::Dependency(size_t ip, size_t id, const std::string &r)	{ idP=ip; idDep=id; reason=r; }
Dependency::~Dependency()											{ }
Dependency& Dependency::operator=(const Dependency &D)				{ idP=D.idP; idDep=D.idDep; reason=D.reason; return *this; }


//--------------------------------------------------------------------------------------------------
Location::Location()														{ idP=0; description.clear(); location.clear(); }
Location::Location(const Location &L)										{ (*this)=L; }
Location::Location(size_t id, const std::string &d, const std::string &l)	{ idP=id; description=d; location=l; }
Location::~Location()														{ }
Location& Location::operator=(const Location &L)							{ idP=L.idP; description=L.description; location=L.location; return *this; }


//--------------------------------------------------------------------------------------------------
WTProject::WTProject()
{
	id=0;
	Name="<no name>";
	Purpose="<no idea>";
	nState=0;
	Priority=0x111;
	deps.clear();
	locs.clear();
}

WTProject::WTProject(const WTProject &P)	{ (*this)=P; }

WTProject::~WTProject() { TL.clear(); deps.clear(); locs.clear(); }

WTProject& WTProject::operator=(const WTProject &P)
{
	id=P.id;
	Name=P.Name;
	Purpose=P.Purpose;
	nState=P.nState;
	Priority=P.Priority;
	TL=P.TL;
	deps=P.deps;
	locs=P.locs;
	return *this;
}

bool WTProject::operator==(const WTProject &P) { return ((id==P.id)||((Name==P.Name)&&(Purpose==P.Purpose))); }
bool WTProject::operator!=(const WTProject &P) { return !((*this)==P); }

bool WTProject::Save()
{
	if (DB.Save(*this)) return true;
	Message(uspf("Error: WTProject::Save()\n", DB.GetLastError().c_str(), "\nSQL:\n", DB.sLastSQL.c_str()));
	return false;
}

bool WTProject::Edit()
{
	UIProject uiP(this);
	uiP.Execute();
	return uiP.bOK;
}

int WTProjectList::Depends(const WTProject &This, const WTProject &That)
{
	auto has_dep=[&](const Dependencies &dl, size_t pid) { for (size_t i=0; i<dl.size(); i++) { if (pid==dl[i].idDep) return true; } return false; };
	if (has_dep(This.deps, That.id)) return DEP_DIRECT;
	//bool b=false;
	for (auto d:This.deps) { if ((Depends((*this)[d.idDep], That))!=DEP_NONE) return DEP_INDIRECT; }
	return DEP_NONE;
}



