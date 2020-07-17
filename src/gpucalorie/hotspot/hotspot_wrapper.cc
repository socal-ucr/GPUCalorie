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

#include "hotspot_wrapper.h"
#include <sys/stat.h>

hotspot_wrapper::hotspot_wrapper() {

    names = NULL;
    vals = NULL;
    tout = NULL;
    flp = NULL;
    model = NULL;
    temp = NULL;
    power = NULL;
    total_power = 0.0;
    overall_power = NULL;
    steady_temp = NULL;
    natural = 0; 
    avg_sink_temp = 0.0;
    natural_convergence = 0;
    r_convec_old = 0.0;
    do_transient = true;
    size = 0;
    num_samples = 0;
}

hotspot_wrapper::~hotspot_wrapper() {
  /* cleanup	*/
  if (do_transient)
    fclose(tout);
  delete_RC_model(model);
  free_flp(flp, FALSE);
  if (do_transient)
    free_dvector(temp);
  free_dvector(power);
  free_dvector(steady_temp);
  free_dvector(overall_power);
  free_names(names);
  free_dvector(vals);

}

void hotspot_wrapper::init(const gpgpu_sim_config &config) {

    static bool hotspot_init=true;
    int idx;

    if(hotspot_init) {
        num_shaders = config.num_shader();
        /*BU_3D: variable for heterogenous R-C model */
        int do_detailed_3D = FALSE; //BU_3D: do_detailed_3D, false by default

        /*copy to hotspot global config */
        strcpy(global_config.flp_file,config.g_floorplan_input_file);
        strcpy(global_config.t_outfile,config.g_thermal_trace_filename);
        strcpy(global_config.config,config.g_thermal_config_file);

        do_detailed_3D = config.g_enable_detailed_3d;

        /* read configuration file	*/
        if (strcmp(global_config.config, NULLFILE))
            size += read_str_pairs(&table[size], MAX_ENTRIES, global_config.config);

        /* no transient simulation, only steady state	*/
        if(!strcmp(global_config.t_outfile, NULLFILE))
            do_transient = FALSE;

        /* get defaults */
        thermal_config = default_thermal_config();
        /* modify according to command line / config file	*/
        thermal_config_add_from_strs(&thermal_config, table, size);

        /* if package model is used, run package model */
        if (((idx = get_str_index(table, size, "package_model_used")) >= 0) && !(table[idx].value==0)) {
            if (thermal_config.package_model_used) {
                avg_sink_temp = thermal_config.ambient + SMALL_FOR_CONVEC;
                natural = package_model(&thermal_config, table, size, avg_sink_temp);
                if (thermal_config.r_convec<R_CONVEC_LOW || thermal_config.r_convec>R_CONVEC_HIGH)
                    printf("Warning: Heatsink convection resistance is not realistic, double-check your package settings...\n"); 
            }
        }

        /* initialization: the flp_file global configuration 
        * parameter is overridden by the layer configuration 
        * file in the grid model when the latter is specified.
        */
        flp = read_flp(global_config.flp_file, FALSE);

        //BU_3D: added do_detailed_3D to alloc_RC_model. Detailed 3D modeling can only be used with grid-level modeling.
        /* allocate and initialize the RC model	*/
        model = alloc_RC_model(&thermal_config, flp, do_detailed_3D); 
        if (model->type == BLOCK_MODEL && do_detailed_3D) 
            fatal("Detailed 3D option can only be used with grid model\n"); //end->BU_3D
        if ((model->type == GRID_MODEL) && !model->grid->has_lcf && do_detailed_3D) 
            fatal("Detailed 3D option can only be used in 3D mode\n");

        populate_R_model(model, flp);

        if (do_transient)
            populate_C_model(model, flp);

        /* allocate the temp and power arrays	*/
        /* using hotspot_vector to internally allocate any extra nodes needed	*/
        if (do_transient)
            temp = hotspot_vector(model);
            power = hotspot_vector(model);
            steady_temp = hotspot_vector(model);
            overall_power = hotspot_vector(model);

        /* set up initial instantaneous temperatures */
        if (do_transient && strcmp(model->config->init_file, NULLFILE)) {
            if (!model->config->dtm_used)	/* initial T = steady T for no DTM	*/
                read_temp(model, temp, model->config->init_file, FALSE);
            else    /* initial T = clipped steady T with DTM	*/
                read_temp(model, temp, model->config->init_file, TRUE);
        } else if (do_transient)    /* no input file - use init_temp as the common temperature	*/
            set_temp(model, temp, model->config->init_temp);

        /* n is the number of functional blocks in the block model
        * while it is the sum total of the number of functional blocks
        * of all the floorplans in the power dissipating layers of the 
        * grid model. 
        */
        if (model->type == BLOCK_MODEL)
            num_functional_blocks = model->block->flp->n_units;
        else if (model->type == GRID_MODEL) {
            for(i=0; i < model->grid->n_layers; i++)
                if (model->grid->layers[i].has_power)
                    num_functional_blocks += model->grid->layers[i].flp->n_units;
        } else 
            fatal("unknown model type\n");

        if(do_transient && !(tout = fopen(global_config.t_outfile, "w")))
            fatal("unable to open temperature trace file for output\n");

        /* names of functional units	*/
        names = alloc_names(MAX_UNITS, STR_SIZE);
        if(read_names(config.g_flp_block_names, names) != num_functional_blocks)
            fatal("no. of units in floorplan and trace file differ\n");

        /* header line of temperature trace	*/
        if (do_transient)
            write_names(tout, names, num_functional_blocks);

        vals = dvector(MAX_UNITS);

        hotspot_init = false;
    }
}

