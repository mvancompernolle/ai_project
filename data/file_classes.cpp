/**************************************************************

file_classes.h (C-Munipack project)
Catalogs of variable stars
Copyright (C) 2010 David Motl, dmotl@volny.cz

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

**************************************************************/
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "file_classes.h"

#define BUFSIZE		4096
#define MAX_LINE	65535
#define ALLOC_BY	64

//----------------------   HELPER CLASSES AND FUNCTIONS   -------------------------------

//
// File reader class
//
// This class can read a lines from a text file
// The length of a single line is limited to MAX_LINE characters (excluding line delimiter and null terminator)
// When a longer line occurs in a file, it is truncated.
// The file can contain the BOM at the beginning.
//
class CFileReader
{
public:
	// Constructor
	CFileReader(FILE *f):m_BufPtr(NULL), m_BufEnd(NULL), m_File(f), m_Eof(false)
	{
		size_t bytes = fread(m_Buf, 1, 3, m_File);
		m_BufEnd = m_Buf + bytes;
		if (bytes==3 && m_Buf[0]==0xEF && m_Buf[1]==0xBB && m_Buf[2]==0xBF) 
			m_BufPtr = m_BufEnd;
		else 
			m_BufPtr = m_Buf;
	}

	// Get next line (store the line to buffer)
	// If the line is longer than the buffer, it is truncated
	// Returns number of characters stored to the buffer without trailing nul character
	// or -1 if the end of file was reached
	int NextLine(char *buf, size_t bufsize)
	{
		int state = 0;
		char *lineptr = buf;
		size_t length;

		length = 0;
		while (true) {
			if (m_BufPtr >= m_BufEnd) {
				size_t bytes = fread(m_Buf, 1, BUFSIZE, m_File);
				if (bytes==0) {
					m_Eof = true;
					break;
				}
				m_BufPtr = m_Buf;
				m_BufEnd = m_Buf + bytes;
			}
			char ch = *m_BufPtr;
			if (state==0) {
				if (ch=='\xD') {
					state = 1;
				} else 
				if (ch=='\xA')  {
					m_BufPtr++;
					break;
				} else {
					if (length+1 < bufsize) {
						*lineptr++ = ch;
						length++;
					}
				}
			} else {
				if (ch=='\xA')
					m_BufPtr++;
				break;
			}
			m_BufPtr++;
		}
		if (!m_Eof || length>0) {
			*lineptr = '\0';
			return length;
		}
		return -1;
	}

private:
	unsigned char	m_Buf[BUFSIZE], *m_BufPtr, *m_BufEnd;
	FILE			*m_File;
	bool			m_Eof;
};

// 
// Make copy of a string
//
static char *StrDup(const char *str)
{
	if (str) {
		size_t len = strlen(str);
		char *buf = (char*)malloc((len+1)*sizeof(char));
		if (buf)
			strcpy(buf, str);
		return buf;
	}
	return NULL;
}

//----------------------   dBase FILE READER   -------------------------------

// Constructor
CDBFReader::CDBFReader(FILE *f):m_File(f), m_Cols(NULL), m_Record(NULL), 
	m_ColCount(0)
{
	char fielddef[32];

	memset(&m_Header, 0, sizeof(tDBFHeader));
	
	// Read file header
	if (fread(&m_Header, sizeof(tDBFHeader), 1, f)==1) {
		// Read field definitions
		int count = (m_Header.head_len - sizeof(tDBFHeader))/32;
		if (count>0 && count<256) {
			m_Cols = static_cast<tDBFField*>(malloc(count*sizeof(tDBFField)));
			m_ColCount = 0;
			for (int i=0, start=1; i<count; i++) {
				if (fread(fielddef, sizeof(fielddef), 1, f)==1) {
					int j = 0;
					while (j<11 && fielddef[j]!='\0') {
						m_Cols[m_ColCount].name[j] = fielddef[j];
						j++;
					}
					m_Cols[m_ColCount].name[j] = '\0';
					m_Cols[m_ColCount].length = fielddef[16];
					m_Cols[m_ColCount].start = start;
					start += fielddef[16];
					m_ColCount++;
				}
			}
			fseek(m_File, m_Header.head_len, SEEK_SET);
			m_Record = (char*)malloc(m_Header.rec_length);
		}
	}
}

// Destructor
CDBFReader::~CDBFReader()
{
	free(m_Cols);
	free(m_Record);
}

