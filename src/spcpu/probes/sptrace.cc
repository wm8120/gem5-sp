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

#define STACK_BOTTOM 0x8000000000

using namespace std;

SPTrace::SPTrace(const SPTraceParams *p)
    : ProbeListenerObject(p),
      skip_trace_num(p->skip_num),
      trace_num(0),
      pre_pc(0),
      start_tracing(false),
      bb_single_branch(true),
      traceStream(NULL),
      statusStream(NULL)
{
    resetFakePC();

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
    //needn't cloase statusStream, cause statusStream is closed after dumping init status
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

    typedef ProbeListenerArg<SPTrace, ScEmuInfo> SPSysemuListener;
    listeners.push_back(new SPSysemuListener(this, "SysEmu", &SPTrace::syscallTrace));

    typedef ProbeListenerArg<SPTrace, SimpleThread*> SPSvcRetListener;
    listeners.push_back(new SPSvcRetListener(this, "SvcRet", &SPTrace::syscallRet));
}

void
SPTrace::trace(const std::pair<SimpleThread*, StaticInstPtr>& p)
{
    // debug use
    //*traceStream << "NUM_INTREGS: " << TheISA::NUM_INTREGS << "\n"; 

    SimpleThread* thread = p.first;
    const StaticInstPtr &inst = p.second;

    Addr pc = thread->pcState().instAddr();
    LivespCPU* spcpu = (LivespCPU *)thread->getCpuPtr();
    Trace::InstRecord *traceData = spcpu->getTraceData();

    ThreadContext* tc = thread->getTC();

    if (trace_num < skip_trace_num)
        goto out;

    if ( !start_tracing )
    {
        if (!inst->isControl()) 
            goto out;

        //dump initial status
        dumpInitStatus(statusStream, thread);

        start_tracing = true;

        goto out;
    }

    if (bb_single_branch)
    {
        if (!inst->isControl())
        {
            //it's not a single branch control, go on processing
            bb_single_branch = false;
        }
        else
        {
            //truncate the previous spstatus.txt
            statusStream = simout.create("spstatus.txt", false);
            if (!statusStream)
                fatal("unable to open status file");

            //dump initial status
            dumpInitStatus(statusStream, thread);

            goto out;
        }
    }

    if (inst->isSyscall()) {
    
        *traceStream << "0x" << std::hex << pc;
        if (inst->isMicroop())
            *traceStream << "." << thread->pcState().microPC();

        *traceStream << ":" << hex << spcpu->getMachInst() << "(svc 0";
        uint32_t callNum = tc->readIntReg(INTREG_X8);
        *traceStream << ";" << dec << callNum << ")\n";
        //dump registers x0-x7
        *traceStream << "0x" << std::hex << fake_pc << ":d503201f(nop):RegChange:";
        for (int i=0; i<8; i++)
        {
            *traceStream << "x" << dec << i << " 0x" << \
                hex << thread->readIntReg(i) << " ";
        }
        *traceStream << "\n";
        fake_pc += 4;
        return;
    }

    // pc
    *traceStream << "0x" << std::hex << pc;
    if (inst->isMicroop())
        *traceStream << "." << thread->pcState().microPC();
    *traceStream << ":";
    
    //disassembly
    //*traceStream << inst->disassemble(pc);
    *traceStream << setfill('0') << setw(8) << hex << spcpu->getMachInst();
    *traceStream << "(" << inst->disassemble(pc) << ")";

    //RegChange
    //destRegNum = inst->numDestRegs();
    //for (int i=0; i < destRegNum; i++)
    //{
    //    //*traceStream << thread->readIntReg(inst->destRegIdx(i)) << " ";
    //    *traceStream << inst->destRegIdx(i) << " ";
    //}
    //*traceStream << ":";

    //MemChange and Stride
    if (inst->opClass() == Enums::MemWrite && traceData->getAddrValid()) 
    {
        MemRecord* memTraceData = spcpu->getMemTrace();
        if (memTraceData == NULL) {
            panic("MemRecord isn't initialized");
        }
        mem_trace(traceData, memTraceData, WRITE);
    }

    if (inst->opClass() == Enums::MemRead && traceData->getAddrValid()) 
    {
        MemRecord* memTraceData = spcpu->getMemTrace();
        if (memTraceData == NULL) {
            panic("MemRecord isn't initialized");
        }
        mem_trace(traceData, memTraceData, READ);
    }

    *traceStream << "\n";

out:
    pre_pc = pc;
    //inc trace_num;
    if ( !inst->isMicroop() || inst->isLastMicroop() )
        trace_num++;
}

