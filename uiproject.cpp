
#include "wt.h"
#include "wtconfig.h"
#include "uiproject.h"
#include "wtproject.h"
#include "dbdata.h"
#include "utilfunc.h"
#include <map>

//=============================================================================
//prioritizing : attempts to get numerical values from subjective indicators.
std::map<int, std::string > PRI =
{
/*
	urgency		=> routine/necessary/critical/acute/vital            ... vital(overrides all)/critical(blocking/necessity)/  /routine ... some event/happening/occurrence/situation that must be handled now
	importance	=> trivial/meaningful/useful/essential/serious    ... planning/preventative ... preventative/preperatory(planning&scheduling of tasks)/usefulness
	difficulty	=> easy/normal/focus/intensive/hard               ... how much effort will be needed ... workload/labour/quantity ... effort
	skilling	=>  none/refresh/familiarize/learn/study            ... know it/refresh/syntax or api/acquire new lang/study additional e.g. neural-net, cloud-infra       what is needed to be able to do the work
	schedule	=> anytime/ongoing/sooner/soonest/immediate         ... time allocation/duration
*/

	//urgency:
	{ 1,	"routine" },
	{ 2,	"necessary" },
	{ 3,	"critical" },
	{ 4,	"acute" },
	{ 5,	"vital" },
	//importance:
	{ (1<<3),	"trivial" },
	{ (2<<3),	"meaningful" },
	{ (3<<3),	"useful" },
	{ (4<<3),	"essential" },
	{ (5<<3),	"serious" },
	//difficulty:
	{ (1<<6),	"easy" },
	{ (2<<6),	"normal" },
	{ (3<<6),	"focus" },
	{ (4<<6),	"intensive" },
	{ (5<<6),	"hard" },
	//skilling:
	{ (1<<9),	"none" },
	{ (2<<9),	"refresh" },
	{ (3<<9),	"familiarize" },
	{ (4<<9),	"learn" },
	{ (5<<9),	"study" },
	//schedule:
	{ (1<<12),	"anytime" },
	{ (2<<12),	"ongoing" },
	{ (3<<12),	"sooner" },
	{ (4<<12),	"soonest" },
	{ (5<<12),	"immediate" },


/*
	//importance => reason for:
	//{ 0x1, "desired/wanted" },
	//{ 0x2, "easing" },
	{ 0x3, "useful" },
	{ 0x4, "needed" },
	{ 0x5, "vital" },
	//workload => effort/difficulty, closer to completion has higher priority:
	{ 0x10, "Investigate" },
	{ 0x20, "Research/Analysis" },
	{ 0x30, "Specify/Plan" },
	{ 0x40, "Model/Design" },
	{ 0x50, "Implement/Test" },
	//urgency => :
	{ 0x100, "ongoing" },
	//{ 0x200, "timely" },
	//{ 0x300, "schedule" },
	{ 0x400, "sooner" },
	{ 0x500, "soonest" },
*/

};


int CalcPriority(int pri) //returns 1 as highest, .., 5 as lowest priority
{
	int t,s,d,i,u;
	t = (6-(pri&7));
	s = (6-((pri>>3)&7));
	d = (6-((pri>>6)&7));
	i = (6-((pri>>9)&7));
	u = (6-((pri>>12)&7));
	return ((t+(2*s)+(3*d)+(4*i)+(5*u))/15);
}

UIProject::UIProject(WTProject *p)
{
	CtrlLayout(*this, "WTProject");
	CenterOwner();
	Sizeable();
	TopMost();
	bDirty=bOK=false;
	
	pP=p;

	FillPRI();
	radStatus.WhenAction = THISBACK(OnStatus);
	cbUrg.WhenAction = THISBACK(OnPRI);
	cbImp.WhenAction = THISBACK(OnPRI);
	cbDif.WhenAction = THISBACK(OnPRI);
	cbSki.WhenAction = THISBACK(OnPRI);
	cbSch.WhenAction = THISBACK(OnPRI);
	btnDone.WhenPush = THISBACK(OnDone);

	show_data();
	
	ebName.WhenAction << [&]{ bDirty=true; };
	ebPERD.WhenAction << [&]{ bDirty=true; };
}


