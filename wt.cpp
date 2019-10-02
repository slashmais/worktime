
#include "wt.h"
#include "wtconfig.h"
#include "wtproject.h"
#include "dbdata.h"
#include "utilfunc.h"
#include <ctime>
#include <utility>

#include "alerter.h"



#define IMAGECLASS WtImg
#define IMAGEFILE <worktime/wt.iml>
#include <Draw/iml.h>


//-------------------------------------
int CurHour()
{
	time_t t;
	struct tm *ptm;
	time(&t);
	ptm = localtime(&t);
	return ptm->tm_hour;
}

int CurWeekDay()
{
	int w;
	time_t t;
	struct tm *ptm;
	time(&t);
	ptm = localtime(&t);
	w=ptm->tm_wday;
	if (!w) w=6; else w--; //monday==0,...,sunday==6
	return w;
}

//-------------------------------------
WT *pWT;

WT::WT()
{
	CtrlLayout(*this, "WorkTime");
	Sizeable();
	SetRect(pConfig->X-XDELTA, pConfig->Y-YDELTA, pConfig->W, pConfig->H);
	Icon(WtImg::wticon());
	pWT=this;
	btnPeriods.WhenPush = THISBACK(OnBtnPeriods);
	btnProjects.WhenPush = THISBACK(OnBtnProjects);
	btnRefresh.WhenPush = THISBACK(DoRefresh);
	btnAddTodo.WhenPush = THISBACK(OnBtnAddTodo);
	btnEditTodo.WhenPush = THISBACK(OnBtnEditTodo);
	btnDropTodo.WhenPush = THISBACK(OnBtnDropTodo);
	btnClose.WhenPush = THISBACK(OnBtnClose);
	bAlertEnabled=pConfig->bAlert;
	chkAlert=bAlertEnabled;
	chkAlert.WhenAction = THISBACK(ToggleAlert);
	bTHistory=false;
	chkTodoHistory.WhenAction = THISBACK(ToggleTodoHistory);
	
	StartAlert([this](){ DoAlert(); });

	DoRefresh();
	
////	/*btnImport.WhenPush << [&]{ do_import(); };*/
	
}


//void WT::do_import()
//{
//	DBsqlite3 dbOld;
//	dbOld.Open("/home/mais/bin/worktimedb.sqlite3");
//	if (dbOld.bDBOK)
//	{
//		String sOldP;
//		DBResult RSOldP;
//		int np, nt, i, j;
//		
//		sOldP="SELECT * FROM projects";
//		np=dbOld.ExecSQL(&RSOldP, sOldP.ToStd());
//		if (np>0)
//		{
//			for (i=0; i<np; i++)
//			{
//				String sOldT;
//				DBResult RSOldT;
//				size_t idold=stot<size_t>(RSOldP.GetVal("idproject", i));
//				WTProject P;
//				P.id=0;
//				P.Name=DB.SQLRestore(RSOldP.GetVal("name", i));
//				P.nState=stot<int>(RSOldP.GetVal("state", i));
//				P.Purpose=DB.SQLRestore(RSOldP.GetVal("perd", i));
//				//P.Priority=stot<int>(RSOldP.GetVal("rep", i));
//				P.Save();
//				
//				sOldT=spf("SELECT * FROM todos WHERE idproject = ", idold);
//				nt=dbOld.ExecSQL(&RSOldT, sOldT.ToStd());
//				if (nt>0)
//				{
//					for (j=0; j<nt; j++)
//					{
//						WTTodo T;
//						T.id=0;
//						T.idP=P.id;
//						T.What=DB.SQLRestore(RSOldT.GetVal("what", j));
//						T.Info=DB.SQLRestore(RSOldT.GetVal("info", j));
//						T.nState=stot<int>(RSOldT.GetVal("state", j));
//						T.Save();
//					}
//				}
//			}
//		}
//		dbOld.Close();
//		DoRefresh();
//		PromptOK("Import done.");
//	}
//}


