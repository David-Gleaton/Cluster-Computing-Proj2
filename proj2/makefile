make:	
	mpicc heatgrid.c -o calheat
clean:
	rm calheat
	rm c.*
static:
	mpicc staticheatgrid.c -o calheat
	./calheat 5000
mpi5:
	mpirun -np 5 --mca mpi_cuda_support 0 calheat 5000
mpi2:
	mpirun -np 2 --mca mpi_cuda_support 0 calheat 50000
mpi10:
	mpirun -np 10 --mca mpi_cuda_support 0 calheat 5000
mpi20:
	mpirun -np 20 --mca mpi_cuda_support 0 calheat 5000	
mpi10back:
	mpicc backupheatgrid.c -o calheat
	mpirun -np 10 --mca mpi_cuda_support 0 calheat 50
10new:
	mpicc newstart.c -o calheat
	mpirun -np 10 --mca mpi_cuda_support 0 calheat 500
nolock:
	mpicc nolock.c -o calheat	
	mpirun -np 10 --mca mpi_cuda_support 0 calheat 500
start:
	mpicc gs.c -o calheat
	mpirun -np 5 --mca mpi_cuda_support 0 calheat 5000

