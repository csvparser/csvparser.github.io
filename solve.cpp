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

// Declarations
class Matrix;
static double lastDet; // Determinant computed by Inv function
Matrix Diag(const int n);
Matrix Diag(const Matrix& v);
Matrix Inv(const Matrix& a);
Matrix Ones(const int rows, const int cols);
Matrix Zeros(const int rows, const int cols);

using Dimension = unsigned long int;
using Index = unsigned long long int;
union Access {
	Index i;
	struct {
		Dimension c;
		Dimension r;
	} at;
};

Index getMatrixIndex(Dimension r, Dimension c)
{
	union Access m;
	m.at.r = r;
	m.at.c = c;
	return m.i;
}

Dimension getMatrixColumn(Index i)
{
	union Access m;
	m.i = i;
	return m.at.c;
}

Dimension getMatrixRow(Index i)
{
	union Access m;
	m.i = i;
	return m.at.r;
}

/*
* a simple exception class
* you can create an exeption by entering: throw Exception("...Error description...");
* and get the error message from the data msg for displaying: err.msg
*/
class Exception
{
public:
	const char* msg;
	Exception(const char* arg) : msg(arg) {	}
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
	Matrix(const int row_count, const int column_count)
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
		if (r > 0 && r <= rows && c > 0 && c <= cols)
		{
			auto it = mp.find(getMatrixIndex(r, c));
			return ((it != mp.end()) ? it->second : 0.0);
		}
		else
			throw Exception("Subscript out of range");
	}

	// index operator. You can use this class like myMatrix(col, row)
	// the indexes are one-based, not zero based.
	// use this function get if you want to read from a const Matrix
	double get(const Dimension r, const Dimension c) const
	{
		if (r > 0 && r <= rows && c > 0 && c <= cols)
		{
			auto it = mp.find(getMatrixIndex(r, c));
			if (it != mp.end()) return it->second; else return 0.0;
		}
		else
			throw Exception("Subscript out of range");
	}

	void set(const Dimension r, const Dimension c, const double v)
	{
		if (r > 0 && r <= rows && c > 0 && c <= cols)
			if (v == 0.0) mp.erase(getMatrixIndex(r, c)); else mp[getMatrixIndex(r, c)] = v;
		else
			throw Exception("Subscript out of range");
	}

	// first and last non-zero element of row r
	bool inRow(const Dimension r, Dimension& first, Dimension& last)
	{
		if (r <= 0 || r > rows)
		{
			throw Exception("Subscript out of range");
			return false;
		}
		auto it = mp.upper_bound(getMatrixIndex(r, 0));
		if (it == mp.end() || getMatrixRow(it->first) != r)
			return false;
		first = getMatrixColumn(it->first);
		it = mp.lower_bound(getMatrixIndex(r + 1, 0));
		it--;
		last = getMatrixColumn(it->first);
		return true;
	}

	// next non-zero element, r and c can be 0 to get first element in a row/matrix
	bool next(Dimension& r, Dimension& c, double& v)
	{
		if (r < 0 || r > rows || c < 0 || c > cols)
		{
			throw Exception("Subscript out of range");
			return false;
		}
		auto it = mp.upper_bound(getMatrixIndex(r, c));
		if (it == mp.end())
			return false;
		r = getMatrixRow(it->first);
		c = getMatrixColumn(it->first);
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

	// add a double value (elements wise)
	Matrix& Add(const double v)
	{
		for (Dimension r = 1; r <= rows; r++)
			for (Dimension c = 1; c <= cols; c++)
				set(r, c, get(r, c) + v);
		return *this;
	}

	// subtract a double value (elements wise)
	Matrix& Subtract(const double v)
	{
		return Add(-v);
	}

	// multiply a double value (elements wise)
	Matrix& Multiply(const double v)
	{
		for (Dimension r = 1; r <= rows; r++)
			for (Dimension c = 1; c <= cols; c++)
				set(r, c, get(r, c) * v);
		return *this;
	}

	// divide a double value (elements wise)
	Matrix& Divide(const double v)
	{
		return Multiply(1 / v);
	}

	// addition of Matrix with Matrix
	friend Matrix operator+(const Matrix& a, const Matrix& b)
	{
		// check if the dimensions match
		if (a.rows == b.rows && a.cols == b.cols)
		{
			Matrix res(a.rows, a.cols);
			for (Dimension r = 1; r <= a.rows; r++)
				for (Dimension c = 1; c <= a.cols; c++)
					res.set(r, c, a(r, c) + b(r, c));
			return res;
		}
		else
			throw Exception("Dimensions does not match");
		// return an empty matrix (this never happens but just for safety)
		return Matrix();
	}

	// addition of Matrix with double
	friend Matrix operator+ (const Matrix& a, const double b)
	{
		Matrix res = a;
		res.Add(b);
		return res;
	}
	// addition of double with Matrix
	friend Matrix operator+ (const double b, const Matrix& a)
	{
		Matrix res = a;
		res.Add(b);
		return res;
	}

	// subtraction of Matrix with Matrix
	friend Matrix operator- (const Matrix& a, const Matrix& b)
	{
		// check if the dimensions match
		if (a.rows == b.rows && a.cols == b.cols)
		{
			Matrix res(a.rows, a.cols);
			for (Dimension r = 1; r <= a.rows; r++)
				for (Dimension c = 1; c <= a.cols; c++)
					res.set(r, c, a(r, c) - b(r, c));
			return res;
		}
		else
			throw Exception("Dimensions does not match");
		// return an empty matrix (this never happens but just for safety)
		return Matrix();
	}

	// subtraction of Matrix with double
	friend Matrix operator- (const Matrix& a, const double b)
	{
		Matrix res = a;
		res.Subtract(b);
		return res;
	}
	// subtraction of double with Matrix
	friend Matrix operator- (const double b, const Matrix& a)
	{
		Matrix res = -a;
		res.Add(b);
		return res;
	}

	// operator unary minus
	friend Matrix operator- (const Matrix& a)
	{
		Matrix res(a.rows, a.cols);
		for (Dimension r = 1; r <= a.rows; r++)
			for (Dimension c = 1; c <= a.cols; c++)
				res.set(r, c, -a(r, c));
		return res;
	}

	// operator multiplication
	friend Matrix operator* (const Matrix& a, const Matrix& b)
	{
		// check if the dimensions match
		if (a.cols == b.rows)
		{
			Matrix res(a.rows, b.cols);
			for (Dimension r = 1; r <= a.rows; r++)
				for (Dimension m = 1; m <= b.cols; m++)
				{
					double temp = 0.0;
					for (Dimension c = 1; c <= b.rows; c++)
						temp += a(r, c) * b(c, m);
					res.set(r, m, temp);
				}
			return res;
		}
		else
			throw Exception("Dimensions does not match");
		// return an empty matrix (this never happens but just for safety)
		return Matrix();
	}

	// multiplication of Matrix with double
	friend Matrix operator* (const Matrix& a, const double b)
	{
		Matrix res = a;
		res.Multiply(b);
		return res;
	}
	// multiplication of double with Matrix
	friend Matrix operator* (const double b, const Matrix& a)
	{
		Matrix res = a;
		res.Multiply(b);
		return res;
	}

	// division of Matrix with Matrix
	friend Matrix operator/ (const Matrix& a, const Matrix& b)
	{
		// check if the dimensions match: must be square and equal sizes
		if (a.rows == a.cols && a.rows == a.cols && b.rows == b.cols)
		{
			Matrix res(a.rows, a.cols);
			res = a * Inv(b);
			return res;
		}
		else
			throw Exception("Dimensions does not match");
		// return an empty matrix (this never happens but just for safety)
		return Matrix();
	}

	// division of Matrix with double
	friend Matrix operator/ (const Matrix& a, const double b)
	{
		Matrix res = a;
		res.Divide(b);
		return res;
	}

	// division of double with Matrix
	friend Matrix operator/ (const double b, const Matrix& a)
	{
		Matrix b_matrix(1, 1);
		b_matrix.set(1, 1, b);
		Matrix res = b_matrix / a;
		return res;
	}

	// returns the number of rows
	Dimension GetRows() const
	{
		return rows;
	}

	// returns the number of columns
	Dimension GetCols() const
	{
		return cols;
	}

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
	Index Size()
	{
		return mp.size();
	}
	// destructor
	~Matrix()
	{
		mp.clear();
	}

};

