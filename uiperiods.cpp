
#include "wt.h"
#include "utilfunc.h"
#include <string>


UIPeriods::UIPeriods()
{
	CtrlLayout(*this, "Configure Periods");
	bOK=false;
	radLen.WhenAction = THISBACK(OnRadLen);
	dlCount.WhenAction = THISBACK(ShowResult);
	radStart.WhenAction = THISBACK(ShowResult);
	btnOK.WhenPush = THISBACK(OnOK);
	btnCancel.WhenPush = THISBACK(OnCancel);
}

void UIPeriods::OnRadLen() { FillDlCount(); ShowResult(); }

void UIPeriods::FillDlCount()
{
	int n=0, l=~radLen;
	switch (++l)
	{
		case 1: n=16; break;
		case 2: n=8; break;
		case 3: n=5; break;
		case 4: n=4; break;
		case 5: n=3; break;
	}
	dlCount.Clear();
	for (int i=n;i>0;i--) dlCount.Add(i);
	dlCount.SetIndex(0);
}

void UIPeriods::ShowResult()
{
	int he, h=~radStart,p=~radLen,c=~dlCount;
	std::string s="";
	p++; h+=4; he=((h+(p*c))%24);
	s=spf(c, " x ", p, "-hour periods from: ", ((h<10)?"0":""), h, "h00 to: ", ((he<10)?"0":""), he, "h00");
	lblDP.SetText(s.c_str());
}

void UIPeriods::SetPeriods(int per)
{
	int n=(per&0xff)-1;
	radLen=n;
	FillDlCount();
	n=((per & 0xff00)>>8); dlCount.SetIndex(dlCount.Find(n));
	n=(((per & 0xff0000)>>16)-4); radStart=n;
	optWE.Set(((per & 0xff000000) > 0));
	ShowResult();
}

int UIPeriods::GetPeriods()
{
	int per=~optWE;
	per<<=8;
	per+=(int)~radStart;
	per+=4; per<<=8;
	per+=(int)~dlCount; per<<=8;
	per+=(int)~radLen; per++;
	return per;
}

void UIPeriods::OnOK() { bOK=true; Close(); }

void UIPeriods::OnCancel() { bOK=false; Close(); }



