
#include "utilfunc.h"
#include <string>
#include <map>

//-------------------------------------

void Message(const String &sMsg)
{
	MsgBox mb;
	mb.ebMsg.SetData(sMsg);
	mb.Execute();
}


//--------------------------------------------------------------------------------------------------
void PRINTSTRING(const std::string &s) { PromptOK(DeQtf(s.c_str())); }

Vector<String> get_multi_file()
{
	Vector<String> VF;
	FileSel FS;
	FS.Types("Any\t*.*");
	FS.Multi();
	VF.Clear();
	if (FS.ExecuteOpen("Select file(s) to be opened")) { for (int i=0;i<FS.GetCount();i++) VF.Add(FS.GetFile(i)); }
	return VF;
}

String get_file(bool bOpen)
{
	String sf("");
	FileSel FS;
	FS.Types("Any\t*.*");
	if (bOpen) { if (FS.ExecuteOpen("Select file to be opened")) sf=FS.Get(); }
	else { if (FS.ExecuteSaveAs("Supply or select filename")); sf=FS.Get(); }
	return sf;
}

//--------------------------------------------------------------------------------------------------
const String chopS(const String &s, int nmaxlen, bool bdots)
{
	String w; w.Clear();
	if (bdots) { nmaxlen-=2; if (nmaxlen<=2) { w=".."; return w; }}
	if (nmaxlen>=s.GetCount()) { w=s; return w; }
	w=s.Mid(0, nmaxlen);
	if (bdots) w.Cat("..");
	return w;
}

const String padS(const String &s, int nmaxlen, char c, bool bfront)
{
	std::string w=s.ToStd();
	int n=nmaxlen-w.length();
	if (n>0) w.insert((bfront?0:w.length()), n, c);
	return String(w.c_str());
}

void AddToSet(String &sSet, const String &s)
{
	if (s.IsEmpty()) return;
	if (!IsInSet(sSet, s)) sSet.Cat(uspf('[', s, ']'));
}

bool IsInSet(const String &sSet, const String &s)
{
	if (s.IsEmpty()) return false;
	return (sSet.Find(uspf('[', s, ']'))>=0);
}

//--------------------------------------------------------------------------------------------------
void LTRIM(std::string& s, const char *sch)
{
	std::string ss=sch;
	int i=0, n=s.length();
	while ((i<n) && (ss.find(s.at(i),0)!=std::string::npos)) i++;
	s = (i>0)?s.substr(i,n-i):s;
}

void RTRIM(std::string& s, const char *sch)
{
	std::string ss=sch;
	int n = s.length()-1;
	int i=n;
	while ((i>0) && (ss.find( s.at(i),0)!=std::string::npos)) i--;
	s = (i<n)?s.substr(0,i+1):s;
}

void TRIM(std::string& s, const char *sch) { LTRIM(s, sch); RTRIM(s, sch); }

void ReplacePhrase(std::string& S, const std::string& sPhrase, const std::string& sNew)
{ //replaces EACH occurrence of sPhrase with sNew
	size_t pos = 0, fpos;
	std::string s=S;
	while ((fpos = s.find(sPhrase, pos)) != std::string::npos)
	{
		s.replace(fpos, sPhrase.size(), sNew);
		pos = fpos + sNew.length();
	}
	S=s;
}

void ReplaceChars(std::string &s, const std::string &sCharList, const std::string &sReplacement)
{ //replaces EACH occurrence of EACH single char in sCharList with sReplacement
    std::string::size_type pos = 0;
    while ((pos=s.find_first_of(sCharList, pos))!= std::string::npos)
    {
        s.replace(pos, 1, sReplacement);
        pos += sReplacement.length();
    }
};

const std::string SanitizeName(const std::string &sp)
{
	std::string s=sp;
	ReplaceChars(s, " \\+-=~`!@#$%^&*()[]{}:;\"|'<>,.?/", "_");
	TRIM(s, "_");
	std::string t;
	do { t=s; ReplacePhrase(s, "__", "_"); } while (t.compare(s)!=0);
	return s;
}
//=====
String SanitizeName(const String &S) { return String(SanitizeName(S.ToStd()).c_str()); }

