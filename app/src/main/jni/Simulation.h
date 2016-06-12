//
// Created by matt on 5/28/16.
//

#ifndef VULKAN_DEPTHPEEL_SIMULATION_H
#define VULKAN_DEPTHPEEL_SIMULATION_H

#include <stdint.h>

#define MAX_BOXES 100

class Simulation {
public:
    Simulation();
    void step();
    void write(uint8_t *buffer, int offset);
//    float positions[100*3];
    float velocities[MAX_BOXES*2];
    float transforms[MAX_BOXES*16];
    float colours[MAX_BOXES*3];
    bool paused;
};


#endif //VULKAN_DEPTHPEEL_SIMULATION_H