/**
* returns a matrix with size cols x rows with ones as values
*/
Matrix Ones(const Dimension rows, const Dimension cols)
{
	Matrix res = Matrix(rows, cols);
	for (Dimension r = 1; r <= rows; r++)
		for (Dimension c = 1; c <= cols; c++)
			res.set(r, c, 1.00);
	return res;
}

/**
* returns a matrix with size cols x rows with zeros as values
*/
Matrix Zeros(const Dimension rows, const Dimension cols)
{
	return Matrix(rows, cols);
}


/**
* returns a diagonal matrix with size n x n with ones at the diagonal
* @param  v a vector
* @return a diagonal matrix with ones on the diagonal
*/
Matrix Diag(const Dimension n)
{
	Matrix res = Matrix(n, n);
	for (Dimension i = 1; i <= n; i++)
		res.set(i, i, 1.00);
	return res;
}

/**
* returns a diagonal matrix
* @param  v a vector
* @return a diagonal matrix with the given vector v on the diagonal
*/
Matrix Diag(const Matrix& v)
{
	Matrix res;
	if (v.GetCols() == 1)
	{
		// the given matrix is a vector n x 1
		Dimension rows = v.GetRows();
		res = Matrix(rows, rows);
		// copy the values of the vector to the matrix
		for (Dimension r = 1; r <= rows; r++)
			res.set(r, r, v(r, 1));
	}
	else if (v.GetRows() == 1)
	{
		// the given matrix is a vector 1 x n
		Dimension cols = v.GetCols();
		res = Matrix(cols, cols);
		// copy the values of the vector to the matrix
		for (Dimension c = 1; c <= cols; c++)
			res.set(c, c, v(1, c));
	}
	else
		throw Exception("Parameter for diag must be a vector");
	return res;
}

