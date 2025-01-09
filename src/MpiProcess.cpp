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
            int batch_size = remainder / (p-i);
            temp_end = temp_start + batch_size;
            remainder -= batch_size;

            MPI_Send(&temp_start, 1, MPI_INT, i, 30, MPI_COMM_WORLD);
            MPI_Send(&temp_end, 1, MPI_INT, i, 40, MPI_COMM_WORLD);
            temp_start = temp_end;
        }
    }
    MPI_Status status;
    MPI_Recv(&temp_start, 1, MPI_INT, 0, 30, MPI_COMM_WORLD, &status);
    MPI_Recv(&temp_end, 1, MPI_INT, 0, 40, MPI_COMM_WORLD, &status);

    this->road_start = temp_start;
    this->road_end = temp_end - 1;
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
void MpiProcess::sendVehicle(std::vector<Vehicle *>& vehicles_to_send){
    int size = vehicles_to_send.size();
    MPI_Send(&size, 1, MPI_INT, this->getNextRank(), 50, MPI_COMM_WORLD);
    for(auto &vehicle: vehicles_to_send){
        int lane_number = vehicle->getLanePtr()->getLaneNumber();
        MPI_Send(&lane_number, 1, MPI_INT, this->getNextRank(), 100, MPI_COMM_WORLD);
        MPI_Send(vehicle, 1, this->mpi_vehicle, this->getNextRank(), 10, MPI_COMM_WORLD);
    }
    printf("Process: %d, sent %d vehicles to process: %d\n", this->getRank(), size, this->getNextRank());
    for(int i = 0; i < size; i++){
        printf("ID: %d, Position: %d, Speed: %d, in Lane: %d\n", vehicles_to_send[i]->getId(), vehicles_to_send[i]->getPosition(), vehicles_to_send[i]->getSpeed(), vehicles_to_send[i]->getLanePtr()->getLaneNumber());
    }
}

// receive all the vehicles that are about to cross the theshold
std::vector<std::vector<Vehicle*>> MpiProcess::receiveVehicle() {
    int size;
    MPI_Recv(&size, 1, MPI_INT, this->getPrevRank(), 50, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Create 2 lists for lanes 0 and 1
    std::vector<std::vector<Vehicle*>> vehicles_to_recv(2);

    if (size > 0) {
        for (int i = 0; i < size; i++) {
            int lane_num;

            // Receive the lane number
            MPI_Recv(&lane_num, 1, MPI_INT, this->getPrevRank(), 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Dynamically allocate a Vehicle object
            Vehicle* vehicle = new Vehicle();

            // Receive the vehicle data
            MPI_Recv(vehicle, 1, this->mpi_vehicle, this->getPrevRank(), 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Place the new vehicle in the appropriate list (0 or 1 for lane_num)
            if (lane_num == 0 || lane_num == 1) {
                vehicles_to_recv[lane_num].push_back(vehicle);
                printf("Process: %d, received vehicle: %d, speed: %d, position: %d\n", this->getRank(), vehicle->getId(), vehicle->getSpeed(), vehicle->getPosition());
            } else {
                printf("Received unexpected lane_num %d\n", lane_num);
                delete vehicle; // Clean up the dynamically allocated object
            }
        }
    }
    return vehicles_to_recv;
}

/**
* Receive the list of vehicles and check if the new vehicle can be sent without passing
* over vehicles ahead of it
* @param vehicles pointer to list of all the Vehicles of curr process
* @param vehicles_to_send pointer to list of Vehicles to be sent in the next process
* @param newVehicle pointer to the new vehicle we want to send
* @return true if the new vehicle can be sent, false otherwise
*/
bool MpiProcess::allowSending(std::vector<Vehicle *>& vehicles, std::vector<Vehicle *>& vehicles_to_send, Vehicle *newVehicle){
    
    for(int i = 0; i < (int)vehicles.size(); i++){
        // check for vehicles of the same lane 
        if(vehicles[i]->getLanePtr()->getLaneNumber() == newVehicle->getLanePtr()->getLaneNumber()){
            if(vehicles[i]->getPosition() > newVehicle->getPosition() && !vehicles[i]->isInList(vehicles_to_send)){
                return false;
            }
        }
    }
    return true;
}


/**
* Receive the last two vehicles of the next process (one from each lane)
* @return the vector of index of the last two vehicles
*/
std::vector<int> MpiProcess::recvLastVehicles(){
    
    std::vector<int> index_last_vehicles(2);

    MPI_Recv(index_last_vehicles.data(), 2, MPI_INT, this->getNextRank(), 50, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return index_last_vehicles;
}

/**
* Receive the last two vehicles of the next process (one from each lane)
* @return the vector of index of the last two vehicles
*/
void MpiProcess::sendLastVehicles(std::vector<Lane*> lanes){
    std::vector<int> index_last_vehicles = {-1, -1};

    for(int i = 0; i < (int)lanes.size(); i++){
        std::vector<std::deque<Vehicle *>> sites = lanes[i]->getSites();
        for(int j = (int)sites.size()-1; j >= 0; j--){
            if(lanes[i]->hasVehicleInSite(j)){
                index_last_vehicles[i] = j;
                break;
            }
        }
    }

    MPI_Send(index_last_vehicles.data(), 2, MPI_INT, this->getPrevRank(), 50, MPI_COMM_WORLD);
    return;
}