WT::~WT()
{
	StopAlert();
	Rect r=GetRect();
	pConfig->X = r.left;
	pConfig->Y = r.top;
	pConfig->W = r.Width();
	pConfig->H = r.Height();
	roster.clear();
	glob_PL.clear();
	pWT=0;
}

//-------------------------------------
#define AR_LINE_HEIGHT (int)15
#define AR_HEADER_HEIGHT (int)20
#define AR_LINE_ADJUST_FACTOR (double)(4.0/5.0)

void WT::do_adjust_controls()
{
	if (roster.size())
	{
		Rect r;
		int nextY=((roster.size()*AR_LINE_HEIGHT)+AR_HEADER_HEIGHT-(int)(double(roster.size())*AR_LINE_ADJUST_FACTOR)); //guessing arRoster-widget height here
		arRoster.HSizePosZ(4, 4).TopPosZ(4, nextY);
		nextY+=8;  btnPeriods.LeftPosZ(4, 60).TopPosZ(nextY, 20);
				   btnProjects.LeftPosZ(68, 60).TopPosZ(nextY, 20);
				   chkAlert.LeftPosZ(132, 60).TopPosZ(nextY, 20);
				   btnRefresh.RightPosZ(8, 60).TopPosZ(nextY, 20);
		nextY+=24; lblTodo.LeftPosZ(4, 512).TopPosZ(nextY, 16);
		nextY+=16; arTodos.HSizePosZ(4, 76).VSizePosZ(nextY, 4);
		nextY+=16; btnAddTodo.RightPosZ(8, 60).TopPosZ(nextY, 20);
		nextY+=24; btnEditTodo.RightPosZ(8, 60).TopPosZ(nextY, 20);
		nextY+=24; btnDropTodo.RightPosZ(8, 60).TopPosZ(nextY, 20);
		nextY+=24; chkTodoHistory.RightPosZ(8, 60).TopPosZ(nextY, 20);
	}
}

void WT::DoAlert()
{
	if (!bAlertEnabled) return;
	int h=CurHour();
	auto pit=mAlert.find(h);
	if ((pit!=mAlert.end())&&!pit->second)
	{
		for (auto &p:mAlert) p.second=false;
		pit->second=true;
		Urgent();
	}
}

void WT::Activate() { TopWindow::Activate(); Urgent(false); }

void WT::GotFocus() { Urgent(false); }

void WT::Close()
{
	ExitDlg dlg;
	dlg.Execute();
	if (dlg.bExit) Break(); }

void WT::OnBtnPeriods()
{
	UIPeriods uiP;
	uiP.SetPeriods(pConfig->periods);
	uiP.Execute();
	if (uiP.bOK) { pConfig->periods=uiP.GetPeriods(); DoRefresh(); }
}

void WT::OnBtnProjects()
{
	UIProjectList uiPL;
	uiPL.Execute();
	if (uiPL.bChanged) DoRefresh();
	else ShowTodos();
}

void WT::OnBtnClose() { Minimize(); }

void WT::LoadData() { glob_PL.clear(); DB.Load(glob_PL); }

void WT::OnBarRoster(Bar &menu)
{
	std::string s;
	Current(Current());
	if (PCur.id)
	{
//		if (!PL[PCur.id].DocEdit.empty())
//		{
//			s=spf("Document '", PL[PCur.id].Name.c_str(), "' ..");
//			menu.Add(true, s.c_str(), THISBACK(OnBarDocument));
//		}
//		if (!PL[PCur.id].DevEdit.empty())
//		{
//			s=spf("Develop '", PL[PCur.id].Name.c_str(), "' ..");
//			menu.Add(true, s.c_str(), THISBACK(OnBarDevelop));
//		}
//		if ((!PL[PCur.id].DocEdit.empty())||(!PL[PCur.id].DevEdit.empty())) menu.Separator();
		s=spf("Edit ", glob_PL[PCur.id].Name.c_str());
		menu.Add(true, s.c_str(), THISBACK(OnBarEditProject));
		menu.Separator();
	}
	menu.Add(true, "Change Periods", THISBACK(OnBtnPeriods));
	menu.Add(true, "WTProject-list", THISBACK(OnBtnProjects));
	menu.Separator();
	menu.Add(true, "Refresh", THISBACK(DoRefresh));
}

