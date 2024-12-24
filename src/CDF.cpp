/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#include "CDF.h"

#include <fstream>
#include <string>
#include <iostream>

/**
 * Reads the data for the cumulative distribution function from a two column comma delimited text file where the first
 * column is the values and the second column is the distribution function at that value.
 * @param file_name path and name of the file to read
 * @return 0 if successful, nonzero otherwise
 */
int CDF::read_cdf(std::string file_name) {
    // Open the file containing the CDF information
    std::ifstream file(file_name);

    // Check of the input file was loaded properly
    if (!file) {
        std::cout << "error: failure to open "<< file_name <<  " file!" << std::endl;
        return 1;
    }

    // Read each line into the CDF information
    std::string line;
    while (std::getline(file, line))
    {
        this->x.push_back(std::stod(line.substr(0, line.find(','))));
        this->cdf.push_back(std::stod(line.substr(1, line.find(','))));
    }

    // Close the file
    file.close();

    // Return with no errors
    return 0;
}

/**
 * Sampled a point from the cumulative distribution function
 * @return sampled point from the distribution
 */
double CDF::query() {
    double u = ((double) std::rand()) / ((double) RAND_MAX);
    for (int i = 0; i < (int) this->cdf.size(); i++) {
        if (this->cdf[i] >= u) {
            return this->x[i];
        }
    }
    return this->cdf.back();
}