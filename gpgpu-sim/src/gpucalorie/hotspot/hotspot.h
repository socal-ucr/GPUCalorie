#ifndef __HOTSPOT_H_
#define __HOTSPOT_H_

#include "util.h"

#include "flp.h"
#include "package.h"
#include "temperature.h"
#include "temperature_block.h"
#include "temperature_grid.h"
#include "util.h"
#include "hotspot.h"

/* global configuration parameters for HotSpot	*/
typedef struct global_config_t_st
{
	/* floorplan input file */
	char flp_file[STR_SIZE];
	/* input power trace file */
	char p_infile[STR_SIZE];
	/* output file for the temperature trace */
	char t_outfile[STR_SIZE];
	/* input configuration parameters from file	*/
	char config[STR_SIZE];
	/* output configuration parameters to file	*/
	char dump_config[STR_SIZE];

	
	/*BU_3D: Option to turn on heterogenous R-C assignment*/
	char detailed_3D[STR_SIZE];
	
} global_config_t;

/* 
 * parse a table of name-value string pairs and add the configuration
 * parameters to 'config'
 */
void global_config_from_strs(global_config_t *config, str_pair *table, int size);
/* 
 * convert config into a table of name-value pairs. returns the no.
 * of parameters converted
 */
int global_config_to_strs(global_config_t *config, str_pair *table, int max_entries);


/* 
 * read a single line of trace file containing names
 * of functional blocks
 */
int read_names(char *line, char **names);

/* read a single line of power trace numbers	*/
int read_vals(FILE *fp, double *vals);

/* write a single line of functional unit names	*/
void write_names(FILE *fp, char **names, int size);

/* write a single line of temperature trace(in degree C) */
void write_vals(FILE *fp, double *vals, int size);

char **alloc_names(int nr, int nc);

void free_names(char **m);
#endif //HOTSPOT_H
