// CSVexample.cpp
// Save and load files: 1.csv, 2.csv, 3.csv
// Display CSV data

#include "stdafx.h"
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>

#include "CSVparser.h"

int main()
{
	CSVdata csv;

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
	map<LLI, string>::iterator it;
	for (bool csvchk = csv.BeginIter(it);csvchk;csvchk = csv.NextIter(it))
	{
		csv.GetIter(it, row, column, value);
		cout << row << ", " << column << ": " << value.c_str() << endl;
	}

	csv.SaveFile("3.csv");

	puts("Press Enter to exit...\n 8");
	//puts(csv(21, 1).c_str());
	getchar();

	return 0;
}

