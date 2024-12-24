/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#ifndef CA_TRAFFIC_SIMULATION_ROAD_H
#define CA_TRAFFIC_SIMULATION_ROAD_H

#include <vector>

#include "Lane.h"
#include "Inputs.h"
#include "CDF.h"

/**
 * Class for the Road in the Simulation. The road has multiple Lanes that each contain Vehicles. Has methods to attempt
 * spawning Vehicles in the Lanes
 */
class Road {
private:
    std::vector<Lane*> lanes;
    CDF* interarrival_time_cdf;
public:
    Road(Inputs inputs);
    ~Road();
    std::vector<Lane*> getLanes();
    int attemptSpawn(Inputs inputs, std::vector<Vehicle*>* vehicles, int* next_id_ptr);

#ifdef DEBUG
    void printRoad();
#endif
};


#endif //CA_TRAFFIC_SIMULATION_ROAD_H
