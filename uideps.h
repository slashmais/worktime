#ifndef _wt_uideps_h_
#define _wt_uideps_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;


struct UIDPanel : public Ctrl
{
	typedef UIDPanel CLASSNAME;
	
	UIDPanel() {}
	virtual~UIDPanel() {}
	
};

struct UIDListPanel : public UIDPanel
{
	typedef UIDListPanel CLASSNAME;
	
	Label lblp, lbld;
	DropList dlP;
	ArrayCtrl arDeps;
	
	UIDListPanel();
	virtual ~UIDListPanel() { }
	
	void OnDLAction(); //update ar
	void show_data();
	
};

struct UIDGraphPanel : public UIDPanel
{
	typedef UIDGraphPanel CLASSNAME;
	
	UIDGraphPanel() {}
	virtual ~UIDGraphPanel() {}
};

struct UIDeps : public TopWindow
{
	typedef UIDeps CLASSNAME;
	
	TabCtrl tabs;
	Button btnOK, btnCancel;
	
	
	UIDeps();
	virtual ~UIDeps();

	void OnOK(); //validate/save
	
};




#endif