void WT::OnBarDocument()
{
	
	todo
	
//	if (PCur.id)
//	{
//		if (PL[PCur.id].DocEdit.empty()) Message("Add Workspace associations to the project.");
//		else
//		{
//			std::string s;
//			s=spf(PL[PCur.id].DocEdit.c_str(), " ", PL[PCur.id].DocFile.c_str());
//		}
//	}
}

void WT::OnBarDevelop()
{
	
	todo
	
//	if (PCur.id)
//	{
//		if (PL[PCur.id].DevEdit.empty()) Message("Add Workspace associations to the project.");
//		else
//		{
//			std::string s;
//			s=spf(PL[PCur.id].DevEdit.c_str(), " ", PL[PCur.id].DevFile.c_str());
//		}
//	}
}

void WT::OnBarEditProject() { if (PCur.id && glob_PL[PCur.id].Edit()) DoRefresh(); }

void WT::DoRefresh()
{
/*
	ShowRoster();
	if (IsValidPID(PCur.id)) Current(PCur); else Current(CurP());
	ShowTodos();
*/

	CurP cp=PCur;
	//for (int i=0;i<24;i++) ah[i]=false;
	//ah[CurHour()]=true;
	LoadData();
	ShowRoster();
	Current(CurP());
	if (IsValidPID(cp.id) && (cp.id!=PCur.id)) Current(cp); //else Current(CurP());
	ShowTodos();
}


bool WT::IsValidPID(size_t id) { bool b=false; for (auto p:glob_PL) if ((b=(id==p.first))) break; return b; }

void WT::ShowRoster()
{
	arRoster.Clear();
	arRoster.Reset();
	arRoster.NoCursor();
	arRoster.AddColumn("Start",60);
	arRoster.AddColumn("Monday",100);
	arRoster.AddColumn("Tuesday",100);
	arRoster.AddColumn("Wednesday",100);
	arRoster.AddColumn("Thursday",100);
	arRoster.AddColumn("Friday",100);
	arRoster.AddColumn("Saturday",100);
	arRoster.AddColumn("Sunday",100);
	arRoster.WhenLeftClick = THISBACK(OnPSelect);
	arRoster.WhenLeftDouble = THISBACK(OnPDblClick);
	arRoster.WhenBar = THISBACK(OnBarRoster);
	BuildRoster();
	for (auto ts:roster) //ts==TimeSlot
	{
		arRoster.Add(ts.stm.c_str(),
					(ts.id[0])?glob_PL[ts.id[0]].Name.c_str():"-",
					(ts.id[1])?glob_PL[ts.id[1]].Name.c_str():"-",
					(ts.id[2])?glob_PL[ts.id[2]].Name.c_str():"-",
					(ts.id[3])?glob_PL[ts.id[3]].Name.c_str():"-",
					(ts.id[4])?glob_PL[ts.id[4]].Name.c_str():"-",
					(ts.id[5])?glob_PL[ts.id[5]].Name.c_str():"-",
					(ts.id[6])?glob_PL[ts.id[6]].Name.c_str():"-");
	}
	do_adjust_controls();
}

void WT::OnPSelect() { Current(Current()); }

void WT::OnPDblClick()
{
	OnPSelect();
	if (PCur.id) { if (glob_PL[PCur.id].Edit()) DoRefresh(); }
	else OnBtnProjects();
}

