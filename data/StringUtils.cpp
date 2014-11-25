/*
 OpenLieroX
 
 string utilities
 
 code under LGPL
 created 01-05-2007
 by Albert Zeyer and Dark Charlie
 */

#ifdef _MSC_VER
#pragma warning(disable: 4786)  // WARNING: identifier XXX was truncated to 255 characters in the debug info...
#endif

#include "StringUtils.h"
#include "MathLib.h"
#include "StringBuf.h"


#include <sstream>
#include <iomanip>


const std::list<std::string> filledStrList( const size_t amount, const char *first, ... )
{
	char *item;
	va_list vl;

	std::list<std::string> list;

	item = const_cast<char*>( first );
	list.push_back(item);

	va_start(vl, first);

	for ( size_t i=1 ; i<amount ; i++ )
	{
		item = const_cast<char*>( va_arg(vl, const char*) );
		list.push_back(item);
	}

	va_end(vl);

	return list;

}

/**
 * Returns of a given string the number of lines
 */
unsigned int calcNumLines(const std::string& text)
{
	unsigned int numLines = 1;

	if(text.size() == 0)
		return 0;

	for( unsigned int i=0 ; i<text.size() ; i++ )
	{
		if ( endofText( text.substr(i) ) )
			numLines++;
	}

	return numLines;
}

/**
 * Formats a string they way it looks like it's right aligned
 */
std::string getRightAlignedString(std::string text, size_t size)
{
	if(text.size() > size)
		return text;

	std::string output;
	for(size_t i=0 ; i<(size-text.size()) ; i++)
		output += ' ';
	output += text;
	return output;
}

///////////////////
// Tells whether the given string is a end of line or not.
bool endofText(const std::string& Text)
{
	if(!Text.empty())
	{
		if(Text.size() >= 2)
		{
			if( Text.at(0) == 0x0A )
				return true;
		}
		return false;
	}
	else
		return true;
}

///////////////////
// Trim the leading & ending spaces from a string
void TrimSpaces(std::string& szLine) {
	size_t n = 0;
	std::string::iterator p;
	for(p = szLine.begin(); p != szLine.end(); p++, n++)
		if(!isspace((unsigned char)*p) || isgraph((unsigned char)*p)) break;
	if(n>0) szLine.erase(0,n);
	
	n = 0;
	std::string::reverse_iterator p2;
	for(p2 = szLine.rbegin(); p2 != szLine.rend(); p2++, n++)
		if(!isspace((unsigned char)*p2) || isgraph((unsigned char)*p2)) break;
	if(n>0) szLine.erase(szLine.size()-n);
}


///////////////////
// Replace a string in text, returns true, if something was replaced
bool replace(const std::string& text, const std::string& what, const std::string& with, std::string& result)
{
	result = text;
	return replace(result, what, with);
}

///////////////////
// Replace a string in text, returns result, replaces maximally max occurences
std::string replacemax(const std::string& text, const std::string& what, const std::string& with, std::string& result, int max)
{
	result = text;
	
	size_t pos = 0;
	size_t what_len = what.length();
	size_t with_len = with.length();
	if((pos = result.find(what, pos)) != std::string::npos) {
		result.replace(pos, what_len, with);
		pos += with_len;
	}
	
	return result;
}

std::string replacemax(const std::string& text, const std::string& what, const std::string& with, int max) {
	std::string result;
	return replacemax(text, what, with, result, max);
}

///////////////////
// Replace a string in text, returns result, replaces maximally max occurences
// returns true, if at least one replace was made
bool replace(std::string& text, const std::string& what, const std::string& with) {
	// Make sure there is something to replace
	if (!what.size())  {
		return false;
	}
	
	bool one_repl = false;
	size_t pos = 0;
	while((pos = text.find(what, pos)) != std::string::npos) {
		text.replace(pos, what.length(), with);
		pos += with.length();
		one_repl = true;
	}
	return one_repl;
}

// chrcasecmp - like strcasecomp, but for a single char
int chrcasecmp(const char c1, const char c2)
{
	return (tolower(c1) == tolower(c2));
}

//////////////////
// Gets the string [beginning of text,searched character)
std::string ReadUntil(const std::string& text, char until_character) {
	size_t pos = 0;
	for(std::string::const_iterator i = text.begin(); i != text.end(); i++, pos++) {
		if(*i == until_character)
			return text.substr(0, pos);
	}
	return text;
}

