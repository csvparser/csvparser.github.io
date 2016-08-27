/****************************************************************************

Introduction:
csvdata is a simple c++ class.
It can load, save and modify Microsoft Excel friendly CSV files.
It supports quotation marks, line breaks and commas in cell values.
It uses std::map to store data. It supports up to 2^32 columns and rows.

Usage:

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

Convert a string to a safe CSV string:
value = SafeStr(str);

Convert a CSV string to a primary form:
value = PrimaryStr(str);

csvdata version 1.3 by Hamid Soltani. (gmail: hsoltanim)
https://csvparser.github.io/
Last modified: Aug. 2016.

*****************************************************************************/

#include "stdafx.h"

#include < map >

#include < fstream >
#include < string >

// header files used by main() function
#include < iostream >
#include < stdlib.h >
#include < stdio.h >
#include < ctime >

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
	std::map< LLI, std::string > csv_map;
	LLI _index(LI row, LI column);
	LI _row(LLI index);
	LI _column(LLI index);
public:
	csvdata();
	~csvdata();
	int LoadFile(const char* filename, bool isclear = true);
	int SaveFile(const char* filename);
	int EraseCell(LI row, LI column);
	int SetCell(LI row, LI column, const std::string& value);
	std::string GetCell(LI row, LI column);
	bool GetCellDouble(LI row, LI column, double& x);
	bool Search(const std::string& value, LI& row, LI& column, bool is_reset = false);
	bool Find(LI row, LI column, std::string& value);
	bool LBElem(LI& row, LI& column, std::string& value);
	bool UBElem(LI& row, LI& column, std::string& value);
	bool BeginIter(std::map< LLI, std::string >::iterator& it);
	bool NextIter(std::map< LLI, std::string >::iterator& it);
	void GetIter(std::map< LLI, std::string >::iterator& it, LI& row, LI& column, std::string& value);
	int Clear();
	std::string& operator() (const LI row, const LI column);
};

const std::string PrimaryStr(const std::string& s);
const std::string SafeStr(const std::string& s);
bool StrDouble(const std::string s, double& x);

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

	bool qflag = false;

	char c;

	ifstream is(filename);

	if (!is.good())
		return 1;

	while (is.get(c))
	{
		if (qflag)
		{
			if (c == '"')
			{
				if (is.peek() == '"')
				{
					is.get(c);
					cell += c;
				}
				else
					qflag = false;
			}
			else
				cell += c;
		}
		else
		{
			if ((c == '"') && (cell.length() == 0))
				qflag = true;
			else if (c == ',')
			{
				if (cell.length() > 0)
				{
					csv_map[_index(row, column)] = cell;
					cell = "";
				}
				column++;
			}
			else if (c == '\n')
			{
				if (cell.length() > 0)
				{
					csv_map[_index(row, column)] = cell;
					cell = "";
				}
				row++;
				column = 0;
			}
			else if (c >= 32)
				cell += c;
		}
	}
	if (cell.length() > 0)
		csv_map[_index(row, column)] = cell;

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
	for (map< LLI, string >::iterator it = csv_map.lower_bound(_index(row, column)); it != csv_map.end(); ++it)
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
	map< LLI, string >::iterator it = csv_map.find(_index(row, column));
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
	map< LLI, string >::iterator it = csv_map.lower_bound(_index(row, column));
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
	map< LLI, string >::iterator it = csv_map.upper_bound(_index(row, column));
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

bool csvdata::BeginIter(map< LLI, string >::iterator& it)
{
	it = csv_map.begin();
	return (it != csv_map.end());
}

bool csvdata::NextIter(map< LLI, string >::iterator& it)
{
	it++;
	return (it != csv_map.end());
}

void csvdata::GetIter(map< LLI, string >::iterator& it, LI& row, LI& column, string& value)
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

// example of using csvdata class
int main()
{
	csvdata csv;

	// assigning some data

	csv.SetCell(0, 0, "Multiplication Table:");
	for (int i = 1;i < 10;i++)
		for (int j = 1;j < 10;j++)
			csv.SetCell(i, j, to_string(i*j));

	csv.SetCell(12, 0, "Some Tests:");

	csv.SetCell(13, 1, "Comma, Test 1");
	csv.SetCell(13, 2, ",Comma Test 2");
	csv.SetCell(13, 3, "\"Comma Test 3,\"");
	csv.SetCell(13, 4, "\"Comma, Test 4\"");
	csv.SetCell(13, 5, "\",Comma Test 5\"");
	csv.SetCell(13, 6, "Comma Test 6,");

	csv.SetCell(14, 1, "Qutation\" Test 1");
	csv.SetCell(14, 2, "\"Qutation Test 2");
	csv.SetCell(14, 3, "Qutation Test 3\"");
	csv.SetCell(14, 4, "\"Qutation\" Test 4\"");
	csv.SetCell(14, 5, "\"\"Qutation Test 5\"");
	csv.SetCell(14, 6, "\"Qutation Test 6\"\"");

	csv.SetCell(15, 1, "Line break\n Test 1");
	csv.SetCell(15, 2, "\nLine break Test 2");
	csv.SetCell(15, 3, "Line break Test 3\n");
	csv.SetCell(15, 4, "\"Line break\n Test 4\"");
	csv.SetCell(15, 5, "\"\nLine break Test 5\"");
	csv.SetCell(15, 6, "\"Line break Test 6\n\"");

	csv.SetCell(16, 1, "Old Value");
	csv.SetCell(16, 1, "New Value");

	csv.SetCell(16, 2, "To be Erased!");

	csv(20, 1) = "By operator!";

	//save and load files
	csv.SaveFile("1.csv");

	csv.Clear();

	csv.SetCell(10, 8, "Date:");
	csv.SetCell(18, 8, "Date:");

	csv.SaveFile("2.csv");

	csv.LoadFile("1.csv");
	csv.LoadFile("2.csv", false);

	csv.EraseCell(16, 2);

	// assign current date after cell "Date:"
	struct tm newtime;
	__time64_t long_time;
	char timebuf[40];
	_time64(&long_time);
	_localtime64_s(&newtime, &long_time);
	strftime(timebuf, 40, "%m/%d/%Y %H:%M:%S", &newtime);

	LI row = 0;
	LI column = 0;
	string value;
	while (csv.Search("Date:", row, column))
	{
		csv.SetCell(row, column + 1, timebuf);
		column++;
	}

	// display method 1
	row = 0;
	column = 0;
	while (csv.LBElem(row, column, value))
	{
		cout << row << ", " << column << ": " << value.c_str() << endl;
		column++;
	}

	// display method 2
	map< LLI, string >::iterator it;
	for (bool csvchk = csv.BeginIter(it);csvchk;csvchk = csv.NextIter(it))
	{
		csv.GetIter(it, row, column, value);
		cout << row << ", " << column << ": " << value.c_str() << endl;
	}

	csv.SaveFile("3.csv");

	puts("Press Enter to exit...\n");
	getchar();

	return 0;
}