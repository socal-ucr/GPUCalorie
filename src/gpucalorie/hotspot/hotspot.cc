/* 
 * This is a trace-level thermal simulator. It reads power values 
 * from an input trace file and outputs the corresponding instantaneous 
 * temperature values to an output trace file. It also outputs the steady 
 * state temperature values to stdout.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "flp.h"
#include "package.h"
#include "temperature.h"
#include "temperature_block.h"
#include "temperature_grid.h"
#include "util.h"
#include "hotspot.h"

/* HotSpot thermal model is offered in two flavours - the block
 * version and the grid version. The block model models temperature
 * per functional block of the floorplan while the grid model
 * chops the chip up into a matrix of grid cells and models the 
 * temperature of each cell. It is also capable of modeling a
 * 3-d chip with multiple floorplans stacked on top of each
 * other. The choice of which model to choose is done through
 * a command line or configuration file parameter model_type. 
 * "-model_type block" chooses the block model while "-model_type grid"
 * chooses the grid model. 
 */

/* Guidelines for choosing the block or the grid model	*/

/**************************************************************************/
/* HotSpot contains two methods for solving temperatures:                 */
/* 	1) Block Model -- the same as HotSpot 2.0	              						  */
/*	2) Grid Model -- the die is divided into regular grid cells       	  */
/**************************************************************************/
/* How the grid model works: 											                        */
/* 	The grid model first reads in floorplan and maps block-based power	  */
/* to each grid cell, then solves the temperatures for all the grid cells,*/
/* finally, converts the resulting grid temperatures back to block-based  */
/* temperatures.                            														  */
/**************************************************************************/
/* The grid model is useful when 				                    						  */
/* 	1) More detailed temperature distribution inside a functional unit    */
/*     is desired.														                            */
/*  2) Too many functional units are included in the floorplan, resulting */
/*		 in extremely long computation time if using the Block Model        */
/*	3) If temperature information is desired for many tiny units,		      */ 
/* 		 such as individual register file entry.						                */
/**************************************************************************/
/*	Comparisons between Grid Model and Block Model:						            */
/*		In general, the grid model is more accurate, because it can deal    */
/*	with various floorplans and it provides temperature gradient across	  */
/*	each functional unit. The block model models essentially the center	  */
/*	temperature of each functional unit. But the block model is typically */
/*	faster because there are less nodes to solve.						              */
/*		Therefore, unless it is the case where the grid model is 		        */
/*	definitely	needed, we suggest using the block model for computation  */
/*  efficiency.															                              */
/**************************************************************************/

/* 
 * parse a table of name-value string pairs and add the configuration
 * parameters to 'config'
 */
void global_config_from_strs(global_config_t *config, str_pair *table, int size)
{
  int idx;
  if ((idx = get_str_index(table, size, "f")) >= 0) {
      if(sscanf(table[idx].value, "%s", config->flp_file) != 1)
        fatal("invalid format for configuration  parameter flp_file\n");
  } else {
      fatal("required parameter flp_file missing. check usage\n");
  }
  if ((idx = get_str_index(table, size, "p")) >= 0) {
      if(sscanf(table[idx].value, "%s", config->p_infile) != 1)
        fatal("invalid format for configuration  parameter p_infile\n");
  } else {
      fatal("required parameter p_infile missing. check usage\n");
  }
  if ((idx = get_str_index(table, size, "o")) >= 0) {
      if(sscanf(table[idx].value, "%s", config->t_outfile) != 1)
        fatal("invalid format for configuration  parameter t_outfile\n");
  } else {
      strcpy(config->t_outfile, NULLFILE);
  }
  if ((idx = get_str_index(table, size, "c")) >= 0) {
      if(sscanf(table[idx].value, "%s", config->config) != 1)
        fatal("invalid format for configuration  parameter config\n");
  } else {
      strcpy(config->config, NULLFILE);
  }
  if ((idx = get_str_index(table, size, "d")) >= 0) {
      if(sscanf(table[idx].value, "%s", config->dump_config) != 1)
        fatal("invalid format for configuration  parameter dump_config\n");
  } else {
      strcpy(config->dump_config, NULLFILE);
  }
  if ((idx = get_str_index(table, size, "detailed_3D")) >= 0) {
      if(sscanf(table[idx].value, "%s", config->detailed_3D) != 1)	
        fatal("invalid format for configuration  parameter lc\n");
  } else {
      strcpy(config->detailed_3D, "off");
  }
}

