/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#ifndef CA_TRAFFIC_SIMULATION_SIMULATION_H
#define CA_TRAFFIC_SIMULATION_SIMULATION_H

#include <vector>

#include "Road.h"
#include "Inputs.h"
#include "Statistic.h"
#include "MpiProcess.h"

/**
 * Class for the simulation. Has a method for running the simulation.
 */
class Simulation {
private:
    Road* road_ptr;
    int time;
    std::vector<Vehicle*> vehicles;
    Inputs inputs;
    int next_id;
    Statistic* travel_time;
    std::vector<Vehicle *> vehicles_to_send;

public:
    Simulation(Inputs inputs);
    ~Simulation();
    int run_simulation(MpiProcess *curr_process);
    void sendVehicles(MpiProcess *curr_proccess);
    void receiveVehicles(MpiProcess *curr_proccess);
    bool isInVector(int value, const std::vector<int>& vec);
};


#endif //CA_TRAFFIC_SIMULATION_SIMULATION_H
