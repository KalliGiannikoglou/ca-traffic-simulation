/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#include <chrono>
#include <algorithm>
#include <cmath>
#include <unordered_set>

#include "Road.h"
#include "Simulation.h"
#include "Vehicle.h"


/**
 * Constructor for the Simulation
 * @param inputs
 */
Simulation::Simulation(Inputs inputs) {

    // Create the Road object for the simulation
    this->road_ptr = new Road(inputs);

    // Initialize the first Vehicle id
    this->next_id = 0;

    // Obtain the simulation inputs
    this->inputs = inputs;

    // Initialize Statistic for travel time
    this->travel_time = new Statistic();
}

/**
 * Destructor for the Simulation
 */
Simulation::~Simulation() {
    // Delete the Road object in the simulation
    delete this->road_ptr;

    // Delete all the Vehicle objects in the Simulation
    for (int i = 0; i < (int) this->vehicles.size(); i++) {
        delete this->vehicles[i];
    }
}

/**
 * Executes the simulation in parallel using the specified number of threads
 * @param num_threads number of threads to run the simulation with
 * @return 0 if successful, nonzero otherwise
 */
int Simulation::run_simulation(MpiProcess *curr_proccess) {
    // Obtain the start time
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    // Set the simulation time to zero
    this->time = 0;

    // Declare a vector for vehicles to be removed each step
    std::vector<int> vehicles_to_remove;

    while (this->time < this->inputs.max_time) {
        std::vector<int> last_vehicles = {-1, -1};
        std::vector<int> first_vehicles = {-1, -1};

        // Receive the last vehicles of the next process
        if(curr_proccess->getRank() != curr_proccess->getNumOfProcesses()-1){
            last_vehicles = curr_proccess->recvLastVehicles();
        }

        // Send the last vehicles to the previous process
        if(curr_proccess->getRank() != 0){
            curr_proccess->sendLastVehicles(this->road_ptr->getLanes(), last_vehicles);

            first_vehicles = curr_proccess->recvFirstVehicles();
        }

        // Send the last vehicles to the previous process
        if(curr_proccess->getRank() != curr_proccess->getNumOfProcesses()-1){
            curr_proccess->sendFirstVehicles(this->road_ptr->getLanes(), first_vehicles);
        }

#ifdef DEBUG
        if(this->vehicles.size() > 0){
            std::cout << "road configuration at time " << time << ":" << std::endl;
            this->road_ptr->printRoad();
            std::cout << "performing lane switches..." << std::endl;
        }
#endif
       
        // Perform the lane switch step for all vehicles
        for (int n = 0; n < (int) this->vehicles.size(); n++) {
            this->vehicles[n]->updateGaps(this->road_ptr, curr_proccess->getStartPosition(),
                        curr_proccess->getEndPosition(), first_vehicles, last_vehicles);
#ifdef DEBUG
            this->vehicles[n]->printGaps();
#endif
        }   

        for (int n = 0; n < (int) this->vehicles.size(); n++) {
            this->vehicles[n]->performLaneSwitch(this->road_ptr);
        }

#ifdef DEBUG
        if(vehicles.size() > 0){
            this->road_ptr->printRoad();
            std::cout << "performing lane movements..." << std::endl;
        }
#endif

        // Perform the independent lane updates
        for (int n = 0; n < (int) this->vehicles.size(); n++) {
            this->vehicles[n]->updateGaps(this->road_ptr, curr_proccess->getStartPosition(),
                        curr_proccess->getEndPosition(), first_vehicles, last_vehicles);
#ifdef DEBUG
            this->vehicles[n]->printGaps();
#endif
        }

        for (int n = 0; n < (int) this->vehicles.size(); n++) {
            int time_on_road = this->vehicles[n]->performLaneMove();

            if (time_on_road != 0) {
                    vehicles_to_remove.push_back(n);
            }
        }


        // End of iteration steps
        // Increment time
        this->time++;

        // Remove finished vehicles
        std::sort(vehicles_to_remove.begin(), vehicles_to_remove.end());
        for (int i = vehicles_to_remove.size() - 1; i >= 0; i--) {
            // Update travel time statistic if beyond warm-up period
            if (this->time > this->inputs.warmup_time) {
                this->travel_time->addValue(this->vehicles[vehicles_to_remove[i]]->getTravelTime(this->inputs));
            }

            // Delete the Vehicle
            delete this->vehicles[vehicles_to_remove[i]];
            this->vehicles.erase(this->vehicles.begin() + vehicles_to_remove[i]);
        }
        vehicles_to_remove.clear();

        // If this is process 0, attempt to spawn new vehicles in the road
        if(curr_proccess->getRank() == 0){
            this->road_ptr->attemptSpawn(this->inputs, &(this->vehicles), &(this->next_id), last_vehicles);
        }

         // Receive the vehicles from the previous process (if this is not process 0)
        if(curr_proccess->getRank() != 0){
            receiveVehicles(curr_proccess);
        }
        
        // Send the vehicles to the next process (if this is not the last process)
        if(curr_proccess->getRank() != curr_proccess->getNumOfProcesses()-1){
            sendVehicles(curr_proccess);
            // empty the vector
            this->vehicles_to_send.clear();
        }

#ifdef DEBUG
        printf("Process: %d, my vehicles are: \n", curr_proccess->getRank());
        for(int i = 0; i < (int)this->vehicles.size(); i++){
            printf("Process: %d, vehicle %d is in position: %d\n", curr_proccess->getRank(), this->vehicles[i]->getId(), this->vehicles[i]->getPosition());
        }
#endif
    }

    // Print the total run time and average iterations per second and seconds per iteration
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto time_elapsed = (std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) /1000000.0;
    std::cout << "--- Simulation Performance ---" << std::endl;
    std::cout << "Process : " << curr_proccess->getRank() << " total computation time: " << time_elapsed << " [s]" << std::endl;
    std::cout << "Process : " << curr_proccess->getRank() << " average time per iteration: " << time_elapsed / inputs.max_time << " [s]" << std::endl;
    std::cout << "Process : " << curr_proccess->getRank() << " average iterating frequency: " << inputs.max_time / time_elapsed << " [iter/s]" << std::endl;

#ifdef DEBUG
    // Print final road configuration
    std::cout << "final road configuration" << std::endl;
    this->road_ptr->printRoad();
#endif

    // The last process calculates the final statistics
    if(curr_proccess->getRank() == curr_proccess->getNumOfProcesses() - 1){
     std::cout << "--- Simulation Results ---" << std::endl;
        std::cout << "Process : " << curr_proccess->getRank()<< " time on road: avg=" << this->travel_time->getAverage() << ", std="
                << pow(this->travel_time->getVariance(), 0.5) << ", N=" << this->travel_time->getNumSamples()
                << std::endl;
    }

    // Return with no errors
    return 0;
}


