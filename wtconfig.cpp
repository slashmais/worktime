
#include "wtconfig.h"
#include "wt.h"
#include "utilfunc.h"
#include "dbdata.h"

#include <fstream>


WTConfig *pConfig;

int XDELTA;
int YDELTA;
DBData DB;

	//periods: 0xEESSCCLL
	// EE -> 00/01(use weekends)
	// SS -> 04/../10 (time workday starts)
	// CC -> 01/../16 (number of periods in day - depends on period-length)
	// LL -> 01/../05 (period-length (hours))
	//periods=0x00090801; //excluding weekends, from 09h00 for 8 1-hour periods (normal workday)

WTConfig::WTConfig(String &sdir)
{
	sDBDir=sdir;
	//if (!Load()) throw std::runtime_error("Cannot configure WT");
	Load();
}

bool WTConfig::IsLocked() { return blocked; }
void WTConfig::SetLock(bool block) { blocked=block; Save(); }
bool WTConfig::Load() { return (DB.Load(this)); }
bool WTConfig::Save() { return (DB.Save(this)); }

void WTConfig::Log(const String &S)
{
	String sApp=GetExeTitle();
	if (!sDBDir.IsEmpty() && !sApp.IsEmpty())
	{
		String sLog=AppendFileName(sDBDir, sApp)+".log";
		FileAppend FA(sLog);
		FA << S << "\n";
	}
}

