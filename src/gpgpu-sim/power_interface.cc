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

#include "power_interface.h"

static const char * pwr_cmp_label[] = {"T_ALU","T_FP","T_DP","T_INT_MUL32","T_SFU","NB_RF","L1","SHD_MEM"};
enum pwr_cmp_t {T_ALU,T_FP,T_DP,T_INT_MUL32,T_SFU,NB_RF,L1,SHD_MEM,NUM_POWER_COMPONENTS};


power_interface::power_interface(const gpgpu_sim_config &config,const int stat_sample_freq)
{

    // Write File Headers for (-metrics trace, -power trace)

    static bool init=true;

    // initialize file name if it is not set
    time_t curr_time;
    time(&curr_time);
    char *date = ctime(&curr_time);
    char *s = date;
    while (*s) {
        if (*s == ' ' || *s == '\t' || *s == ':') *s = '-';
        if (*s == '\n' || *s == '\r' ) *s = 0;
        s++;
    }

   if(init){
	g_power_trace_filename = config.g_power_trace_filename;
	g_metric_trace_filename = config.g_metric_trace_filename;
	g_power_simulation_enabled=config.g_power_simulation_enabled;
	g_power_trace_enabled=config.g_power_trace_enabled;
        g_power_trace_zlevel=config.g_power_trace_zlevel;
        num_shaders = config.num_shader();

	gpu_stat_sample_freq=stat_sample_freq;

	if (g_power_trace_enabled ){
            power_trace_file = gzopen(g_power_trace_filename, "w");
	    metric_trace_file = gzopen(g_metric_trace_filename, "w");
	    if ((power_trace_file == NULL) || (metric_trace_file == NULL)) {
                printf("error - could not open trace files \n");
		exit(1);
	    }
	    gzsetparams(power_trace_file, g_power_trace_zlevel, Z_DEFAULT_STRATEGY);

	    gzsetparams(metric_trace_file, g_power_trace_zlevel, Z_DEFAULT_STRATEGY);
	    for(unsigned i=0; i<config.num_shader(); i++){
                std::string power_label = "SM"+std::to_string((long long int)i) + ",";
                gzprintf(power_trace_file,power_label.c_str());

	        for(unsigned i=0; i<NUM_POWER_COMPONENTS; i++){
                    std::string comp_label = "SM"+std::to_string((long long int)i) +
                                             "_" + pwr_cmp_label[i] + ",";
                    gzprintf(metric_trace_file,comp_label.c_str());
	        }
	    }
            gzprintf(power_trace_file,"L2,");
            gzprintf(power_trace_file,"MC1,");
            gzprintf(power_trace_file,"MC2,");
            gzprintf(power_trace_file,"MC3");
            gzprintf(power_trace_file,"\n");

            gzprintf(metric_trace_file,"L2,");
            gzprintf(metric_trace_file,"MEM");
	    gzprintf(metric_trace_file,"\n");

	    gzclose(power_trace_file);
	    gzclose(metric_trace_file);
	}
        init = false;
   }
}

void power_interface::cycle(const gpgpu_sim_config &config, const struct shader_core_config *shdr_config, class power_stat_t *power_stats, unsigned stat_sample_freq, unsigned tot_cycle, unsigned cycle, unsigned tot_inst, unsigned inst){

	static bool init=true;

	if(init){ // If first cycle, don't have any power numbers yet
	    init=false;
	    return;
	}

	if ((tot_cycle+cycle) % gpu_stat_sample_freq == 0) {
            open_files();
            for(int SM = 0; SM < num_shaders;SM++){
                //get component accesses
                unsigned alu = power_stats->get_tot_alu_accessess(SM);
                unsigned fp = power_stats->get_tot_fp_accessess(SM);
                unsigned dp = power_stats->get_tot_dp_accessess(SM);
                unsigned int_mul32 = power_stats->get_tot_imul32_accessess(SM);
                unsigned sfu = power_stats->get_tot_sfu_accessess(SM);
                unsigned nb_rf = power_stats->get_tot_rf_accessess(SM);
                unsigned l1 = power_stats->get_l1d_hits(SM);
                unsigned shd_mem = power_stats->get_shmem_read_access(SM);
                
		gzprintf(metric_trace_file,"%u,%u,%u,%u,%u,%u,%u,%u,",alu,fp,dp,int_mul32,sfu,nb_rf,l1,shd_mem);

                //calculate epi

            }
            unsigned l2 = power_stats->get_l2_read_hits() + power_stats->get_l2_write_hits();
            unsigned dram = power_stats->get_dram_req();
            gzprintf(metric_trace_file,"%u,%u\n",l2,dram);
	    power_stats->save_stats();
            close_files();
	}
}
void power_interface::open_files()
{
    if(g_power_simulation_enabled){
            power_trace_file = gzopen(g_power_trace_filename,  "a");
            metric_trace_file = gzopen(g_metric_trace_filename, "a");
    }
}
void power_interface::close_files()
{
    if(g_power_simulation_enabled){
  		  gzclose(power_trace_file);
  		  gzclose(metric_trace_file);
  	 }
}
