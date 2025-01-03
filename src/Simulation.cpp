/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#include <chrono>
#include <algorithm>
#include <cmath>
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

#ifdef DEBUG
        if(this->vehicles.size() > 0){
            std::cout << "road configuration at time " << time << ":" << std::endl;
            this->road_ptr->printRoad();
            std::cout << "performing lane switches..." << std::endl;
        }
#endif

        // Send the vehicles to the next process (if this is not the last process)
        if(curr_proccess->getRank() != curr_proccess->getNumOfProcesses()-1){
            std::vector<Vehicle *> vehicles_to_send;
            for(int i = 0; i < (int)this->vehicles.size(); i++){
                if(curr_proccess->allowSending(vehicles, vehicles_to_send, vehicles[i])
                    && this->vehicles[i]->getPosition()+ this->vehicles[i]->getSpeed() > curr_proccess->getEndPosition()){
                   vehicles_to_send.push_back(this->vehicles[i]);
                   printf("Process: %d, sending vehicle %d to process: %d\n", curr_proccess->getRank(), this->vehicles[i]->getId(), curr_proccess->getNextRank());
                }
            }
            curr_proccess->sendVehicle(vehicles_to_send, curr_proccess->getNextRank());
            // Code to remove from curr process the vehicles that have been sent
            for (int i = 0; i < (int) vehicles_to_send.size(); i++) {
                for(int j = 0; j < (int) this->vehicles.size(); j++){
                    if(this->vehicles[j]->getId() == vehicles_to_send[i]->getId()){
                        printf("Process: %d, deleting vehicle %d\n", curr_proccess->getRank(), this->vehicles[j]->getId());
                        
                        // Delete the Vehicle
                        this->vehicles[j]->getLanePtr()->removeVehicle(this->vehicles[j]->getPosition());  
                        delete this->vehicles[j];
                        this->vehicles.erase(this->vehicles.begin() + j);
                    }
                }
            }
            // empty the vector
            vehicles_to_send.clear();
        }
        
        // Receive the vehicles from the previous process (if this is not process 0)
        if(curr_proccess->getRank() != 0){
            std::vector<std::vector<Vehicle *>> vehicles_to_recv;
            vehicles_to_recv = curr_proccess->receiveVehicle(curr_proccess->getPrevRank());
        
            for(int i = 0; i < (int)vehicles_to_recv.size(); i++){
                for(auto vehicle: vehicles_to_recv[i]){
                    this->road_ptr->attemptSpawn(i, curr_proccess->getStartPosition(), vehicle, &(this->vehicles));
                }
            }
        }
       
        // Perform the lane switch step for all vehicles
        for (int n = 0; n < (int) this->vehicles.size(); n++) {
            if(curr_proccess->getRank() == 1){
                printf("Attempting to update gaps for vehicle %d\n", this->vehicles[n]->getId());
            }
            this->vehicles[n]->updateGaps(this->road_ptr);
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
            this->vehicles[n]->updateGaps(this->road_ptr);
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
            printf("Process %d, vehicles to remove: %d\n", curr_proccess->getRank(), this->vehicles[vehicles_to_remove[i]]->getId());
            // Update travel time statistic if beyond warm-up period
            if (this->time > this->inputs.warmup_time) {
                this->travel_time->addValue(this->vehicles[vehicles_to_remove[i]]->getTravelTime(this->inputs));
            }

            // Delete the Vehicle
            delete this->vehicles[vehicles_to_remove[i]];
            this->vehicles.erase(this->vehicles.begin() + vehicles_to_remove[i]);
        }
        vehicles_to_remove.clear();

        if(curr_proccess->getRank() == 0){
            // Spawn new Vehicles
            this->road_ptr->attemptSpawn(this->inputs, &(this->vehicles), &(this->next_id));
        }

        printf("Process %d, time: %d\n", curr_proccess->getRank(), this->time);
        for(auto &vehicle: this->vehicles){
            printf("Process %d, vehicle %d, position: %d, speed: %d\n", curr_proccess->getRank(), vehicle->getId(), vehicle->getPosition(), vehicle->getSpeed());
        }

        MPI_Barrier(MPI_COMM_WORLD);
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

    // Print the average Vehicle time on the Road
    std::cout << "--- Simulation Results ---" << std::endl;
    std::cout << "Process : " << curr_proccess->getRank()<< " time on road: avg=" << this->travel_time->getAverage() << ", std="
              << pow(this->travel_time->getVariance(), 0.5) << ", N=" << this->travel_time->getNumSamples()
              << std::endl;

    // Return with no errors
    return 0;
}
