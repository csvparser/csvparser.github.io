/*
solve is a modified version of matrix2 program.
It can efficiently solve matrix systems.
Modified by by Hamid Soltani. (gmail: hsoltanim)
https://csvparser.github.io/
Last modified: Sep. 2016.
*/

#include "stdafx.h"

#include < cstdlib >
#include < cstdio >
#include < math.h >
#include < map >

#include < iostream >
#include < iomanip > 
#include < time.h >

using namespace std;

using Dimension = unsigned long int;
using Index = unsigned long long int;
union Access {
	Index i;
	struct {
		Dimension c;
		Dimension r;
	} at;
};

inline Index getMatrixIndex(Dimension r, Dimension c)
{
	union Access m;
	m.at.r = r;
	m.at.c = c;
	return m.i;
}

inline void getMatrixRC(Index i, Dimension& r, Dimension& c)
{
	union Access m;
	m.i = i;
	r = m.at.r;
	c = m.at.c;
}

static double lastDet; // Determinant computed by Inv function

// error codes
#define MER_OUT_OF_RANGE 1
#define MER_INVALID_DIMS 2
#define MER_ZERO_DET 3
#define MER_NOT_SQUARE 4

// a simple exception class
class Exception
{
public:
	const int code;
	Exception(const int arg) : code(arg) {	}
};


class Matrix
{
private:
	Dimension rows;
	Dimension cols;
	map< Index, double > mp;

public:
	// constructor
	Matrix()
	{
		rows = 0;
		cols = 0;
	}

	// constructor
	Matrix(const Dimension row_count, const Dimension column_count)
	{
		// create a Matrix object with given number of rows and columns
		rows = row_count;
		cols = column_count;
	}

	// assignment operator
	Matrix(const Matrix& a)
	{
		rows = a.rows;
		cols = a.cols;
		mp = a.mp;
	}

	// index operator. You can use this class like myMatrix(col, row)
	// the indexes are one-based, not zero based.
	const double operator()(const Dimension r, const Dimension c) const
	{
		if (r <= 0 || r > rows || c <= 0 || c > cols)
			throw Exception(MER_OUT_OF_RANGE);
		auto it = mp.find(getMatrixIndex(r, c));
		return ((it != mp.end()) ? it->second : 0.0);
	}

	// set matrix element
	void set(const Dimension r, const Dimension c, const double v)
	{
		if (r <= 0 || r > rows || c <= 0 || c > cols)
			throw Exception(MER_OUT_OF_RANGE);
		if (v == 0.0) 
			mp.erase(getMatrixIndex(r, c)); 
		else 
			mp[getMatrixIndex(r, c)] = v;
	}

	// begin iteration, next element after [r, c]
	bool beginIter(map< Index, double >::iterator& it, Dimension& r, Dimension& c, double& v)
	{
		it = mp.upper_bound(getMatrixIndex(r, c));
		if (it == mp.end())
			return false;
		getMatrixRC(it->first, r, c);
		v = it->second;
		return true;
	}

	// next iteration
	bool nextIter(map< Index, double >::iterator& it, Dimension& r, Dimension& c, double& v)
	{
		if (++it == mp.end())
			return false;
		getMatrixRC(it->first, r, c);
		v = it->second;
		return true;
	}

	// assignment operator
	Matrix& operator= (const Matrix& a)
	{
		rows = a.rows;
		cols = a.cols;
		mp = a.mp;
		return *this;
	}

	// returns the number of rows
	inline Dimension GetRows() const { return rows; }

	// returns the number of columns
	inline Dimension GetCols() const { return cols; }

	// output operator
	friend ostream& operator<<(ostream& os, const Matrix& M)
	{
		for (Dimension r = 1; r <= M.rows; r++)
			for (Dimension c = 1; c <= M.cols; c++)
				os << ((c == 1) ? ((r == 1) ? "[" : " ") : "")
					<< setw(8) << setprecision(4) << M(r, c)
					<< ((c == M.cols) ? ((r == M.rows) ? "]" : ";\n") : ",");
		return os;
	}

	// Number of non-zero elements
	inline Index Size() { return mp.size(); }

	// destructor
	~Matrix()
	{
		mp.clear();
	}
};

// returns a matrix with size cols x rows with ones as values
Matrix Ones(const Dimension rows, const Dimension cols)
{
	Matrix res = Matrix(rows, cols);
	for (Dimension r = 1; r <= rows; r++)
		for (Dimension c = 1; c <= cols; c++)
			res.set(r, c, 1.00);
	return res;
}

// returns a diagonal matrix with size n x n with ones at the diagonal
Matrix Diag(const Dimension n, const double v = 1.00)
{
	Matrix res = Matrix(n, n);
	for (Dimension i = 1; i <= n; i++)
		res.set(i, i, v);
	return res;
}

