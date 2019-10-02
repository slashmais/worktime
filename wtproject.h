#ifndef _WTPROJECT_H_
#define _WTPROJECT_H_

#include <string>
#include <map>
#include <vector>

struct WTTodo
{
	size_t id, idP;
	std::string What, Info;
	int nState; //need=0, bug=1, later=2, test=3, done=4
	WTTodo() : id(0), idP(0), What("<what>"), Info("<info>"), nState(0) {}
	virtual ~WTTodo() {}
	WTTodo& operator=(const WTTodo &T) { id=T.id; idP=T.idP; What=T.What; Info=T.Info; nState=T.nState; return *this; }
	bool Edit();
	bool Save();
};

enum //NB: don't change this: directly reflects radio-button-control values
{
	TS_NEED=0,
	TS_BUG,
	TS_LATER,
	TS_TEST,
	TS_DONE,
	TS_NOTE,
};

//typedef std::map<size_t, WTTodo> WTTodoList;
struct WTTodoList : public std::map<size_t, WTTodo>
{
	virtual ~WTTodoList() {}
};

struct Dependency
{
	size_t idP, idDep;
	std::string reason;
	Dependency();
	Dependency(const Dependency &D);
	Dependency(size_t ip, size_t id, const std::string &r); // : idP(ip), idDep(id), reason(r) {}
	virtual ~Dependency();
	Dependency& operator=(const Dependency &D);
};
struct Dependencies : public std::vector<Dependency> { virtual ~Dependencies() {} };

struct Location
{
	size_t idP;
	std::string description, location;
	Location();
	Location(const Location &L);
	Location(size_t id, const std::string &d, const std::string &l);
	virtual ~Location();
	Location& operator=(const Location &L);
};
struct Locations : public std::vector<Location> { virtual ~Locations() {} };

struct WTProject
{
	size_t id;
	std::string Name, Purpose;
	int nState, Priority; //nState: see enum below
	Dependencies deps;
	Locations locs;
	std::map<size_t, int> mdep;
	WTTodoList TL;
	WTProject();
	WTProject(const WTProject &P);
	~WTProject();

	WTProject& operator=(const WTProject &P);

	bool operator==(const WTProject &P);
	bool operator!=(const WTProject &P);
	bool Edit();
	bool Save();
};
enum //don't change this: directly reflects radio-button-control values
{
	PS_CURRENT=0,
	PS_WAIT,
	PS_DONE,
};

struct WTProjectList : public std::map<size_t, WTProject>
{
	enum { DEP_NONE=0, DEP_DIRECT, DEP_INDIRECT, };
	virtual ~WTProjectList() {}
	void clear() { for (auto &p:(*this)) p.second.TL.clear(); std::map<size_t, WTProject>::clear(); }
	int Depends(const WTProject &This, const WTProject &That); //returns DEP_*
};


extern WTProjectList glob_PL;


#endif
