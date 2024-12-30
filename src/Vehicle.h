/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#ifndef CA_TRAFFIC_SIMULATION_VEHICLE_H
#define CA_TRAFFIC_SIMULATION_VEHICLE_H

#include "Inputs.h"
#include "Road.h"
#include "Statistic.h"

// Forward declarations
class Lane;
class MpiProcess;

/**
 * Constructor for a Vehicle in the simulation. Has methods for performing movements based on the CA rules of the
 * simulation.
 */
class Vehicle {
private:
    Lane* lane_ptr;
    int id;
    int position;
    int speed;
    int max_speed;
    int gap_forward;
    int gap_other_forward;
    int gap_other_backward;
    int look_forward;
    int look_other_forward;
    int look_other_backward;
    double prob_slow_down;
    double prob_change;
    int time_on_road;

public:
    Vehicle(){}
    Vehicle(Lane* lane_ptr, int id, int initial_position, Inputs inputs);
    ~Vehicle();
    int updateGaps(Road* road_ptr);
    int performLaneSwitch(Road* road_ptr);
    int performLaneMove();
    int getId();
    double getTravelTime(Inputs inputs);
    int setSpeed(int speed);
    int getPosition();
    int getSpeed();
    Lane *getLanePtr();
    void setLanePtr(Lane* lane_ptr);
    void setPosition(int position);

    // MpiProcess gets access to the private fields
    friend class MpiProcess;
    
#ifdef DEBUG
    void printGaps();
#endif
};


#endif //CA_TRAFFIC_SIMULATION_VEHICLE_H
