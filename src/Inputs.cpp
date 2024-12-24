/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>

#include "Inputs.h"

/**
 * Helper function to parse a line in the input file and return the parameter value of the line
 * @param line a string which is a line from the input file
 * @return the parameter on the line
 */
std::string parseLine(std::string line) {
    return line.substr(0, line.find(' '));
}

/**
 * Loads the inputs options from a text file into the class variables
 * @return 0 if successful, nonzero otherwise
 */
int Inputs::loadFromFile() {
    // Open the input file for reading the simulation inputs
    std::fstream input_file;
    input_file.open("cats-input.txt", std::fstream::in);

    // Check of the input file was loaded properly
    if (!input_file) {
        std::cout << "error: failure to open \"cats-input.txt\" file!" << std::endl;
        return 1;
    }

    // Split the lines in the input file into a vector of strings
    std::vector<std::string> input_lines;
    std::string line;
    while (std::getline(input_file, line))
    {
        input_lines.push_back(line);
    }

    // Parse each line of the input file into the variable it corresponds to
    int n = 0;
    this->num_lanes           = std::stoi(parseLine(input_lines[n++]));
    this->length              = std::stoi(parseLine(input_lines[n++]));
    this->max_speed           = std::stoi(parseLine(input_lines[n++]));
    this->look_forward        = std::stoi(parseLine(input_lines[n++]));
    this->look_other_forward  = std::stoi(parseLine(input_lines[n++]));
    this->look_other_backward = std::stoi(parseLine(input_lines[n++]));
    this->prob_slow_down      = std::stod(parseLine(input_lines[n++]));
    this->prob_change         = std::stod(parseLine(input_lines[n++]));
    this->max_time            = std::stoi(parseLine(input_lines[n++]));
    this->step_size           = std::stod(parseLine(input_lines[n++]));
    this->warmup_time         = std::stoi(parseLine(input_lines[n++]));

    // Close the input file
    input_file.close();

    // Return with zero errors
    return 0;
}