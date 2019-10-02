#ifndef _WTCONFIG_H_
#define _WTCONFIG_H_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#include "dbdata.h"
#include <string>

extern int XDELTA;
extern int YDELTA;
extern DBData DB;

struct WTConfig
{
	String sDBDir;
	bool blocked;
	int X,Y,W,H; //main window position & size
	int periods;
	bool bAlert;
	
	WTConfig(String &sdir);

	bool IsLocked();
	void SetLock(bool block=true);
	bool Load();
	bool Save();
	void Log(const String &S);
};

extern WTConfig *pConfig;

#endif //_WTCONFIG_H_