void UIProject::show_data()
{
	ebName.SetData(pP->Name.c_str());
	SetState(pP->nState);
	ebPERD.SetData(pP->Purpose.c_str());
	SetPRI(pP->Priority);

	if (pP->id!=0) { lblDep.Enable(); arDep.Enable(); show_deps(); }
	else { lblDep.Disable(); arDep.Disable(); }
	
	show_locs();
}

void UIProject::show_deps()
{
	int r=0, dd, dc;
	auto get_reason=[](Dependencies &ds, size_t id)->String
		{
			String S; S.Clear();
			//bool b=false;
			for (auto d:ds) { if (d.idDep==id) { S=d.reason.c_str(); break; }}
			return S;
		};
	arDep.Reset();
	arDep.Clear();
	arDep.AddColumn("WTProject").Sorting();
	arDep.AddColumn("Reason").Edit(eb);
	arDep.WhenAcceptEdit << [&]{ OnOXClick(); };
	for (auto p:glob_PL)
	{
		if ((p.first==pP->id)||(p.second.nState==PS_DONE)) continue; //not self or finished projects

		dd=glob_PL.Depends(*pP, p.second);
		dc=glob_PL.Depends(p.second, *pP); //pot. circular

		OptionX &ox=(OptionX&)arDep.CreateCtrl<OptionX>(r, 0, false);
		ox.WhenPush << [&]{ ox.Set((ox.Get()==1)?0:1); OnOXClick(); };
		ox.SETID(p.first).Set((dd!=WTProjectList::DEP_NONE)?1:0)
						.SetLabel(p.second.Name.c_str())
						.Enable(((dd!=WTProjectList::DEP_INDIRECT)&&(dc==WTProjectList::DEP_NONE))); //only if direct
		String S=get_reason(pP->deps, p.first);
		if (S.IsEmpty())
		{
			if (dd==WTProjectList::DEP_INDIRECT) S="(indirect)";
			else if (dc!=WTProjectList::DEP_NONE) S="(pre-depends)";
		}
		arDep.Set(r, 1, S);
		r++;
	}
}

void UIProject::show_locs()
{
	arLocs.Reset();
	arLocs.Clear();
	arLocs.WhenBar << [&](Bar &bar)
		{
			bar.Add("add row", [&]{ arLocs.Add("", ""); arLocs.SetCursor(arLocs.GetCount()-1); });
			bar.Add("delete row", [&]{ int cur=arLocs.GetCursor(); if (cur>=0) arLocs.Remove(cur); });
		};
	arLocs.AddColumn("Description",100).Edit(eb);
	arLocs.AddColumn("Location",150).Edit(ebDirSel);
	for (auto p:pP->locs) arLocs.Add(p.description.c_str(), p.location.c_str());
}

void UIProject::OnOXClick()//OptionX *pox)
{
	WaitCursor();
	int cur=arDep.GetCursor();
	pP->deps.clear();
	int n=arDep.GetCount();
	String S; S.Clear();
	for (int r=0;r<n;r++)
	{
		OptionX *p=(OptionX*)arDep.GetCtrl(r,0);
		S=arDep.Get(r,1).ToString();
		if ((p->IsEnabled())&&(~(*p)==1)) pP->deps.push_back(Dependency(pP->id, p->GETID(), S.ToStd()));
	}
	DB.save_deps(pP->deps, pP->id);
	SetState((pP->deps.size()>0)?PS_WAIT:PS_CURRENT); OnStatus();
	show_deps();
	arDep.SetCursor(cur);
}

void UIProject::SetState(int ns)
{
	int n=0;
	if ((ns>=0) && (ns<=2)) n=ns;
	radStatus = n;
}

int UIProject::GetState() { return (int)~radStatus; }

void UIProject::OnPRI()
{
	std::string s;
	s=spf(CalcPriority(GetPRI())); //or ttos  ..."%d",
	lblCalcP.SetText(s.c_str());
	bDirty=true;
}