// returns a diagonal matrix
Matrix Diag(Matrix& v)
{
	Matrix res;
	if (v.GetCols() != 1 && v.GetRows() != 1)
		throw Exception(MER_INVALID_DIMS);
	res = (v.GetCols() == 1) ? Matrix(v.GetRows(), v.GetRows()) : Matrix(v.GetCols(), v.GetCols());
	map< Index, double >::iterator it;
	Dimension r = 0;
	Dimension c = 0;
	double t;
	bool check = v.beginIter(it, r, c, t);
	while (check)
	{
		res.set(r + c - 1, r + c - 1, t);
		check = v.nextIter(it, r, c, t);
	}
	return res;
}

// returns the inverse of Matrix a, stores determinent in lastDest
Matrix Inv(const Matrix& a)
{
	Matrix res;
	Dimension rows = a.GetRows();
	Dimension cols = a.GetCols();

	if (rows != cols)
	{
		throw Exception(MER_NOT_SQUARE);
		return res;
	}
	// calculate inverse using gauss-jordan elimination
	res = Diag(rows);   // a diagonal matrix with ones at the diagonal
	Matrix ai = a;    // make a copy of Matrix a
	lastDet = 1.0;
	for (Dimension c = 1; c <= cols; c++)
	{
		// element (c, c) should be non zero. if not, swap content
		// of lower rows
		Dimension r;
		for (r = c; r <= rows && ai(r, c) == 0.0; r++) {}
		if (r >rows)
		{
			lastDet = 0.0;
			throw Exception(MER_ZERO_DET);
			return res;
		}
		if (r != c)
			// swap rows
			for (Dimension s = 1; s <= cols; s++)
			{
				double temp = ai(c, s);
				ai.set(c, s, ai(r, s));
				ai.set(r, s, temp);
				temp = res(c, s);
				res.set(c, s, res(r, s));
				res.set(r, s, temp);
			}

		// eliminate non-zero values on the other rows at column c
		for (Dimension r = 1; r <= rows; r++)
		{
			if (r != c)
			{
				// eleminate value at column c and row r
				if (ai(r, c) != 0.0)
				{
					double f = -ai(r, c) / ai(c, c);
					// add (f * row c) to row r to eleminate the value at column c
					for (Dimension s = c; s <= cols; s++)
						ai.set(r, s, ai(r, s) + f *ai(c, s));
					for (Dimension s = 1; s <= cols; s++)
						res.set(r, s, res(r, s) + f *res(c, s));
				}
			}
			else
			{
				// make value at (c, c) one, divide each value on row r with the value at ai(c,c)
				double f = ai(c, c);
				lastDet *= f;
				for (Dimension s = c; s <= cols; s++)
					ai.set(r, s, ai(r, s) / f);
				for (Dimension s = 1; s <= cols; s++)
					res.set(r, s, res(r, s) / f);
			}
		}
	}
	return res;
}

// Solve equation a*x=v, a is n*n matrix and x,v are n*eqn matrices, eqn: number of equation sets
Matrix Solve(const Matrix& a, const Matrix& v)
{
	map< Index, double >::iterator it;

	Matrix vi;
	Dimension n = a.GetRows();

	if (a.GetCols() != n || v.GetRows() != n)
	{
		throw Exception(MER_INVALID_DIMS);
		return vi;
	}

	Matrix ai = a; 
	vi = v;
	Dimension eqn = v.GetCols();

	for (Dimension c = 1; c <= n; c++)
	{
		Dimension r;
		for (r = c; r <= n && ai(r, c) == 0.0; r++) {}
		if (r > n)
		{
			throw Exception(MER_ZERO_DET);
			return vi;
		}
		if (r != c)
		{
			for (Dimension s = 1; s <= n; s++)
			{
				double temp = ai(c, s);
				ai.set(c, s, ai(r, s));
				ai.set(r, s, temp);
			}
			for (Dimension eq = 1; eq <= eqn; eq++)
			{
				double temp = vi(c, eq);
				vi.set(c, eq, vi(r, eq));
				vi.set(r, eq, temp);
			}
		}

		double tc = ai(c, c);
		for (Dimension r = c + 1; r <= n; r++)
		{
			double t = ai(r, c);
			if (t != 0.0)
			{
				double f = -t / tc;
				ai.set(r, c, 0.0);
				Dimension s = c;
				Dimension ci = c;
				bool check = ai.beginIter(it, ci, s, t);
				while (check && (ci == c))
				{
					ai.set(r, s, ai(r, s) + f *t);
					check = ai.nextIter(it, ci, s, t);
				}
				for (Dimension eq = 1; eq <= eqn; eq++)
					vi.set(r, eq, vi(r, eq) + f *vi(c, eq));
			}
		}
	}
	for (Dimension r = n;r >= 1;r--)
		for (Dimension eq = 1; eq <= eqn; eq++)
		{
			double temp = 0.0;
			double t;
			Dimension c = r;
			Dimension ri = r;
			bool check = ai.beginIter(it, ri, c, t);
			while (check && (ri == r))
			{
				temp += t*vi(c, eq);
				check = ai.nextIter(it, ri, c, t);
			}
			vi.set(r, eq, (vi(r, eq) - temp) / ai(r, r));
		}
	return vi;
}