void Simulation::sendVehicles(MpiProcess *curr_proccess){

    for(int i = 0; i < (int)this->vehicles.size(); i++){
        // Check if the threshold will be exceeded and if the vehicle is allowed to be sent
        if(this->vehicles[i]->getPosition() + this->vehicles[i]->getSpeed() > curr_proccess->getEndPosition()
            && curr_proccess->allowSending(vehicles, this->vehicles_to_send, vehicles[i])){
            
            this->vehicles_to_send.push_back(this->vehicles[i]);
#ifdef DEBUG
            printf("Process: %d, sending vehicle %d to process: %d\n", curr_proccess->getRank(), this->vehicles[i]->getId(), curr_proccess->getNextRank());
#endif
        }
    }

    curr_proccess->sendVehicle(this->vehicles_to_send);

    // Code to remove from curr process the vehicles that have been sent
    // Store indices of vehicles to delete
    std::vector<int> indices_to_remove; 
    for (int i = 0; i < (int) this->vehicles_to_send.size(); i++) {
        for (int j = 0; j < (int) this->vehicles.size(); j++) {
            if (this->vehicles[j]->getId() == this->vehicles_to_send[i]->getId()) {
                this->vehicles[j]->getLanePtr()->removeVehicle(this->vehicles[j]->getPosition());
                indices_to_remove.push_back(j);
            }
        }
    }
    // Remove vehicles (in reverse order)
    std::sort(indices_to_remove.rbegin(), indices_to_remove.rend());
    for (int index : indices_to_remove) {
        delete this->vehicles[index];
        this->vehicles.erase(this->vehicles.begin() + index);
    }

}

void Simulation::receiveVehicles(MpiProcess *curr_proccess) {
    // Receive the vehicles that are about to cross the threshold
    std::vector<std::vector<Vehicle *>> vehicles_to_recv = curr_proccess->receiveVehicle();
    // unordered_set of vehicles to remove from curr process
    std::vector<int> ids_to_remove;

    for (int i = 0; i < (int)vehicles_to_recv.size(); ++i) {
        for (auto* vehicle : vehicles_to_recv[i]) {
            // if this is not the last process and the vehicle is about to cross the threshold
            if (curr_proccess->getRank() != curr_proccess->getNumOfProcesses()-1 &&
                vehicle->getPosition() + vehicle->getSpeed() > curr_proccess->getEndPosition()) {

                vehicle->setLanePtr(this->road_ptr->getLanes()[i]);
                this->vehicles_to_send.push_back(vehicle);
                // Store id of vehicle to remove
                ids_to_remove.push_back(vehicle->getId());
#ifdef DEBUG
                printf("Received vehicle %d and promoted it instantly\n", vehicle->getId());
#endif
            }
        }
    }

    for(int i=0; i < (int)vehicles_to_recv.size(); i++){
        for(int j=0; j < (int)vehicles_to_recv[i].size(); j++){
            if(isInVector(vehicles_to_recv[i][j]->getId(), ids_to_remove)){
                // delete vehicles_to_recv[i][j];
                vehicles_to_recv[i].erase(vehicles_to_recv[i].begin() + j);
            }
        }
    }

    // Spawn the received vehicles in their proper positions
    for(int i = 0; i < (int)vehicles_to_recv.size(); i++){
        for(auto vehicle: vehicles_to_recv[i]){
            this->road_ptr->attemptSpawn(i, vehicle, &(this->vehicles));
        }
    }
}

bool Simulation::isInVector(int value, const std::vector<int>& vec) {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}