// Get next m_Record
bool CDBFReader::Next()
{
	if (m_File && m_Record) 
		return fread(m_Record, m_Header.rec_length, 1, m_File)==1;
	return false;
}

// Get field name
const char *CDBFReader::FieldName(int col) const
{
	if (col>=0 && col<m_ColCount)
		return m_Cols[col].name;
	return NULL;
}

// Get field value
bool CDBFReader::GetField(int col, char *m_Buf, size_t buflen)
{
	if (m_Record && col>=0 && col<m_ColCount) {
		int start = (int)m_Cols[col].start;
		int length = (int)m_Cols[col].length;
		int pos = (int)strspn(m_Record + start, " ");
		if (pos >= length) 
			length = 0;
		else
			length -= pos;
		if (length>0) {
			if (length+1 > (int)buflen) {
				memcpy(m_Buf, m_Record+start+pos, buflen-1);
				length = buflen-1;
			} else {
				memcpy(m_Buf, m_Record+start+pos, length);
				while (length>0 && m_Buf[length-1]==' ')
					length--;
			}
		}
		m_Buf[length] = '\0';
		return true;
	} else {
		m_Buf[0] = '\0';
		return false;
	}
}

//--------------------   TEXT FILE READER   -----------------------

// Constructor
CTextReader::CTextReader(FILE *f):m_Line(NULL), m_LineLength(0)
{
	m_Reader = new CFileReader(f);
	m_Line = (char*)malloc(MAX_LINE+1);
}

// Destructor
CTextReader::~CTextReader(void)
{
	delete m_Reader;
	free(m_Line);
}

// Read next line
bool CTextReader::ReadLine(void)
{
	int length = m_Reader->NextLine(m_Line, MAX_LINE+1);
	while (length>=0) {
		if (length>0) {
			m_LineLength = length;
			return true;
		}
		length = m_Reader->NextLine(m_Line, MAX_LINE+1);
	}
	m_LineLength = 0;
	return false;
}

// Get field
bool CTextReader::GetField(int start, int length, char *m_Buf, int buflen) const
{
	if (start < (int)m_LineLength) {
		if (start+length > (int)m_LineLength)
			length = m_LineLength - start;
		if (length >= buflen)
			return false;
		int pos = (int)strspn(m_Line + start, " ");
		if (pos >= length) {
			m_Buf[0] = '\0';
		} else {
			memcpy(m_Buf, m_Line+start+pos, length-pos);
			int i = length-pos;
			while (i>0 && m_Buf[i-1]==' ')
				i--;
			m_Buf[i] = '\0';
		}
		return true;
	}
	m_Buf[0] = '\0';
	return false;
}

//----------------------------   CSV FILE READER   ---------------------------

//
// Constructor
//
CCSVFileBase::CCSVFileBase(char field_sep, char quote_char):m_FieldSep(field_sep), 
	m_QuoteChar(quote_char) 
{
	memset(&m_Header, 0, sizeof(tCSVHeader));
}

//
// Destructor
//
CCSVFileBase::~CCSVFileBase() 
{
	ClearHeader(m_Header);
}

//
// Quote a string if necessary
//
char *CCSVFileBase::StrQuote(const char *str, bool quote_always)
{
	if (!str)
		return NULL;

	bool quote = quote_always;
	if (!quote_always) {
		for (const char *sptr=str; *sptr!='\0'; sptr++) {
			char ch = *sptr;
			if (ch == m_FieldSep || ch == m_QuoteChar) {
				quote = true;
				break;
			}
		}
	}

	if (quote) {
		int length = 2;
		for (const char *sptr=str; *sptr!='\0'; sptr++) {
			char ch = *sptr;
			if (ch == m_QuoteChar) 
				length++;
			length++;
		}
		char *buf = (char*)malloc((length+1)*sizeof(char));
		char *dptr = buf;
		*dptr++ = m_QuoteChar;
		for (const char *sptr=str; *sptr!='\0'; sptr++) {
			char ch = (char)(*sptr);
			if (ch == m_QuoteChar) 
				*dptr++ = m_QuoteChar;
			*dptr++ = ch;
		}
		*dptr++ = m_QuoteChar;
		*dptr = '\0';
		return buf;
	} else {
		int length = strlen(str);
		char *buf = (char*)malloc((length+1)*sizeof(char));
		strcpy(buf, str);
		return buf;
	}
}

