/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#ifndef CA_TRAFFIC_SIMULATION_LANE_H
#define CA_TRAFFIC_SIMULATION_LANE_H

#include <vector>
#include <deque>

#include "Inputs.h"
#include "CDF.h"

// Forward Declarations
class Vehicle;

/**
 * Class for a lane in the road of the simulation. Each lane contains the "sites" for the vehicles and allows access
 * to all the information about the vehicles on the road through its methods.
 */
class Lane {
private:
    std::vector<std::deque<Vehicle*>> sites;
    int lane_num;
    int steps_to_spawn;
public:
    Lane(Inputs inputs, int lane_num);
    int getSize();
    int getLaneNumber();
    std::vector<std::deque<Vehicle*>> getSites();
    bool hasVehicleInSite(int site);
    int addVehicle(int site, Vehicle* vehicle_ptr);
    int removeVehicle(int site);
    int attemptSpawn(Inputs inputs, std::vector<Vehicle*>* vehicles, int* next_id_ptr, CDF* interarrival_time_cdf, std::vector<int> last_vehicles);
    int attemptSpawn(Vehicle* vehicle_ptr, std::vector<Vehicle*>* vehicles);
    
#ifdef DEBUG
    void printLane();
#endif
};


#endif //CA_TRAFFIC_SIMULATION_LANE_H
