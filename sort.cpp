/****************************************************************************

sort is a c++ program to compare speed of different sort alogorithms.
The program also verifies the algorithms accuracy by comparing with std::sort function.
by Hamid Soltani. (gmail: hsoltanim)
https://csvparser.github.io/
Last modified: Aug. 2016.

*****************************************************************************/

#include "stdafx.h"

#include < iostream >
#include < array >
#include < time.h > 

using namespace std;

template < class T, size_t n >
void selectionsort(array< T, n > &A)
{
	for (size_t i = 0; i < n; i++)
	{
		T min = i;
		for (size_t j = i + 1; j < n; j++)
			if (A[min]>A[j])
				min = j;
		if (min != i)
			swap((T)A[i], (T)A[min]);
	}
}

template < class T, size_t n >
void bubblesort(array< T, n > &A)
{
	size_t up = n;
	do {
		size_t next_up = 0;
		for (size_t i = 0; i < up - 1; i++)
			if (A[i] > A[i + 1])
			{
				next_up = i + 1;
				swap((T)A[i], (T)A[i + 1]);
			}
		up = next_up;
	} while (up);
}


template < class T, size_t n >
void mergesort(array< T, n > &A, array< T, n > &H, size_t low, size_t high)
{
	if (high <= low + 1)
		return;
	size_t mid = (high + low) / 2;

	mergesort(A, H, low, mid);
	mergesort(A, H, mid, high);

	size_t l = low;
	size_t m = mid;
	for (size_t i = low; i < high; i++)
		if ((m == high) || ((l < mid) && (A[l] < A[m])))
			H[i] = A[l++];
		else
			H[i] = A[m++];

	for (size_t i = low; i < high; i++)
		A[i] = H[i];
}

template < class T, size_t n >
void mergesort(array< T, n > &A)
{
	array< T, n > H;
	mergesort(A, H, 0, n);
}

template < class T, size_t n >
void quicksort(array< T, n > &A, size_t low, size_t high)
{
	if (high <= low + 1)
		return;
	size_t i = low;
	size_t j = high - 1;
	T pivot = A[(low + high) / 2];
	do
	{
		while ((i < high-1) && (A[i] < pivot))
			i++;
		while ((j > low) && (A[j]>pivot))
			j--;
		if (i <= j)
		{
			if (i < j)
				swap((T)A[j], (T)A[i]);
			i++;
			j--;
		}
	} while (i < j);
	quicksort(A, low, j+1);
	quicksort(A, i, high);
}

template < class T, size_t n >
void quicksort(array< T, n > &A)
{
	quicksort(A, 0, n);
}

template < class T, size_t n >
void showarray(const array< T, n > &A)
{
	for (int i = 0; i < n; i++)
		cout << (T)A[i] << " ";
	cout << endl;
}

int main()
{
	clock_t t;

	srand(time(NULL));
	const int n = 10000;
	array< int, n > A;
	for (unsigned int i = 0; i < A.size(); i++)
		A[i] = rand() % 1000;
	// showarray(A);

	array< int, n > S = A;
	t = clock();
	sort(S.begin(),S.end());
	t = clock() - t;
	cout << "C++ Std Sort:" << (double) t / CLOCKS_PER_SEC << endl;
	// showarray(C);

	array< int, n > B;
	B = A;
	t = clock();
	selectionsort(B);
	t = clock() - t;
	cout << "Selection Sort: " << (double) t / CLOCKS_PER_SEC 
	     << ((S == B) ? " Accurate" : " Not accuarte") << endl;

	B = A;
	t = clock();
	bubblesort(B);
	t = clock() - t;
	cout << "Bubble Sort: " << (double)t / CLOCKS_PER_SEC
		<< ((S == B) ? " Accurate" : " Not accuarte") << endl;

	B = A;
	t = clock();
	mergesort(B);
	t = clock() - t;
	cout << "Merge Sort: " << (double)t / CLOCKS_PER_SEC
		<< ((S == B) ? " Accurate" : " Not accuarte") << endl;

	B = A;
	t = clock();
	quicksort(B);
	t = clock() - t;
	cout << "Quick Sort: " << (double)t / CLOCKS_PER_SEC
		<< ((S == B) ? " Accurate" : " Not accuarte") << endl;

	puts("\nPress Enter to exit...\n");
	getchar();
    return 0;
}