char *CCSVFileBase::StrUnquote(const char *str)
{
	int state, length;
	const char *ptr;
	char *buf, *dptr;

	if (!str)
		return NULL;

	// Compute length required for an unquoted string
	length = state = 0;
	for (ptr=str; *ptr!='\0'; ptr++) {
		unsigned char ch = (unsigned char)(*ptr);
		switch (state)
		{
		case 0:
			if (ch==m_QuoteChar) {
				state = 1;
			} else if (ch>' ') {
				state = 3;
				length = 1;
			}
			break;

		case 1:
			if (ch==m_QuoteChar)
				state = 2;
			else
				length++;
			break;

		case 2:
			if (ch==m_QuoteChar) {
				length++;
				state = 1;
			} else 
				state = 4;
			break;

		case 3:
			length++;
			break;

		case 4:
			break;
		}
	}
	
	// Unquote string
	dptr = buf = (char*)malloc((length+1)*sizeof(char));
	length = state = 0;
	for (ptr=str; *ptr!='\0'; ptr++) {
		unsigned char ch = (unsigned char)(*ptr);
		switch (state)
		{
		case 0:
			if (ch==m_QuoteChar) {
				state = 1;
			} else if (ch>' ') {
				state = 3;
				*dptr++ = ch;
				length++;
			}
			break;

		case 1:
			if (ch==m_QuoteChar)
				state = 2;
			else {
				*dptr++ = ch;
				length++;
			}
			break;

		case 2:
			if (ch==m_QuoteChar) {
				*dptr++ = m_QuoteChar;
				length++;
				state = 1;
			} else 
				state = 4;
			break;

		case 3:
			*dptr++ = ch;
			length++;
			break;

		case 4:
			break;
		}
	}
	// For unquoted strings, trim trailing white spaces
	if (state==3) {
		dptr--;
		while (dptr>=buf && *dptr<=' ') {
			dptr--;
			length--;
		}
		dptr++;
	}
	*dptr = '\0';
	return buf;
}

char *CCSVFileBase::Implode(const tCSVRecord &data)
{
	int i, count;
	char *ptr, *buf;

	char **list = (char**)malloc(data.count*sizeof(char*));
	count = 0;
	for (i=0; i<data.count; i++) {
		if (data.list[i]) {
			list[i] = StrQuote(data.list[i], false);
			count = i+1;
		} else {
			list[i] = NULL;
		}
	}
	size_t length = 1;
	for (i=0; i<count; i++) {
		if (length>0)
			length++;
		if (list[i])
			length += strlen(list[i]);
	}
	ptr = buf = (char*)malloc((length+1)*sizeof(char));
	for (i=0; i<count; i++) {
		if (ptr>buf) 
			*ptr++ = m_FieldSep;
		if (list[i]) {
			strcpy(ptr, list[i]);
			ptr += strlen(ptr);
			free(list[i]);
		}
	}
	*ptr = '\0';
	free(list);

	return buf;
}

bool CCSVFileBase::IsValid(const char *buf) const
{
	const char *ptr;

	for (ptr=buf; *ptr!='\0'; ptr++) {
		unsigned char ch = (unsigned char)(*ptr);
		if (ch=='#')
			return false;
		else if (ch==m_FieldSep || ch==m_QuoteChar || ch>' ')
			return true;
	}
	return false;
}

void CCSVFileBase::Explode(const char *str, tCSVRecord &data)
{
	char *buf, *start, *ptr;

	data.count = 0;
	if (str) {
		buf = (char*)malloc((strlen(str)+1)*sizeof(char));
		strcpy(buf, str);
		int state = 0, column = 0;
		for (ptr=start=buf; *ptr!='\0'; ptr++) {
			unsigned char ch = (unsigned char)(*ptr);
			switch (state)
			{
			case 0:
				if (ch=='#') {
					// Comment line
					state = 9;
					break;
				} else if (ch==m_FieldSep) {
					*ptr = '\0';
					SetData(data, column++, StrUnquote(start));
					start = ptr+1;
					state = 0;
				} else if (ch==m_QuoteChar) 
					state = 1;
				else if (ch>' ')
					state = 3;
				break;

			case 1:
				if (ch==m_QuoteChar)
					state = 2;
				break;

			case 2:
				if (ch==m_FieldSep) {
					*ptr = '\0';
					SetData(data, column++, StrUnquote(start));
					start = ptr+1;
					state = 0;
				} else if (ch==m_QuoteChar) {
					state = 1;
				} else
					state = 4;
				break;

			case 3:
				if (ch==m_FieldSep) {
					*ptr = '\0';
					SetData(data, column++, StrUnquote(start));
					start = ptr+1;
					state = 0;
				}
				break;
				
			case 4:
				if (ch==m_FieldSep) {
					*ptr = '\0';
					SetData(data, column++, StrUnquote(start));
					start = ptr+1;
					state = 0;
				}
				break;
			}
		}
		// Last field
		if (state==2 || state==3) 
			SetData(data, column, StrUnquote(start));
		free(buf);
	}
}