std::string ReadUntil(const std::string& text, std::string::const_iterator& it, char until_character, const std::string& alternative) {
	std::string::const_iterator start = it;
	for(; it != text.end(); it++) {
		if(*it == until_character)
			return std::string(start, it);
	}
	return alternative;
}


std::string	ReadUntil(FILE* fp, char until_character) {
	char buf[256];
	std::string res;
	res = "";
	size_t buf_pos = 0;
	while(true) {
		if(fread(&buf[buf_pos],1,1,fp) == 0 || buf[buf_pos] == until_character) {
			res.append(buf,buf_pos);
			break;
		}
		buf_pos++;
		if(buf_pos >= sizeof(buf)) {
			buf_pos = 0;
			res.append(buf,sizeof(buf));
		}
	}
	
	return res;
}

bool PrettyPrint(const std::string& prefix, const std::string& buf, PrintOutFct printOutFct, bool firstLineWithPrefix) {
	std::string::const_iterator it = buf.begin();
	bool firstLine = true;
	while(true) {
		std::string tmp = ReadUntil(buf, it, '\n', std::string(it, buf.end()));
		if(it == buf.end()) {
			if(tmp != "") {
				(*printOutFct) ( (!firstLineWithPrefix && firstLine) ? tmp : (prefix + tmp) );
				return false;
			}
			return !firstLine || firstLineWithPrefix;
		}
		++it;
		(*printOutFct) ( (!firstLineWithPrefix && firstLine) ? (tmp + "\n") : (prefix + tmp + "\n") );
		firstLine = false;
	}
}


Iterator<char>::Ref HexDump(Iterator<char>::Ref start, PrintOutFct printOutFct, size_t mark, size_t count) {
	std::string tmpLeft;
	std::string tmpRight;
	unsigned int tmpChars = 0;
	static const unsigned int charsInLine = 16;
	
	size_t c = 0;
	while(start->isValid() && c < count) {
		unsigned char ch = start->get();
		
		tmpLeft += FixedWidthStr_LeftFill(hex(ch), 2, '0');
		tmpLeft += (c == mark) ? "]" : (c + 1 == mark) ? "[" : " ";
		if(ch >= 32 && ch <= 126)
			tmpRight += ch;
		else
			tmpRight += '.';
		
		tmpChars++;
		if(tmpChars == charsInLine / 2) {
			tmpLeft += "  ";
			tmpRight += " ";
		}
		if(tmpChars == charsInLine) {
			(*printOutFct) ( tmpLeft + "| " + tmpRight + "\n" );
			tmpChars = 0;
			tmpLeft = tmpRight = "";
		}
		
		start->next();
		c++;
	}
	
	tmpLeft += std::string((charsInLine - tmpChars) * 3, ' ');
	tmpRight += std::string((charsInLine - tmpChars), ' ');	
	if(tmpChars < charsInLine / 2) { tmpLeft += "  "; tmpRight += " "; }
	(*printOutFct) ( tmpLeft + "| " + tmpRight + "\n" );
	
	return start;
}


//////////////////////////
// Contains a definition of some common colors, used internally by StrToCol
Color GetColorByName(const std::string& str, bool& fail)
{
	fail = false;
	
	struct ColorEntry  {
		std::string sName;
		Uint8 r, g, b, a;
	};
#define OP SDL_ALPHA_OPAQUE
#define TR SDL_ALPHA_TRANSPARENT
	
	// TODO: more? These are taken from the HTML specification
	static const ColorEntry predefined[] = {
		{ "white", 255, 255, 255,OP },
		{ "black", 0, 0, 0, OP },
		{ "red", 255, 0, 0, OP },
		{ "green", 0, 127, 0, OP },
		{ "blue", 0, 0, 255, OP },
		{ "silver", 0xC0, 0xC0, 0xC0, OP },
		{ "gray", 127, 127, 127, OP },
		{ "grey", 127, 127, 127, OP },
		{ "purple", 127, 0, 127, OP },
		{ "fuchsia", 255, 0, 255, OP },
		{ "pink", 255, 0, 255, OP },
		{ "lime", 0, 255, 0, OP },
		{ "olive", 127, 127, 0, OP },
		{ "yellow", 255, 255, 0, OP },
		{ "navy", 0, 0, 127, OP },
		{ "teal", 0, 127, 127, OP },
		{ "aqua", 0, 255, 255, OP },
		{ "gold", 255, 215, 0, OP },
		{ "transparent", 0, 0, 0, TR }
	};
	
	for (size_t i = 0; i < sizeof(predefined) / sizeof(ColorEntry); ++i)
		if (stringcaseequal(str, predefined[i].sName))
			return Color(predefined[i].r, predefined[i].g, predefined[i].b, predefined[i].a);
	
	fail = true;
	
	return Color();
}

