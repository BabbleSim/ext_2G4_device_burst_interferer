/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef BS_BURSTINT_ARGS_H
#define BS_BURSTINT_ARGS_H

#include "bs_types.h"
#include "bs_pc_2G4_types.h"
#include "bs_cmd_line.h"
#include "bs_cmd_line_typical.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BS_BUSRTINT_MAX_TIMES 256

typedef struct{
  BS_BASIC_DEVICE_OPTIONS_FIELDS
  p2G4_power_t powerdBm;
  p2G4_freq_t center_freq;
  p2G4_modulation_t type;
  uint n_times_start;
  uint n_times_end;
  bs_time_t times_start[BS_BUSRTINT_MAX_TIMES];
  bs_time_t times_end[BS_BUSRTINT_MAX_TIMES];
} burstint_args_t;

void bs_burstint_argsparse(int argc, char *argv[], burstint_args_t *args);

#ifdef __cplusplus
}
#endif

#endif