void CCSVFileBase::SetData(tCSVRecord &data, int column, char *buf)
{
	if (column>=0) {
		if (column >= data.capacity) {
			data.capacity = column+1;
			data.list = (char**)realloc(data.list, data.capacity*sizeof(char*));
		}
		if (column > data.count)
			memset(data.list+data.count, 0, (column-data.count)*sizeof(char*));
		if (column >= data.count) {
			data.list[column] = buf;
			data.count = column+1;
		} else {
			free(data.list[column]);
			data.list[column] = buf;
		}
	}
}

void CCSVFileBase::ClearData(tCSVRecord &data)
{
	for (int i=0; i<data.count; i++)
		free(data.list[i]);
	free(data.list);
	data.count = data.capacity = 0;
	data.list = NULL;
}

void CCSVFileBase::ClearHeader(tCSVHeader &data)
{
	for (int i=0; i<data.count; i++)
		free(data.fields[i]);
	free(data.fields);
	data.count = data.capacity = 0;
	data.fields = NULL;
}

void CCSVFileBase::ParseHeader(const char *buf, tCSVHeader &hdr)
{
	tCSVRecord rec;

	ClearHeader(hdr);

	memset(&rec, 0, sizeof(tCSVRecord));
	Explode(buf, rec);
	if (rec.count>0) {
		m_Header.count = m_Header.capacity = rec.count;
		m_Header.fields = rec.list;
	}
}

char *CCSVFileBase::MakeHeader(const tCSVHeader &hdr)
{
	tCSVRecord rec;

	memset(&rec, 0, sizeof(tCSVRecord));
	rec.count = m_Header.count;
	rec.list = m_Header.fields;
	return Implode(rec);
}

//----------------------------   CSV FILE READER   ---------------------------

// Constructor
CCSVReader::CCSVReader(FILE *f, char field_sep, char quote_char):CCSVFileBase(field_sep, quote_char)
{
	char buf[MAX_LINE+1];

	memset(&m_Rec, 0, sizeof(tCSVRecord));

	// First line is a header
	m_Reader = new CFileReader(f);
	while (m_Reader->NextLine(buf, MAX_LINE+1)>=0) {
		if (IsValid(buf)) {
			tCSVRecord hdr;
			memset(&hdr, 0, sizeof(tCSVRecord));
			Explode(buf, hdr);
			if (hdr.count>0) {
				m_Header.count = m_Header.capacity = hdr.count;
				m_Header.fields = (char**)calloc(m_Header.capacity, sizeof(char*));
				for (int i=0; i<hdr.count; i++) 
					m_Header.fields[i] = hdr.list[i];
				free(hdr.list);
			}
			break;
		}
	}
}

// Destructor
CCSVReader::~CCSVReader()
{
	ClearData(m_Rec);
	delete m_Reader;
}

// Read next line
bool CCSVReader::ReadLine(void)
{
	char buf[MAX_LINE+1];

	ClearData(m_Rec);
	while (m_Reader->NextLine(buf, MAX_LINE+1)>=0) {
		if (IsValid(buf)) {
			Explode(buf, m_Rec);
			return true;
		}
	}
	return false;
}

const char *CCSVReader::ColumnName(int column) const
{
	if (column>=0 && column<m_Header.count)
		return m_Header.fields[column];
	return NULL;
}

const char *CCSVReader::GetStr(int column, const char *defval) const
{
	if (column>=0 && column<m_Header.count && column<m_Rec.count) 
		return m_Rec.list[column];
	return NULL;
}