/////////////////////
// Helper function for HexToCol, accepts only adjusted values
static void HexToCol_Pure(const std::string& hex, Color& col, bool& fail)
{
	col.r = MIN(from_string<int>(hex.substr(0,2), std::hex, fail), 255);
	col.g = MIN(from_string<int>(hex.substr(2,2), std::hex, fail), 255);
	col.b = MIN(from_string<int>(hex.substr(4,2), std::hex, fail), 255);
	if (hex.size() >= 8)
		col.a = MIN(from_string<int>(hex.substr(6,2), std::hex, fail), 255);
	else
		col.a = SDL_ALPHA_OPAQUE;
}

////////////////////////
// Helper function for StrToCol
static Color HexToCol(const std::string& hex, bool& fail)
{
	fail = false;
	Color res;
	
	// For example FFF for white
	if (hex.size() == 3)  {
		std::string tmp;
		tmp += hex[0]; tmp += hex[0];
		tmp += hex[1]; tmp += hex[1];
		tmp += hex[2]; tmp += hex[2];
		
		HexToCol_Pure(tmp, res, fail);
		
		return res;
	}
	
	// Same as the above but with alpha
	if (hex.size() == 4)  {
		std::string tmp;
		tmp += hex[0]; tmp += hex[0];
		tmp += hex[1]; tmp += hex[1];
		tmp += hex[2]; tmp += hex[2];
		tmp += hex[3]; tmp += hex[3];
		
		HexToCol_Pure(tmp, res, fail);
		
		return res;
	}
	
	// For example FFFFFF for white
	if (hex.size() == 6)  {
		HexToCol_Pure(hex, res, fail);
		return res;
	}
	
	// Same as the above but with alpha
	if (hex.size() >= 8)  {
		HexToCol_Pure(hex, res, fail);
		return res;
	}
	
	fail = true;
	return Color();
}

std::string ColToHex(Color col) {
	std::string buf;
	buf += FixedWidthStr_LeftFill(itoa(col.r, 16), 2, '0') +
	buf += FixedWidthStr_LeftFill(itoa(col.g, 16), 2, '0') +
	buf += FixedWidthStr_LeftFill(itoa(col.b, 16), 2, '0');
	if(col.a != SDL_ALPHA_OPAQUE)
		buf += FixedWidthStr_LeftFill(itoa(col.a, 16), 2, '0');
	
	return "#" + buf;
}


//////////////////////
// Returns true if the value ends with a percent sign
static bool is_percent(const std::string& str)
{
	if (!str.size())
		return false;
	return (*(str.rbegin())) == '%';
}

///////////////////
// Returns a color defined function-like: rgba(0, 0, 0, 0)
static Color ColFromFunc(const std::string& func, bool& fail)
{
	StringBuf tmp(func);
	
	tmp.trimBlank();
	tmp.adjustBlank();
	std::string func_name = tmp.readUntil('(');
	TrimSpaces(func_name);
	
	// Get the params
	StringBuf params = tmp.readUntil(')');
	std::vector<std::string> tokens = params.splitBy(',');
	if (tokens.size() < 3)  {
		fail = true;
		return Color();
	}
	
	// Adjust the tokens
	for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); it++)  {
		TrimSpaces(*it);
	}
	
	// The param count is >= 3
	Color res;
	res.r = MIN(255, from_string<int>(tokens[0], fail)); if (is_percent(tokens[0])) res.r = (Uint8)MIN(255.0f, (float)res.r * 2.55f);
	res.g = MIN(255, from_string<int>(tokens[1], fail)); if (is_percent(tokens[1])) res.g = (Uint8)MIN(255.0f, (float)res.g * 2.55f);
	res.b = MIN(255, from_string<int>(tokens[2], fail)); if (is_percent(tokens[2])) res.b = (Uint8)MIN(255.0f, (float)res.b * 2.55f);
	if (tokens.size() >= 4 && stringcaseequal(func_name, "rgba"))  {
		res.a = MIN(255, from_string<int>(tokens[3], fail)); if (is_percent(tokens[3])) res.a = (Uint8)MIN(255.0f, (float)res.a * 2.55f);
	} else
		res.a = SDL_ALPHA_OPAQUE;
	
	return res;
}

