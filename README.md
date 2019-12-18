# Parallel Tempering in MPI

To use this code, substitute in every place that have a `...` the things that are necessary depending on your purpose and model.


## partempMPI_taskFarm.c
	
This code is the simplest implementation of Parallel Tempering technique, it run one temperature per core, carrying out a load balance, because some process has to wait by the others in a MPI_BARRIER.

<p align="center">
  <img align=middle src="presentation/Imagens/taskFarm.png" width="500">
</p>


## partempMPI_loadBalance.c

This code is a implementation of Parallel Tempering technique that avoids the load balance existent when a unique temperature is simulated per core. In addition, this implementation permites run Parallel Tempering of more temperatures than the number of disponibilized cores with a simple master-worker communication protocol.

<p align="center">
<img align=middle src="presentation/Imagens/loadBalance.png" width="500">
</p>


### Some benchmark

Quickly measuring the time of a small simulation, we can simply benchmark the algorithm. Below we can see measures of the running time of the simulation for different number of cores.

<p align="center">
<img align=middle src="presentation/Imagens/exectime.png" width="300">
<img align=middle src="presentation/Imagens/speedup.png" width="300">
<img align=middle src="presentation/Imagens/efficiency2.png" width="300">
</p>