/* 
 * convert config into a table of name-value pairs. returns the no.
 * of parameters converted
 */
int global_config_to_strs(global_config_t *config, str_pair *table, int max_entries)
{
  if (max_entries < 6)
    fatal("not enough entries in table\n");

  sprintf(table[0].name, "f");
  sprintf(table[1].name, "p");
  sprintf(table[2].name, "o");
  sprintf(table[3].name, "c");
  sprintf(table[4].name, "d");
  sprintf(table[5].name, "detailed_3D");
  sprintf(table[0].value, "%s", config->flp_file);
  sprintf(table[1].value, "%s", config->p_infile);
  sprintf(table[2].value, "%s", config->t_outfile);
  sprintf(table[3].value, "%s", config->config);
  sprintf(table[4].value, "%s", config->dump_config);
  sprintf(table[5].value, "%s", config->detailed_3D);

  return 6;
}

/* 
 * read a single line of trace file containing names
 * of functional blocks
 */
int read_names(char *line, char **names)
{
  char *src;
  int i;

  /* chop the names from the line read	*/
  for(i=0,src=line; *src && i < MAX_UNITS; i++) {
      if(!sscanf(src, "%[^,]", names[i])) {
        fatal("invalid format of names\n");
      }
      src += strlen(names[i]);
      while (ispunct((int)*src))
        src++;
  }
  if(*src && i == MAX_UNITS)
    fatal("no. of units exceeded limit\n");

  return i;
}

/* read a single line of power trace numbers	*/
int read_vals(FILE *fp, double *vals)
{
  char line[LINE_SIZE], temp[LINE_SIZE], *src;
  int i;

  /* skip empty lines	*/
  do {
      /* read the entire line	*/
      fgets(line, LINE_SIZE, fp);
      if (feof(fp))
        return 0;
      strcpy(temp, line);
      src = strtok(temp, " \r\t\n");
  } while (!src);

  /* new line not read yet	*/	
  if(line[strlen(line)-1] != '\n')
    fatal("line too long\n");

  /* chop the power values from the line read	*/
  for(i=0,src=line; *src && i < MAX_UNITS; i++) {
      if(!sscanf(src, "%s", temp) || !sscanf(src, "%lf", &vals[i]))
        fatal("invalid format of values\n");
      src += strlen(temp);
      while (isspace((int)*src))
        src++;
  }
  if(*src && i == MAX_UNITS)
    fatal("no. of entries exceeded limit\n");

  return i;
}

/* write a single line of functional unit names	*/
void write_names(FILE *fp, char **names, int size)
{
  int i;
  for(i=0; i < size-1; i++)
    fprintf(fp, "%s\t", names[i]);
  fprintf(fp, "%s\n", names[i]);
}

/* write a single line of temperature trace(in degree C) */
void write_vals(FILE *fp, double *vals, int size)
{
  int i;
  for(i=0; i < size-1; i++)
    fprintf(fp, "%.2f\t", vals[i]-273.15);
  fprintf(fp, "%.2f\n", vals[i]-273.15);
}

char **alloc_names(int nr, int nc)
{
  int i;
  char **m;

  m = (char **) calloc (nr, sizeof(char *));
  assert(m != NULL);
  m[0] = (char *) calloc (nr * nc, sizeof(char));
  assert(m[0] != NULL);

  for (i = 1; i < nr; i++)
    m[i] =  m[0] + nc * i;

  return m;
}

void free_names(char **m)
{
  free(m[0]);
  free(m);
}
