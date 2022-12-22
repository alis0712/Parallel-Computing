#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <sys/time.h>


double *createMatrix(int n, int m)  // creates an empty matrix of size nxm
{
  double *matrix;
  matrix = malloc(m*n*sizeof(*matrix));

  for(int h = 0; h < m*n; h++)
    {
      matrix[h] = h + 1;
    }
  return matrix;
}

double *takeMatrix(int n, int m)  // takes in values from the user and creates a matrix of size nxm
{
  double *matrix;
  matrix = malloc(n*sizeof(*matrix));

  for(int h = 0; h < m*n; h++)
    {
      scanf("%lf", &matrix[h]);
    }
  return matrix;
}


int main(int argc, char* argv[])
{
  // Define variables

  int id, p, np, ierr, i, j, k;
  double time1, time2, time3;
  struct timeval start, end;
  long double execTime;
  int firstProcid;
  int secondProcid;

  MPI_Status status;

  ierr = MPI_Init(&argc, &argv);     // MPI environment is initialized
  MPI_Comm_size(MPI_COMM_WORLD, &p); // Each process gets and unique ID (rank)
  MPI_Comm_rank(MPI_COMM_WORLD, &id);// Number of processes in communicator will be assigned to variable -> processCount
  
  int N;        // Allowing the user to enter in a matrix
  if(id == 0)   // Set the root process to be 0
    {
      scanf("%d", &N);
      printf("Matrix Size is: %dx%d\n", N, N);
    }

  firstProcid = id * N/p;
  secondProcid = (id + 1) * N/p;


   MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);  // broadcast matrix size to all processors
   //MPI_Barrier(MPI_COMM_WORLD);    // add a barrier so that all the processes are completed 

  double a[N][N], b[N][N], c[N][N]; // AxB = C // Matrix Holders
  double aa[N/p][N] /* ,bb[N/p][N] */, cc[N/p][N];

  // conventional one-processor solution

  void matmultcon()
{
  int i, j, k;
  for(i=0;i<N;i++)
    for(j=0;j<N;j++)
      for(k=0,c[i][j]=0.0; k<N; k++)
	c[i][j] = c[i][j] + a[i][k] * b[k][j];
} // end conventional one-processor solution

   /****************MASTER TASK*******************/
  if (id == 0)                                  // Matrix A is 2.0 and Matrix B is 3.0 for simplicity
  {
    for (i=0; i<N; i++)
      {
      for (j=0; j<N; j++)
	{
	  aa[i][j]= 2.0;
	  b[i][j]= 3.0;
        }
      }
    // Print Matrix A
    printf("\nMarix A\n\n");
      for(int i = 0; i < N; i++)
	{
	  for(int j =0; j < N; j++)
	    {
	      printf("%.0f\t", aa[i][j]);
	    }
	  printf("\n");
	}

    // Print Matrix B
      printf("\nMatrix B\n\n");
      for(int i = 0; i < N; i++)
	{
	  for(int j = 0; j < N; j++)
	    {
	      printf("%.0f\t", b[i][j]);
	    }
	  printf("\n");
	}
  }

  
  time1 = MPI_Wtime(); // get time just before work section

  if(id == 0)
    {
       MPI_Scatter(a, N*N/p, MPI_DOUBLE, MPI_IN_PLACE, N*N/p, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
  else
    {
      MPI_Scatter(a, N*N/p, MPI_DOUBLE, aa[firstProcid], N*N/p, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    
  //MPI_Scatter(b, N*N/p, MPI_INT, bb, N*N/p, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&b, N*N, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Barrier(MPI_COMM_WORLD);


/*****************WORKER SECTION*******************/
   // Start Timer

 gettimeofday(&start, NULL);

    /* // Matrix multiplication */
    /* for(int k = 0; k < N; k++) */
    /* { */
    /*   for (int i = 0; i < N/p; i++) */
    /*   { */
    /* 	cc[i][k] = 0.0; */
    /*     for (int j = 0; j < N; j++) */
    /*     { */
    /*       cc[i][k] += aa[i][j] * b[j][k]; */
    /*     } */
    /*   } */
    /* } */
 if(id == 0)
   {
 printf("processor %d(from row %d to %d)\n", firstProcid, secondProcid-1);
 for(int i = firstProcid;  i < secondProcid; i++)
   {
     for(j = 0; j < N; j++)
       {
	 cc[i][j] = 0.0;
	   for(k = 0; k < N; k++)
	     {
	     cc[i][j] += aa[i][k] * b[k][j];
	     }
       }
   }
   }
 else
   {
     for(i = 0; i < p; i++)
       {
	 if(id == i)
	   {
	     for(int i = firstProcid;  i < secondProcid; i++)
   {
     for(j = 0; j < N; j++)
       {
	 cc[i][j] = 0.0;
	   for(k = 0; k < N; k++)
	     {
	     cc[i][j] += aa[i][k] * b[k][j];
	     }
       }
   }
   }
       }
   }
     
	 
    MPI_Gather(cc[firstProcid], N*N/p, MPI_DOUBLE, c, N*N/p, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

 time2 = MPI_Wtime(); //get time just after work section
 gettimeofday(&end, NULL);
 execTime = (((end.tv_sec +  end.tv_usec) * 1e-9) + ((start.tv_sec + start.tv_usec) * 1e-9));
 


 /**************************** END WORKER SECTION******************************/

 // Print the result matrix
  if(id == 0)
    {
     printf("\nResult matrix is as follows:  \n");
      for(i = 0; i < N/p; i++)
       {
	for(j = 0; j < N; j++)
	  printf("%6.2f  ", cc[i][j]);
       }
      printf("\n");
    }
    
   

  // run conventional 1-processor solution
  matmultcon();
  time3 = MPI_Wtime();
  
  

    printf("\nApproximate %d-processor time Tp: %f seconds \n",p,(time2-time1));
    printf("\nApproximate 1-processor conventional solution time T1: %f seconds \n", time3-time2);
    printf("\nUtilization Rate : %f\n",(time3-time2)/((time2-time1)*p));
    printf("\nExecution Time is = %Lf s \n", execTime);
    MPI_Barrier(MPI_COMM_WORLD);

   MPI_Finalize();
}


      

  
