/************************************************************************************/
/* The code bellow is a implementation of Parallel Tempering technique that run one */
/*  temperature per core, carrying out a load balance, because some process has to  */
/*  wait by the others in a MPI_BARRIER.					    */
/* 										    */
/* All places that have a ... must be changed depending on the purpose and model.   */
/* 										    */
/* This code was originally made by Rômulo Cenci with the help of the teachers:	    */
/*  Gavin Pringle, Axel Kohlmeyer and Ivan Girotto, at the 2º Latin American School */
/*  on Parallel Programming for High Performance Computing, as final project.	    */
/* 										    */
/* The original sequential code was provided by Leonardo Rigon, and the main	    */
/*  ideias of the algorithm was from Lucas Nicolao.				    */
/************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

#define Ntemps 48           // Number of configurations (temperatures)
#define Ntrials 1000        // Number of trials change in temperatures
#define Kmax 500            // Number of steps until change temperatures
#define Tini 0.050
#define Tfim 0.005

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
  int jobnum, i, k, trials;
  int confT[Ntemps];
  double E, T;
  double allE[Ntemps], allT[Ntemps];
  double delta, aux;
  FILE *f1;

  MPI_Comm comm=MPI_COMM_WORLD;
  MPI_Request req;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &jobnum);

  // initiate the temperature of each configuration
  if(jobnum==0){
    for(i=0; i<Ntemps; i++){
      confT[i]=i;
      allT[i]=Tini-(Tini-Tfim)*((double)i/(Ntemps-1));
    }
  }

  // initial conditions of the configurations
  ...
  
  for(trials=0; trials<Ntrials; trials++){
    MPI_Scatter(allT, 1, MPI_DOUBLE, &T, 1, MPI_DOUBLE, 0, comm);

    // update all the ensembles in the same time, one per thread
    for(k=0; k<Kmax; k++){
      update(..., T);
    }

    // wait all the evolutions finish
    MPI_Barrier(MPI_COMM_WORLD);

    // measure the energy of each ensemble and send to master
    E = ener(...);

    MPI_Gather(&E, 1, MPI_DOUBLE, allE, 1, MPI_DOUBLE, 0, comm);
    
    // decides who changes temperature with who, ensuring detailed balance condition is satisfied
    if(rank==0){
      if(trials%2==0){
	for(i=0; i<Ntemps-1; i+=2){
	  delta=-(1./allT[confT[i+1]]-1./allT[confT[i]])*(allE[confT[i+1]]-allE[confT[i]]);
	  if(delta<0 || FRANDOM<exp(-delta)){
	    aux=confT[i];
	    confT[i]=confT[i+1];
	    confT[i+1]=aux;
	    
	    aux=allT[i+1];
	    allT[i+1]=allT[i];
	    allT[i]=aux;
	  }
	}
      }
      else{
	for(i=1; i<Ntemps-1; i+=2){
	  delta=-(1./allT[confT[i+1]]-1./allT[confT[i]])*(allE[confT[i+1]]-allE[confT[i]]);
	  if(delta<0 || FRANDOM<exp(-delta)){
	    aux=confT[i];
	    confT[i]=confT[i+1];
	    confT[i+1]=aux;
	    
	    aux=allT[i+1];
	    allT[i+1]=allT[i];
	    allT[i]=aux;
	  }
	}
      }
    }
  }

  MPI_Finalize();
}
