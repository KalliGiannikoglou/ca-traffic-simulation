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
public:
    Simulation(Inputs inputs);
    ~Simulation();
    int run_simulation(MpiProcess *curr_process);
    void sendVehicles(std::vector<Vehicle *> vehicles_to_send, MpiProcess *curr_proccess);
    void receiveVehicles(std::vector<Vehicle *> vehicles_to_send, MpiProcess *curr_proccess);
};


#endif //CA_TRAFFIC_SIMULATION_SIMULATION_H
