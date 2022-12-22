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

  int id, p, np, ierr, source;
  double time1, time2, time3;
  struct timeval start, end;
  long double execTime;

  int rows, offset, i, j, k, n;
  MPI_Status status;

  ierr = MPI_Init(&argc, &argv);     // MPI environment is initialized
  MPI_Comm_size(MPI_COMM_WORLD, &p); // Each process gets and unique ID (rank)
  MPI_Comm_rank(MPI_COMM_WORLD, &id);// Number of processes in communicator will be assigned to variable -> processCount
  
  char node[MPI_MAX_PROCESSOR_NAME]; //get how many nodes there are
  MPI_Get_processor_name(node, &n);

  //np = p - 1; // Number of slave tasks will be assigned to variable -> np

  int N;        // Allowing the user to enter in a matrix
  if(id == 0)   // Set the root process to be 0
    {
      scanf("%d", &N);
      printf("Matrix Size is: %dx%d\n", N, N);
    }


   MPI_Bcast(&N, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);  // broadcast matrix size to all processors
   MPI_Barrier(MPI_COMM_WORLD);    // add a barrier so that all the processes are completed 

  double a[N][N], b[N][N], c[N][N]; // AxB = C // Matrix Holders

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
	  a[i][j]= 2.0;
	  b[i][j]= 3.0;
        }
      }
    // Print Matrix A
    printf("\nMarix A\n\n");
      for(int i = 0; i < N; i++)
	{
	  for(int j =0; j < N; j++)
	    {
	      printf("%.0f\t", a[i][j]);
	    }
	  printf("\n");
	}

    // Print Matrix B
      printf("\nMarix B\n\n");
      for(int i = 0; i < N; i++)
	{
	  for(int j =0; j < N; j++)
	    {
	      printf("%.0f\t", b[i][j]);
	    }
	  printf("\n");
	}
  }
 
  MPI_Bcast(&a, N*N, MPI_DOUBLE, 0, MPI_COMM_WORLD); // broadcast matrix a to all processors
  MPI_Bcast(&b, N*N, MPI_DOUBLE, 0, MPI_COMM_WORLD); // broadcast matrix b to all processors
  rows = N/p;
  // offset = 0; 
  MPI_Bcast(&rows, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);  // broadcast all the rows to the processors

 time1 = MPI_Wtime(); // get time just before work section

 

 /*****************WORKER SECTION*******************/
   // Start Timer

 //gettimeofday(&start, NULL);
 if(id == 0) // Root (Master) Process
  {
    offset = rows;   // offset determines the starting point of the row which will be sent to the slave processes
    // send matrix data to the worker tasks

    for (int dest=1; dest<p; dest++)
    {
      MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);  // Acknowledging the offset of matrix a
      MPI_Send(&rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);    // Acknowledging the number of rows
      MPI_Send(&a[offset][0],rows*N, MPI_DOUBLE,dest,1,MPI_COMM_WORLD); // Send rows of matrix a which will be assigned to slave process to compute
      MPI_Send(&b, N*N, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD); // Matrix B is then sent
      offset = offset + rows; //offset is modified according to number of rows sent to each process
    }

    // Matrix multiplication
    for (k=0; k<N; k++)
      for (i=0; i<rows; i++) // set the initial values of the row summation
	{
        c[i][k] = 0.0;
        for (j=0; j<N; j++) // matrix A's element(i,j) will be multiplied with Matrix B's element (j,k)
          c[i][k] = c[i][k] + a[i][j] * b[j][k];
      }

    // wait for results from all worker tasks
    for (i=1; i<p; i++)
    {
      source = i;
      MPI_Recv(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status); // Receive the offset of particular slave process
      MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status); // Receive the number of rows that each slave process processed
      MPI_Recv(&c[offset][0], rows*N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status); // Calculated rows of the each process will be stored int Matrix C according to their offset and the processed time
    }


  }

 else if(id != 0)
  {
    printf("\nReceiver node %s with id:%d\n\n",node,id);
    source = 0; // Source process Id is defined

    // Slave process waits for the message buffers with tag 1, that Root process sent
    // Each process will receive and execute this separately on their processes
    MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status); // The slave process receives the offset value sent by root process
    MPI_Recv(&rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status); // The slave process receives number of rows sent by root process
    MPI_Recv(&a, rows*N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status); // The slave process receives the sub portion of the matrix a which is assigned by root 
    MPI_Recv(&b, N*N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status); // The slave process receives matrix B

    //Matrix multiplication
    for (k=0; k<N; k++)
      for (i=0; i<rows; i++)
	{
        c[i][k] = 0.0;
        for (j=0; j<N; j++)
          c[i][k] = c[i][k] + a[i][j] * b[j][k];
    }

    MPI_Send(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD); // offset will be sent to root which determines the starting point of the calculated value in matrix c
    MPI_Send(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD); // Number of rows the processor calculated will be sent to the root process
    MPI_Send(&c, rows*N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD); // Resulting matrix with calculated rows will be sent to root process
    
  }

 time2 = MPI_Wtime(); //get time just after work section
// gettimeofday(&end, NULL);
 ///execTime = (((end.tv_sec +  end.tv_usec) * 1e-9) + ((start.tv_sec + start.tv_usec) * 1e-9));
 


 /**************************** END WORKER SECTION******************************/

 // Print the result matrix
  if(id == 0)
    {
      printf("Result matrix is as follows:  \n");
      for(i = 0; i < N; i++)
       {
	for(j = 0; j < N; j++)
	  printf("%6.2f  ", c[i][j]);
      printf("\n");
       }
    }

  // run conventional 1-processor solution
  matmultcon();
  time3 = MPI_Wtime();
  
  

    printf("Approximate %d-processor time Tp: %f seconds \n",p,(time2-time1));
    printf("Approximate 1-processor conventional solution time T1: %f seconds \n", time3-time2);
    printf("Utilization Rate : %f\n",(time3-time2)/((time2-time1)*p));
    printf("Execution Time is = %Lf s \n", execTime);
    MPI_Barrier(MPI_COMM_WORLD);

   MPI_Finalize();
}


      

  
