
#include "wt.h"
#include "wtconfig.h"
#include "utilfunc.h"
#include "dbdata.h"
#include "wtproject.h"

UIProjectList::UIProjectList()
{
	CtrlLayout(*this, "Projects");
	CenterOwner();
	TopMost();
	Sizeable();
	btnAdd.WhenPush = THISBACK(OnAdd);
	btnEdit.WhenPush = THISBACK(OnEdit);
	btnDelete.WhenPush = THISBACK(OnDelete);
	btnTodos.WhenPush = THISBACK(OnTodos);
	btnClose.WhenPush = THISBACK(OnClose);
	bChanged=false;
	ShowList();
}

void UIProjectList::ShowList()
{
	std::string s;
	//arProjects.Clear();
	arProjects.Reset();
	arProjects.AddColumn("ID",0);
	arProjects.AddColumn("Name",150).HeaderTab().WhenAction=THISBACK1(DoSorting,1);
	arProjects.AddColumn("Priority",50).HeaderTab().WhenAction=THISBACK1(DoSorting,2); arProjects.HeaderTab(2).AlignCenter();
	arProjects.AddColumn("Status",80).HeaderTab().WhenAction=THISBACK1(DoSorting,3); arProjects.HeaderTab(3).AlignCenter();
	arProjects.AddColumn("Purpose",350).HeaderTab().WhenAction=THISBACK1(DoSorting,4);
	arProjects.AddColumn("Todos",50).HeaderTab().WhenAction=THISBACK1(DoSorting,5); arProjects.HeaderTab(5).AlignRight();
	arProjects.WhenLeftDouble = THISBACK(OnPDblClick);
	for (auto P:glob_PL)
	{
		const String a((P.second.nState==PS_CURRENT)?"Current":(P.second.nState==PS_WAIT)?"(Wait)":"Done");
		const String b(AsString(CalcPriority(P.second.Priority)));
		const String c(AsString(P.second.TL.size()));
		s=spf(P.first); //or ttos?
		arProjects.Add(s.c_str(),
						P.second.Name.c_str(),
						AttrText(b).Align(ALIGN_CENTER),
						AttrText(a).Align(ALIGN_CENTER),
						P.second.Purpose.c_str(),
						AttrText(c).Align(ALIGN_RIGHT));
	}
	DoSorting(1);
}

size_t UIProjectList::Current()
{
	int r = arProjects.GetCursor();
	if (r>=0) return stot<size_t>((const char*)arProjects.Get(r, 0).ToString());
	return 0;
}

void UIProjectList::Current(size_t idP)
{
	if (idP>0)
	{
		std::string s;
		s=spf(idP);  //or ttos?
		arProjects.FindSetCursor(s.c_str());
	}
}

void UIProjectList::DoSorting(int col)
{
	arProjects.ToggleSortColumn(col);
	arProjects.DoColumnSort();
}

void UIProjectList::OnAdd()
{
	WTProject P;
	if (P.Edit()) { glob_PL[P.id]=P; ShowList(); Current(P.id); bChanged=true; }
}

void UIProjectList::OnEdit()
{
	size_t idP=Current();
	if (idP) { if (glob_PL[idP].Edit()) { ShowList(); Current(idP); bChanged=true; }}
}

void UIProjectList::OnDelete()
{
	size_t idP=Current();
	if (idP)
	{
		std::string s;
		s=spf("Do you want to Delete '", glob_PL[idP].Name.c_str(), "'?");
		if (PromptOKCancel(s.c_str())) { DB.Delete(glob_PL[idP]); DB.Load(glob_PL); ShowList(); bChanged=true; }
	}
}

void UIProjectList::OnTodos()
{
	size_t idP = Current();
	if (idP)
	{
		UITodoList uiTL(idP);
		uiTL.Execute();
		///TODO: changed? ... bChanged=true;
			ShowList();
			Current(idP);
	}
}

void UIProjectList::OnClose() { Close(); }

void UIProjectList::OnPDblClick()
{
	size_t idP=Current();
	if (idP) { if (glob_PL[idP].Edit()) { ShowList(); Current(idP); bChanged=true; }}
	else { OnAdd(); }
}


