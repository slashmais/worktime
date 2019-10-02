#ifndef _wt_utilfunc_h_
#define _wt_utilfunc_h_

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#include <string>
#include <sstream>
#include <vector>

//--------------------------------------------------------------------------------------------------
struct MsgBox : public TopWindow //WithmsgboxLayout<TopWindow>
{
	typedef MsgBox CLASSNAME;
	DocEdit ebMsg;
	Button btnOK;
	MsgBox()
	{
		SetRect(0,0,300,300);
		TopMost();
		Title("Message").CenterOwner().Sizeable();
		Add(ebMsg.WantFocus(false).SetEditable(false).HSizePosZ(4, 4).VSizePosZ(4, 32));
		Add(btnOK.SetLabel(t_("OK")).HCenterPosZ(60, 0).BottomPosZ(4, 20));
		btnOK.WhenPush << [this]{ Close(); };
	}
	virtual ~MsgBox() {}
};

void Message(const String &sMsg);

//--------------------------------------------------------------------------------------------------
void PRINTSTRING(const std::string &s);
template<typename...P> void tu_p(std::stringstream &ss) { PRINTSTRING(ss.str()); }
template<typename H, typename...P> void tu_p(std::stringstream &ss, H h, P...p) { ss << h; tu_p(ss, p...); }
template<typename...P> void telluser(P...p) { std::stringstream ss(""); tu_p(ss, p...); }
inline void telluser(const String &S) { telluser(S.ToStd()); }
#define TODO telluser("To Do: '", __func__, "', in ", __FILE__, " [", long(__LINE__), "]");
#define todo TODO

//--------------------------------------------------------------------------------------------------
Vector<String> get_multi_file();
String get_file(bool bOpen=true);

//--------------------------------------------------------------------------------------------------
inline bool is_name_char(int c)	{ return (((c>='0')&&(c<='9')) || ((c>='A')&&(c<='Z')) || ((c>='a')&&(c<='z')) || (c=='_')); }


//--------------------------------------------------------------------------------------------------
template<typename...T> void uspf_p(StringStream&) {}
template<typename H, typename...T> void uspf_p(StringStream &SS, H h, T...t) { SS << h; uspf_p(SS, t...); }
template<typename...T> String uspf(T...t) { StringStream SS; uspf_p(SS, t...); return SS.GetResult(); }

//-----------------------------------------------------------------------------
template<typename...P> void spf_p(std::stringstream &s) {}
template<typename H, typename...P> void spf_p(std::stringstream &ss, H h, P...p) { ss << h; spf_p(ss, p...); }
template<typename...P> std::string spf(P...p) { std::stringstream ss(""); spf_p(ss, p...); return ss.str(); }

//-----------------------------------------------------------------------------
const String chopS(const String &s, int nmaxlen, bool bdots=false);
const String padS(const String &s, int nmaxlen, char c=' ', bool bfront=true);

inline bool SeqS(const String &l, const String &r) { return (l.Compare(r)==0); }
inline bool SieqS(const String &l, const String &r) { return (ToLower(l).Compare(ToLower(r))==0); }

void AddToSet(String &sSet, const String &s);
bool IsInSet(const String &sSet, const String &s);

//--------------------------------------------------------------------------------------------------
///todo: whitespace chars - beep, para.., backspace, etc
#define TRIM_WHITESPACE " \t\n\r"
void LTRIM(std::string& s, const char *sch=TRIM_WHITESPACE);
void RTRIM(std::string& s, const char *sch=TRIM_WHITESPACE);
void TRIM(std::string& s, const char *sch=TRIM_WHITESPACE);
void ReplacePhrase(std::string &sTarget, const std::string &sPhrase, const std::string &sReplacement); //each occ of phrase with rep
void ReplaceChars(std::string &sTarget, const std::string &sChars, const std::string &sReplacement); //each occ of each char from chars with rep
const std::string SanitizeName(const std::string &sp);
//=====
String SanitizeName(const String &S);

const std::string ucase(const char *sz);
const std::string ucase(const std::string &s);
void toucase(std::string &s);
const std::string lcase(const char *sz);
const std::string lcase(const std::string &s);
void tolcase(std::string &s);

int scmp(const std::string &s1, const std::string &s2);
int sicmp(const std::string &s1, const std::string &s2);
bool seqs(const std::string &s1, const std::string &s2);
bool sieqs(const std::string &s1, const std::string &s2);

bool hextoch(const std::string &shex, char &ch); //expect shex <- { "00", "01", ... , "fe", "ff" }

void fshex(std::string &shex, const std::string &sraw, int len=0); //format raw to hex string

//--------------------------------------------------------------------------------------------------
bool MakeBUP(String sFile);
bool istextfile(String sFile);
bool issqlitefile(String sFile);
bool isimagefile(String sFile);
bool isvideofile(String sFile);
bool isaudiofile(String sFile);

//--------------------------------------------------------------------------------------------------
const std::string PathAppend(const std::string &sPath, const std::string &sApp);
String PathAppend(const String &sPath, const String &sApp);

//--------------------------------------------------------------------------------------------------
int StartPeriodic(int msecs, std::function<void()> cb); //callback at every msecs interval
void EndPeriodic(int p); //kills the returned p from StartPeriodic()
void StopPeriodic(); //kills all periodics

//--------------------------------------------------------------------------------------------------
template<typename T> bool is_numeric(T t)
{
	return (std::is_integral<decltype(t)>{}||std::is_floating_point<decltype(t)>{});
}

//NB: use absolute value of n...
template<typename N>const std::string tohex(N n, unsigned int leftpadtosize=1, char pad='0')
{
	if (!is_numeric(n)) return "\0";
	std::string H="", h="0123456789ABCDEF";
	uint64_t U=n;
	//char h[]={ '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
	while (U) { H.insert(0,1,h[(U&0xf)]); U>>=4; }
	while (H.size()<leftpadtosize) H.insert(0, 1, pad);
	return H;
}

template<typename T> void ensv(const std::vector<T> &v, char delim, std::string &list, bool bIncEmpty=true) //en-[delimiter-]separated-values
{
	list.clear();
	std::ostringstream oss("");
	for (auto t:v) { if (!oss.str().empty()) oss << delim; oss << t; }
	list=oss.str();
}

template<typename T> const std::string ttos(const T &v)
{
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

template<typename T> const T stot(const std::string &str)
{
    std::istringstream ss(str);
    T ret;
    ss >> ret;
    return ret;
}

template<typename T> size_t desv(const std::string &list, char delim, std::vector<T> &v, bool bIncEmpty=true) //de-[delimiter-]separated-values
{
	std::istringstream iss(list);
	std::string s;
	v.clear();
	auto stt=[](const std::string &str)->const T{ std::istringstream ss(str); T t; ss >> t; return t; };
	while (std::getline(iss, s, delim)) { TRIM(s); if (!s.empty()||bIncEmpty) v.push_back(stt(s)); }
	return v.size();
}



#endif
