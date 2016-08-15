#include "stdafx.h"
#include "csvdata.h"

#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

/****************************************************************************/

LLI csvdata::_index(LI row, LI column)
{
	_I i;
	i.at.row = row;
	i.at.column = column;
	return i.index;
}
LI csvdata::_row(LLI index)
{
	_I i;
	i.index = index;
	return i.at.row;
}
LI csvdata::_column(LLI index)
{
	_I i;
	i.index = index;
	return i.at.column;
}

csvdata::csvdata()
{

}

csvdata::~csvdata()
{
	Clear();
}

int csvdata::LoadFile(const char* filename, bool isclear)
{
	if (isclear)
		Clear();
	LI row = 0;
	LI column = 0;

	string cell = "";

	bool cellread = false;
	bool qflag = false;
	bool dqflag = false;

	char c;

	ifstream is(filename);

	if (!is.good())
		return 1;

	while (is.get(c))
	{
		LI saverow = row;
		LI savecolumn = column;
		if (c == '"')
		{
			if (dqflag)
			{
				dqflag = false;
			}
			else
			{
				if (qflag)
				{
					dqflag = true;
				}
				else if (cell.length() == 0)
				{
					qflag = true;
				}
			}
			cell += c;
		}
		else if (c == ',')
		{
			if (qflag && dqflag)
			{
				dqflag = false;
				qflag = false;
				column++;
				cellread = true;
			}
			else if (qflag)
			{
				cell += c;
			}
			else
			{
				column++;
				cellread = true;
			}
		}
		else if (c == '\n')
		{
			if (qflag && dqflag)
			{
				dqflag = false;
				qflag = false;
				row++;
				column = 0;
				cellread = true;
			}
			else if (qflag)
			{
				cell += c;
			}
			else
			{
				row++;
				column = 0;
				cellread = true;
			}
		}
		else if (c != '\r')
		{
			cell += c;
		}

		if (cellread)
		{
			if (cell.length() > 0)
			{
				csv_map[_index(saverow, savecolumn)] = PrimaryStr(cell);
				cell = "";
			}
			cellread = false;
		}
	}

	is.close();
	return 0;
}

int csvdata::SaveFile(const char* filename)
{
	LI row = 0;
	LI column = 0;

	ofstream os(filename);

	if (!os.good())
	{
		return 1;
	}

	for (auto it : csv_map)
	{
		LLI ind = it.first;
		if (row < _row(ind))
		{
			while (row<_row(ind))
			{
				os << "\n";
				row++;
			}
			column = 0;
		}
		while (column < _column(ind))
		{
			os << ",";
			column++;
		}
		os << SafeStr(it.second).c_str();
	}

	os << "\n";
	os.close();
	return 0;
}

int csvdata::EraseCell(LI row, LI column)
{
	csv_map.erase(_index(row, column));
	return 0;
}

int csvdata::SetCell(LI row, LI column, const string& value)
{
	unsigned int len = (unsigned)value.length();
	if (len == 0)
	{
		csv_map.erase(_index(row, column));
		return 1;
	}
	else
	{
		csv_map[_index(row, column)] = value;
		return 0;
	}
}

string csvdata::GetCell(LI row, LI column)
{
	auto it = csv_map.find(_index(row, column));
	if (it != csv_map.end())
		return it->second;
	else
		return "";
}

bool csvdata::GetCellDouble(LI row, LI column, double& x)
{
	auto it = csv_map.find(_index(row, column));
	if (it != csv_map.end())
	{
		return StrDouble(it->second, x);
	}
	else
		return false;
}

bool csvdata::Search(const string& value, LI& row, LI& column, bool is_reset)
{
	if (is_reset)
	{
		row = 0;
		column = 0;
	}
	for (map<LLI, string>::iterator it = csv_map.lower_bound(_index(row, column)); it != csv_map.end(); ++it)
	{
		if (strcmp(it->second.c_str(), value.c_str()) == 0)
		{
			LLI ind = it->first;
			row = _row(ind);
			column = _column(ind);
			return true;
		}
	}
	return false;
}

bool csvdata::Find(LI row, LI column, string& value)
{
	map<LLI, string>::iterator it = csv_map.find(_index(row, column));
	if (it != csv_map.end())
	{
		value = it->second;
		return true;
	}
	else
	{
		return false;
	}
}

bool csvdata::LBElem(LI& row, LI& column, string& value)
{
	map<LLI, string>::iterator it = csv_map.lower_bound(_index(row, column));
	if (it != csv_map.end())
	{
		LLI ind = it->first;
		row = _row(ind);
		column = _column(ind);
		value = it->second;
		return true;
	}
	else
	{
		return false;
	}
}

bool csvdata::UBElem(LI& row, LI& column, string& value)
{
	map<LLI, string>::iterator it = csv_map.upper_bound(_index(row, column));
	if (it != csv_map.end())
	{
		LLI ind = it->first;
		row = _row(ind);
		column = _column(ind);
		value = it->second;
		return true;
	}
	else
	{
		return false;
	}
}

bool csvdata::BeginIter(map<LLI, string>::iterator& it)
{
	it = csv_map.begin();
	return (it != csv_map.end());
}

bool csvdata::NextIter(map<LLI, string>::iterator& it)
{
	it++;
	return (it != csv_map.end());
}

void csvdata::GetIter(map<LLI, string>::iterator& it, LI& row, LI& column, string& value)
{
	row = _row(it->first);
	column = _column(it->first);
	value = it->second;
}

int csvdata::Clear()
{
	csv_map.clear();
	return 0;
}

string& csvdata::operator() (const LI row, const LI column)
{
	return csv_map[_index(row, column)];
}

/****************************************************************************/

const string PrimaryStr(const string& s)
{
	string t;
	unsigned int len = (unsigned)s.length();
	if ((len>0) && (s[0] == '"') && (s[len - 1] == '"'))
		for (unsigned int i = 1; i < len - 1;i++)
		{
			t += s[i];
			if ((s[i] == '"') && (s[i + 1] == '"'))
				i++;
		}
	else
		t = s;
	return t;
}

const string SafeStr(const string& s)
{
	string t;
	unsigned int len = (unsigned)s.length();
	if ((s[0] == '"') && (s[len - 1] == '"'))
	{
		t = "\"";
		for (unsigned int i = 1; i < len - 1;i++)
			if (s[i] == '"')
			{
				t += "\"\"";
				if (s[i + 1] == '"')
					i++;
			}
			else
			{
				t += s[i];
			}
		t += "\"";
	}
	else
	{
		unsigned int i = 0;
		bool qneed = (s[0] == '\"');
		while ((!qneed) && (i < len))
		{
			qneed = ((s[i] == ',') || (s[i] == '\n'));
			i++;
		}
		if (qneed)
		{
			t = "\"";
			for (unsigned int i = 0; i < len;i++)
			{
				if (s[i] == '"')
					t += "\"\"";
				else
					t += s[i];
			}
			t += "\"";
		}
		else
		{
			t = s;
		}
	}
	return t;
}

bool StrDouble(const string s, double& x)
{
	double d;
	try {
		d = stod(s);
	}
	catch (const invalid_argument&) {
		return false;
	}
	catch (const out_of_range&) {
		return false;
	}
	x = d;
	return true;
}
