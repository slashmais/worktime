
#include "wt.h"
#include "wtconfig.h"
#include "utilfunc.h"
#include "dbdata.h"
#include "wtproject.h"

UITodoList::UITodoList(size_t idP)
{
	if (idP)
	{
		std::string s;
		s=spf("Todos for '", glob_PL[idP].Name.c_str(), "'");
		CtrlLayout(*this, s.c_str());
		CenterOwner();
		Sizeable();
		TopMost();
		Sizeable();
		idProject=idP;
		btnAdd.WhenPush = THISBACK(OnAdd);
		btnEdit.WhenPush = THISBACK(OnEdit);
		btnDelete.WhenPush = THISBACK(OnDelete);
		btnClose.WhenPush = THISBACK(OnClose);
		ShowList();
	}
}

void UITodoList::ShowList()
{
	if (idProject)
	{
		std::string s;
		arTodos.Reset();
		arTodos.AddColumn("ID",0);
		arTodos.AddColumn("What",150).HeaderTab().WhenAction=THISBACK1(DoSorting,1);
		arTodos.AddColumn("State",120).HeaderTab().WhenAction=THISBACK1(DoSorting,2); arTodos.HeaderTab(2).AlignCenter();
		arTodos.AddColumn("Information",300).HeaderTab().WhenAction=THISBACK1(DoSorting,3);
		arTodos.WhenLeftDouble = THISBACK(OnTDblClick);
		for (auto p:glob_PL[idProject].TL)
		{
			const String a((p.second.nState==TS_NEED)?"Need":
							(p.second.nState==TS_BUG)?"BUG":
							(p.second.nState==TS_LATER)?"Later":
							(p.second.nState==TS_TEST)?"Test":"Done");
			s=spf(p.first); //or ttos?
			arTodos.Add(s.c_str(),
						p.second.What.c_str(),
						AttrText(a).Align(ALIGN_CENTER),
						p.second.Info.c_str());
		}
		DoSorting(2);
	}
}

size_t UITodoList::Current()
{
	int r = arTodos.GetCursor();
	if (r>=0) return stot<size_t>((const char*)arTodos.Get(r, 0).ToString());
	return 0;
}

void UITodoList::Current(size_t idT)
{
	if (idT>0)
	{
		std::string s;
		s=spf(idT); //or ttos?
		arTodos.FindSetCursor(s.c_str());
	}
}

void UITodoList::DoSorting(int col) { arTodos.ToggleSortColumn(col); arTodos.DoColumnSort(); }

void UITodoList::OnAdd()
{
	WTTodo T;
	T.idP = idProject;
	if (T.Edit()) { glob_PL[idProject].TL[T.id]=T; ShowList(); Current(T.id); }
}

void UITodoList::OnEdit()
{
	size_t idT=Current();
	if (idT) { if (glob_PL[idProject].TL[idT].Edit()) { ShowList(); Current(idT); }}
}

void UITodoList::OnDelete()
{
	size_t idT=Current();

	if (idT)
	{
		std::string s;
		s=spf("Do you want to Delete '", glob_PL[idProject].TL[idT].What.c_str(), "'?");
		if (PromptOKCancel(s.c_str())) { DB.Delete(glob_PL[idProject].TL[idT]); DB.Load(glob_PL[idProject].TL, idProject); ShowList(); }
	}
}

void UITodoList::OnClose() { Close(); }

void UITodoList::OnTDblClick()
{
	size_t idT=Current();
	if (idT) { if (glob_PL[idProject].TL[idT].Edit()) { ShowList(); Current(idT); }}
	else { OnAdd(); }
}



