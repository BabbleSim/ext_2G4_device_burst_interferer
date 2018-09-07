/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdlib.h>
#include <string.h>
#include "bs_burstint_args.h"
#include "bs_tracing.h"
#include "bs_oswrap.h"
#include "bs_utils.h"
#include "bs_pc_2G4_utils.h"

char executable_name[] = "bs_device_2G4_burst_interf";
void component_print_post_help(){
  fprintf(stdout,
          "Generate bursts of interference of a given <type> centered in a\n"
          "given <centerf> lasting from <timestart> to <timeend>\n\n");
}

burstint_args_t *args_g;

static void cmd_trace_lvl_found(char * argv, int offset){
  bs_trace_set_level(args_g->verb);
}

static void cmd_gdev_nbr_found(char * argv, int offset){
  bs_trace_set_prefix_dev(args_g->global_device_nbr);
}

static double power_value;
static double center_freq;
static char *type_argv;

static void cmd_powervalue_found(char * argv, int offset){
  args_g->powerdBm = p2G4_power_from_d(power_value);
}

static void cmd_center_freq_found(char * argv, int offset){
  int error = p2G4_freq_from_d(center_freq, 0, &args_g->center_freq);
  if (error) {
    bs_trace_error_line("Could not parse center frequency %s\n", argv);
  }
  bs_trace_raw(9,"center frequency set to %.2lfMHz (+2400)\n",
               ((double)args_g->center_freq)/(1<<P2G4_freq_FRACB));
}

static void cmd_type_found(char * argv, int offset){
  int error = p2G4_modulation_from_string(type_argv, &args_g->type,
                                          OnlyNonReceivable, 1);
  if ( error != 0 ) {
    bs_trace_error_line("Could not interpret modulation type %s\n",type_argv);
  }
}

/**
 * Check the arguments provided in the command line: set args based on it
 * or defaults, and check they are correct
 */
void bs_burstint_argsparse(int argc, char *argv[], burstint_args_t *args)
{
  int i;

  args_g = args;
  bs_args_struct_t args_struct[] = {
      BS_BASIC_DEVICE_2G4_TYPICAL_OPTIONS_ARG_STRUCT,
      { false, false , false, "power",       "powerdBm", 'f', (void*)&power_value, cmd_powervalue_found,  "In dBm power of the interference (20dBm)"},
      { false, true  , false, "centerfreq",  "centerf",  'f', (void*)&center_freq, cmd_center_freq_found, "center_freq in MHz"},
      { false, false , false, "type",      "modu_type",  's', (void*)&type_argv,   cmd_type_found,        "{CW,(WLAN),BLE,WN{1|2|4|8|16|20|40|80}} Type of interference (default CW)"},
      { true , false , false, "timestart",      "time",  'l',  NULL,               NULL,            "Times at which the interference burst shall start (omit to run from the begining) (up to " STR(BS_BUSRTINT_MAX_TIMES) " bursts)"},
      { true , false , false, "timeend"  ,      "time",  'l',  NULL,               NULL,            "Times at which the interference burst shall end (omit the last to run until the end of the simulation) (up to " STR(BS_BUSRTINT_MAX_TIMES) " burst)"},
      ARG_TABLE_ENDMARKER
  };

  bs_args_typical_dev_set_defaults((bs_basic_dev_args_t *)args, args_struct);
  args->n_times_start = 0;
  args->n_times_end   = 0;
  args->type          = P2G4_MOD_CWINTER;
  args->center_freq   = P2G4_INVALID_FREQ;
  args->powerdBm = p2G4_power_from_d(20);
  static char default_phy[] ="2G4";

  for (i=1; i<argc; i++) {
    int offset;
    double time;
    if ( !bs_args_parse_one_arg(argv[i], args_struct) ){
      if ( ( offset = bs_is_option(argv[i], "timestart", 0) ) ) {
        while ( ( i + 1 < argc ) && ( argv[i+1][0] != '-' ) ) {
          i += 1;
          if ( sscanf(argv[i],"%lf",&time) != 1 ){
            bs_trace_error_line("Could not parse timestart entry nbr %i (%s)\n",
                                args->n_times_start+1, argv[i]);
          }
          args->times_start[args->n_times_start] = time;
          bs_trace_raw(9,"added timestart[%i] = %"PRItime" to list\n",
                       args->n_times_start,
                       args->times_start[args->n_times_start]);
          args->n_times_start += 1;
        }
      }
      else if ( ( offset = bs_is_option(argv[i], "timeend", 0) ) ) {
        while ( ( i + 1 < argc ) && ( argv[i+1][0] != '-' ) ) {
          i += 1;
          if ( sscanf(argv[i],"%lf",&time) != 1 ){
            bs_trace_error_line("Could not parse timeend entry nbr %i (%s)\n",
                                args->n_times_end+1, argv[i]);
          }
          args->times_end[args->n_times_end] = time;
          bs_trace_raw(9,"added timeend[%i] = %"PRItime" to list\n",
                       args->n_times_end, args->times_end[args->n_times_end]);
          args->n_times_end += 1;
        }
      }
      else {
        bs_args_print_switches_help(args_struct);
        bs_trace_error_line("Unknown command line switch '%s'\n",argv[i]);
      }
    }
  }

  bs_args_typical_dev_post_check((bs_basic_dev_args_t *)args, args_struct, default_phy);
  if ( args->center_freq == P2G4_INVALID_FREQ ) {
    bs_args_print_switches_help(args_struct);
    bs_trace_error_line("The center frequency needs to be specified\n");
  }

  if ( args->n_times_start == 0 ){
    args->n_times_start = 1;
    args->times_start[0] = 1;
  }
  if ( args->n_times_end == args->n_times_start - 1 ){
    args->n_times_end = args->n_times_start ;
    args->times_end[args->n_times_end-1] = TIME_NEVER;
  }
  if ( args->n_times_end != args->n_times_start ){
    bs_trace_error_line("You need to provide the same number of end and start "
                        "times (or one less if you want to let the last burst "
                        "run forever) (run with --help for more info)\n");
  }
}