void UIProject::FillPRI()
{
	for (auto p:PRI)
	{
		int x;
		if (p.first<=5)
		{
			cbUrg.Add(p.first, p.second.c_str());
		}
		else if ((p.first>=(1<<3))&&(p.first<=(5<<3)))
		{
			x=((p.first>>3)&7);
			cbImp.Add(x, p.second.c_str());
		}
		else if ((p.first>=(1<<6))&&(p.first<=(5<<6)))
		{
			x=((p.first>>6)&7);
			cbDif.Add(x, p.second.c_str());
		}
		else if ((p.first>=(1<<9))&&(p.first<=(5<<9)))
		{
			x=((p.first>>9)&7);
			cbSki.Add(x, p.second.c_str());
		}
		else if ((p.first>=(1<<12))&&(p.first<=(5<<12)))
		{
			x=((p.first>>12)&7);
			cbSch.Add(x, p.second.c_str());
		}
		//else cbP.Add(p.first, p.second.c_str());
	}
}

void UIProject::SetPRI(int pri)
{
	auto valid=[pri](int n)->bool{ return ((((pri>>n)&7)>=1)&&(((pri>>n)&7)<=5)); };
	int r=(1+(1<<3)+(1<<6)+(1<<9)+(1<<12));
	
	if ((((pri&7)>=1)&&((pri&7)<=5))&&valid(3)&&valid(6)&&valid(9)&&valid(12)) r=pri;
	
	int x;
	x=(r&7);
	cbUrg.SetIndex(cbUrg.FindKey(x));
	x=((r>>3)&7);
	cbImp.SetIndex(cbImp.FindKey(x));
	x=((r>>6)&7);
	cbDif.SetIndex(cbDif.FindKey(x));
	x=((r>>9)&7);
	cbSki.SetIndex(cbSki.FindKey(x));
	x=((r>>12)&7);
	cbSch.SetIndex(cbSch.FindKey(x));

//	cbUrg.SetIndex(cbUrg.FindKey((r&7)));
//	cbImp.SetIndex(cbImp.FindKey(((r>>3)&7)));
//	cbDif.SetIndex(cbDif.FindKey(((r>>6)&7)));
//	cbSki.SetIndex(cbSki.FindKey(((r>>9)&7)));
//	cbSch.SetIndex(cbSch.FindKey(((r>>12)&7)));

	OnPRI();
}

int UIProject::GetPRI()
{
	int pri=(int)cbUrg.GetKey(cbUrg.GetIndex());
	pri+=((int)cbImp.GetKey(cbImp.GetIndex())<<3);
	pri+=((int)cbDif.GetKey(cbDif.GetIndex())<<6);
	pri+=((int)cbSki.GetKey(cbSki.GetIndex())<<9);
	pri+=((int)cbSch.GetKey(cbSch.GetIndex())<<12);
	return pri;
}

void UIProject::OnDone()
{
	if (!bDirty) Close();
	if ((Validate()))
	{
		pP->Name = (const char*)ebName.GetData().ToString();
		pP->nState = GetState();
		pP->Purpose = (const char*)ebPERD.GetData().ToString();
		pP->Priority = GetPRI();
		
		pP->locs.clear();
		for (int r=0; r<arLocs.GetCount(); r++) pP->locs.push_back(Location(pP->id, arLocs.Get(r,0).ToString().ToStd(), arLocs.Get(r,1).ToString().ToStd()));
		
		bOK=pP->Save();
		Close();
	}
}

void UIProject::OnStatus()
{
	int n=(int)~radStatus;
	bool b = (n==0);
	cbUrg.Enable(b);
	cbImp.Enable(b);
	cbDif.Enable(b);
	cbSki.Enable(b);
	cbSch.Enable(b);
	if (n==PS_DONE) DB.remove_dep(pP->id);
	bDirty=true;
}

bool UIProject::Validate()
{
	std::string s;
	s = (const char*)ebName.GetData().ToString();
	TRIM(s); if (s.empty()) { Message("Invalid Name"); return false; ebName.SetFocus(); }
	s = (const char*)ebPERD.GetData().ToString();
	TRIM(s); if (s.empty()) { Message("Invalid Purpose"); return false; ebPERD.SetFocus(); }
	return true;
}

