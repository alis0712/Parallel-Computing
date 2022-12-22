#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define N 1024
//double initialTime;
//double finalTime;

void shuffle (int *array, int length) // Shuffling array elements
{
    int i, temp;
    while (length-- > 1)
      {
        i = rand() % (length + 1);
        temp  = array[i];
        array[i] = array[length];
        array[length] = temp;
    }
}


void printList(int *array, int length) // print array elements
{
 
  int i = 0;
  for(i = 0; i < length; i++)
    {
      //array[i] = rand();
      printf("%d\t", array[i]);
    }
  printf("\n");
}

int *merge(int *list1, int n1, int *list2, int n2) // merging array list 1 and list 2 after being sorted
  {
    // int i, j, k;
    int* temp;

    temp = (int*)malloc((n1+n2)*sizeof(int));
   int i=0; int j=0; int k=0;

    while(i < n1 && j < n2)
      {
	if(list1[i] < list2[j])
	  {
	    temp[k] = list1[i];
	    i++, k++;
	  }

	else
	  {
	    temp[k] = list2[j];

	    j++; k++;
	  }
      }

      if(i == n1)
	  {
	  while(j < n2)
	    {
	      temp[k] = list2[j];
	      j++; k++;
	    }
	  }
	else
	  {
	    while(i < n1)
	      {
		temp[k] = list1[i];
		i++; k++;
	      }
	  }
	return temp;
      }


void swap(int *list, int i, int j) // make swaps along the away while sorting
    {
      int k;
      k = list[i];
      list[i] = list[j];
      list[j] = k;
    }

void bubblesort(int *list, int n) // sorting the list accordingly
    {
      int i, j;
      for(i = n - 2; i >= 0; i--)
	{
	  for(j = 0; j <= i; j++)
	    {
	      if(list[j] > list[j + 1])
		{
		  swap(list, j, j + 1);
		}
	    }
	}
    }

    int main(int argc, char **argv)
    {
      int *data;
      //int *list;
      int *l1;
      int *l2;
      int m,n = N;
      int id,p;
      int rows;
      int i;
      int step;
      double time0, time1, time2, time3;
      // int aa[N/p][N], int cc[N/p][N];

      MPI_Status status;

      MPI_Init(&argc, &argv);
      MPI_Comm_rank(MPI_COMM_WORLD, &id);
      MPI_Comm_size(MPI_COMM_WORLD, &p);
      MPI_Barrier(MPI_COMM_WORLD);

      if(id == 0)
	{
	  
	  int offset;
	  rows = n/p; // dividing the array into equal sized chunks
	  offset = n%p; // setting the remainder to zero
	  data = (int*)malloc((n+p-offset)*sizeof(int)); // creating the array for all the data points
	  printf("The size of the list is %d elements\n",N);
	  for(i = 0; i < n; i++) // printing the unsorted array elements
	    {
	      data[i] = N - i;
	    }
	  if(offset != 0)
	    {
	      for(i = n; i < n + p - offset; i++)
		{
		  data[i] = 0;
		}
	      rows = rows + 1;
	    }

	  printf("Initializing bubblesort sequentially \n");
	  //initialTime = clock();
	  time0 = MPI_Wtime();
	  shuffle(data, N);
	  printf("The unsorted list sequentially is: \n"); 
	  printList(data, N); // printing unsorting list
	  bubblesort(data,N);
	  printf("The sorted list sequentially  is: \n"); 
	  printList(data,N);
	  //finalTime = clock();
	  time1 = MPI_Wtime();
	  //printf("Execution time sequentially  is = %f seconds \n", (finalTime - initialTime)/CLOCKS_PER_SEC);
	   
	  
	  
	  
	  printf("Initializing bubblesort in parallely\n");
	  //initialTime = clock();
	  time2 = MPI_Wtime();

	  MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD); // initializing broadcast
	  l1 = (int*)malloc(rows*sizeof(int)); // creating an array for list 1
	  MPI_Scatter(data, rows, MPI_INT, l1, rows, MPI_INT, 0, MPI_COMM_WORLD); // scattering list1 across the processors
	  
	  shuffle(data, N); // shuffling the data 
	  printf("The unsorted list is: \n"); 
	  printList(data, N); // printing unsorting list
	  printf("\n");
	  bubblesort(l1, rows); // sorting all the array elements in list 1 (could be halved)
	}
      else
	{
	  MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
	  l1 = (int*)malloc(rows*sizeof(int));
	  MPI_Scatter(data, rows, MPI_INT, l1, rows, MPI_INT, 0, MPI_COMM_WORLD);
	  bubblesort(l1, rows);
	}

      step = 1;// up to log_2 p merge steps while printing
      while(step < p)
	{
	  if(id % (2*step) == 0)
	    {
	      if(id + step < p) // id is multiple of 2*step: merge in chunk from id+step (if it exists)
		{

		   MPI_Recv(&m, 1, MPI_INT, id + step, 0, MPI_COMM_WORLD, &status);
		  l2 = (int*)malloc(m*sizeof(int)); // initializing array elements for list 2
		   MPI_Recv(l2, m, MPI_INT, id + step, 0, MPI_COMM_WORLD, &status);
		   //MPI_Gather(l2, (N*N)/p , MPI_INT, cc, (N*N)/p, MPI_INT, 0, MPI_COMM_WORLD);

		  l1 = merge(l1, rows, l2, m); // merging list of elements obtained from list 1 and list 2 
		  rows = rows + m;
		}
	    }
	  else
	    {

	     
	      int next  = id-step; // id is multiple of 2*step: merge in the next chunk from id - step (if it exists)
	      MPI_Send(&rows, 1, MPI_INT, next, 0, MPI_COMM_WORLD);
	      MPI_Send(l1, rows, MPI_INT, next, 0, MPI_COMM_WORLD);
	      // MPI_Gather(l1, (N*N)/p , MPI_INT, cc, (N*N)/p, MPI_INT, 0, MPI_COMM_WORLD);
	     
	      break;
	    }
	  step = step*2;
	}
      if(id == 0)
	{
	  //finalTime = clock();
     	  printf("The sorted list becomes: \n");
	  printList(l1,N);
	  time3 = MPI_Wtime();
	
	  //printf("Execution time parallely is = %f seconds \n", (finalTime - initialTime)/CLOCKS_PER_SEC);
		  printf("Approximate %d-processor time Tp: %f seconds \n",p,(time3-time2));
          printf("Approximate 1-processor conventional solution time T1: %f seconds \n",(time1-time0));
          printf("Utilization Rate : %f\n",(time1-time0)/((time3-time2)*p));
	}
    
      MPI_Finalize();
    }
	
      

    