static Color ColFromSeperatedNums(const std::string& txt, bool& fail) {
	std::vector<std::string> tokens = explode(txt, ",");
	if (tokens.size() < 3)  {
		fail = true;
		return Color();
	}
	
	Color res;
	res.r = MIN(255, from_string<int>(tokens[0], fail)); if (is_percent(tokens[0])) res.r = (Uint8)MIN(255.0f, (float)res.r * 2.55f);
	res.g = MIN(255, from_string<int>(tokens[1], fail)); if (is_percent(tokens[1])) res.g = (Uint8)MIN(255.0f, (float)res.g * 2.55f);
	res.b = MIN(255, from_string<int>(tokens[2], fail)); if (is_percent(tokens[2])) res.b = (Uint8)MIN(255.0f, (float)res.b * 2.55f);
	if(tokens.size() >= 4) {
		res.a = MIN(255, from_string<int>(tokens[3], fail)); if (is_percent(tokens[3])) res.a = (Uint8)MIN(255.0f, (float)res.a * 2.55f);
	} else
		res.a = SDL_ALPHA_OPAQUE;
	
	return res;
}

//////////////////
// Converts a string to a colour
Color StrToCol(const std::string& str, bool& fail) {
	fail = false;
	
	// Create the temp and copy it there
	std::string temp = str;
	TrimSpaces(temp);
	
	if(temp.size() > 0 && temp[temp.size()-1] == ';') {
		temp.erase(temp.size()-1);
		TrimSpaces(temp);
	}
	
	// Check for a blank string
	if (temp.size() == 0)  {
		fail = true;
		return Color();
	}
	
	// Is the # character present?
	if (temp[0] == '#')  { // str != "" here
		temp.erase(0,1);
		stringlwr(temp);
		return HexToCol(temp, fail);
	}
	
	// Is the function-style present?
	if (temp.size() >= 4)
		if (stringcaseequal(temp.substr(0, 4), "rgba"))
			return ColFromFunc(temp, fail);
	if (temp.size() >= 3)
		if (stringcaseequal(temp.substr(0, 3), "rgb"))
			return ColFromFunc(temp, fail);
	
	// like "r,g,b", e.g. this is used in gamescripts
	if(temp.find(",") != std::string::npos)
		return ColFromSeperatedNums(temp, fail);
	
	// Check if it's a known predefined color
	return GetColorByName(str, fail);
}

Color StrToCol(const std::string& str)
{
	bool fail = false;
	return StrToCol(str, fail);
}

short stringcasecmp(const std::string& s1, const std::string& s2) {
	std::string::const_iterator p1, p2;
	p1 = s1.begin();
	p2 = s2.begin();
	short dif;
	while(true) {
		if(p1 == s1.end()) {
			if(p2 == s2.end())
				return 0;
			// not at end of s2
			return -1; // s1 < s2
		}
		if(p2 == s2.end())
			// not at end of s1
			return 1; // s1 > s2
		
		dif = (short)(unsigned char)tolower((unsigned char)*p1) - (short)(unsigned char)tolower((unsigned char)*p2);
		if(dif != 0) return dif; // dif > 0  <=>  s1 > s2
		
		p1++; p2++;
	}
}

bool stringcaseequal(const std::string& s1, const std::string& s2) {
	if (s1.size() != s2.size()) return false;
	return stringcasecmp(s1, s2) == 0;
}

bool subStrEqual(const std::string& s1, const std::string s2, size_t p) {
	if((s1.size() < p || s2.size() < p) && s1.size() != s2.size()) return false; 
	for(size_t i = 0; i < p && i < s1.size(); i++)
		if(s1[i] != s2[i]) return false;
	return true;
}

