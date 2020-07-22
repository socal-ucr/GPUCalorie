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

#include "gpu_calorie.h"

gpu_calorie::gpu_calorie(const gpgpu_sim_config &config,const int stat_sample_freq)
{
    g_power_simulation_enabled=config.g_power_simulation_enabled;
    gpu_stat_sample_freq=stat_sample_freq;
    g_power_trace_enabled=config.g_power_trace_enabled;
    g_dtm_enabled=config.g_dtm_enabled;
    
    if(g_power_simulation_enabled) {
        m_power_interface = new power_interface(config);
        if(config.g_thermal_simulation_enabled) {
          m_hotspot_wrapper = new hotspot_wrapper();
          m_hotspot_wrapper->init(config);
        }
    }
}

void gpu_calorie::cycle(const gpgpu_sim_config &config,class power_stat_t *power_stats,double core_period){

  static bool init=true;

  if(init){ // If first cycle, don't have any power numbers yet
    init=false;
    return;
  }
  m_power_interface->cycle(config,power_stats,core_period);
  if(config.g_thermal_simulation_enabled)
    m_hotspot_wrapper->compute(power_stats,core_period*gpu_stat_sample_freq);
}

double gpu_calorie::get_max_chip_temp() {
  return m_hotspot_wrapper->find_max_temp();
}

void gpu_calorie::print_heatmap() {
  m_hotspot_wrapper->print_heatmap();
}
