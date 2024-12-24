/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#ifndef CA_TRAFFIC_SIMULATION_CDF_H
#define CA_TRAFFIC_SIMULATION_CDF_H

#include <vector>
#include <string>

/**
 * Class for a Cumulative Distribution Function that has a method for sampling a point from the distribution.
 */
class CDF {
private:
    std::vector<float> x;
    std::vector<float> cdf;
public:
    int read_cdf(std::string file_name);
    double query();
};


#endif //CA_TRAFFIC_SIMULATION_CDF_H
