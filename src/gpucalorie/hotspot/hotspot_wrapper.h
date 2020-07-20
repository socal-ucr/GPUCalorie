// Copyright (c) 2009-2011, Tor M. Aamodt, Tayler Hetherington, Ahmed ElTantawy,
// The University of British Columbia
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
// Neither the name of The University of British Columbia nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef HOTSPOT_WRAPPER_H_
#define HOTSPOT_WRAPPER_H_

#include "../../gpgpu-sim/gpu-sim.h"
#include "../../gpgpu-sim/power_stat.h"
#include "../../gpgpu-sim/shader.h"
#include "hotspot.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <iostream>
#include <fstream>
#include <zlib.h>
#include <string.h>

class hotspot_wrapper {
public:
    hotspot_wrapper();
    ~hotspot_wrapper();

    void init(const gpgpu_sim_config &config);
    void update_power(class power_stat_t *power_stats);
    void update_temps(double * temps);
    void compute(class power_stat_t *power_stats, double time_elapsed);
    double find_max_temp();
private:

    unsigned num_shaders;
    char **names;
    double *vals;
    /* trace file pointers	*/
    FILE *tout;
    /* floorplan	*/
    flp_t *flp;
    /* hotspot temperature model	*/
    RC_model_t *model;
    /* instantaneous temperature and power values	*/
    double *temp;
    double *power;
    double total_power;

    /* steady state temperature and power values	*/
    double *overall_power, *steady_temp;
    /* thermal model configuration parameters	*/
    thermal_config_t thermal_config;
    /* global configuration parameters	*/
    global_config_t global_config;
    /* table to hold options and configuration */
    str_pair table[MAX_ENTRIES];

    /* variables for natural convection iterations */
    int natural; 
    double avg_sink_temp;
    int natural_convergence;
    double r_convec_old;

    bool  do_transient;

    int size;
    int num_functional_blocks;
    unsigned long num_samples;
};

#endif /* HOTSPOT_WRAPPER_H_ */