const std::string ucase(const char *sz)
{
	std::string w(sz);
	for (auto &c:w) c=std::toupper(static_cast<unsigned char>(c));
	return w;
}

const std::string ucase(const std::string &s) { return ucase(s.c_str()); };

void toucase(std::string& s) { s=ucase(s.c_str()); }

const std::string lcase(const char *sz)
{
	std::string w(sz);
	for (auto &c:w) c=std::tolower(static_cast<unsigned char>(c));
	return w;
}

const std::string lcase(const std::string &s) { return lcase(s.c_str()); };

void tolcase(std::string &s) { s=lcase(s.c_str()); }


int scmp(const std::string &s1, const std::string &s2) { return s1.compare(s2); }
int sicmp(const std::string &s1, const std::string &s2)
{
	std::string t1=s1, t2=s2;
	return ucase(t1.c_str()).compare(ucase(t2.c_str()));
}
bool seqs(const std::string &s1, const std::string &s2) { return (scmp(s1,s2)==0); }
bool sieqs(const std::string &s1, const std::string &s2) { return (sicmp(s1,s2)==0); }

bool hextoch(const std::string &shex, char &ch) //shex <- { "00", "01", ... , "fe", "ff" }
{
	if (shex.length()==2)
	{
		std::string sh=shex;
        tolcase(sh);
		int n=0;
		if		((sh[0]>='0')&&(sh[0]<='9')) n=(16*(sh[0]-'0'));
		else if ((sh[0]>='a')&&(sh[0]<='f')) n=(16*(sh[0]-'a'+10));
		else return false;
		if		((sh[1]>='0')&&(sh[1]<='9')) n+=(sh[1]-'0');
		else if ((sh[1]>='a')&&(sh[1]<='f')) n+=(sh[1]-'a'+10);
		else return false;
		ch=char(n);
	}
	return true;
}

void fshex(std::string &shex, const std::string &sraw, int len)
{
	//output: hex-values + tab + {char/.}'s for len of raw
	int i, l=((len)?len:sraw.length());
	auto c2s=[](unsigned char u)->std::string
		{
			char b[3];
			char *p=b;
			if(u<0x80) *p++=(char)u;
			else if(u<=0xff) { *p++=(0xc0|(u>>6)); *p++=(0x80|(u&0x3f)); }
			return std::string(b, p-b);
		};
	shex.clear();
	for (i=0; i<l; i++) { shex+=tohex<unsigned char>(sraw[i], 2); shex+=" "; }
	shex+="    ";//simulate 4-spaces 'tab'
	unsigned char u;
	for (i=0; i<l; i++) { u=sraw[i]; if (((u>=32)&&(u<=126))||((u>159)&&(u<255))) shex+=c2s(u); else shex+='.'; }
}

//--------------------------------------------------------------------------------------------------
const std::string PathAppend(const std::string &sPath, const std::string &sApp)
{
	std::string s;
	s=spf(sPath, ((sPath[sPath.length()-1]=='/')?"":"/"), sApp.c_str());
	return s;
}

String PathAppend(const String &sPath, const String &sApp)
{
	String s;
	s=uspf(sPath, ((sPath[sPath.GetLength()-1]=='/')?"":"/"), sApp);
	return s;
}

//--------------------------------------------------------------------------------------------------
bool MakeBUP(String sFile)
{
	if (!FileExists(sFile)) return false;
	String sbup;
	int i=1;
	do sbup=uspf(sFile, ".~", i++, "~"); while (FileExists(sbup));
	return FileCopy(sFile, sbup);
}

bool istextfile(String sFile)
{
	auto istxt=[](char c)->bool{ return (((c>=32)&&(c<=127))||(c=='\t')||(c=='\r')||(c=='\n')); };
	String F=LoadFile(sFile);
	if (!F.IsEmpty())
	{
		if (CheckUtf8(F)) return true;
		else { for (auto c:F) if (!istxt(c)) return false; }
	}
	return true; //empty file defaults to text
}

