-------------------------------------------------------------------------------
-------------------------------------------------------------------------------
||                 ______     ______     ______    ______                    ||
||                /\  ___\   /\  __ \   /\__  _\  /\  ___\                   ||
||                \ \ \____  \ \  __ \  \/_/\ \/  \ \___  \                  ||
||                 \ \_____\  \ \_\ \_\    \ \_\   \/\_____\                 ||
||                  \/_____/   \/_/\/_/     \/_/    \/_____/                 ||
||                                                                           ||
||                 CELLULAR - AUTOMATA - TRAFFIC - SIMULATION                ||
-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved

Author        : Maitreya Venkataswamy
Created       : April 1, 2019
Last Modified : April 1, 2019

Affiliation   : Georgia Institute of Technology

Contents:
    1. SOFTWARE DESCRIPTION
    2. INSTALLATION
    3. EXECUTION

-------------------------------------------------------------------------------
                            1. SOFTWARE DESCRIPTION
-------------------------------------------------------------------------------

This software uses cellular automata to simulate the movement of vehicles
through a two lane road. The software has a release mode for maximum
performance, and a debug mode for debugging the software.

The CA algorithm implemented in this code is described in "Two lane traffic 
simulations using cellular automata" by M. Rickert, et al.

https://doi.org/10.1016/0378-4371(95)00442-4

The software requires a GNU C++ compiler supporting C++17.
The software requres CMake 3.9 or higher to build the program.

-------------------------------------------------------------------------------
                                2. INSTALLATION
-------------------------------------------------------------------------------

To build the simulation program, run the following commands

    $ mkdir build; cd build
    $ cmake ..
    $ make

This will build the executable "cats".

To build the simulation program in debug mode, run the following
commands

    $ mkdir build; cd build
    $ cmake -DCMAKE_BUILD_TYPE=Debug ..
    $ make

This will build the executable "cats" in debug mode. The debug mode makes the
following modifications to the program:

    1. The random number generator is seeded with a constant, so that the
        the results are reproducible.
    2. Print statements are included in many parts of the code to assist in the
        debugging process. These include simple visualizations of the road at
        each step in the simulation.

-------------------------------------------------------------------------------
                                3. EXECUTION
-------------------------------------------------------------------------------

To run the simulation program, copy the executable file "cats" into a directory
where you want to run the simulation. A configuration file 

    "cats-input.txt"

needs to be placed alongside the executable. A sample configuration file is
included in the root directory of the repository. Also required by the program
is a file with the CDF of interarrival times. This file is formatted as a
text file called 

    "interarrival-cdf.dat" 
    
and has two columns of numbers: the interarrival times in ascending order and
the cumulative probability for those interarrival times. A sample CDF file is
included in the root directory of the repository. To run the program, execute 
the command

    $ ./cats

