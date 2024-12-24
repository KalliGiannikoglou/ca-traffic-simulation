#include "MpiProcess.h"
#include <cstddef>


const int NO_RANK = -1;

MpiProcess::MpiProcess(int argc, char **argv){

    // Initialize the MPI environment
    MPI_Init(&argc, &argv);
  
    // Get the total number of processes 
    int num_of_processes;
    MPI_Comm_size(MPI_COMM_WORLD, &num_of_processes);

    // Get the rank of the calling process
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    printf("Hello world from process %d out of %d processors\n", my_rank, num_of_processes);

    this->rank = my_rank;
    this->num_of_processes = num_of_processes;

    // Set prev rank
    if(this->rank == 0)
        this->prev_rank = NO_RANK;
    else
        this->prev_rank = this->rank - 1;
    
    // Set next rank
    if(this->rank == num_of_processes - 1)
        this->next_rank = NO_RANK;
    else
        this->next_rank = this->rank + 1;

    printf("My prev rank is: %d and my next rank is: %d\n", this->prev_rank, this->next_rank);

    this->defineMpiVehicle();
}

int MpiProcess::getRank(){ return this->rank; }

int MpiProcess::getNextRank(){ return this->next_rank; }

int MpiProcess::getPrevRank(){ return this->prev_rank; }

int MpiProcess::getNumOfProcesses(){ return this->num_of_processes; }

int MpiProcess::getStartPosition(){ return this->road_start; }
        
int MpiProcess::getEndPosition(){ return this->road_end; }


MpiProcess::~MpiProcess(){}


void MpiProcess::divideRoad(int road_length){
    int temp_start, temp_end, remainder;

    if(this->getRank() == 0){
        temp_start = 0;
        remainder = road_length;
        int p = this->getNumOfProcesses();
        
        for(int i = 0; i < p; i++){
            temp_end = temp_start + remainder / (p-i);
            remainder = road_length - temp_end;

            MPI_Send(&temp_start, 1, MPI_INT, i, 30, MPI_COMM_WORLD);
            MPI_Send(&temp_end, 1, MPI_INT, i, 40, MPI_COMM_WORLD);

            temp_start = temp_end;
        }
    }

    MPI_Status status;
    MPI_Recv(&temp_start, 1, MPI_INT, 0, 30, MPI_COMM_WORLD, &status);
    MPI_Recv(&temp_end, 1, MPI_INT, 0, 40, MPI_COMM_WORLD, &status);

    this->road_start = temp_start;
    this->road_end = temp_end;

    printf("Process: %d, my road start: %d, my road end: %d\n", this->getRank(), this->road_start, this->road_end);

}

void MpiProcess::defineMpiVehicle(){
     int block_lengths[13] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        MPI_Datatype types[13] = {
            MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, 
            MPI_INT, MPI_INT, MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_INT};

        MPI_Aint offsets[13];
        offsets[0] = offsetof(Vehicle, id);
        offsets[1] = offsetof(Vehicle, position);
        offsets[2] = offsetof(Vehicle, speed);
        offsets[3] = offsetof(Vehicle, max_speed);
        offsets[4] = offsetof(Vehicle, gap_forward);
        offsets[5] = offsetof(Vehicle, gap_other_forward);
        offsets[6] = offsetof(Vehicle, gap_other_backward);
        offsets[7] = offsetof(Vehicle, look_forward);
        offsets[8] = offsetof(Vehicle, look_other_forward);
        offsets[9] = offsetof(Vehicle, look_other_backward);
        offsets[10] = offsetof(Vehicle, prob_slow_down);
        offsets[11] = offsetof(Vehicle, prob_change);
        offsets[12] = offsetof(Vehicle, time_on_road);

        MPI_Type_create_struct(13, block_lengths, offsets, types, &mpi_vehicle);
        MPI_Type_commit(&mpi_vehicle);
}

// send all the vehicles that are about to cross the thresold
void MpiProcess::sendVehicle(std::vector<Vehicle *>& vehicles_to_send, int destProcess){
    int size = vehicles_to_send.size();
    MPI_Send(&size, 1, MPI_INT, destProcess, 10, MPI_COMM_WORLD);
    for(auto &vehicle: vehicles_to_send){
        MPI_Send(&vehicle, 1, this->mpi_vehicle, destProcess, 10, MPI_COMM_WORLD);
    }
    printf("Process: %d, sent %d vehicles to process: %d\n", this->getRank(), size, destProcess);
    for(int i = 0; i < size; i++){
        printf("ID: %d, Position: %d, Speed: %d\n", vehicles_to_send[i]->getId(), vehicles_to_send[i]->getPosition(), vehicles_to_send[i]->getSpeed());
    }
}

// receive all the vehicles that are about to cross the theshold
std::vector<Vehicle> MpiProcess::receiveVehicle(std::vector<Vehicle>& vehicles_to_recv, int srcProcess){
    int size;
    MPI_Recv(&size, 1, MPI_INT, srcProcess, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if(size > 0){
        vehicles_to_recv.resize(size);
        for (int i = 0; i < size; i++) {
            MPI_Recv(&vehicles_to_recv[i], 1, this->mpi_vehicle, srcProcess, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        printf("Process: %d, received %d vehicles from process: %d\n", this->getRank(), size, srcProcess);
        for(int i = 0; i < size; i++){
            printf("ID: %d, Position: %d, Speed: %d\n", vehicles_to_recv[i].getId(), vehicles_to_recv[i].gap_forward, vehicles_to_recv[i].getSpeed());
        }
    }
    return vehicles_to_recv;
}