bool subStrCaseEqual(const std::string& s1, const std::string s2, size_t p) {
	if((s1.size() < p || s2.size() < p) && s1.size() != s2.size()) return false; 
	for(size_t i = 0; i < p && i < s1.size(); i++)
		if(tolower(s1[i]) != tolower(s2[i])) return false;
	return true;
}

static size_t maxStartingEqualStr(const std::list<std::string>& strs, bool caseSensitive) {
	if(strs.size() == 0) return 0;
	
	size_t l = 0;
	while(true) {
		int i = 0;
		char c = 0;
		for(std::list<std::string>::const_iterator it = strs.begin(); it != strs.end(); ++it, ++i) {
			if(it->size() <= l) return l;
			if(i == 0)
				c = (*it)[l];
			else {
				bool equal = false;
				if(caseSensitive) equal = (*it)[l] == c;
				else equal = tolower((*it)[l]) == tolower(c);
				if(!equal) return l;
			}
		}
		
		l++;
	}
}

size_t maxStartingEqualStr(const std::list<std::string>& strs) {
	return maxStartingEqualStr(strs, true);
}

size_t maxStartingCaseEqualStr(const std::list<std::string>& strs) {
	return maxStartingEqualStr(strs, false);
}


std::vector<std::string> explode(const std::string& str, const std::string& delim) {
	std::vector<std::string> result;
	
	size_t delim_len = delim.size();
	std::string rest = str;
	size_t pos;
	while((pos = rest.find(delim)) != std::string::npos) {
		result.push_back(rest.substr(0,pos));
		rest.erase(0,pos+delim_len);
	}
	result.push_back(rest);
	
	return result;
}

// reads up to maxlen-1 chars from fp
void freadstr(std::string& result, size_t maxlen, FILE *fp) {
	if (!fp) return;
	
	char buf[1024];
	size_t ret, c;
	result = "";
	
	for(size_t len = 0; len < maxlen; len += sizeof(buf)) {
		c = MIN(sizeof(buf), maxlen - len);
		ret = fread(buf, 1, c, fp);
		if(ret > 0)
			result.append(buf, ret);
		if(ret < c)
			break;
	}
}


size_t fwrite(const std::string& txt, size_t len, FILE* fp) {
	size_t len_of_txt = MIN(txt.size()+1, len-1);
	size_t ret = fwrite(txt.c_str(), 1, len_of_txt, fp);
	if(ret != len_of_txt)
		return ret;
	for(; len_of_txt < len; len_of_txt++)
		if(fwrite("\0", 1, 1, fp) == 0)
			return len_of_txt;
	return len;
}


size_t findLastPathSep(const std::string& path) {
	size_t slash = path.rfind('\\');
	size_t slash2 = path.rfind('/');
	if(slash == std::string::npos)
		slash = slash2;
	else if(slash2 != std::string::npos)
		slash = MAX(slash, slash2);
	return slash;
}


void stringlwr(std::string& txt) {
	for(std::string::iterator i = txt.begin(); i != txt.end(); i++)
		*i = tolower((unsigned char)*i);
}

std::string stringtolower(const std::string& txt)
{
	std::string res;
	for(std::string::const_iterator i = txt.begin(); i != txt.end(); i++)
		res += tolower((unsigned char)*i);
	return res;
}


bool strincludes(const std::string& str, const std::string& what) {
	return str.find(what) != std::string::npos;
}

std::string GetFileExtension(const std::string& path) {
	std::string filename = GetBaseFilename(path);
	size_t p = filename.rfind('.');
	if(p == std::string::npos) return "";
	return filename.substr(p+1);
}

std::string GetBaseFilename(const std::string& filename) {
	size_t p = findLastPathSep(filename);
	if(p == std::string::npos) return filename;
	return filename.substr(p+1);
}

std::string GetDirName(const std::string& filename) {
	size_t p = findLastPathSep(filename);
	if(p == std::string::npos) return "";
	return filename.substr(0, p);
}

std::string GetBaseFilenameWithoutExt(const std::string& filename) {
	std::string f = GetBaseFilename(filename);
	size_t p = f.rfind('.');
	if(p == std::string::npos) return f;
	return f.substr(0,p);	
}