#include <fstream>
bool issqlitefile(String sFile)
{
	bool b=false;
	FileIn fi(sFile);
	if (fi)
	{
		String s;
		s.Clear();
		int c, i=0;
		do { c=fi.Get(); s.Cat(c); i++; } while ((c>=0)&&(i<15));
		if (s.GetCount()==15) b=SieqS(s, "sqlite format 3");
	}
	return b;
}

bool isimagefile(String sFile)
{
	bool b=false;
	String ext=ToLower(GetFileExt(sFile));
	b=(SeqS(ext, ".jpg")||SeqS(ext, ".bmp")||SeqS(ext, ".png")||SieqS(ext, ".jpeg"));
	
	//todo...check sigs...
	
	return b;
}

bool isvideofile(String sFile)
{
	bool b=false;
	String ext=ToLower(GetFileExt(sFile));
	b=(SeqS(ext, ".mp4")||SeqS(ext, ".mpg")||SeqS(ext, ".mpeg")||SeqS(ext, ".avi")||SeqS(ext, ".mov"));
	
	//todo...check sigs...
	
	return b;
}

bool isaudiofile(String sFile)
{
	bool b=false;
	String ext=ToLower(GetFileExt(sFile));
	b=(SeqS(ext, ".mp3")||SeqS(ext, ".raw"));
	
	//todo...check sigs...
	
	return b;
}

//--------------------------------------------------------------------------------------------------
/*
#include <thread>

bool b_init_periodic=false;
bool b_run_periodic=false;
struct per_dat
{
	int minp, maxp, sleep_interval;
	std::map<int, std::function<void()> > m_periodic;
	std::map<int, std::vector<int> > periodic_n_p;
	per_dat() { clear(); sleep_interval=1000000; }
	~per_dat() { clear(); }
	void clear() { m_periodic.clear(); periodic_n_p.clear(); minp=99999; maxp=0; }
	int add(int msecs, std::function<void()> cb)
	{
		int p=m_periodic.size();
		m_periodic[p]=cb;
		periodic_n_p[msecs].push_back(p);
		if (msecs<minp) minp=msecs;
		if (msecs>maxp) maxp=msecs;
		return p;
	}
	void remove(int p)
	{
		bool b=false;
		auto it=periodic_n_p.begin();
		while (!b&&(it!=periodic_n_p.end()))
		{
			auto vit=it->second.begin();
			while (!b&&(vit!=it->second.end())) { if ((b=(p==(*vit)))) it->second.erase(vit); else vit++; }
			if (!b) it++;
		}
		if (b) m_periodic.erase(p);
		if (m_periodic.empty()) { b_run_periodic=false; clear(); }
	}
	void checkrun(int m)
	{
		if ((m<minp)||(m>maxp)) return;
		for (auto p:periodic_n_p) { if ((m%p.first)==0) { for (auto pp:p.second) m_periodic[pp](); }}
	}
} periodic_data;

void periodic_timer(per_dat *pd)
{
	int m=0;
	while (b_run_periodic)
	{
		std::this_thread::sleep_for(std::chrono::nanoseconds(pd->sleep_interval));
		m++;
		if (m>=pd->minp) pd->checkrun(m);
		if (m>pd->maxp) m=0;
	}
}

int StartPeriodic(int msecs, std::function<void()> cb)
{
	int p;
	if (!b_init_periodic) { periodic_data.clear(); b_init_periodic=true; }
	p=periodic_data.add(msecs, cb);
	if (!b_run_periodic) { b_run_periodic=true; std::thread(periodic_timer, &periodic_data).detach(); }
	return p;
}

void EndPeriodic(int p) { if (b_init_periodic) periodic_data.remove(p);  }

void StopPeriodic()
{
	b_run_periodic=false;
	std::this_thread::sleep_for(std::chrono::nanoseconds(periodic_data.sleep_interval*2));
	periodic_data.clear();
}
*/

