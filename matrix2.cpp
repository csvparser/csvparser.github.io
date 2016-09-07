/*
matrix2 is a modified version of matrix program.
It uses std::map to store matrix elements and only stores non-zero elements.
It ignores unimportant bits of double numbers which results an absolute zero 
values for elements of M*Inv(M) not located on diagonal.
Original matrix program by Jos de Jong, Nov 2007. Updated March 2010
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

using namespace std;

#define unImpBits 30

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

double add(double x, double y)
{
	if (rounded(x) == rounded(-y))
		return 0.0;
	else
		return x + y;
}

double sub(double x, double y)
{
	if (rounded(x) == rounded(y))
		return 0.0;
	else
		return x - y;
}

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
		} else
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
				set(r, c, add(get(r, c), v));
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
					res.set(r, c, add(a(r, c), b(r, c)));
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
					res.set(r, c, sub(a(r, c), b(r, c)));
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
						temp = add(temp, a(r, c) * b(c, m));
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
	// this is a matrix of 3 x 3 or larger
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
						ai.set(r, s, add(ai(r, s), f *ai(c, s)));
						res.set(r, s, add(res(r, s), f *res(c, s)));
						//ai(r, s) += f * ai(c, s);
						//res(r, s) += f * res(c, s);
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
					//ai(r, s) /= f;
					//res(r, s) /= f;
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
		A.set(2, 1, 1.23);

		// print the whole matrix
		cout << "A= \n" << A << "\n";
		cout << A(2, 1) << "\n";

		Matrix B = Ones(rows, cols)+Diag(rows);
		cout << "B= \n" << B << "\n";

		Matrix B2 = Matrix(rows, 1);    // a vector
		count = 1;
		for (Dimension r = 1; r <= rows; r++)
			B2.set(r, 1, ++count * 2);
		cout << "B2= \n" << B2 << "\n";

		cout << "A + B= \n" << A + B << "\n";
		cout << "A - B= \n" << A - B << "\n";
		cout << "A * B2= \n" << A * B2 << "\n";

		cout << "Diag. B2= \n" << Diag(B2) << "\n";

		cout << "A= \n" << A << "\n";
		cout << "3 - A= \n" << 3 - A << "\n";
		Matrix A_inv = Inv(A);
		cout << "Inv(A)= \n" << A_inv << "\n";
		cout << "Det(A)=" << lastDet << "\n";
		cout << "A * Inv(A)= \n" << A * Inv(A) << "\n";

		cout << "B / A = \n" << B / A << "\n";
		cout << "A / 3 = \n" << A / 3 << "\n";

		rows = 2;
		cols = 5;
		Matrix H = Matrix(rows, cols);
		for (Dimension r = 1; r <= rows; r++)
			for (Dimension c = 1; c <= cols; c++)
				H.set(r, c, ++count);
		cout << "H= \n" << H << "\n";

		cout << "\n\n\n";

		Dimension test = 100;
		Dimension band = 5;
		cout << "Creating a " << test << "*" << test 
			<< " Matrix with " << 2 * band + 1
			<< " non-zero middle elements at each row, please wait...";

		Matrix M = Matrix(test, test);
		for (Dimension i = 1;i <= test;i++)
		{
			Dimension a = (i <= band) ? 1 : i - band;
			Dimension b = (i > test - band) ? test : i + band;
			for (Dimension j = a;j <= b;j++)
				M.set(i, j, 1.0 + rand() % 10);
		}
		cout << "\n\nNumber of non-zero elements of Matrix M: " << M.Size() << "\n\n";
		cout << "Computing Inv M, please wait...";
		Matrix MI = Inv(M);
		cout << "\nNumber of non-zero elements of Matrix Inv M: " << MI.Size() << "\n\n";
		cout << "Computing M*Inv M, please wait...";
		Matrix MMI = M*Inv(M);
		cout << "\nNumber of non-zero elements of Matrix M*Inv M: " << MMI.Size() << "\n\n";
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
