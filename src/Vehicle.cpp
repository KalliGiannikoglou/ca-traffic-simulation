/*
 * Copyright (C) 2019 Maitreya Venkataswamy - All Rights Reserved
 */

#include <cstdlib>
#include <iomanip>

#include "Statistic.h"
#include "Vehicle.h"
#include "Lane.h"
#include "Road.h"

/**
 * Constructor for the Vehicle
 * @param lane_ptr pointer to the Lane in which the Vehicle starts in
 * @param id unique ID number of the Vehicle
 * @param initial_position initial site number of the Vehicle in the Lane
 * @param inputs instance of the Inputs class with the simulation inputs
 */
Vehicle::Vehicle(Lane* lane_ptr, int id, int initial_position, Inputs inputs) {
    // Set the ID number of the Vehicle
    this->id = id;

    // Set the initial position of the Vehicle
    this->position = initial_position;

    // Set the Lane pointer to the pointer to the Lane that contains the Vehicle
    this->lane_ptr = lane_ptr;

    // Set the maximum speed of the Vehicle
    this->max_speed = inputs.max_speed;

    // Set the initial speed of the Vehicle to the maximum speed
    this->speed = this->max_speed;

    // Set the look forward distance of the Vehicle
    this->look_forward = this->speed + 1;

    // Set the other lane look forward distance of the Vehicle
    this->look_other_forward = this->look_forward;

    // Set the other lane look backward distance of the Vehicle
    this->look_other_backward = inputs.look_other_backward;

    // Set the slow down probability of the Vehicle
    this->prob_slow_down = inputs.prob_slow_down;

    // Set the lane change probability of the Vehicle
    this->prob_change = inputs.prob_change;

    // Initialize the time spend on the Road
    this->time_on_road = 0;
}

Vehicle::~Vehicle() {}

/**
 * Update the perceived gaps between the Vehicle and the surrounding Vehicles in the Road
 * @param road_ptr pointer to the Road that the Vehicle is in
 * @return 0 if successful, nonzero otherwise

 */
int Vehicle::updateGaps(Road* road_ptr, int start_postition, int end_position,
                         std::vector<int> first_vehicles, std::vector<int> last_vehicles) {
    // Locate the preceding Vehicle and update the forward gap
    this->gap_forward = this->lane_ptr->getSize() - 1;
    for (int i = this->position + 1; i <= end_position; i++) {
        if (this->lane_ptr->hasVehicleInSite(i)) {
            this->gap_forward = i - this->position - 1;
            break;
        }
        // if last position is reached and there is not a vehicle, 
        // and if the lane pointer of our lane is not -1 (so, there is a vehicle in our lane)
        // update the gap based on the vehicle ahead
        if(i == end_position && last_vehicles[this->lane_ptr->getLaneNumber()] != -1){
            this->gap_forward = last_vehicles[this->lane_ptr->getLaneNumber()] - this->position - 1;
            printf("vehicle %d, gap_forward: %d\n", this->id, this->gap_forward);
            break;
        }
    }

    // Update vehicle look forward distances
    this->look_forward = this->speed + 1;
    this->look_other_forward = this->look_forward;

    // Determine the other lane of interest
    Lane* other_lane_ptr;
    if (this->lane_ptr->getLaneNumber() == 0) {
        other_lane_ptr = road_ptr->getLanes()[1];
    } else {
        other_lane_ptr = road_ptr->getLanes()[0];
    }

    // Update the forward gap in the other lane
    this->gap_other_forward = this->lane_ptr->getSize() - 1;
    for (int i = this->position; i <= end_position; i++) {
        if (other_lane_ptr->hasVehicleInSite(i)) {
            this->gap_other_forward = i - this->position - 1;
            break;
        }
        // if last position is reached and there is not a vehicle, 
        // and if the lane pointer of the other lane is not -1 (so, there is a vehicle in the other lane)
        // update the gap based on the vehicle ahead
        if(i == end_position && last_vehicles[other_lane_ptr->getLaneNumber()] != -1){
            this->gap_forward = last_vehicles[other_lane_ptr->getLaneNumber()] - this->position - 1;
            printf("vehicle %d, gap_forward: %d\n", this->id, this->gap_forward);
            break;
        }
    }

    // Update the backward gap in the other lane
    this->gap_other_backward = this->lane_ptr->getSize() - 1;
    for (int i = this->position; i >= start_postition; i--) {
        if (other_lane_ptr->hasVehicleInSite(i)) {
            this->gap_other_backward = this->position - i - 1;
            break;
        }
        // if starting position is reached and there is not a vehicle there, 
        // and if the  other lane pointer of our lane is not -1 (so, there is a vehicle in lane)
        // update the backward gap based on the vehicle behind
        if(i == start_postition && first_vehicles[other_lane_ptr->getLaneNumber()] != -1){
            this->gap_other_backward = this->position - first_vehicles[other_lane_ptr->getLaneNumber()] - 1;
            break;
        }
    }

    // Return with zero errors
    return 0;
}

/**
 * Moved the Vehicle to the other Lane in the Road
 * @param road_ptr pointer to the Road in which the Vehicle is on
 * @return 0 if successful, nonzero otherwise
 */
