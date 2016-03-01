#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <iostream>
#include <math.h>
#include <string.h>


using namespace std;

//#define PROG	500000
#define PROG	500

// pseudo kod w C wziety z Wikipedii: https://en.wikipedia.org/wiki/Merge_sort
void TopDownMergeSort(long int tablica[], long int t[], int n);
void TopDownSplitMerge(long int tablica[], int iBegin, int iEnd, long int t[]);
void TopDownMerge(long int tablica[], int iBegin, int iMiddle, int iEnd, long int t[]);
void CopyArray(long int t[], int iBegin, int iEnd, long int tablica[]);


// Array A[] has the items to sort; array B[] is a work array.
void TopDownMergeSort(long int tablica[], long int t[], int n)
{
	TopDownSplitMerge(tablica, 0, n, t);
}

// iBegin is inclusive; iEnd is exclusive (A[iEnd] is not in the set).
void TopDownSplitMerge(long int tablica[], int iBegin, int iEnd, long int t[])
{
	if (iEnd - iBegin < 2)                       // if run size == 1
		return;                                 //   consider it sorted
	// recursively split runs into two halves until run size == 1,
	// then merge them and return back up the call chain
	int iMiddle = (iEnd + iBegin) / 2;              // iMiddle = mid point
	TopDownSplitMerge(tablica, iBegin, iMiddle, t);  // split / merge left  half
	TopDownSplitMerge(tablica, iMiddle, iEnd, t);  // split / merge right half
	TopDownMerge(tablica, iBegin, iMiddle, iEnd, t);  // merge the two half runs
	CopyArray(t, iBegin, iEnd, tablica);              // copy the merged runs back to A
}

//  Left half is A[iBegin :iMiddle-1].
// Right half is A[iMiddle:iEnd-1   ].
void TopDownMerge(long int tablica[], int iBegin, int iMiddle, int iEnd, long int t[])
{
	int i = iBegin, j = iMiddle, k = 0;

	// While there are elements in the left or right runs...
	for (k = iBegin; k < iEnd; k++) {
		// If left run head exists and is <= existing right run head.
		if (i < iMiddle && (j >= iEnd || tablica[i] <= tablica[j])) {
			t[k] = tablica[i];
			i = i + 1;
		}
		else {
			t[k] = tablica[j];
			j = j + 1;
		}
	}
}

void CopyArray(long int t[], int iBegin, int iEnd, long int tablica[])
{
	int k = 0;

	for (k = iBegin; k < iEnd; k++)
		tablica[k] = t[k];
}



int main(int argc, char *argv[])
{
	double begin_t, end_t, begin_t2, end_t2;
	int liczba_watkow, id, oczekiwana_liczba_watkow, liczba_poziomow_scalania, liczba_scalen;
	int i, b = 1;

	long int n = 1;
	bool bZlyWybor = true;
	
	while (bZlyWybor)
	{
		std::cout << "Podaj granice losowanych liczb max. 10000 <-wybrana liczba, +wybrana liczba> : \n";
		std::cin >> b;
		std::cout << "Podaj liczbe elementow tablicy do sortowania (max 6 mln) : \n";
		std::cin >> n;
		std::cout << "Podaj liczbe watkow jaka chcesz uzyc : \n";
		std::cin >> oczekiwana_liczba_watkow;
		if ( ( b>0 && b <= 10000 ) && ( n > 0 && n <= 6000000 ) )
		{
			bZlyWybor = false;
		}
		else
		{
			std::cout << "Podales zle wartosci. \n";
		}
	}


	srand( time( NULL ) );  //inicjalizacja ziarna do rand();

	//dynamicznie przydzielana pamiec do tablic, t - tablica pomocnicza (robocza)
	long int *tablica = new long int[n];
	long int *t = new long int[n];

	//wypelnianie tablicy losowymi liczbami z podanego przez uzytkownika zakresu
	for ( i = 0; i < n; i++ )
    {
        tablica[ i ] = ( ( rand() % (2*b) ) - b );
    }

	begin_t = omp_get_wtime();

	#pragma omp parallel if(n > PROG) num_threads(oczekiwana_liczba_watkow) default(none) \
	shared(n, liczba_watkow, tablica, t) private(id)
	{
		#pragma omp single
		{
			liczba_watkow = omp_get_num_threads();
		}
		id = omp_get_thread_num();
		TopDownMergeSort(tablica + id*n / liczba_watkow, t + id*n / liczba_watkow, (n / liczba_watkow));
	}

	end_t = omp_get_wtime();


	begin_t2 = omp_get_wtime();

	if ( liczba_watkow % 2 == 0 )
	{
		liczba_poziomow_scalania = log2(liczba_watkow);
		for (int i = liczba_poziomow_scalania - 1; i >= 0; i--)
		{
			liczba_scalen = pow(2, i);
			int j;

			#pragma omp parallel for if(n > PROG) num_threads(oczekiwana_liczba_watkow) \
			default(shared) private(j)
			for (j = 1; j <= liczba_scalen; j++)
			{
				TopDownMerge(tablica, (j - 1) / liczba_scalen * n, j / 2*liczba_scalen * n, j / liczba_scalen * n, t);
			}

		}
		CopyArray(t, 0, n-1, tablica);
	}

	end_t2 = omp_get_wtime();

	printf("\n Czas obliczen: %f.\n", end_t - begin_t);
	printf("\n Czas scalania: %f.\n", end_t2 - begin_t2);


	//zwalnianie pamieci
	delete[] t;
	delete[] tablica;

	system("pause");
	return 0;
}



