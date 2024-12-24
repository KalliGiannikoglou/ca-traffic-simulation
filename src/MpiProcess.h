#ifndef CA_TRAFFIC_SIMULATION_MPIPROCESS_H
#define CA_TRAFFIC_SIMULATION_MPIPROCESS_H

#include <mpi.h>
#include <stdio.h>

#include "Inputs.h"
#include "Vehicle.h"

using namespace std;

class MpiProcess{
    private:
        int rank;
        int next_rank;
        int prev_rank;
        int num_of_processes;

        int road_start;
        int road_end;


    public:
        MpiProcess(int argc, char** argv);
        ~MpiProcess();

        MPI_Datatype mpi_vehicle;
        int getRank();
        int getNextRank();
        int getPrevRank();
        int getNumOfProcesses();
        int getStartPosition();
        int getEndPosition();

        void defineMpiVehicle();
        void divideRoad(int road_length);
        void sendVehicle(std::vector<Vehicle *>& vehicles, int destProcess);
        std::vector<Vehicle> receiveVehicle(std::vector<Vehicle>& vehicles, int srcProcess);


};

#endif