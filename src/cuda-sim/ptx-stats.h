// Copyright (c) 2009-2011, Wilson W.L. Fung, Tor M. Aamodt
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

#pragma once

#include "../option_parser.h"

#ifdef __cplusplus
// stat collection interface to cuda-sim
class ptx_instruction;
void ptx_file_line_stats_add_exec_count(const ptx_instruction* pInsn);
#endif

// stat collection interface to gpgpu-sim

void ptx_file_line_stats_create_exposed_latency_tracker(int n_shader_cores);
void ptx_file_line_stats_commit_exposed_latency(int sc_id, int exposed_latency);

class gpgpu_context;
class ptx_stats {
 public:
  ptx_stats(gpgpu_context* ctx) {
    ptx_line_stats_filename = NULL;
    gpgpu_ctx = ctx;
  }
  char* ptx_line_stats_filename;
  bool enable_ptx_file_line_stats;
  gpgpu_context* gpgpu_ctx;
  // set options
  void ptx_file_line_stats_options(option_parser_t opp);

  // output stats to a file
  void ptx_file_line_stats_write_file();
  // stat collection interface to gpgpu-sim
  void ptx_file_line_stats_add_latency(unsigned pc, unsigned latency);
  void ptx_file_line_stats_add_dram_traffic(unsigned pc, unsigned dram_traffic);
  void ptx_file_line_stats_add_smem_bank_conflict(unsigned pc,
                                                  unsigned n_way_bkconflict);
  void ptx_file_line_stats_add_uncoalesced_gmem(unsigned pc, unsigned n_access);
  void ptx_file_line_stats_add_inflight_memory_insn(int sc_id, unsigned pc);
  void ptx_file_line_stats_sub_inflight_memory_insn(int sc_id, unsigned pc);
  void ptx_file_line_stats_add_warp_divergence(unsigned pc,
                                               unsigned n_way_divergence);
//<AliJahan/>
  enum instr_stats_t {T_DECODE = 0, T_ALU = 1,
                    T_FP  = 2, T_INT_MUL  = 3, T_TEX = 4, 
                    T_FP_MUL = 5, T_TRANS = 6, T_INT_DIV = 7, 
                    T_FP_DIV = 8, T_INT = 9, T_SP = 10, T_SFU = 11, T_DP = 12, 
                    T_FP_SQRT = 13, T_FP_LG = 14, T_FP_EXP = 15, T_FP_SIN = 16, 
                    T_COLL_UNT = 17, T_TENSOR = 18, NB_MEM_ACC = 19,
                    NB_RF_RD = 20, NB_RF_WR = 21, NB_NON_RF_OPRND = 22,
                    T_INT_MUL24 = 23, T_INT_MUL32 = 24, DIVERGENCE = 25,
                    NB_LD = 26, NB_ST = 27, IS_SHMEM_INST = 28, 
                    IS_SSTAR_INST = 29, IS_TEX_INST = 30, IS_CONST_INST = 31, IS_PARAM_INST = 32,
                    NB_LOCAL_MEM_RD = 33, NB_LOCAL_MEM_WR = 34, NB_TEX_MEM = 35, 
                    NB_CONST_MEM = 36, NB_GLOB_MEM_RD = 37, NB_GLOB_MEM_WR = 38,
                    IC_HIT = 39, IC_MISS = 40, DC_L1_LD = 41, DC_L1_ST = 42
                    };
  void update_instr_stats(unsigned, enum instr_stats_t, int);
//</AliJahan>
};