// Addition of Matrix with Matrix
Matrix Add(Matrix& a, Matrix& b)
{
	if (a.GetRows() != b.GetRows() || a.GetCols() != b.GetCols())
	{
		throw Exception(MER_INVALID_DIMS);
		return Matrix();
	}
	map< Index, double >::iterator ita;
	map< Index, double >::iterator itb;
	Matrix res(a.GetRows(), a.GetCols());
	Dimension ra = 0;
	Dimension ca = 0;
	Dimension rb = 0;
	Dimension cb = 0;
	double ta, tb;
	bool checka = a.beginIter(ita, ra, ca, ta);
	bool checkb = b.beginIter(itb, rb, cb, tb);
	while (checka || checkb)
	{
		if (checka && checkb && ita->first == itb->first)
		{
			res.set(ra, ca, ta + tb);
			checka = a.nextIter(ita, ra, ca, ta);
			checkb = b.nextIter(itb, rb, cb, tb);
		}
		if (!checkb || ita->first < itb->first)
		{
			res.set(ra, ca, ta);
			checka = a.nextIter(ita, ra, ca, ta);
		}
		if (!checka || itb->first < ita->first)
		{
			res.set(rb, cb, tb);
			checkb = b.nextIter(itb, rb, cb, tb);
		}
	}
	return res;
}

// Subtraction of Matrix with Matrix
Matrix Sub(Matrix& a, Matrix& b)
{
	if (a.GetRows() != b.GetRows() || a.GetCols() != b.GetCols())
	{
		throw Exception(MER_INVALID_DIMS);
		return Matrix();
	}
	map< Index, double >::iterator ita;
	map< Index, double >::iterator itb;
	Matrix res(a.GetRows(), a.GetCols());
	Dimension ra = 0;
	Dimension ca = 0;
	Dimension rb = 0;
	Dimension cb = 0;
	double ta, tb;
	bool checka = a.beginIter(ita, ra, ca, ta);
	bool checkb = b.beginIter(itb, rb, cb, tb);
	while (checka || checkb)
	{
		if (checka && checkb && ita->first == itb->first)
		{
			res.set(ra, ca, ta - tb);
			checka = a.nextIter(ita, ra, ca, ta);
			checkb = b.nextIter(itb, rb, cb, tb);
		}
		if (!checkb || ita->first < itb->first)
		{
			res.set(ra, ca, ta);
			checka = a.nextIter(ita, ra, ca, ta);
		}
		if (!checka || itb->first < ita->first)
		{
			res.set(rb, cb, -tb);
			checkb = b.nextIter(itb, rb, cb, tb);
		}
	}
	return res;
}

// Multiplication of Matrix with Matrix
Matrix Mul(Matrix& a, Matrix& b)
{
	Dimension ar = a.GetRows();
	Dimension bc = b.GetCols();

	if (a.GetCols() != b.GetRows())
	{
		throw Exception(MER_INVALID_DIMS);
		return Matrix();
	}
	map< Index, double >::iterator it;

	Matrix res(ar, bc);
	for (Dimension r = 1; r <= ar; r++)
	{
		for (Dimension m = 1;m <= bc;m++)
		{
			Dimension c = 0;
			Dimension ri = r;
			double t;
			double temp = 0.0;
			bool check = a.beginIter(it, ri, c, t);
			while (check && (ri == r))
			{
				temp += t*b(c, m);
				check = a.nextIter(it, ri, c, t);
			}
			res.set(r, m, temp);
		}
	}
	return res;
}

// Transpose of Matrix
Matrix Trans(Matrix& a)
{
	map< Index, double >::iterator it;
	Matrix res(a.GetCols(), a.GetRows());
	Dimension r = 0;
	Dimension c = 0;
	double t;
	bool check = a.beginIter(it, r, c, t);
	while (check)
	{
		res.set(c, r, t);
		check = a.nextIter(it, r, c, t);
	}
	return res;
}

