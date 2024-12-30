/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#include "Road.h"
#include "Inputs.h"
#include "Vehicle.h"

/**
 * Constructor for the Road
 * @param inputs instance of the Inputs class with simulation inputs
 */
Road::Road(Inputs inputs) {
#ifdef DEBUG
    std::cout << "creating new road with " << inputs.num_lanes << " lanes..." << std::endl;
#endif
    // Create the Lane objects for the Road
    for (int i = 0; i < inputs.num_lanes; i++) {
        this->lanes.push_back(new Lane(inputs, i));
    }
#ifdef DEBUG
    std::cout << "done creating road" << std::endl;
#endif

    this->interarrival_time_cdf = new CDF();
    int status = this->interarrival_time_cdf->read_cdf("interarrival-cdf.dat");
    if (status != 0) {
        throw std::exception();
    }
}

/**
 * Destructor of the Road
 */
Road::~Road() {
    // Delete the Lane objects of the Road
    for (int i = 0; i < (int) this->lanes.size(); i++) {
        delete this->lanes[i];
    }
}

/**
 * Getter for the Lanes of the road
 * @return
 */
std::vector<Lane*> Road::getLanes() {
    return this->lanes;
}

/**
 * Attempts to spawn Vehicles on each Lane of the Road
 * @param inputs instance of the Inputs class with the simulation Inputs
 * @param vehicles pointer to the array of Vehicles that exist
 * @param next_id_ptr pointer to the id of the next spawned Vehicle
 * @return 0 if successful, nonzero otherwise
 */
int Road::attemptSpawn(Inputs inputs, std::vector<Vehicle*>* vehicles, int* next_id_ptr) {
    for (int i = 0; i < (int) this->lanes.size(); i++) {
        this->lanes[i]->attemptSpawn(inputs, vehicles, next_id_ptr, this->interarrival_time_cdf);
    }

    // Return with no errors
    return 0;
}

int Road::attemptSpawn(int lane_num, int starting_site, Vehicle* vehicle_ptr, std::vector<Vehicle*>* vehicles) {
   
    this->lanes[lane_num]->attemptSpawn(starting_site, vehicle_ptr, vehicles);
    return 0;
}


/**
 * Debug function to print all the Lanes of the Road for visualizing the sites in the Road
 */
#ifdef DEBUG
void Road::printRoad() {
    for (int i = this->lanes.size() - 1; i >= 0; i--) {
        this->lanes[i]->printLane();
    }
}
#endif