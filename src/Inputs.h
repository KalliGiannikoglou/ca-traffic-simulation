/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#ifndef CA_TRAFFIC_SIMULATION_INPUTS_H
#define CA_TRAFFIC_SIMULATION_INPUTS_H

#include <iostream>

/**
 * Class for the input options of a simulation that acts as a structure to organize the inputs in one place.
 * Has methods to load all the inputs from a file from an input text file.
 */
class Inputs {
public:
    int num_lanes;
    int length;
    double percent_full;
    int max_speed;
    int look_forward;
    int look_other_forward;
    int look_other_backward;
    double prob_slow_down;
    double prob_change;
    int max_time;
    double step_size;
    int warmup_time;
    int loadFromFile();
};


#endif //CA_TRAFFIC_SIMULATION_INPUTS_H