void SPTrace::mem_trace(Trace::InstRecord* traceData, MemRecord* memTraceData, enum RDWR rw)
{
    int stride = memTraceData->getStride();
    
    if (rw == READ)
        *traceStream << ":MemRead";
    else 
        *traceStream << ":MemWrite";

    *traceStream << ":vaddr ";
    Addr mem_addr = traceData->getAddr();
    *traceStream << "0x" << mem_addr << ",data " ;

    *traceStream << memTraceData->strData().c_str();

    *traceStream << ":Stride:" << std::dec << stride;
}

void SPTrace::syscallTrace(const ScEmuInfo& p)
{
    const Addr vaddr = p.vaddr;
    const uint8_t* data = p.data;
    int origin_size = p.size;

    uint8_t* cur_data = (uint8_t*) data;
    Addr cur_vaddr = (Addr) vaddr;
    int size = origin_size;

    while (size-8 > 0)
    {
        *traceStream << "0x" << std::hex << fake_pc << ":d503201f(nop)" << \
            ":MemWrite:vaddr " << std::hex << (void *)cur_vaddr << \
            ",data ";
        MemRecord memr(cur_data, 8);
        *traceStream << memr.strData() << ":Stride:8" << "\n";
        fake_pc += 4;
        cur_data += 8;
        cur_vaddr += 8;
        size -= 8;
    }
    for (; size > 0; size--)
    {
        *traceStream << "0x" << std::hex << fake_pc << ":d503201f(nop)" << \
            ":MemWrite:vaddr " << std::hex << (void*)cur_vaddr << \
            ",data 0x" << std::setfill('0') << std::setw(2) << \
            std::hex << (unsigned)*cur_data << ":Stride:1" << "\n";
        fake_pc += 4;
        cur_data++;
        cur_vaddr++;
    }
}

void SPTrace::syscallRet(SimpleThread* const & thread)
{
    *traceStream << "0x" << std::hex << fake_pc << ":d503201f(nop):RegChange:";
    for (int i=0; i<8; i++)
    {
        *traceStream << "x" << dec << i << " 0x" << \
            hex << thread->readIntReg(i) << " ";
    }
    
    *traceStream << "\n";
    resetFakePC();
}

void SPTrace::resetFakePC()
{
    fake_pc = STACK_BOTTOM*16;
}

void SPTrace::dumpInitStatus(std::ostream* statusStream, SimpleThread* thread)
{
    //dump registers 
    *statusStream << "#registers" << "\n";
    //X31 need to be output seperatedly, because readIntReg(31) return the 
    // value of INTREG_ZERO rather than INTREG_SPX
    for(int i=0; i < TheISA::NUM_ARCH_INTREGS; i++)
    {
        *statusStream << "x" << std::dec << i << "=";
        if (i==31)
            *statusStream << "0x" << std::hex << thread->readIntReg(INTREG_SPX) << "\n";
        else
            *statusStream << "0x" << std::hex << thread->readIntReg(i) << "\n";
    }
    *statusStream << "\n";

    //vfp/simd
    for (int i=0; i<128; i+=4) {
        *statusStream << "v" << dec << i/4 << "=0x";
        *statusStream << setfill('0') << setw(8) << hex << thread->readFloatRegBits(i+3);
        *statusStream << setfill('0') << setw(8) << hex << thread->readFloatRegBits(i+2);
        *statusStream << setfill('0') << setw(8) << hex << thread->readFloatRegBits(i+1);
        *statusStream << setfill('0') << setw(8) << hex << thread->readFloatRegBits(i);
        *statusStream << "\n";
    }
    *statusStream << "\n";

    //tpidr_el0
    *statusStream << "tpidr_el0=" << hex << "0x" << thread->readMiscRegNoEffect(MISCREG_TPIDR_EL0) << "\n";
    *statusStream << "\n";

    //dump stack info
    *statusStream << "#stack\n";
    *statusStream << "stack_base=0x8000000000\n";
    *statusStream << "stack_limit=0x4000\n";

    //interval size
    *statusStream << "#interval\n";
    *statusStream << "interval=0x5F5E100"; //100,000,000

    simout.close(statusStream);
}

/** SPTrace SimObject */
    SPTrace*
SPTraceParams::create()
{
    return new SPTrace(this);
}