void WT::do_fill_pic(int nP)
{
	PIC.clear();
	if (!glob_PL.size()) return;
	double dP=nP, dU=0.0, dF;
	for (auto pp:glob_PL) { if (pp.second.nState==PS_CURRENT) dU+=(6.0-CalcPriority(pp.second.Priority)); }
	dF=(dP/(dU+1.0));
	for (auto pp:glob_PL)
	{
		if (pp.second.nState==PS_CURRENT)
		{
			int p=CalcPriority(pp.second.Priority);
			PIC[p][pp.first]=(int)(dF*(6.0-p)+0.5);
		}
	}
}

void WT::BuildRoster()
{
	int i,j,b,h,l,n,th;
	size_t id;
	roster.clear();
	mAlert.clear();
	l=(pConfig->periods & 0xff); //length of a period
	n=((pConfig->periods & 0xff00)>>8); //number of periods (per day)
	h=((pConfig->periods & 0xff0000)>>16); //start hour
	b=((pConfig->periods & 0xff000000) > 0); //include weekends
	do_fill_pic((n*(b?7:5))); //weekends/workdays
	for (j=0;j<n;j++)
	{
		TimeSlot ts;
		for (i=0;i<8;i++)
		{
			th=(h+(j*l));
			if (i==0) { ts.stm=spf(((th<10)?"0":""), th, "h00"); mAlert[th]=false; }
			else if ((i>=6) && !b) { ts.id[i-1]=0; }
			else
			{
				id=0;
				for (auto &p:PIC) //NB: use _references_ to decrement count (debug-cost: ~1/2 hour for s'thing so stupid)
				{
					for (auto &ic:p.second) { if (ic.second>0) { id=ic.first; ic.second--; break; }}
					if (id) break;
				}
				ts.id[i-1]=id;
			}
		}
		roster.push_back(ts);
	}
	PIC.clear();
}

void WT::Current(CurP cur)
{
	int i,j;
	if (PCur.id)
	{
		j=0; while (j<(int)roster.size())
		{
			i=0; while (i<7)
			{
				if (roster.at(j).id[i]==PCur.id) arRoster.Set(j, i+1, glob_PL[PCur.id].Name.c_str());
				i++;
			}
			j++;
		}
		PCur.id=0;
	}
	if (cur.id)
	{
		PCur=cur;
		j=0; while (j<(int)roster.size())
		{
			i=0; while (i<7)
			{
				if (roster.at(j).id[i]==PCur.id) arRoster.Set(j, i+1, AttrText(glob_PL[PCur.id].Name.c_str()).Ink(Color(0,255,0)));
				i++;
			}
			j++;
		}
		ShowTodos();
	}
	ShowTimeProject(); //hi-lites what u _should_ be doing..!
}

WT::CurP WT::Current()
{
	CurP cur=GetTimeProject();
	//size_t id=0;
	int r,c;
	if ((r=arRoster.GetClickRow())>=0)
	{
		c=arRoster.GetClickColumn();
		if ((c>0) && (c<arRoster.GetColumnCount()))
		{
			cur.r=r;
			cur.c=(c-1);
			cur.id = roster.at(r).id[c-1];
		}
	}
	return cur;
}

WT::CurP WT::GetTimeProject()
{
	int H=CurHour(), D=CurWeekDay(), j=0,b=0,h,l,n,th;
	CurP tcur;
	l=(pConfig->periods & 0xff); //length of a period
	n=((pConfig->periods & 0xff00)>>8); //number of periods (per day)
	h=((pConfig->periods & 0xff0000)>>16); //start hour
	while (!b && (j<n)) { th=(h+(j*l)); if (!(b=((H>=th) && (H<(th+l))))) j++; }
	if (b) { tcur.r=j; tcur.c=D; tcur.id=roster.at(j).id[D]; }
	return tcur;
}