std::list<std::string> SplitFilename(const std::string& filename, size_t numPartsFromRight) {
	std::list<std::string> ret;
	std::string restFn = filename;
	while(ret.size() < numPartsFromRight) {
		std::string next = GetBaseFilename(restFn);
		ret.push_front(next);
		if(next.size() == restFn.size()) break;
		restFn.erase(restFn.size() - next.size() - 1);
	}
	return ret;
}



void ucfirst(std::string& text)
{
	if (text == "") return;
	
	text[0] = toupper(text[0]);
	bool wasalpha = isalpha((unsigned char)text[0]) != 0;
	
	for (std::string::iterator it=text.begin()+1;it != text.end();it++)  {
		if (isalpha((unsigned char)*it))  {
			if (wasalpha)
				*it = tolower((unsigned char)*it);
			else
				*it = toupper((unsigned char)*it);
			wasalpha = true;
		} else {
			wasalpha = false;
		}
	}
	
	
}


/////////////////////////
// Find a substring in a string
// WARNING: does NOT support UTF8, use Utf8StringCaseFind instead
size_t stringcasefind(const std::string& text, const std::string& search_for)
{
	if (text.size() == 0 || search_for.size() == 0 || search_for.size() > text.size())
		return std::string::npos;
	
	std::string::const_iterator it1 = text.begin();
	std::string::const_iterator it2 = search_for.begin();
	
	size_t number_of_same = 0;
	size_t result = 0;
	
	// Go through the text
	while (it1 != text.end())  {
		char c1 = (char)tolower((unsigned char)*it1);
		char c2 = (char)tolower((unsigned char)*it2);
		
		// The two characters are the same
		if (c1 == c2)  {
			number_of_same++;  // If number of same characters equals to the size of the substring, we've found it!
			if (number_of_same == search_for.size())
				return result - number_of_same + 1;
			it2++;
		} else {
			number_of_same = 0;
			it2 = search_for.begin();
		}
		
		result++;
		it1++;
	}
	
	return std::string::npos; // Not found
}

/////////////////////////
// Find a substring in a string, starts searching from the end of the text
// WARNING: does NOT support UTF8
size_t stringcaserfind(const std::string& text, const std::string& search_for)
{
	// HINT: simply the above one with reverse iterators
	
	if (text.size() == 0 || search_for.size() == 0 || search_for.size() > text.size())
		return std::string::npos;
	
	std::string::const_reverse_iterator it1 = text.rbegin();
	std::string::const_reverse_iterator it2 = search_for.rbegin();
	
	size_t number_of_same = 0;
	size_t result = 0;
	
	// Go through the text
	while (it1 != text.rend())  {
		char c1 = (char)tolower((unsigned char)*it1);
		char c2 = (char)tolower((unsigned char)*it2);
		
		// The two characters are the same
		if (c1 == c2)  {
			number_of_same++;  // If number of same characters equals to the size of the substring, we've found it!
			if (number_of_same == search_for.size())
				return text.size() - result - 1;
			it2++;
		} else {
			number_of_same = 0;
			it2 = search_for.rbegin();
		}
		
		result++;
		it1++;
	}
	
	return std::string::npos; // Not found
}

/////////////////
// Get next word from a string
std::string GetNextWord(std::string::const_iterator it, const std::string& str)
{
	// Check
	if (str == "" || it == str.end())
		return "";
	
	// Check
	if (it == str.end())
		return "";
	
	// Get the word
	std::string res;
	while (it != str.end())  {
		if (isspace((unsigned char)*it))
			return res;
		res += *it;
		it++;
	}
	
	return res;
}

