/****************************************************************************
compare is a c++ program to compare two double numbers 
when insignificant bits not considered.
by Hamid Soltani. (gmail: hsoltanim)
https://csvparser.github.io/
Last modified: Sep. 2016.
*****************************************************************************/

#include "stdafx.h"
#include < bitset >
#include < iostream >
#include < math.h >

using namespace std;

#define unImpBits 4

union bits64 {
	double x;
	unsigned long long i;
};

double rounded(double r)
{
	bits64 v;
	v.x = r;
	v.i = (v.i + (1 << (unImpBits - 1))) >> unImpBits << unImpBits;
	return v.x;
}

int main()
{
	double a = 0.2;
	double b = 1 / sqrt(5) / sqrt(5);
	cout << "Comparing a, b:\n";
	cout << "All bits considered: "
		<< ((a == b) ? "Eqaul" : "Not equal") << endl;
	cout << unImpBits << " insignificant bits not considered: " 
		<< ((rounded(a) == rounded(b)) ? "Eqaul" : "Not equal") << endl;
	cout << "Press Enter to exit...\n";
	getchar();
	return 0;
}

