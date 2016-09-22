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
	map< Index, Real >::iterator it;
	Dimension r;
	Dimension c;
	Real v;
	bool good;
};

struct MatrixElemTr
{
	set< Index >::iterator it;
	Dimension r;
	Dimension c;
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
			auto it = mp.find(getMatrixIndex(r, c));
			if (it == mp.end())
				return;
		}
		mp[getMatrixIndex(r, c)] = v;
		mptr.insert(getMatrixIndex(c, r));
	}

	// inc matrix element
	void inc(const Dimension r, const Dimension c, const Real v)
	{
		if (r <= 0 || r > rows || c <= 0 || c > cols)
			throw Exception(MER_OUT_OF_RANGE);
		if (v == 0.0)
			return;
		mp[getMatrixIndex(r, c)] += v;
		mptr.insert(getMatrixIndex(c, r));
	}

	void erase(const Dimension r, const Dimension c)
	{
		if (r <= 0 || r > rows || c <= 0 || c > cols)
			throw Exception(MER_OUT_OF_RANGE);
		mp.erase(getMatrixIndex(r, c));
		mptr.erase(getMatrixIndex(c, r));
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

	// begin iteration, next element after [r, c], look in columns
	void setIterTr(MatrixElemTr& me, const Dimension r, const Dimension c)
	{
		me.it = mptr.upper_bound(getMatrixIndex(c, r));
		if (me.good = (me.it != mptr.end()))
		{
			getMatrixRC(*me.it, me.c, me.r);
			me.v = mp[getMatrixIndex(me.r, me.c)];
		}
	}

	// next iteration, look in columns
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
				<< setw(6) << M(r, c)
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

// swap rows
void SwapRows(Matrix& m, const Dimension r1, const Dimension r2)
{
	MatrixElem me1;
	MatrixElem me2;
	m.setIter(me1, r1, 0);
	m.setIter(me2, r2, 0);
	while ((me1.good && me1.r == r1) || (me2.good && me2.r == r2))
	{
		if (me1.good && me2.good && me1.c == me2.c)
		{
			m.set(r1, me1.c, me2.v);
			m.set(r2, me2.c, me1.v);
			m.incIter(me1);
			m.incIter(me2);
		}
		else if (me1.good && me1.r == r1 && (!me2.good || me2.r != r2 || me1.c < me2.c))
		{
			m.erase(r1, me1.c);
			m.set(r2, me1.c, me1.v);
			m.setIter(me1, r1, me1.c);
		}
		else if (me2.good && me2.r == r2 && (!me1.good || me1.r != r1 || me2.c < me1.c))
		{
			m.set(r1, me2.c, me2.v);
			m.erase(r2, me2.c);
			m.setIter(me2, r2, me2.c);
		}
	}
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
		MatrixElemTr metr;
		ai.setIterTr(metr, c - 1, c);
		while (metr.v == 0.0 && metr.good && metr.c == c)
			ai.incIterTr(metr);
		if (!metr.good || metr.c != c)
		{
			throw Exception(MER_ZERO_DET);
			return vi;
		}
		Dimension r = metr.r;
		if (r != c)
		{
			SwapRows(ai, r, c);
			SwapRows(vi, r, c);
		}

		Real tc = ai(c, c);
		ai.setIterTr(metr, c, c);
		while (metr.good && metr.c == c)
		{
			r = metr.r;
			Real f = -ai(r, c) / tc;
			ai.incIterTr(metr);
			ai.erase(r, c);
			if (f != 0.0)
			{
				MatrixElem me;
				for (ai.setIter(me, c, c);me.good && me.r == c;ai.incIter(me))
					ai.inc(r, me.c, f *me.v);
				for (vi.setIter(me, c, 0);me.good && me.r == c;vi.incIter(me))
					vi.inc(r, me.c, f *me.v);
			}
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
		if (!me1.good || (me1.it)->first > (me2.it)->first)
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
				else if (me.c < metr.r)
					a.incIter(me);  // also: a.setIter(me, r, metr.r - 1);
				else
					b.incIterTr(metr);  // also: b.setIterTr(metr, me.c - 1, m);
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
		SwapRows(A3, 1, 2);
		SwapRows(A3, 5, 3);
		cout << "Swapped Rows:\n" << A3 << "\n";
		cout << "\n\n\n";

		// examples part 3
		Dimension test = 50;
		cout << "Creating a " << test << "*" << test << " Matrix, Solve, please wait...";
		Matrix M = Matrix(test, test);
		Matrix N = Matrix(test, 1);
		for (Dimension i = 1;i <= test;i++)
		{
			for (Dimension j = 1;j <= test;j++)
				M.set(i, j, rand() % 10);
			N.set(i, 1, rand() % 10);
		}
		clock_t t1 = clock();
		Matrix X = Solve(M, N);
		clock_t t2 = clock();
		cout << "\nSolve(M, N)= \n";
		cout << Trans(X);
		cout << "\n\nSolve(M, N) computation time: " << (Real)(t2 - t1) / CLOCKS_PER_SEC;

		// examples part 4
		cout << "\n\nComputing M*M, please wait...";
		t1 = clock();
		X = Mul(M, M);
		t2 = clock();
		cout << "\n\nM*M computation time: " << (Real)(t2 - t1) / CLOCKS_PER_SEC;

		// examples part 5
		cout << "\n\nComputing M*Inv(M) using Solve func, please wait...";
		t1 = clock();
		Matrix MMI = Mul(M, Solve(M, Diag(test)));
		t2 = clock();
		cout << "\n\nM*Inv M computation time: " << (Real)(t2 - t1) / CLOCKS_PER_SEC;
		Real e1 = 1e300;
		Real d1 = e1;
		Real e2 = -e1;
		Real d2 = e2;
		for (Dimension i = 1; i <= test;i++)
			for (Dimension j = 1; j <= test;j++)
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
		Dimension band = 10;
		cout << "\n\nCreating a " << test << "*" << test << " Matrix with " << 2 * band + 1
			<< " non-zero middle elements at each row\nSolve, please wait...";
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
		X = Solve(M, N);
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