void hotspot_wrapper::update_power(class power_stat_t *power_stats) {

    for(unsigned i =0;i < num_shaders;++)
        vals[i] = power_stats->get_sm_power(i);

    vals[num_shaders] = power_stats->get_l2_power();
    vals[num_shaders+1] = power_stats->get_dram_power();
    vals[num_shaders+2] = power_stats->get_dram_power();
    vals[num_shaders+3] = power_stats->get_dram_power();
    return;
}

void hotspot_wrapper::update_temps(double *vals) {

    for(unsigned i =0;i < num_shaders;++)
        power_stats->set_sm_temps(vals[i],i);

    power_stats->set_l2_temps(vals[num_shaders]);
    power_stats->set_dram_temps(vals[num_shaders+1]);
    return;
}

void hotspot_wrapper::compute(class power_stat_t *power_stats,double time_elapsed) {

    update_power(power_stats);

    int idx, i, j, base, count;

      /* permute the power numbers according to the floorplan order	*/
      if (model->type == BLOCK_MODEL)
        for(i=0; i < num_functional_blocks; i++)
          power[get_blk_index(flp, names[i])] = vals[i];
      else
        for(i=0, base=0, count=0; i < model->grid->n_layers; i++) {
            if(model->grid->layers[i].has_power) {
                for(j=0; j < model->grid->layers[i].flp->n_units; j++) {
                    idx = get_blk_index(model->grid->layers[i].flp, names[count+j\]);
                    power[base+idx] = vals[count+j];
                }
                count += model->grid->layers[i].flp->n_units;
            }	
            base += model->grid->layers[i].flp->n_units;	
        }

      /* compute temperature	*/
      if (do_transient) {
          /* if natural convection is considered, update transient convection resistance first */
          if (natural) {
              avg_sink_temp = calc_sink_temp(model, temp);
              natural = package_model(model->config, table, size, avg_sink_temp);
              populate_R_model(model, flp);
          }
          /* for the grid model, only the first call to compute_temp
           * passes a non-null 'temp' array. if 'temp' is  NULL, 
           * compute_temp remembers it from the last non-null call. 
           * this is used to maintain the internal grid temperatures 
           * across multiple calls of compute_temp
           */
          if (model->type == BLOCK_MODEL || num_samples == 0)
            compute_temp(model, power, temp, time_elapsed);
          else
            compute_temp(model, power, NULL, time_elapsed);

          /* permute back to the trace file order	*/
          if (model->type == BLOCK_MODEL)
            for(i=0; i < num_functional_blocks; i++)
              vals[i] = temp[get_blk_index(flp, names[i])];
          else
            for(i=0, base=0, count=0; i < model->grid->n_layers; i++) {
                if(model->grid->layers[i].has_power) {
                    for(j=0; j < model->grid->layers[i].flp->n_units; j++) {
                        idx = get_blk_index(model->grid->layers[i].flp, names[count+j]);
                        vals[count+j] = temp[base+idx];
                    }
                    count += model->grid->layers[i].flp->n_units;	
                }	
                base += model->grid->layers[i].flp->n_units;	
            }

          /* output instantaneous temperature trace	*/
          write_vals(tout, vals, num_functional_blocks);

          update_temps(vals);
    }

    if (model->type == BLOCK_MODEL) 
        for(i=0; i < n; i++) 
            overall_power[i] += power[i]; 
    else 
        for(i=0, base=0; i < model->grid->n_layers; i++) { 
            if(model->grid->layers[i].has_power) 
                for(j=0; j < model->grid->layers[i].flp->n_units; j++) 
                    overall_power[base+j] += power[base+j]; 
            base += model->grid->layers[i].flp->n_units;         
       }
    
    num_samples++;
}