bool CCSVReader::GetInt(int column, int *value) const
{
	char *endptr;
	const char *str = GetStr(column);
	if (str) {
		int val = strtol(str, &endptr, 10);
		if (endptr!=str) {
			*value = val;
			return true;
		}
	}
	*value = 0;
	return false;
}

int CCSVReader::GetInt(int column, int defval) const
{
	char *endptr;
	const char *str = GetStr(column);
	if (str) {
		int val = strtol(str, &endptr, 10);
		if (endptr!=str) 
			return val;
	}
	return defval;
}

bool CCSVReader::GetDbl(int column, double *value) const
{
	char *endptr;
	const char *str = GetStr(column);
	if (str) {
		double val = strtod(str, &endptr);
		if (endptr!=str) {
			*value = val;
			return true;
		}
	}
	*value = 0;
	return false;
}

double CCSVReader::GetDbl(int column, double defval) const
{
	char *endptr;
	const char *str = GetStr(column);
	if (str) {
		double val = strtod(str, &endptr);
		if (str!=endptr)
			return val;
	}
	return defval;
}

bool CCSVReader::GetBool(int column, bool *value) const
{
	char *endptr;
	const char *str = GetStr(column);
	if (str) {
		bool val = strtol(str, &endptr, 10)!=0;
		if (endptr!=str) {
			*value = val;
			return true;
		}
	}
	*value = false;
	return false;
}

bool CCSVReader::GetBool(int column, bool defval) const
{
	char *endptr;
	const char *str = GetStr(column);
	if (str) {
		bool val = strtol(str, &endptr, 10)!=0;
		if (str!=endptr)
			return val;
	}
	return defval;
}

//----------------------------   CSV FILE WRITER   ------------------------------------

// Constructor
CCSVWriter::CCSVWriter(FILE *f, char field_sep, char quote_char, bool save_no_header):CCSVFileBase(field_sep, quote_char), 
	m_File(f), m_NoHeader(save_no_header), m_State(EMPTY_FILE)
{
	memset(&m_Rec, 0, sizeof(tCSVRecord));
}

// Destructor
CCSVWriter::~CCSVWriter()
{
	Close();
	ClearData(m_Rec);
}

// Flush unsaved data and close the file
void CCSVWriter::Close(void)
{
	if (m_File) {
		Flush();
		m_File = NULL;
	}
}

int CCSVWriter::AddColumn(const char *name)
{
	int index = -1;

	assert(name != NULL);
	assert(m_State == EMPTY_FILE || m_State == HEADER);
	
	if (name) {
		size_t len = strlen(name);
		if (m_Header.count >= m_Header.capacity) {
			m_Header.capacity += ALLOC_BY;
			m_Header.fields = (char**)realloc(m_Header.fields, m_Header.capacity*sizeof(char*));
		}
		index = m_Header.count;
		m_Header.fields[index] = (char*)malloc((len+1)*sizeof(char));
		strcpy(m_Header.fields[index], name);
		m_Header.count++;
		m_State = HEADER;
	}
	return index;
}

void CCSVWriter::Append(void)
{
	assert(m_State == HEADER || m_State == DATA);
	Flush();
}

void CCSVWriter::Flush(void)
{
	if (m_State == HEADER && !m_NoHeader) {
		// Write file header
		char *buf = MakeHeader(m_Header);
		if (buf) {
			fprintf(m_File, "%s\n", buf);
			free(buf);
		}
	}
	if (m_State == DATA) {
		// Write single line
		char *buf = Implode(m_Rec);
		if (buf) {
			fprintf(m_File, "%s\n", buf);
			free(buf);
		}
	}
	ClearData(m_Rec);
	m_State = EMPTY_DATA;
}

void CCSVWriter::SetStr(int column, const char *data)
{
	assert(column>=0 && column<m_Header.count);
	assert(m_State == EMPTY_DATA || m_State == DATA);
	if (column>=0 && column<m_Header.count) {
		SetData(m_Rec, column, StrDup(data));
		m_State = DATA;
	}
}

void CCSVWriter::SetInt(int column, int value)
{
	char buf[256];

	sprintf(buf, "%d", value);
	SetStr(column, buf);
}

void CCSVWriter::SetDbl(int column, double value, int prec)
{
	char buf[256];

	sprintf(buf, "%.*f", prec, value);
	SetStr(column, buf);
}

void CCSVWriter::SetBool(int column, bool value)
{
	SetStr(column, (value ? "1" : "0"));
}
