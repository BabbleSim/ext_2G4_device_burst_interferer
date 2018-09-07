/*
 * Copyright 2018 Oticon A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>
#include "bs_tracing.h"
#include "bs_pc_2G4.h"
#include "bs_types.h"
#include "bs_burstint_args.h"

/**
 * Generate bursts of interference of a given <type> centered
 * in a given <center_freq> lasting from <timestart> to <timeend>
 */

static bs_time_t offset;

static bs_time_t offset_time(bs_time_t time) {
  bs_time_t phy_time;
  if (time != TIME_NEVER) {
    phy_time = (bs_time_t)time + offset;
  } else {
    phy_time = TIME_NEVER;
  }
  return phy_time;
}

static uint8_t clean_up() {
  bs_trace_raw(8, "Cleaning up\n");
  p2G4_dev_disconnect_c();
  return 0;
}

int main(int argc, char *argv[]) {
  bs_trace_register_cleanup_function(clean_up);

  burstint_args_t args;

  bs_burstint_argsparse(argc, argv, &args);
  offset = args.start_offset;

  bs_trace_raw(9,"Connecting...\n");
  p2G4_dev_initcom_c(args.device_nbr, args.s_id, args.p_id, NULL);

  bs_time_t time = 0;
  p2G4_tx_t tx_s;
  p2G4_tx_done_t tx_done_s;
  memset(&tx_s, 0, sizeof(tx_s));

  tx_s.radio_params.modulation = args.type;
  tx_s.packet_size = 0;
  tx_s.phy_address = 0;
  tx_s.radio_params.center_freq = args.center_freq;
  tx_s.power_level = args.powerdBm;
  tx_s.abort.abort_time = TIME_NEVER;
  tx_s.abort.recheck_time = TIME_NEVER;

  for (int i = 0; i < args.n_times_start; i ++) {
    if (args.times_end[i] <= args.times_start[i]) {
      bs_trace_error_line("End times need to be bigger than starting times (index %i: %i <= %i)\n",
                          i, args.times_end[i], args.times_start[i]);
    }
    if (args.times_start[i] <= time) {
      bs_trace_error_line("The list of times needs to be ordered (index %i: %i <= %i)\n",
                          i, args.times_start[i], time);
    }

    time =  args.times_start[i];
    tx_s.start_time = offset_time(args.times_start[i]);
    tx_s.end_time   = offset_time(args.times_end[i]);

    int ret_val = p2G4_dev_req_tx_c_b(&tx_s, NULL, &tx_done_s);
    if (ret_val) {
      bs_trace_raw_manual_time(3, time, "The phy disconnected\n");
      break;
    }

    time = args.times_end[i];
  }

  bs_trace_raw(9,"Disconnecting...\n");
  p2G4_dev_disconnect_c();

  return 0;
}