void WT::ShowTimeProject()
{
	CurP tcur=GetTimeProject();
	if (tcur.id) arRoster.Set(tcur.r, tcur.c+1, AttrText(glob_PL[tcur.id].Name.c_str()).Ink(Color(0,0,255)).Bold().Underline());
	if (!PCur.id) { PCur=tcur; ShowTodos(); }
}

void WT::ShowTodos()
{
	arTodos.Clear();
	arTodos.Reset();
	arTodos.AddColumn("ID",0);
	arTodos.AddColumn("What",150).HeaderTab().WhenAction=THISBACK1(DoTodoSort,1);
	arTodos.AddColumn("Status",40).HeaderTab().WhenAction=THISBACK1(DoTodoSort,2);
	arTodos.AddColumn("Information",300).HeaderTab().WhenAction=THISBACK1(DoTodoSort,3);
	arTodos.WhenLeftDouble = THISBACK(OnTDblClick);
	if (PCur.id)
	{
		std::string s,st;
		Color c;
		s=spf( "WTTodo's for '", glob_PL[PCur.id].Name.c_str(), "'");
		lblTodo.SetLabel(s.c_str());
		for (auto p:glob_PL[PCur.id].TL)
		{
			s=spf(p.first); //or ttos  ..."%llu",
			switch(p.second.nState)
			{
				case TS_NEED: { st="Need"; c=Blue(); } break;
				case TS_BUG: { st="BUG"; c=Red(); } break;
				case TS_LATER: { st="Later"; c=Color(0,255,255); } break;
				case TS_TEST: { st="Test"; c=Color(0,128,0); } break;
				case TS_DONE: { st="Done"; c=Gray(); } break;
				case TS_NOTE: { st="Note"; c=Black(); } break;
			}
			if ((p.second.nState!=TS_DONE) || ((p.second.nState==TS_DONE) && bTHistory))
				arTodos.Add(s.c_str(),
							AttrText(p.second.What.c_str()).NormalInk(c),
							AttrText(st.c_str()).Align(ALIGN_CENTER).NormalInk(c),
							p.second.Info.c_str());
		}
		DoTodoSort(2);
	}
}

void WT::DoTodoSort(int col) { arTodos.ToggleSortColumn(col); arTodos.DoColumnSort(); }

void WT::OnBtnAddTodo()
{
	if (PCur.id)
	{
		WTTodo T;
		T.idP = PCur.id;
		if (T.Edit()) { glob_PL[PCur.id].TL[T.id]=T; ShowTodos(); }
	}
}

void WT::OnBtnEditTodo()
{
	int r;
	size_t idT;
	if ((r=arTodos.GetCursor())>=0)
	{
		idT = stot<size_t>((const char*)arTodos.Get(r, 0).ToString());
		if (idT) if (glob_PL[PCur.id].TL[idT].Edit()) ShowTodos();
	}
}

void WT::OnBtnDropTodo()
{
	int r;
	size_t idT;
	if ((r=arTodos.GetCursor())>=0)
	{
		idT = stot<size_t>((const char*)arTodos.Get(r, 0).ToString());
		if (idT)
		{
			std::string s;
			s=spf("Do you want to Delete '", glob_PL[PCur.id].TL[idT].What.c_str(), "'?");
			if (PromptOKCancel(s.c_str())) { DB.Delete(glob_PL[PCur.id].TL[idT]); DB.Load(glob_PL[PCur.id].TL, PCur.id); ShowTodos(); }
		}
	}
}

void WT::OnTDblClick()
{
	int r;
	size_t idT=0;
	if ((r=arTodos.GetCursor())>=0)
	{
		idT = stot<size_t>((const char*)arTodos.Get(r, 0).ToString());
		if (idT) if (glob_PL[PCur.id].TL[idT].Edit()) ShowTodos();
	}
	else { OnBtnAddTodo(); }
}

void WT::ToggleAlert() { pConfig->bAlert=bAlertEnabled=chkAlert; }

void WT::ToggleTodoHistory() { bTHistory=chkTodoHistory; ShowTodos(); }