// Multiplication of Matrix with Transpose of a Matrix
Matrix MulTrans(Matrix& a, Matrix& b)
{
	Dimension ar = a.GetRows();
	Dimension br = b.GetRows();

	if (a.GetCols() != b.GetCols())
	{
		throw Exception(MER_INVALID_DIMS);
		return Matrix();
	}
	map< Index, double >::iterator it1;
	map< Index, double >::iterator it2;

	Matrix res(ar, br);
	for (Dimension r = 1; r <= ar; r++)
	{
		for (Dimension m = 1;m <= br;m++)
		{
			Dimension c1 = 0;
			Dimension c2 = 0;
			Dimension ri = r;
			Dimension mi = m;
			double t1;
			double t2;
			double temp = 0.0;
			bool checka = a.beginIter(it1, ri, c1, t1);
			bool checkb = b.beginIter(it2, mi, c2, t2);
			while (checka && checkb && (ri == r) && (mi == m))
				if (c1 == c2)
				{
					temp += t1*t2;
					checka = a.nextIter(it1, ri, c1, t1);
					checkb = b.nextIter(it2, mi, c2, t2);
				} 
				else if (c1 < c2)
					checka = a.nextIter(it1, ri, c1, t1);
				else 
					checkb = b.nextIter(it2, mi, c2, t2);
			res.set(r, m, temp);
		}
	}
	return res;
}

int main(int argc, char *argv[])
{
	// below some demonstration of the usage of the Matrix class
	try
	{
		// examples part 1
		Dimension cols = 3;
		Dimension rows = 3;
		Matrix A = Matrix(cols, rows);
		int count = 0;
		for (Dimension r = 1; r <= rows; r++)
			for (Dimension c = 1; c <= cols; c++)
				A.set(r, c, ++count);
		A.set(2, 1, 2);
		cout << "A= \n" << A << "\n";
		Matrix V = Matrix(rows, 1);
		V.set(1, 1, 10);
		V.set(2, 1, 22);
		V.set(3, 1, 46);
		cout << "V= \n" << V << "\n";
		cout << "Inv(A)*V= \n" << Mul(Inv(A),V) << "\n";
		cout << "Solve(A, V)= \n" << Solve(A, V) << "\n";
		cout << "\n\n\n";
		
		// examples part 2
		Dimension nadd = 5;
		Matrix A1 = Matrix(nadd, nadd);
		Matrix A2 = Matrix(nadd, nadd);
		count = 0;
		for (Dimension r = 1; r <= nadd; r++)
		{
			A1.set(r, 1, ++count);
			A2.set(r, r, ++count);
		}
		cout << "A1= \n" << A1 << "\n";
		cout << "A2= \n" << A2 << "\n";
		Matrix A3 = Sub(A1, A2);
		cout << "A1-A2= \n" << A3 << "\n";
		cout << "\n\n\n";

		// examples part 3
		Dimension test = 100;
		Dimension band = 10;
		cout << "Creating a " << test << "*" << test
			<< " Matrix with " << 2 * band + 1
			<< " non-zero middle elements at each row"
			<< "\nComputing Inverse and Solve, please wait...";
		Matrix M = Matrix(test, test);
		Matrix N = Matrix(test, 1);
		for (Dimension i = 1;i <= test;i++)
		{
			Dimension a = (i <= band) ? 1 : i - band;
			Dimension b = (i > test - band) ? test : i + band;
			for (Dimension j = a;j <= b;j++)
				M.set(i, j, 1.0 + rand() % 10);
			N.set(i, 1, 1.0 + rand() % 10);
		}
		clock_t t1 = clock();
		Matrix X1 = Mul(Inv(M), N);
		clock_t t2 = clock();
		Matrix X2 = Solve(M, N);
		clock_t t3 = clock();
		cout << "\nInv(M)*N, Solve(M, N)= \n";
		for (Dimension i = 1;i <= test;i++)
			cout << "(" << X1(i, 1) << ", " << X2(i, 1) << ") " << (!(i % 5) ? "\n" : "");
		cout << "\n\nInv(M)*N computation time: " << (double)(t2 - t1) / CLOCKS_PER_SEC;
		cout << "\nSolve(M, N) computation time: " << (double)(t3 - t2) / CLOCKS_PER_SEC;

		// examples part 4
		cout << "\n\n\ncomputing M*M and M*Trans(Trans(M)), please wait...";
		t1 = clock();
		X1 = Mul(M, M);
		t2 = clock();
		X2 = MulTrans(M, Trans(M));
		t3 = clock();
		cout << "\n\nM*M computation time: " << (double)(t2 - t1) / CLOCKS_PER_SEC;
		cout << "\nM*Trans(Trans(M)) computation time: " << (double)(t3 - t2) / CLOCKS_PER_SEC;
		cout << "\n\n";
	}
	catch (Exception err)
	{
		cout << "Error code " << err.code << endl;
	}
	catch (...)
	{
		cout << "An error occured...\n";
	}
	cout << "Press Enter to exit...\n";
	getchar();
	return EXIT_SUCCESS;
}
