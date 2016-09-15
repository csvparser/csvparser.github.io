/*
solve2 is a modified version of solve program.
It is optimized for solving matrices with limited number of non-zero near diagonal elements.
Modified by by Hamid Soltani. (gmail: hsoltanim)
https://csvparser.github.io/
Last modified: Sep. 2016.
*/

#include "stdafx.h"

#include < cstdlib >
#include < cstdio >
#include < math.h >
#include < map >
#include < set >
#include < vector >

#include < iostream >
#include < iomanip > 
#include < time.h >

using namespace std;

using Real = double;
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

static Real lastDet; // Determinant computed by Inv function

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

struct MatrixElem
{
public:
	map< Index, Real >::iterator it;
	Dimension c;
	Dimension r;
	Real v;
	bool good;
};

struct MatrixElemTr
{
public:
	set< Index >::iterator it;
	Dimension c;
	Dimension r;
	Real v;
	bool good;
};

class Matrix
{
private:
	Dimension rows;
	Dimension cols;
	map< Index, Real > mp;
	set< Index > mptr;

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
		mptr = a.mptr;
	}

	// index operator. You can use this class like myMatrix(col, row)
	// the indexes are one-based, not zero based.
	const Real operator()(const Dimension r, const Dimension c) const
	{
		if (r <= 0 || r > rows || c <= 0 || c > cols)
			throw Exception(MER_OUT_OF_RANGE);
		auto it = mp.find(getMatrixIndex(r, c));
		return ((it != mp.end()) ? it->second : 0.0);
	}

	// set matrix element
	void set(const Dimension r, const Dimension c, const Real v)
	{
		if (r <= 0 || r > rows || c <= 0 || c > cols)
			throw Exception(MER_OUT_OF_RANGE);
		if (v == 0.0)
		{
			mp.erase(getMatrixIndex(r, c));
			mptr.erase(getMatrixIndex(c, r));
		}
		else
		{
			mp[getMatrixIndex(r, c)] = v;
			mptr.insert(getMatrixIndex(c, r));
		}
	}

	// begin iteration, next element after [r, c]
	void setIter(MatrixElem& me, const Dimension r, const Dimension c)
	{
		me.it = mp.upper_bound(getMatrixIndex(r, c));
		if (me.good = (me.it != mp.end()))
		{
			getMatrixRC(me.it->first, me.r, me.c);
			me.v = me.it->second;
		}
	}

	// next iteration
	void incIter(MatrixElem& me)
	{
		if (me.good = (++me.it != mp.end()))
		{
			getMatrixRC(me.it->first, me.r, me.c);
			me.v = me.it->second;
		}
	}

	// begin iteration, next element after [r, c]
	void setIterTr(MatrixElemTr& me, const Dimension r, const Dimension c)
	{
		me.it = mptr.upper_bound(getMatrixIndex(c, r));
		if (me.good = (me.it != mptr.end()))
		{
			getMatrixRC(*me.it, me.c, me.r);
			me.v = mp[getMatrixIndex(me.r, me.c)];
		}
	}

	// next iteration
	void incIterTr(MatrixElemTr& me)
	{
		if (me.good = (++me.it != mptr.end()))
		{
			getMatrixRC(*me.it, me.c, me.r);
			me.v = mp[getMatrixIndex(me.r, me.c)];
		}
	}

	// assignment operator
	Matrix& operator= (const Matrix& a)
	{
		rows = a.rows;
		cols = a.cols;
		mp = a.mp;
		mptr = a.mptr;
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
		mptr.clear();
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
Matrix Diag(const Dimension n, const Real v = 1.00)
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
	MatrixElem me;
	for (v.setIter(me, 0, 0);me.good;v.incIter(me))
		res.set(me.r + me.c - 1, me.r + me.c - 1, me.v);
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
				Real temp = ai(c, s);
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
					Real f = -ai(r, c) / ai(c, c);
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
				Real f = ai(c, c);
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
				Real temp = ai(c, s);
				ai.set(c, s, ai(r, s));
				ai.set(r, s, temp);
			}
			for (Dimension eq = 1; eq <= eqn; eq++)
			{
				Real temp = vi(c, eq);
				vi.set(c, eq, vi(r, eq));
				vi.set(r, eq, temp);
			}
		}

		Real tc = ai(c, c);
		MatrixElemTr metr;
		vector< Dimension > rn;

		for (ai.setIterTr(metr, c, c);metr.good && metr.c == c;ai.incIterTr(metr))
			rn.push_back(metr.r);

		for (Dimension r : rn)
		{
			Real f = -ai(r, c) / tc;
			ai.set(r, c, 0.0);
			MatrixElem me;
			for (ai.setIter(me, c, c);me.good && me.r == c;ai.incIter(me))
				ai.set(r, me.c, ai(r, me.c) + f *me.v);
			for (vi.setIter(me, c, 0);me.good && me.r == c;vi.incIter(me))
				vi.set(r, me.c, vi(r, me.c) + f *me.v);
		}
	}
	for (Dimension r = n;r >= 1;r--)
		for (Dimension eq = 1; eq <= eqn; eq++)
		{
			Real temp = 0.0;
			MatrixElem me;
			for (ai.setIter(me, r, r);me.good && me.r == r;ai.incIter(me))
				temp += me.v*vi(me.c, eq);
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
	Matrix res(a.GetRows(), a.GetCols());
	MatrixElem me1, me2;
	a.setIter(me1, 0, 0);
	b.setIter(me2, 0, 0);
	while (me1.good || me2.good)
	{
		if (me1.good && me2.good && (me1.it)->first == (me2.it)->first)
		{
			res.set(me1.r, me1.c, me1.v + me2.v);
			a.incIter(me1);
			b.incIter(me2);
		}
		if (!me2.good || (me1.it)->first < (me2.it)->first)
		{
			res.set(me1.r, me1.c, me1.v);
			a.incIter(me1);
		}
		if (!me1.good || (me1.it)->first >(me2.it)->first)
		{
			res.set(me2.r, me2.c, me2.v);
			b.incIter(me2);
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
	Matrix res(a.GetRows(), a.GetCols());
	MatrixElem me1, me2;
	a.setIter(me1, 0, 0);
	b.setIter(me2, 0, 0);
	while (me1.good || me2.good)
	{
		if (me1.good && me2.good && (me1.it)->first == (me2.it)->first)
		{
			res.set(me1.r, me1.c, me1.v - me2.v);
			a.incIter(me1);
			b.incIter(me2);
		}
		if (!me2.good || (me1.it)->first < (me2.it)->first)
		{
			res.set(me1.r, me1.c, me1.v);
			a.incIter(me1);
		}
		if (!me1.good || (me1.it)->first >(me2.it)->first)
		{
			res.set(me2.r, me2.c, -me2.v);
			b.incIter(me2);
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

	MatrixElem me;
	MatrixElemTr metr;
	Matrix res(ar, bc);
	for (Dimension r = 1; r <= ar; r++)
		for (Dimension m = 1;m <= bc;m++)
		{
			Real temp = 0.0;
			a.setIter(me, r, 0);
			b.setIterTr(metr, 0, m);
			while (me.good && me.r == r && metr.good && metr.c == m)
				if (me.c == metr.r)
				{
					temp += me.v*metr.v;
					a.incIter(me);
					b.incIterTr(metr);
				} 
				else if(me.c < metr.r)
					a.incIter(me);
				else
					b.incIterTr(metr);
			res.set(r, m, temp);
		}
	return res;
}

// Transpose of Matrix
Matrix Trans(Matrix& a)
{
	Matrix res(a.GetCols(), a.GetRows());
	MatrixElem me;
	for (a.setIter(me, 0, 0);me.good;a.incIter(me))
		res.set(me.c, me.r, me.v);
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
		cout << "Inv(A)*V= \n" << Mul(Inv(A), V) << "\n";
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
		Dimension test = 50;
		Dimension band = 5;
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
		cout << "\n\nInv(M)*N computation time: " << (Real)(t2 - t1) / CLOCKS_PER_SEC;
		cout << "\nSolve(M, N) computation time: " << (Real)(t3 - t2) / CLOCKS_PER_SEC;

		// examples part 4
		cout << "\n\n\ncomputing M*M and M*Trans(Trans(M)), please wait...";
		t1 = clock();
		X1 = Mul(M, M);
		t2 = clock();
		cout << "\n\nM*M computation time: " << (Real)(t2 - t1) / CLOCKS_PER_SEC;

		// examples part 5
		cout << "\n\n\ncomputing M*Inv(M) using Solve func, please wait...";
		t1 = clock();
		Matrix MMI = Mul(M, Solve(M, Diag(test)));
		t2 = clock();
		cout << "\n\nM*Inv M computation time: " << (Real)(t2 - t1) / CLOCKS_PER_SEC;
		Real e1 = 1e300;
		Real d1 = e1;
		Real e2 = -e1;
		Real d2 = e2;
		for (int i = 1; i <= test;i++)
			for (int j = 1; j <= test;j++)
				if (i == j)
				{
					if (d1 > MMI(i, j)) d1 = MMI(i, j);
					if (d2 < MMI(i, j)) d2 = MMI(i, j);
				}
				else {
					if (e1 > MMI(i, j)) e1 = MMI(i, j);
					if (e2 < MMI(i, j)) e2 = MMI(i, j);
				}
				typedef numeric_limits< Real > dbl;
				cout << setprecision(dbl::max_digits10)
					<< "\n\nDiagonal elements ranging from " << d1 << " to " << d2
					<< "\nNon-diagonal elements ranging from " << e1 << " to " << e2;

		// examples part 6
		test = 1000;
		band = 5;
		cout << "\n\nCreating a " << test << "*" << test
			<< " Matrix with " << 2 * band + 1
			<< " non-zero middle elements at each row"
			<< "\nSolve, please wait...";
		M = Matrix(test, test);
		N = Matrix(test, 1);
		for (Dimension i = 1;i <= test;i++)
		{
			Dimension a = (i <= band) ? 1 : i - band;
			Dimension b = (i > test - band) ? test : i + band;
			for (Dimension j = a;j <= b;j++)
				M.set(i, j, 1.0 + rand() % 10);
			N.set(i, 1, 1.0 + rand() % 10);
		}
		t1 = clock();
		X1 = Solve(M, N);
		t2 = clock();
		cout << "\n\nSolve(M, N) computation time: "
			<< setprecision(6) << (Real)(t2 - t1) / CLOCKS_PER_SEC;

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