/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#include <iostream>

#include "Inputs.h"
#include "Simulation.h"
#include "MpiProcess.h"

/**
 * Main point of execution of the program
 * @param argc number of command line arguments
 * @param argv command line arguments
 * @return 0 if successful, nonzero otherwise
 */
int main(int argc, char** argv) {
    std::cout << "================================================" << std::endl;
    std::cout << "||    CELLULAR AUTOMATA TRAFFIC SIMULATION    ||" << std::endl;
    std::cout << "================================================" << std::endl;

#ifndef DEBUG
    srand(time(NULL));
#endif

    MpiProcess* curr_process = new MpiProcess(argc, argv);
    
    //Read the inputs from the file and broadcast them to all processes
    Config config;
    Inputs inputs = curr_process->broadcastConfig(config);

    // Create a Simulation object for the current simulation
    Simulation* simulation_ptr = new Simulation(inputs);

    curr_process->divideRoad(inputs.length);

    // Run the Simulation
    simulation_ptr->run_simulation(curr_process);

    // Delete the Simulation object
    delete simulation_ptr;

    MPI_Finalize();

    // Return with no errors
    return 0;
}
