// Copyright (c) 2009-2011, Tor M. Aamodt, Ahmed El-Shafiey, Tayler Hetherington
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

#ifndef GPU_CALORIE_H_
#define GPU_CALORIE_H_

#include "../gpgpu-sim/gpu-sim.h"
#include "../gpgpu-sim/power_stat.h"
#include "../gpgpu-sim/shader.h"
#include "hotspot/hotspot_wrapper.h"
#include "power_interface.h"
#include <iostream>
#include <fstream>
#include <zlib.h>
#include <string.h>

class gpu_calorie {
    public:
    gpu_calorie(const gpgpu_sim_config &config,const int stat_sample_freq);
    void cycle(const gpgpu_sim_config &config, class power_stat_t *power_stats,double core_period);
    double get_max_chip_temp();
    void print_heatmap();
    private:

    class power_interface * m_power_interface;
    class hotspot_wrapper * m_hotspot_wrapper;

    bool g_power_simulation_enabled;
    bool g_power_trace_enabled;
    bool g_dtm_enabled;
    unsigned gpu_stat_sample_freq;
    int num_shaders;

};
#endif /* GPU_CALORIE_H_ */
