#include <stdio.h>
#include <string.h>
#include <mpi.h>


// a) write a program that in which process i sends a greeting to process (i+1)%p (Be careful of how i calculates from whom it should receive!)
// b) Should process i send its message to process i + 1 first and then receive the message from process i-1?
// c) Should it first receive and then send? Does it matter?
// d) what happens when the program is run on one processor?

// Main Program a)

void main (int argc, char* argv[])
{
  int rank;           // rank of process
  int p;              // number of processes
  int source;         // rank of sender
  int dst;            // rank of receiver
  int tag = 0;        // tag for messages
  char message[100];  // storage for message
  MPI_Status status;  // return status for receive

  MPI_Init(&argc, &argv);

  MPI_Comm_rank (MPI_COMM_WORLD, &rank);                        
  printf("rank is %d\n", rank);                 // process rank

  MPI_Comm_size(MPI_COMM_WORLD, &p);
  printf("p, the total number of processes, %d\n", p);

  if( rank != 0)
    {
      // Create message
      sprintf(message, "Greetings from process, %d!", rank);
      dst = 0;

      // Use i+1 so that '\0' gets transmitted
      MPI_Send(message, strlen(message)+1, MPI_CHAR, dst, tag, MPI_COMM_WORLD);

    }

  else // if rank = 0
    {
      for (source = 1; source < p; source++)
	{
	  MPI_Recv(message, 100, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
	  printf("%s\n", message);
	}
    }

  MPI_Finalize();
}

  
// b) no since the processes within a communicator are ordered. The rank of a process is its position in the overall order. In a communicator with p processes each process has a unique rank (ID number) between 0 and p-1.

// c) the order of sending and receiving matters since almost all MPI commands are built around point-to-point operations. So no it should be first send and then receive

// d) if the program is run on one processor then each process won't have a unique rank, the rank will be set 0. 