/*
* returns the inverse of Matrix a, stores determinent in DT
*/
Matrix Inv(const Matrix& a, double& DT)
{
	Matrix res;
	Dimension rows = a.GetRows();
	Dimension cols = a.GetCols();

	if (rows != cols)
	{
		throw Exception("Matrix must be square");
		return res;
	}
	// calculate inverse using gauss-jordan elimination
	//   http://mathworld.wolfram.com/MatrixInverse.html
	//   http://math.uww.edu/~mcfarlat/inverse.htm
	res = Diag(rows);   // a diagonal matrix with ones at the diagonal
	Matrix ai = a;    // make a copy of Matrix a
	DT = 1;
	for (Dimension c = 1; c <= cols; c++)
	{
		// element (c, c) should be non zero. if not, swap content
		// of lower rows
		Dimension r;
		for (r = c; r <= rows && ai(r, c) == 0.0; r++) {}
		if (r >rows)
		{
			DT = 0;
			throw Exception("Determinant of matrix is zero");
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
				//Swap(ai(c, s), ai(r, s));
				//Swap(res(c, s), res(r, s));
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

					// add (f * row c) to row r to eleminate the value
					// at column c
					for (Dimension s = 1; s <= cols; s++)
					{
						ai.set(r, s, ai(r, s) + f *ai(c, s));
						res.set(r, s, res(r, s) + f *res(c, s));
					}
				}
			}
			else
			{
				// make value at (c, c) one,
				// divide each value on row r with the value at ai(c,c)
				double f = ai(c, c);
				DT *= f;
				for (Dimension s = 1; s <= cols; s++)
				{
					ai.set(r, s, ai(r, s) / f);
					res.set(r, s, res(r, s) / f);
				}
			}
		}
	}
	return res;
}
/*
* returns the inverse of Matrix a, stores determinent in lastDet
*/
Matrix Inv(const Matrix& a)
{
	return Inv(a, lastDet);
}

