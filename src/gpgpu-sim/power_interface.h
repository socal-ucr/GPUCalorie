// Copyright (c) 2009-2011, Tor M. Aamodt, Ahmed El-Shafiey, Tayler Hetherington
// The University of British Columbia
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution. Neither the name of
// The University of British Columbia nor the names of its contributors may be
// used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef POWER_INTERFACE_H_
#define POWER_INTERFACE_H_

#include "gpu-sim.h"
#include "power_stat.h"
#include "shader.h"
#include <iostream>
#include <fstream>
#include <zlib.h>
#include <string.h>

class power_interface {
    public:
    power_interface(const gpgpu_sim_config &config,const int stat_sample_freq);
    void cycle(const gpgpu_sim_config &config, const struct shader_core_config *shdr_config,
               class power_stat_t *power_stats, unsigned stat_sample_freq, unsigned tot_cycle,
               unsigned cycle, unsigned tot_inst, unsigned inst);
    private:

    void open_files();
    void close_files();

    char *g_power_trace_filename;
    char *g_metric_trace_filename;
    bool g_power_simulation_enabled;
    bool g_power_trace_enabled;
    int g_power_trace_zlevel;
    unsigned gpu_stat_sample_freq;
    int num_shaders;

    gzFile power_trace_file;
    gzFile metric_trace_file;
    gzFile steady_state_tacking_file;


};
#endif /* POWER_INTERFACE_H_ */