int Vehicle::performLaneSwitch(Road* road_ptr) {
    // Evaluate if the Vehicle will change lanes and then perform the lane change
    if (this->gap_forward < this->look_forward &&
        this->gap_other_forward > this->look_other_forward &&
        this->gap_other_backward > this->look_other_backward &&
        ((double) rand()) / ((double) RAND_MAX) <= this->prob_change ) {

        // Determine the lane that the Vehicle is switching to
        Lane* other_lane_ptr;
        if (this->lane_ptr->getLaneNumber() == 0) {
            other_lane_ptr = road_ptr->getLanes()[1];
        } else {
            other_lane_ptr = road_ptr->getLanes()[0];
        }

#ifdef DEBUG
        std::cout << "vehicle " << this->id << " switched lane " << this->lane_ptr->getLaneNumber() << " -> "
            << other_lane_ptr->getLaneNumber() << std::endl;
#endif

        // Copy the Vehicle pointer to the other Lane
        other_lane_ptr->addVehicle(this->position, this);

        // Remove the Vehicle pointer from the current Lane
        this->lane_ptr->removeVehicle(this->position);

        // Set the pointer to the Lane in the Vehicle to the new lane
        this->lane_ptr = other_lane_ptr;
    }

    // Return with zero errors
    return 0;
}

/**
 * Moves the Vehicle to the next site in the current Lane during the time-step based on the speed of the Vehicle
 * @return 0 if successful, nonzero otherwise
 */
int Vehicle::performLaneMove() {
    // Increment the time on road counter
    this->time_on_road++;

    // Update Vehicle speed based on vehicle speed update rules
    if (this->speed != this->max_speed) {
        this->speed++;
#ifdef DEBUG
        std::cout << "vehicle " << this->id << " increased speed " << this->speed - 1 << " -> " << this->speed
            << std::endl;
#endif
    }

    this->speed = std::min(this->speed, this->gap_forward);
#ifdef DEBUG
    if (this->speed == 0) {
        std::cout << "vehicle " << this->id << " stopped behind preceding vehicle" << std::endl;
    }
#endif

    if (this->speed > 0) {
        if ( ((double) rand()) / ((double) RAND_MAX) <= this->prob_slow_down ) {
            this->speed--;
#ifdef DEBUG
            std::cout << "vehicle " << this->id << " decreased speed " << this->speed + 1 << " -> " << this->speed
                << std::endl;
#endif
        }
    }

    if (this->speed > 0) {
        // Compute the new position of the vehicle
        int new_position = (this->position + this->speed) % this->lane_ptr->getSize();

        // If the vehicle reached the end of the road, remove the Vehicle from the Lane and return the time on road
        if (this->position > new_position) {
#ifdef DEBUG
            std::cout << "vehicle " << this->id << " spent " << this->time_on_road << " steps on the road" << std::endl;
#endif

            // Remove vehicle from the Road
            this->lane_ptr->removeVehicle(this->position);

            // Return the time on the Road
            return this->time_on_road;
        }

#ifdef DEBUG
        std::cout << "vehicle " << this->id << " moved " << this->position << " -> " << new_position << std::endl;
#endif

        // Update Vehicle position in the Lane object sites
        this->lane_ptr->addVehicle(new_position, this);

        // Remove vehicle from the old site
        this->lane_ptr->removeVehicle(this->position);

        // Update the Vehicle position value
        this->position = new_position;
    }

    // Return with no errors
    return 0;
}

/**
 * Getter method for teh Lane pointer of the Vehicle
 * @return pointer to the Lane in which the Vehicle is
 */
Lane* Vehicle::getLanePtr(){
    return this->lane_ptr;
}

/**
 * Getter method for the ID number of the Vehicle
 * @return
 */
int Vehicle::getId() {
    return this->id;
}

/**
 * Getter method for the total time the Vehicle has spent on the Road
 * @param inputs
 * @return
 */
double Vehicle::getTravelTime(Inputs inputs) {
    return inputs.step_size * this->time_on_road;
}

/**
 * Setter method for the speed of the Vehicle
 * @param speed
 * @return
 */
int Vehicle::setSpeed(int speed) {
    this->speed = speed;

    // Return with no errors
    return 0;
}
/**
 * Debug method for printing the gap information of the Vehicle
 */
#ifdef DEBUG
void Vehicle::printGaps() {
    std::cout << "vehicle " << std::setw(2) << this->id << " gaps, >:" << this->gap_forward << " ^>:"
        << this->gap_other_forward << " ^<:" << this->gap_other_backward << std::endl;
}
#endif

int Vehicle::getPosition(){
    return this->position;
}

int Vehicle::getSpeed(){
    return this->speed;
}

void Vehicle::setLanePtr(Lane* lane_ptr){
    this->lane_ptr = lane_ptr;
}

void Vehicle::setPosition(int position){
    this->position = position;
}

bool Vehicle::isInList(std::vector<Vehicle *>& vehicles){
    for(int i = 0; i < (int) vehicles.size(); i++){
        if(vehicles[i]->getId() == this->id){
            return true;
        }
    }
    return false;
}

void Vehicle::setId(int id){
    this->id = id;
}