#ifndef _wt_uiproject_h_
#define _wt_uiproject_h_

#include "layouts.h"

struct WTProject;

//-----------------------------------------------------------------------------
struct OptionX : public Option
{
	typedef OptionX CLASSNAME;
	size_t ID;
	virtual ~OptionX() {}
	OptionX& SETID(size_t id) { ID=id; return *this; }
	size_t GETID() { return ID; }
};
	

struct EbDirSel : public EditString
{
	typedef EbDirSel CLASSNAME;
	Button btnD;
	EbDirSel()
	{
		SetFrame(NullFrame());
		btnD.SetLabel(".."); Add(btnD.RightPosZ(0,15).VSizePosZ());
		btnD.WhenPush << [&] { FileSel fs; fs.ActiveDir(GetData()); if (fs.ExecuteSelectDir()) SetData(~fs); };
	}
	virtual ~EbDirSel() {}
};



//-----------------------------------------------------------------------------
struct UIProject : public WithprojectLayout<TopWindow>
{
	typedef UIProject CLASSNAME;
	bool bDirty, bOK;
	WTProject *pP;
	EditString eb;
	EbDirSel ebDirSel;
	
	UIProject(WTProject *p);
	virtual ~UIProject() {}

	void show_data();
	void show_deps();
	void show_locs();
	void OnOXClick();
	
	void OnDone();
	void SetState(int ns);
	int GetState();
	void OnStatus();
	void OnPRI(); //cb action => calc & show priority
	void FillPRI();
	void SetPRI(int pri);
	int GetPRI();
	bool Validate();
};





#endif
