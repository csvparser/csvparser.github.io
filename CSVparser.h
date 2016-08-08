/******************************************************************

Introduction:
CSVparser.h is a simple c++ libary. It can load, save and modify Microsoft Excel friendly CSV files.
It supports quotation marks, line breaks and commas in cell values.
It uses std::map to store data. It supports up to 2^32 columns and rows.

Usage:
Create a CSVdata:
CSVdata csv;

Load a CSV file:
csv.LoadFile("filename.csv");

Keep current data and load a CSV file:
csv.LoadFile("filename.csv",false);

Save to a CSV file:
csv.SaveFile("filename.csv");

Assign a value to cell:
csv.SetCell(row, column, value);

Get cell value:
value = csv.GetCell(row, column);

Erase a cell:
csv.EraseCell(row, column); // same as SetCell(row, column, "");

Convert a string to a safe CSV string:
value = csv.SafeStr(str);

Check if a cell has value and assign it to a string:
check = csv.Find(row, column, &amp;str);

Get lower and upper bounds:
check = csv.LBElem(&amp;row, &amp;column, &amp;str);
check = csv.UBElem(&amp;row, &amp;column, &amp;str);

Search for a value from position row, column:
check = csv.Search(value, &amp;row, &amp;column);

Iteration:
check = csv.BeginIter(&amp;it);
check = csv.NextIter(&amp;it);
check = csv.GetIter(it, &amp;row, &amp;column, &amp;str);

CSVparser version 1.0 by Hamid Soltani. (yahoo: hsoltanim)
Last modified: Aug. 2016.

*******************************************************************/
#pragma once

#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

using LI = unsigned long int;
using LLI = unsigned long long int;

class CSVdata
{
private:
	map<LLI, string> csv_map;
	union _I {
		LLI index;
		struct {
			LI column;
			LI row;
		} at;
	};
	LLI _index(LI row, LI column)
	{
		_I i;
		i.at.row = row;
		i.at.column = column;
		return i.index;
	}
	LI _row(LLI index)
	{
		_I i;
		i.index = index;
		return i.at.row;
	}
	LI _column(LLI index)
	{
		_I i;
		i.index = index;
		return i.at.column;
	}
public:

	CSVdata()
	{

	}

	int LoadFile(const char* filename, bool isclear = true)
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
				if (qflag & dqflag)
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
				if (qflag & dqflag)
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
					csv_map[_index(saverow, savecolumn)] = cell;
					cell = "";
				}
				cellread = false;
			}
		}

		is.close();
		return 0;
	}

	int SaveFile(const char* filename)
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
			os << it.second.c_str();
		}

		os << "\n";
		os.close();
		return 0;
	}

	string SafeStr(const string s)
	{
		string t;
		unsigned int len = (unsigned)s.length();
		if ((s[0] == '"')&(s[len - 1] == '"'))
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
			while ((!qneed)&(i < len))
			{
				qneed = ((s[i] == ',') | (s[i] == '\n'));
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

	int EraseCell(LI row, LI column)
	{
		csv_map.erase(_index(row, column));
	}

	int SetCell(LI row, LI column, const string value)
	{
		unsigned int len = (unsigned)value.length();
		if (len == 0)
		{
			csv_map.erase(_index(row, column));
		}
		else
		{
			csv_map[_index(row, column)] = SafeStr(value);
			return 0;
		}
	}

	string GetCell(LI row, LI column)
	{
		auto it = csv_map.find(_index(row, column));
		if (it != csv_map.end())
			return it->second;
		else
			return "";
	}

	bool Search(string value, LI& row, LI& column)
	{
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

	bool Find(LI row, LI column, string& value)
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

	bool LBElem(LI& row, LI& column, string& value)
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

	bool UBElem(LI& row, LI& column, string& value)
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

	bool BeginIter(map<LLI, string>::iterator& it)
	{
		it = csv_map.begin();
		return (it != csv_map.end());
	}

	bool NextIter(map<LLI, string>::iterator& it)
	{
		it++;
		return (it != csv_map.end());
	}

	void GetIter(map<LLI, string>::iterator& it, LI& row, LI& column, string& value)
	{
		LLI ind = it->first;
		row = _row(ind);
		column = _column(ind);
		value = it->second;
	}

	int Clear()
	{
		csv_map.clear();
		return 0;
	}

};
