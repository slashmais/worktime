
#include "wt.h"
#include "wtconfig.h"
#include "utilfunc.h"
#include <string>

//--------------------------------------------------------------------------------------------------
#define WTDATABASE (const char*)"wtdb.sqlite3"
#define WTDATADIR (const char*)"wtdata"

String sWTErr;

//=============================================================================
void SetXYDELTAS()
{
	//upp does not supply methods to get decoration sizes (title-bar-height, window-frame-thickness)
	//and uses only the 'client'-rectangle (excluding decorations) for positioning which the OS use
	//as absolute coordinates (i.e. inclusive of decorations) which causes the window-position to
	//drift to the right and downwards;
	//this is a workaround to get the decoration sizes
	TopWindow w,w1;
	w.SetRect(10,10,1,1);
	w.OpenMain();
	w1.SetRect(w.GetRect());
	w1.OpenMain();
	Rect rw=w.GetRect();
	Rect rw1=w1.GetRect();
	XDELTA=rw1.left-rw.left; //left-frame thickness
	YDELTA=rw1.top-rw.top; //title-bar height
	w.Close();
	w1.Close();
}
//=============================================================================

//--------------------------------------------------------------------------------------------------
bool check_lock()
{
	if (pConfig)
	{
		if (pConfig->IsLocked()) return (PromptYesNo("The database is locked by a previous instance."
														"&Overriding this lock may cause loss of data."
														"&&Do you want to override the lock?"));
		pConfig->SetLock();
		return true;
	}
	return false;
}

bool WTInitialize()
{
	bool bExist; //b=false,
	String sDB, sP;
	
	sP=GetHomeDirectory();
#ifdef PLATFORM_POSIX
	sP=PathAppend(sP, ".config/wtdata/");
#else //assume windows
	sP=PathAppend(sP, "wtdata\\");
#endif
	RealizePath(sP);
	sDB=PathAppend(sP, WTDATABASE);
	bExist=FileExists(sDB);
	if (!DB.Open(sDB.ToStd())) { sWTErr=uspf("Error (open): ", DB.GetLastError().c_str()); return false; }
	if (!bExist) { if (!DB.ImplementSchema()) { sWTErr=uspf("Error (schema): ", DB.GetLastError().c_str()); return false; }}
	pConfig=new WTConfig(sP);
	return check_lock();
}

void WTTerminate()
{
	if (pConfig) { pConfig->SetLock(false); delete pConfig; }
	DB.Close();
}

#ifdef PLATFORM_POSIX
#include <unistd.h>
#include <signal.h>
//void SignalHandler(int sig) { if ((sig==SIGINT)||(sig==SIGSTOP)) WTTerminate(); }
void SignalHandler(int sig) { WTTerminate(); }
#endif

//--------------------------------------------------------------------------------------------------
GUI_APP_MAIN
{
	
//	Message("DO BACKUP OF: ~/.config/wtdata/"); return; /* & then remember to comment this line */
	
	SetXYDELTAS();
	if (WTInitialize())
	{

#ifdef PLATFORM_POSIX
	signal(SIGHUP,	SignalHandler);
	signal(SIGINT,	SignalHandler);
	signal(SIGSTOP,	SignalHandler);
	signal(SIGKILL,	SignalHandler);
	signal(SIGQUIT,	SignalHandler);
	signal(SIGABRT,	SignalHandler);
	signal(SIGTERM,	SignalHandler);
	signal(SIGTSTP,	SignalHandler);
#endif

		WT().Run();
	}
	else if (!sWTErr.IsEmpty()) Message(sWTErr);
	WTTerminate();
}