////////////////////////
// Checks for standalone < and > and replaces them with the corresponding entities
std::string HtmlEntityUnpairedBrackets(const std::string &txt)
{
	// Check
	if (!txt.size())
		return "";
	
	// Get the positions of unclosed brackets
	bool wait_for_close = false;
	size_t wait_for_close_pos = 0;
	std::list<size_t> unpaired_pos;
	size_t curpos = 0;
	for (std::string::const_iterator it = txt.begin(); it != txt.end(); it++, curpos++)  {
		if (*it == '<')  {
			if (wait_for_close)
				unpaired_pos.push_back(wait_for_close_pos);
			wait_for_close = true;
			wait_for_close_pos = curpos;
		}
		
		// One character after the < character
		if (wait_for_close && curpos == wait_for_close_pos + 1)  {
			// Make sure it's a a-z A-Z letter or a slash
			if (!((*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z') || *it == '/'))  {
				unpaired_pos.push_back(wait_for_close_pos);
				wait_for_close = false;
				wait_for_close_pos = 0;
			}
		}
		
		// Closing bracket
		if (wait_for_close && *it == '>')  {
			wait_for_close = false;
			wait_for_close_pos = 0;
		}
	}
	
	if (wait_for_close)
		unpaired_pos.push_back(wait_for_close_pos);
	
	// Replace the unclosed brackets with html entities
	std::string result;
	size_t startpos = 0;
	for (std::list<size_t>::iterator it = unpaired_pos.begin(); it != unpaired_pos.end(); it++)  {
		result += txt.substr(startpos, *it - startpos) + "&lt;";
		startpos = *it + 1;
	}
	
	// Last chunk
	if (startpos < txt.size())
		result += txt.substr(startpos);
	
	return result;
}


////////////////////
// Helper function for AutoDetectLinks
static std::string GetLink(std::string::const_iterator& it, const std::string::const_iterator& end, size_t& pos)
{
	/*
	 The URI standard, RFC 2396, <http://www.ietf.org/rfc/rfc2396.txt>
	 
	 ; HTTP
	 
	 httpurl = "http://" hostport [ "/" hpath [ "?" search ]]
	 hpath = hsegment *[ "/" hsegment ]
	 hsegment = *[ char | ";" | ":" | "@" | "&" | "=" ]
	 search = *[ char | ";" | ":" | "@" | "&" | "=" ]
	 
	 lowalpha = ...
	 hialpha = ...
	 digit = ...
	 
	 alpha = lowalpha | hialpha
	 safe = "$" | "-" | "_" | "." | "+"
	 extra = "!" | "*" | "'" | "(" | ")" | ","
	 national = "{" | "}" | "|" | "\" | "^" | "~" | "[" | "]" | "`"
	 punctuation = "<" | ">" | "#" | "%" | <">
	 
	 reserved = ";" | "/" | "?" | ":" | "@" | "&" | "="
	 hex = digit | "A" | "B" | "C" | "D" | "E" | "F" | "a" | "b" | "c" | "d" | "e" | "f"
	 escape = "%" hex hex
	 
	 unreserved = alpha | digit | safe | extra
	 char = unreserved | escape
	 xchar = unreserved | reserved | escape
	 digits = 1*digit
	 */
	
	const std::string valid_url_chars = "/" "%" "?" // reserved
	";" ":" "@" "&" "=" // search
	"$" "-" "_" "." "+" // safe
	"!" "*" "'" "(" ")" "," // extra
	"{" "}" "|" "\\" "^" "~" "[" "]" "`" // national
	"#" "\"";	// punctuation (part of)
	
	std::string link;
	bool was_dot = false;
	bool was_ques = false;
	for (; it != end; it++, ++pos)  {
		// Breaking characters
		if (!isalnum((unsigned char)*it) && valid_url_chars.find(*it) == std::string::npos)  {
			if (was_ques)  {
				link.resize(link.size() - 1);
				it--;
				was_ques = false;
			}
			break;
		}
		
		// Multiple question marks
		if (*it == '?')  {
			if (was_ques)  {
				link.resize(link.size() - 1);
				it--;
				was_ques = false;
				break;
			}
			was_ques = true;
		} else
			was_ques = false;
		
		// Multiple dots
		if (*it == '.')  {
			if (was_dot)  {
				link.resize(link.size() - 1);
				it--;
				was_dot = false;
				break;
			}
			was_dot = true;
		} else
			was_dot = false;
		
		link += *it;
	}
	
	if ((was_ques || was_dot) && link.size())  {
		link.resize(link.size() - 1);
	}
	
	TrimSpaces(link);
	
	return link;
}

//////////////////////////
// Automatically find hyperlinks in the given text and encapsulate them with <a> and </a>
std::string AutoDetectLinks(const std::string& text)
{
	static const std::string prefixes[] = { "www.", "http://", "https://", "mailto:", "ftp://" };
	
	std::string result;
	size_t pos = 0;
	bool in_tag = false;
	for (std::string::const_iterator it = text.begin(); it != text.end(); it++, pos++)  {
		if (*it == '<')  {
			in_tag = true;
			result += *it;
			continue;
		}
		
		if (*it == '>')  {
			in_tag = false;
			result += *it;
			continue;
		}
		
		// Do not search inside html tags
		if (in_tag)  {
			result += *it;
			continue;
		}
		
		for (size_t i = 0; i < sizeof(prefixes)/sizeof(std::string); ++i)  {
			if (text.size() - pos > prefixes[i].size() + 4)  {  // 4 = minimum length of the address, for example a.de
				if (stringcaseequal(text.substr(pos, prefixes[i].size()), prefixes[i]))  {
					
					// Get the link
					std::string link = GetLink(it, text.end(), pos);
					
					// Add the link
					result += "<a href=\"" + link + "\">" + link + "</a>";
					break;
				}
			}
		}
		
		if (it != text.end())
			result += *it;
		else
			break;
	}
	
	return result;
}

std::string EscapeHtmlTags( const std::string & src )
{
	std::string ret ( src );
	
	replace( ret, "&", "&amp;" );
	replace( ret, "<", "&lt;" );
	replace( ret, ">", "&gt;" );
	replace( ret, "\"", "&quot;" );
	replace( ret, "\'", "&apos;" );
	
	return ret;
}







// Base 64 encoding
// Copied from wget sources
std::string Base64Encode(const std::string &data)
{
	std::string dest;
	/* Conversion table.  */
	static const char tbl[64] = {
		'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
		'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
		'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
		'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
	};
	/* Access bytes in DATA as unsigned char, otherwise the shifts below
     don't work for data with MSB set. */
	const unsigned char *s = (const unsigned char *)data.c_str();
	/* Theoretical ANSI violation when length < 3. */
	const unsigned char *end = s + data.length() - 2;
	
	/* Transform the 3x8 bits to 4x6 bits, as required by base64.  */
	for (; s < end; s += 3)
    {
		dest += tbl[s[0] >> 2];
		dest += tbl[((s[0] & 3) << 4) + (s[1] >> 4)];
		dest += tbl[((s[1] & 0xf) << 2) + (s[2] >> 6)];
		dest += tbl[s[2] & 0x3f];
    }
	
	/* Pad the result if necessary...  */
	switch (data.length() % 3)
    {
		case 1:
			dest += tbl[s[0] >> 2];
			dest += tbl[(s[0] & 3) << 4];
			dest += '=';
			dest += '=';
			break;
		case 2:
			dest += tbl[s[0] >> 2];
			dest += tbl[((s[0] & 3) << 4) + (s[1] >> 4)];
			dest += tbl[((s[1] & 0xf) << 2)];
			dest += '=';
			break;
    }
	
	return dest;
}

// Substitute space with + and all non-alphanum symbols with %XX
std::string UrlEncode(const std::string &data)
{
	std::string ret;
	for( size_t f=0; f<data.size(); f++ )
	{
		char c = data[f];
		if( isalnum(c) || c == '.' || c == '-' || c == '_' )
			ret += c;
		else
		{
			std::ostringstream os;
			// unsigned(c) will produce numbers like 0xFFFFFF80 for value -128, so I'm using unsigned((unsigned char)c)
			os << "%" << std::hex << std::setw(2) << std::setfill('0') << unsigned((unsigned char)c); 
			ret += os.str();
		};
	};
	return ret;
}; 




bool strSeemsLikeChatCommand(const std::string& str) {
	if(str.size() == 0) return false;
	if(str[0] == '/') {
		if(str.size() == 1) return true;
		if(str[1] == '/') return false;
		return true;
	}
	return false;
}



/*template<> VectorD2<int> from_string< VectorD2<int> >(const std::string& s, bool& fail) {
	std::string tmp = s;
	TrimSpaces(tmp);
	if(tmp.size() > 2 && tmp[0] == '(' && tmp[tmp.size()-1] == ')')
		tmp = tmp.substr(1, tmp.size() - 2);
	size_t f = tmp.find(',');
	if(f == std::string::npos) { fail = true; return VectorD2<int>(); }
	VectorD2<int> v;
	fail = false;
	v.x = from_string<int>(tmp.substr(0, f), fail);
	if(fail) return VectorD2<int>();
	v.y = from_string<int>(tmp.substr(f + 1), fail);
	if(fail) return VectorD2<int>();
	return v;
}*/
