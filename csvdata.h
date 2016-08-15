/****************************************************************************

Introduction:
csvdata is a simple c++ class.
It can load, save and modify Microsoft Excel friendly CSV files.
It supports quotation marks, line breaks and commas in cell values.
It uses std::map to store data. It supports up to 2^32 columns and rows.

Usage:
Convert a string to a safe CSV string:
value = SafeStr(str);

Convert a CSV string to a primary form:
value = PrimaryStr(str);

Create a csvdata:
csvdata csv;

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

Get cell value as a double:
check=GetCellDouble(row, column, &x)

Erase a cell:
csv.EraseCell(row, column); // same as SetCell(row, column, "");

Check if a cell has value and assign it to a string:
check = csv.Find(row, column, &str);

Get lower and upper bounds:
check = csv.LBElem(&row, &column, &str);
check = csv.UBElem(&row, &column, &str);

Search for a value from position row, column:
check = csv.Search(value, &row, &column);

Search for a value from position 0, 0:
check = csv.Search(value, &row, &column, true);

Iteration:
check = csv.BeginIter(&it);
check = csv.NextIter(&it);
csv.GetIter(it, &row, &column, &str);

Clear data:
csv.Clear();

() Operator (read and write):
csv(row, column)

csvdata version 1.2 by Hamid Soltani. (gmail: hsoltanim)
Last modified: Aug. 2016.

*****************************************************************************/

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
union _I {
	LLI index;
	struct {
		LI column;
		LI row;
	} at;
};

/****************************************************************************/

class csvdata
{
private:
	map<LLI, string> csv_map;
	LLI _index(LI row, LI column);
	LI _row(LLI index);
	LI _column(LLI index);
public:
	csvdata();
	~csvdata();
	int LoadFile(const char* filename, bool isclear = true);
	int SaveFile(const char* filename);
	int EraseCell(LI row, LI column);
	int SetCell(LI row, LI column, const string& value);
	string GetCell(LI row, LI column);
	bool GetCellDouble(LI row, LI column, double& x);
	bool Search(const string& value, LI& row, LI& column, bool is_reset = false);
	bool Find(LI row, LI column, string& value);
	bool LBElem(LI& row, LI& column, string& value);
	bool UBElem(LI& row, LI& column, string& value);
	bool BeginIter(map<LLI, string>::iterator& it);
	bool NextIter(map<LLI, string>::iterator& it);
	void GetIter(map<LLI, string>::iterator& it, LI& row, LI& column, string& value);
	int Clear();
	string& operator() (const LI row, const LI column);
};

const string PrimaryStr(const string& s);
const string SafeStr(const string& s);
bool StrDouble(const string s, double& x);