/*
* returns the inverse of Matrix a, stores determinent in DT
*/
Matrix Solve(const Matrix& a, const Matrix& v)
{
	Matrix vi;
	Dimension n = a.GetRows();

	if (a.GetCols() != n || v.GetRows() != n)
	{
		throw Exception("Invalid dimensions");
		return vi;
	}

	Matrix ai = a;    // make a copy of Matrix a
	vi = v;
	Dimension eqn = v.GetCols(); // number of equation sets
	
	for (Dimension c = 1; c <= n; c++)
	{
		// element (c, c) should be non zero. if not, swap content
		// of lower rows
		Dimension r;
		for (r = c; r <= n && ai(r, c) == 0.0; r++) {}
		if (r > n)
		{
			throw Exception("Determinant of matrix is zero");
			return vi;
		}
		if (r != c)
		{
			// swap rows
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

		// eliminate non-zero values on below rows
		double tc = ai(c, c);
		for (Dimension r = c + 1; r <= n; r++)
		{
			double t = ai(r, c);
			if (t != 0.0)
			{
				double f = -t / tc;

				// add (f * row c) to row r to eleminate the value
				// at column c
				ai.set(r, c, 0.0);
				/*
				Dimension first;
				Dimension last;
				ai.inRow(c, first, last);
				for (Dimension s = c + 1; s <= last; s++)
					ai.set(r, s, ai(r, s) + f *ai(c, s));
					*/
				Dimension s = c;
				Dimension ci = c;
				while (ai.next(ci, s, t) && (ci == c))
					ai.set(r, s, ai(r, s) + f *t);
				vi.set(r, 1, vi(r, 1) + f *vi(c, 1));
			}
		}
	}
	for (Dimension r = n;r >= 1;r--)
	{
		for (Dimension eq = 1; eq <= eqn; eq++)
		{
			double temp = 0.0;
			double t;
			Dimension c = r;
			Dimension ri = r;
			while (ai.next(ri, c, t) && (ri == r))
				temp += t*vi(c, eq);
			vi.set(r, eq, (vi(r, eq) - temp) / ai(r, r));
		}
	}
	// cout << "\nSize ai:" << ai.Size() << "\n";
	// cout << ai << "\n";
	return vi;
}


int main(int argc, char *argv[])
{
	// below some demonstration of the usage of the Matrix class
	try
	{
		// create an empty matrix of 3x3 (will initially contain zeros)
		Dimension cols = 3;
		Dimension rows = 3;
		Matrix A = Matrix(cols, rows);

		// fill in some values in matrix a
		int count = 0;
		for (Dimension r = 1; r <= rows; r++)
			for (Dimension c = 1; c <= cols; c++)
				A.set(r, c, ++count);

		// adjust a value in the matrix (indexes are one-based)
		A.set(2, 1, 2);

		// print the whole matrix
		cout << "A= \n" << A << "\n";

		Matrix V = Matrix(rows, 1);    // a vector
		V.set(1, 1, 10);
		V.set(2, 1, 22);
		V.set(3, 1, 46);
		cout << "V= \n" << V << "\n";

		cout << "Inv(A)*V= \n" << Inv(A)*V << "\n";
		cout << "Solve(A, V)= \n" << Solve(A, V) << "\n";

		cout << "\n\n\n";

		Dimension test = 100;
		Dimension band = 5;
		cout << "Creating a " << test << "*" << test
			<< " Matrix with " << 2 * band + 1
			<< " non-zero middle elements at each row, please wait...";
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
		// cout << "\nM=\n" << M << "\n";
		clock_t t1 = clock();
		// Matrix X1 = N;
		Matrix X1 = Inv(M)*N;
		clock_t t2 = clock();
		Matrix X2 = Solve(M, N);
		clock_t t3 = clock();
		cout << "\nInv(M)*N, Solve(M, N)= \n";
		for (Dimension i = 1;i <= test;i++)
			cout << X1(i,1) << " , " << X2(i, 1) << "\n";
		cout << "\nInv(M)*N computation time:    " << (double)(t2 - t1) / CLOCKS_PER_SEC;
		cout << "\nSolve(M, N) computation time: " << (double)(t3 - t2) / CLOCKS_PER_SEC;
		cout << "\n";
		
	}
	catch (Exception err)
	{
		cout << "Error: \n" << err.msg << endl;
	}
	catch (...)
	{
		cout << "An error occured...\n";
	}
	cout << "Press Enter to exit...\n";
	getchar();
	return EXIT_SUCCESS;
}
