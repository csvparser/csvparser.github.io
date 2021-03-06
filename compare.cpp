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
#define bitsTol (1 << unImpBits)
#define almostZero (bitsTol << 1)

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

bool isequal(double a, double b)
{
	bits64 mx;
	bits64 mn;
	mx.x = a;
	mn.x = b;
	if (mx.i < mn.i)
	{
		mx.x = b;
		mn.x = a;
	}
	// Check if it is zero not considering insignificant bits
	if ((mx.i << 1) < almostZero)
		return ((mn.i << 1) < almostZero);
	return (mx.i < mn.i + bitsTol);
}

int main()
{
	double a = -4;
	double b = -100 / sqrt(5) / sqrt(5) / sqrt(5) / sqrt(5);
	// a = 0.0; b = -0.0;
	bits64 _a, _b;
	_a.x = a;
	_b.x = b;
	bitset<64> __a = _a.i;
	bitset<64> __b = _b.i;
	cout << "        a: " << __a << "\n        b: " << __b << "\n" << "\n";
	_a.x = rounded(a);
	_b.x = rounded(b);
	__a = _a.i;
	__b = _b.i;
	cout << "Rounded a: " << __a << "\nRounded b: " << __b << "\n" << "\n";
	cout << "Comparing a, b:\n";
	cout << "All bits considered: "
		<< ((a == b) ? "Eqaul" : "Not equal") << endl;
	cout << unImpBits << " insignificant bits not considered: "
		<< (isequal(a, b) ? "Eqaul" : "Not equal") << endl;
	cout << "Rounded to " << unImpBits << " bits: " 
		<< ((rounded(a) == rounded(b)) ? "Eqaul" : "Not equal") << endl;
	cout << "Press Enter to exit...\n";
	getchar();
	return 0;
}
