//
// Created by matt on 5/28/16.
//

#ifndef VULKAN_DEPTHPEEL_SIMULATION_H
#define VULKAN_DEPTHPEEL_SIMULATION_H

#include <stdint.h>


class Simulation {
public:
    Simulation();
    void step();
    void write(uint8_t *buffer, int offset);
//    float positions[100*3];
    float velocities[100*2];
    float transforms[100*16];
    float colours[100*3];
};


#endif //VULKAN_DEPTHPEEL_SIMULATION_H