hotspot_wrapper::ending() {

  /* for computing averag   */
  if (model->type == BLOCK_MODEL)
    for(i=0; i < num_functional_blocks; i++) {
        overall_power[i] /= num_samples;
        total_power += overall_power[i];
    }
  else
    for(i=0, base=0; i < model->grid->n_layers; i++) {
        if(model->grid->layers[i].has_power)
          for(j=0; j < model->grid->layers[i].flp->n_units; j++) {
              overall_power[base+j] /= num_samples;
              total_power += overall_power[base+j];
          }
        base += model->grid->layers[i].flp->n_units;	
    }

  /* natural convection r_convec iteration, for steady-state only */ 		
  natural_convergence = 0;
  if (natural) { /* natural convection is used */
      while (!natural_convergence) {
          r_convec_old = model->config->r_convec;
          /* steady state temperature	*/
          steady_state_temp(model, overall_power, steady_temp);
          avg_sink_temp = calc_sink_temp(model, steady_temp) + SMALL_FOR_CONVEC;
          natural = package_model(model->config, table, size, avg_sink_temp);
          populate_R_model(model, flp);
          if (avg_sink_temp > MAX_SINK_TEMP)
            fatal("too high power for a natural convection package -- possible thermal runaway\n");
          if (fabs(model->config->r_convec-r_convec_old)<NATURAL_CONVEC_TOL) 
            natural_convergence = 1;
      }
  }	else /* natural convection is not used, no need for iterations */
    /* steady state temperature	*/
    steady_state_temp(model, overall_power, steady_temp);

  /* print steady state results	*/
  //BU_3D: Only print steady state results to stdout when DEBUG3D flag is not set
#if DEBUG3D < 1
  fprintf(stdout, "Unit\tSteady(Kelvin)\n");
  dump_temp(model, steady_temp, "stdout");
#endif //end->BU_3D

  /* dump steady state temperatures on to file if needed	*/
  if (strcmp(model->config->steady_file, NULLFILE))
    dump_temp(model, steady_temp, model->config->steady_file);

  /* for the grid model, optionally dump the most recent 
   * steady state temperatures of the grid cells	
   */
  if (model->type == GRID_MODEL &&
      strcmp(model->config->grid_steady_file, NULLFILE))
    dump_steady_temp_grid(model->grid, model->config->grid_steady_file);

#if VERBOSE > 2
  if (model->type == BLOCK_MODEL) {
      if (do_transient) {
          fprintf(stdout, "printing temp...\n");
          dump_dvector(temp, model->block->n_nodes);
      }
      fprintf(stdout, "printing steady_temp...\n");
      dump_dvector(steady_temp, model->block->n_nodes);
  } else {
      if (do_transient) {
          fprintf(stdout, "printing temp...\n");
          dump_dvector(temp, model->grid->total_n_blocks + EXTRA);
      }
      fprintf(stdout, "printing steady_temp...\n");
      dump_dvector(steady_temp, model->grid->total_n_blocks + EXTRA);
  }
#endif
 

}
