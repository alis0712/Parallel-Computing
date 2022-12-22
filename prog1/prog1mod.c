#include <stdio.h>
#include <mpi.h>

// a) write a program that in which process i sends a greeting to process (i+1)%p (Be careful of how i calculates from whom it should receive!)
// b) Should process i send its message to process i + 1 first and then receive the message from process i-1?
// c) Should it first receive and then send? Does it matter?
// d) what happens when the program is run on one processor?

// Main Program a)
     
   void main(int argc, char **argv)
   {
     int p,id; // number of processes and rank

       MPI_Init(&argc, &argv);

      // find out MY process ID, and how many processes were started.

       MPI_Comm_rank(MPI_COMM_WORLD, &id);
       MPI_Comm_size(MPI_COMM_WORLD, &p);

      printf("Greetings from process %i out of %i processors\n",(id+1)%p, p);

       MPI_Finalize();
   }


// b) no since the processes within a communicator are ordered. The rank of a process is its position in the overall order. In a communicator with p processes each process has a unique rank (ID number) between 0 and p-1.

// c) the order of sending and receiving matters since almost all MPI commands are built around point-to-point operations. So no it should first send and then receive

// d) if the program is run on one processor then each process won't have a unique rank, the rank will be set 0. 
