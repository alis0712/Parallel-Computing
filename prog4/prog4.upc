#include <upc_relaxed.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "bupc_timers.h"

//#define N 16
//#define N 32
#define N 64
//#define N 128
//#define N 256


// Segmentation dump
//#define N 512
//#define N 2048


typedef double element;

#define second() (TIME()/1000000.0)
struct mat
{
	
	element x[N][N];
	
};

shared struct mat a [THREADS];
shared struct mat b [THREADS];
shared struct mat c [THREADS];

element result [N][N]; 

void initializeMatrix(shared struct mat* p, shared struct mat* q, shared struct mat* r)
{
	int i,j;
	for(i = 0; i < N; i++)
	{
		for(j = 0; j < N; j++)
		{
			if(MYTHREAD == 0)
			{
				p->x[i][j] = (MYTHREAD + 1)*(2.0);
				q->x[i][j] = (MYTHREAD + 1)*(3.0);
				r->x[i][j] = (MYTHREAD + 1)*(0.0);
			}
			else
			{
				p->x[i][j] = (2.0);
				q->x[i][j] = (3.0);
				r->x[i][j] = (0.0);
			}
		}
	}
}

void printMatrix(int n, double matrix[n][n])
{
  for(int i = 0; i < n; i++)
  {
		for(int j = 0; j < n; j++)
		{
		  printf("%6.2f" , matrix[i][j]);
		  printf(" ");
		}
		printf("\n");
   }
}

void matmult(element *col, element *row)
{
	int i, j;
	for (i = 0; i < N; i++)
	{
		for (j = 0; j < N; j++)
		{
			result[i][j] = col[i] * row[j];
		}
	}
}

int main (int argc, char** argv)
{
	int i, j, k, m, n;
	int size;
	int matown;
	element sum; 
	element acol[N], brow[N];
	double sec;
	struct mat temp;
	shared struct mat *ptr;
	
	
	// intialize matrices
	if(MYTHREAD == 0)
	{
		upc_forall(i = 0; i < THREADS; i++; &a[i])
		{
			initializeMatrix(&a[i], &b[i], &c[i]);
			//initializeResultMatrix(&c[i]);
		}
	}
	
	
	if(MYTHREAD == 0)
	{
		printf("Matrix A is: \n");
		printMatrix(N,a);
		
		printf("Matrix B is: \n");
		printMatrix(N,b); 
	}
	
	printf("THREAD %d starting\n", MYTHREAD); // not sure why this was needed
	upc_barrier;
  
	sec = second(); // start the timer
	size = (int) sqrt(THREADS);
	
	for (k = 0; k < N * size; k++)
	{
		// copy all the columns in a 
		matown = ((MYTHREAD / size) * size) + k / N;
		temp = a[matown];
		
		for(i = 0; i < N ; i++)
		{
			acol[i] = temp.x[i][k % N];
		}
		
		// copy all the rows in b
		matown = ((k / N) * size) + MYTHREAD % size;
		temp = b[matown];
		
		for(i = 0; i < N ; i++)
		{
			brow[i] = temp.x[k % N][i];
		}
		
		// now multiply them together
		upc_forall(i = 0; i < THREADS; i++; &c[i])
		{
			matmult(acol, brow);
			ptr = &c[i];
			
			for (m = 0; m < N; m++)
			{
				for(n = 0; n < N; n++)
				{
					ptr->x[m][n] += result[m][n];
				}
			}
		}
		
		upc_barrier;
	}
	
  //printf("THREAD %d: running for %2f seconds\n", MYTHREAD, (second() - sec));
  printf("Utilization rate is : %2f\n",(second() - sec)*MYTHREAD);
  
  if(MYTHREAD == 0)
  {
	  printf("The Result Matrix C is: \n");
	  printMatrix(N,c);
	  printf("Total execution time is %2f seconds\n", (second() - sec));
	 
	 
  }
 
  
  upc_barrier;
}
		