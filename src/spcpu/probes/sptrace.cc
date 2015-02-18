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

#include "base/output.hh"
#include "cpu/spcpu/probes/sptrace.hh"
#include "cpu/spcpu/spcpu.hh"

SPTrace::SPTrace(const SPTraceParams *p)
    : ProbeListenerObject(p),
      skip_trace_num(p->skip_num),
      trace_num(0),
      start_tracing(false),
      traceStream(NULL),
      statusStream(NULL)
{
    traceStream = simout.create(p->trace_file, false);
    if (!traceStream)
        fatal("unable to open trace file");

    statusStream = simout.create("spstatus.txt", false);
    if (!statusStream)
        fatal("unable to open status file");
}

SPTrace::~SPTrace()
{
    simout.close(traceStream);
}

void
SPTrace::init()
{}

void
SPTrace::regProbeListeners()
{
    typedef ProbeListenerArg<SPTrace, std::pair<SimpleThread*,StaticInstPtr>>
        SPTraceListener;
    listeners.push_back(new SPTraceListener(this, "Commit",
                                             &SPTrace::trace));
}

void
SPTrace::trace(const std::pair<SimpleThread*, StaticInstPtr>& p)
{
    // debug use
    //*traceStream << "NUM_INTREGS: " << TheISA::NUM_INTREGS << "\n"; 


    SimpleThread* thread = p.first;
    const StaticInstPtr &inst = p.second;
    int destRegNum = 0;

    Addr pc = thread->pcState().instAddr();
    LivespCPU* spcpu = (LivespCPU *)thread->getCpuPtr();
    Trace::InstRecord *traceData = spcpu->getTraceData();

    if (trace_num < skip_trace_num)
        goto out;

    if ( !start_tracing )
    {
        if (!inst->isControl()) 
            goto out;

        //dump registers 
        *statusStream << "#registers" << "\n";
        for(int i=0; i < TheISA::NUM_ARCH_INTREGS; i++)
        {
            *statusStream << "x" << std::dec << i << "=" << \
                    "0x" << std::hex << thread->readIntReg(i) << "\n";
        }
        *statusStream << "\n";

        //dump stack info
        *statusStream << "#stack\n";
        *statusStream << "stack_base=0x8000000000\n";
        *statusStream << "stack_limit=0x4000\n";

        start_tracing = true;

        goto out;
    }

    // pc
    *traceStream << "0x" << std::hex << pc;
    if (inst->isMicroop())
        *traceStream << "." << thread->pcState().microPC();
    *traceStream << ":";
    

    //disassembly
    *traceStream << inst->disassemble(pc);
    *traceStream << ":";

    //RegChange
    destRegNum = inst->numDestRegs();
    for (int i=0; i < destRegNum; i++)
    {
        //*traceStream << thread->readIntReg(inst->destRegIdx(i)) << " ";
        *traceStream << inst->destRegIdx(i) << " ";
    }
    *traceStream << ":";

    //MemChange and Stride
    if (inst->opClass() == Enums::MemWrite && traceData->getAddrValid()) 
    {
        mem_trace(traceData, WRITE);
    }

    if (inst->opClass() == Enums::MemRead && traceData->getAddrValid()) 
    {
        mem_trace(traceData, READ);
    }

    *traceStream << "\n";

out:
    //inc trace_num;
    trace_num++;
}

void SPTrace::mem_trace(Trace::InstRecord* traceData, enum RDWR rw)
{
    int stride = traceData->getDataStatus();
    
    if (rw == READ)
        *traceStream << "MemRead";
    else 
        *traceStream << "MemWrite";

    *traceStream << ":vaddr ";
    Addr mem_addr = traceData->getAddr();
    *traceStream << "0x" << mem_addr << ",data " ;

    switch (stride)
    {
        case 0:
            break;
        case 3:
            *traceStream << "0x" << std::hex << traceData->getFloatData();
            break;
        case 5:
            {
                Trace::TwinU32 tu32 = traceData->getTwinU32Data();
                *traceStream << "0x" << std::hex << tu32.a << ",0x" << std::hex << tu32.b;
                break;
            }
        case 9:
            {
                Trace::TwinU64 tu64 = traceData->getTwinU64Data();
                *traceStream << "0x" << std::hex << tu64.a << ",0x" << std::hex << tu64.b;
                stride = 8;
                break;
            }
        default:
            *traceStream << "0x" << std::hex << traceData->getIntData();
    }
    *traceStream << ":Stride:" << std::dec << stride;
}

/** SPTrace SimObject */
SPTrace*
SPTraceParams::create()
{
    return new SPTrace(this);
}
