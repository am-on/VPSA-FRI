#include <stdio.h>
#include <string.h>
#include <mpi.h> // knji�nica MPI

int main(int argc, char *argv[])
{
	int my_rank; // rank (oznaka) procesa
	int num_of_processes; // �tevilo procesov
	int source; // rank po�iljatelja
	int destination; // rank sprejemnika
	int tag = 0; // zaznamek sporo�ila
	char message[100]; // rezerviran prostor za sporo�ilo
	MPI_Status status; // status sprejema

	MPI_Init(&argc, &argv); // inicializacija MPI okolja
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // poizvedba po ranku procesa
	MPI_Comm_size(MPI_COMM_WORLD, &num_of_processes); // poizvedba po �tevilu procesov

	fflush(stdout);

	if( my_rank == 0 ) {
		destination = 1;
		sprintf(message, "%d - %d", my_rank, destination);

		MPI_Send(
			message,
			(int)strlen(message)+1,
			MPI_CHAR,
			destination,
			tag,
			MPI_COMM_WORLD
		);

		printf("   %d sent %d, msg %s \n", my_rank, destination, message);
	} else {
		int recv_rank = my_rank - 1;
		MPI_Recv(
			message,
			100,
			MPI_CHAR,
			recv_rank,
			tag,
			MPI_COMM_WORLD,
			&status
		);

		destination = (num_of_processes - 1) == my_rank ? 0 : (my_rank + 1);
		sprintf(message, "%s - %d", message, destination);

		MPI_Send(
			message,
			(int)strlen(message)+1,
			MPI_CHAR,
			destination,
			tag,
			MPI_COMM_WORLD
		);

		printf("   %d sent %d, msg %s \n", my_rank, destination, message);
	}

	if( my_rank == 0 ) {
		int recv_rank = num_of_processes - 1;
		MPI_Recv(
			message,
			100,
			MPI_CHAR,
			recv_rank,
			tag,
			MPI_COMM_WORLD,
			&status
		);

		printf("final msg: %s\n", message);
		fflush(stdout);
	}

	MPI_Finalize();

	return 0;
}
