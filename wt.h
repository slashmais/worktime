#ifndef _WTUI_H_
#define _WTUI_H_

#include "layouts.h"

#include <string>
#include <vector>
#include <map>
#include "utilfunc.h"

//-----------------------------------------------------------------------------
int CalcPriority(int pri); //in uiproject.cpp

//-----------------------------------------------------------------------------
struct TimeSlot
{
	std::string stm;
	size_t id[7];
};
typedef std::vector<TimeSlot> Roster;

//	//-----------------------------------------------------------------------------
//	struct ExitDlg : public WithExitDlgLayout<TopWindow>
//	{
//		typedef ExitDlg CLASSNAME;
//		bool bExit;
//		virtual ~ExitDlg() {}
//		ExitDlg()
//		{
//			CtrlLayout(*this, "Confirm..");
//			CenterOwner();
//	//		CenterScreen();
//			TopMost();
//			bExit=false;
//			btnExit.WhenPush=THISBACK(OnBtnExit);
//			btnCancel.WhenPush=THISBACK(OnBtnCancel);
//		}
//		void OnBtnExit() { bExit=true; Close(); }
//		void OnBtnCancel() { bExit=false; Close(); }
//	};

//-----------------------------------------------------------------------------
struct WT : public WithmainLayout<TopWindow>
{
	typedef WT CLASSNAME;
	//-------------------------------------
	struct ExitDlg : public WithExitDlgLayout<TopWindow>
	{
		typedef ExitDlg CLASSNAME;
		bool bExit;
		virtual ~ExitDlg() {}
		ExitDlg()
		{
			CtrlLayout(*this, "Confirm..");
			//CenterOwner();
			CenterScreen();
			TopMost();
			bExit=false;
			btnExit.WhenPush=THISBACK(OnBtnExit);
			btnCancel.WhenPush=THISBACK(OnBtnCancel);
		}
		void OnBtnExit() { bExit=true; Close(); }
		void OnBtnCancel() { bExit=false; Close(); }
	};
	//-------------------------------------
	struct CurP
	{
		int r,c;
		size_t id;
		CurP() : r(0),c(0),id(0) {}
		virtual ~CurP() {}
		CurP(int r, int c, size_t id) : r(r),c(c),id(id) {}
		CurP(const CurP &cur) { *this=cur; }
		CurP& operator=(const CurP &cur) { r=cur.r; c=cur.c; id=cur.id; return *this; }
		void clear() { r=c=0; id=0; }
	};
	std::map<int, std::map<size_t, int> > PIC; //<priority<id,count>>
	Roster roster;
	CurP PCur;
	std::map<int, bool> mAlert;
	bool bAlertEnabled;
	//bool ah[24], bTHistory; //ah[curhour]=>(t/f==alerted/not alerted)
	bool bTHistory;
	void do_adjust_controls();
	void do_fill_pic(int nP);
	WT();
	~WT();
	void DoAlert();
	void Activate();
	virtual void GotFocus();
	void Close();
	void OnBarRoster(Bar &menu);
	void OnBarDocument();
	void OnBarDevelop();
	void OnBarEditProject();
	void DoRefresh();
	bool IsValidPID(size_t id);
	void OnBtnPeriods();
	void OnBtnProjects();
	void OnBtnAddTodo();
	void OnBtnEditTodo();
	void OnBtnDropTodo();
	void OnBtnClose();
	void LoadData();
	void BuildRoster();
	void ShowRoster();
	void Current(CurP curp);
	CurP Current();
	CurP GetTimeProject();
	void ShowTimeProject();
	void ShowTodos();
	void DoTodoSort(int col);
	void OnPSelect();
	void OnPDblClick();
	void OnTDblClick();
	void ToggleAlert();
	void ToggleTodoHistory();
	
	//void do_import();
	
};

extern WT *pWT;

//-----------------------------------------------------------------------------
struct UIPeriods : WithperiodsLayout<TopWindow>
{
	typedef UIPeriods CLASSNAME;
	bool bOK;
	UIPeriods();
	void OnRadLen();
	void FillDlCount();
	void ShowResult();
	void SetPeriods(int per);
	int GetPeriods();
	void OnOK();
	void OnCancel();
};

//-----------------------------------------------------------------------------
struct UIProjectList : public WithprojectlistLayout<TopWindow> 
{
	typedef UIProjectList CLASSNAME;
	bool bChanged;
	UIProjectList();
	void ShowList();
	size_t Current();
	void Current(size_t idP);
	void DoSorting(int col);
	void OnAdd();
	void OnEdit();
	void OnDelete();
	void OnTodos();
	void OnClose();
	void OnPDblClick();
};

//-----------------------------------------------------------------------------
struct UITodo : public WithtodoLayout<TopWindow>
{
	typedef UITodo CLASSNAME;
	bool bOK;
	UITodo();
	void OnOK();
	void OnCancel();
	void SetState(int n);
	int GetState();
	bool Validate();
};

//-----------------------------------------------------------------------------
struct UITodoList : public WithtodolistLayout<TopWindow> 
{
	typedef UITodoList CLASSNAME;
	size_t idProject;
	UITodoList(size_t idp);
	void ShowList();
	size_t Current();
	void Current(size_t idT);
	void DoSorting(int col);
	void OnAdd();
	void OnEdit();
	void OnDelete();
	void OnClose();
	void OnTDblClick();
};


#endif //_WTUI_H_


