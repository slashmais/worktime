
#include "wt.h"
#include "utilfunc.h"
#include <map>

UITodo::UITodo()
{
	CtrlLayout(*this, "WTTodo");
	CenterOwner();
	Sizeable();
	TopMost();
	bOK=false;
	btnOK.WhenPush = THISBACK(OnOK);
	btnCancel.WhenPush = THISBACK(OnCancel);
}

void UITodo::SetState(int n) { radState = n; }

int UITodo::GetState() { return (int)~radState; }

void UITodo::OnOK() { if ((bOK=Validate())) Close(); }

void UITodo::OnCancel() { bOK=false; Close(); }

bool UITodo::Validate()
{
	std::string s;
	s = (const char*)ebWhat.GetData().ToString();
	TRIM(s); if (s.empty()) { Message("Invalid 'What'"); return false; ebWhat.SetFocus(); }
	return true;
}




