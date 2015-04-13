/*
 * Copyright (c) 2012-2014 ARM Limited
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Dam Sunwoo
 *          Curtis Dunham
 */

#ifndef __CPU_SIMPLE_PROBES_SIMPOINT_HH__
#define __CPU_SIMPLE_PROBES_SIMPOINT_HH__

#include "base/hashmap.hh"
#include "cpu/spcpu/sp_simple_thread.hh"
#include "params/SPTrace.hh"
#include "sim/probe/probe.hh"
#include "cpu/spcpu/spcpu.hh"
#include "cpu/spcpu/spse_translating_port_proxy.hh"

/**
 * Probe for tracing simpoint interval
 */

enum RDWR {READ=0, WRITE=1};

class SPTrace : public ProbeListenerObject
{
  public:
    SPTrace(const SPTraceParams *params);
    virtual ~SPTrace();

    virtual void init();

    virtual void regProbeListeners();

    /**
     * trace simpoint interval
     */
    void trace(const std::pair<SimpleThread*, StaticInstPtr>&);

    /**
     * system call trace
     */
     void syscallTrace(const ScEmuInfo&);

    /**
     * system call return
     */
     void syscallRet(SimpleThread* const &);

    /**
     * print memory access trace
     */
    void mem_trace(Trace::InstRecord*, MemRecord*, enum RDWR rw);

    /**
     * reset fake_pc to STACK_BOTTOM*16
     */
    void resetFakePC();

    /**
     * dump initial status
     */
    void dumpInitStatus(std::ostream* statusStream, SimpleThread* thread);

  private:
    /** skip the first skip_trace_num instructions from simulation start **/
    const uint64_t skip_trace_num;

    /** how many instruction traced **/
    uint64_t trace_num;

    /** the fake pc to print out when syscall emulation **/
    uint64_t fake_pc;

    /** What's the pc of last executed inst? **/
    uint64_t pre_pc;

    /** if start tracing **/
    bool start_tracing;

    /** after fastforward the beginning partial bb
     if the following bb is a single branch bb **/
    bool bb_single_branch;

    /** Pointer to trace stream */
    std::ostream *traceStream;

    /** Pointer to status file stream */
    std::ostream *statusStream;
};

#endif // __CPU_SIMPLE_PROBES_SIMPOINT_HH__
