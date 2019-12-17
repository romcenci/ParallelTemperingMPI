#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

#define Ntemps 25  // Number of ensembles (temperatures)

int main(int argc, char **argv){
  int jobnum, i, k, rank, trials, size;
  int confT[PT];
  double E;
  int dest;

  MPI_Comm comm=MPI_COMM_WORLD;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &size);
  MPI_Request req, reqs[size];
  MPI_Status stat;

  if(rank==0){
    // initiates sending one process to each rank
    for(jobnum=0; jobnum<size-1; jobnum++){
      MPI_Send(&jobnum, 1, MPI_INT, jobnum+1, 0, comm);
    }

    // while still has jobs to do
    for(jobnum=size-1; jobnum<Ntemps; jobnum++){
      // wait for any rank finish to send another job to it 
      MPI_Recv(&E, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, comm, &stat);
      printf("%d %lf\n", stat.MPI_SOURCE, E);
      MPI_Send(&jobnum, 1, MPI_INT, stat.MPI_SOURCE, 0, comm);
    }

    for(dest=1; dest<size; dest++){
      MPI_Recv(&E, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, comm, &stat);
      jobnum=-1;
      MPI_Send(&jobnum, 1, MPI_INT, stat.MPI_SOURCE, 0, comm);	
    }
  }

  // worker work: receive the temperature and 
  else{
    while(1){
      MPI_Recv(&jobnum, 1, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
      if(jobnum>=0) {
	sleep(1.0);
	E=0.1*jobnum;
	MPI_Send(&E, 1, MPI_DOUBLE, 0, 0, comm);
      }
      else{
	break;
      }
    }
  }

  MPI_Finalize();
}
