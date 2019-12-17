/****************************************************************************************/
/* The code bellow is a implementation of Parallel Tempering technique that avoids      */
/*   the load balance existent when a unique temperature is simulated per core. In      */
/*   addition, this implementation permites run Parallel Tempering of more temperatures */
/*   than the number of disponibilized cores with a simple master-worker communication  */
/*   protocol.									        */
/*   										        */
/* All places that have a ... must be changed depending on the purpose and model.       */
/* 										        */
/* This code was originally made by Rômulo Cenci with the help of the teachers:	        */
/*  Gavin Pringle, Axel Kohlmeyer and Ivan Girotto, at the 2º Latin American School     */
/*  on Parallel Programming for High Performance Computing, as final project.	        */
/* 										        */
/* The original sequential code was provided by Leonardo Rigon, and the main	        */
/*  ideias of the algorithm was from Lucas Nicolao.				        */
/****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

#define Ntemps 100          // Number of configurations (temperatures)
#define Ntrials 1000        // Number of trials change in temperatures
#define Kmax 100            // Number of steps until change temperatures
#define Tini 0.04
#define Tfim 0.01

// Here goes the update function, that depends on the system
void update(..., double T){
  ...
}

// This function measure energy of a configuration
double ener(...){
  double E;
  
  ...
  
  return E;
}

int main(int argc, char **argv){
  int jobnum, jobnumE, i, k, rank, trials, Nranks;
  int confT[Ntemps];
  double T;
  double allE[Ntemps], allT[Ntemps];
  double delta, aux;
  int dest;

  MPI_Comm comm=MPI_COMM_WORLD;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &rank);
  MPI_Comm_size(comm, &Nranks);
  MPI_Status stat;

  ///////////////////////////////////////////////////////////////////////////////
  // Create a new type that is associated with the configuration
  // If is another system or model, all the lines until MPI_Datatype needs to change
  // The type exampled below was of a system of particles in 2d: x[i] e y[i]
  struct{
    int jobnumE;
    double E;
    ...                      // Ex: double x[N], y[N];
  } configs[Ntemps]; 
  
  int count = 2;
  int array_of_blocklengths[2] = {1, 2*N+1};
  MPI_Datatype array_of_types[2] = {MPI_INT, MPI_DOUBLE};
  MPI_Aint extent;
  MPI_Type_extent(MPI_INT, &extent);
  MPI_Aint array_of_displacements[2]={0, extent};
  ///////////////////////////////////////////////////////////////////////////////

  MPI_Datatype MPI_Config;
  MPI_Type_create_struct(count, array_of_blocklengths, array_of_displacements, array_of_types, &MPI_Config);
  MPI_Type_commit(&MPI_Config);

  // initiate the temperature of each configuration
  if(rank==0){
    for(jobnum=0; jobnum<Ntemps; jobnum++){
      // Here goes the distributions of temperatures
      allT[jobnum]=Tini-(Tini-Tfim)*((double)jobnum/(Ntemps-1));

      // Map of what configuration is associated with which temperature
      confT[jobnum]=jobnum;

      // initial conditions of the configurations
      ...
    }
  }

  // bcast list of temperatures to all process so just send a integer instead of a double
  MPI_Bcast(&allT, Ntemps, MPI_DOUBLE, 0, comm);

  for(trials=0; trials<Ntrials; trials++){
    // master process
    if(!rank){
      // initiates sending one process to each rank
      for(jobnum=0; jobnum<Nranks-1; jobnum++){
	configs[confT[jobnum]].jobnumE = jobnum;
	MPI_Send(&configs[confT[jobnum]], 1, MPI_Config, jobnum+1, 0, comm);
      }

      // still has jobs to do
      for(jobnum=Nranks-1; jobnum<Ntemps; jobnum++){
	// wait for any rank finish to send another job to it 
	MPI_Recv(&jobnumE, 1, MPI_INT, MPI_ANY_SOURCE, 0, comm, &stat);
	MPI_Recv(&configs[confT[jobnumE]], 1, MPI_Config, stat.MPI_SOURCE, 1, comm, MPI_STATUS_IGNORE);

	configs[confT[jobnum]].jobnumE = jobnum;
	MPI_Send(&configs[confT[jobnum]], 1, MPI_Config, stat.MPI_SOURCE, 0, comm);
      }

      // for the last ones just recv energies and send a -1 to says that its over
      for(dest=1; dest<Nranks; dest++){
	MPI_Recv(&jobnumE, 1, MPI_INT, MPI_ANY_SOURCE, 0, comm, &stat);
	MPI_Recv(&configs[confT[jobnumE]], 1, MPI_Config, stat.MPI_SOURCE, 1, comm, MPI_STATUS_IGNORE);
	
	configs[confT[jobnum]].jobnumE = -1;
	MPI_Send(&configs[confT[jobnum]], 1, MPI_Config, stat.MPI_SOURCE, 0, comm);
      }

      for(jobnum=0; jobnum<Ntemps; jobnum++){
	printf("%d %d %lf\n", trials, jobnum, configs[confT[jobnum]].E);
      }
    }

    // worker process
    else{
      while(1){
	MPI_Recv(&configs[0], 1, MPI_Config, 0, 0, comm, MPI_STATUS_IGNORE);
	if(configs[0].jobnumE>=0) {
	  // update Nranks ensembles at the same time, one per rank
	  for(k=0; k<Kmax; k++){
	    update(..., allT[configs[0].jobnumE]);
	  }
	
	  // measure the energy and send back to the root
	  configs[0].E = ener(...);
	  MPI_Send(&configs[0].jobnumE, 1, MPI_INT, 0, 0, comm);
	  MPI_Send(&configs[0], 1, MPI_Config, 0, 1, comm);
	}
	else{
	  break;
	}
      }
    }

    // decides who changes temperature with who, ensuring detailed balance condition is satisfied
    if(rank==0){
      if(trials%2==0){
	for(i=0; i<Ntemps-1; i+=2){
	  delta=-(1./allT[i+1]-1./allT[i])*(configs[confT[i+1]].E-configs[confT[i]].E);
	  if(delta<0 || FRANDOM<exp(-delta)){
	    aux=confT[i];
	    confT[i]=confT[i+1];
	    confT[i+1]=aux;
	  }
	}
      }
      else{
	for(i=1; i<Ntemps-1; i+=2){
	  delta=-(1./allT[i+1]-1./allT[i])*(configs[confT[i+1]].E-configs[confT[i]].E);
	  if(delta<0 || FRANDOM<exp(-delta)){
	    aux=confT[i];
	    confT[i]=confT[i+1];
	    confT[i+1]=aux;
	  }
	}
      }
    }
  }
  fclose(f1);

  MPI_Finalize();
}
