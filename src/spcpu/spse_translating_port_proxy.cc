#include <string>

#include "arch/isa_traits.hh"
#include "base/chunk_generator.hh"
#include "config/the_isa.hh"
#include "mem/page_table.hh"
#include "sim/process.hh"
#include "sim/system.hh"
#include "spse_translating_port_proxy.hh"

#include <iostream>

SPSETranslatingPortProxy::SPSETranslatingPortProxy(MasterPort& port, Process* p, AllocType alloc)
:SETranslatingPortProxy(port, p, alloc),ppSysemu(NULL)
{ 
}

void SPSETranslatingPortProxy::setProbePointArg(ProbePointArg<ScEmuInfo>* pp)
{
    ppSysemu = pp;
}

void SPSETranslatingPortProxy::writeBlob(Addr addr, uint8_t *p, int size) const {
    if (ppSysemu == NULL)
        fatal("ProbePointArg of SPSETranslatingPortProxy didn't initialize!");
    ScEmuInfo sc = {.vaddr = addr, .data = p, .size = size};
    ppSysemu->notify(sc);

    if (!tryWriteBlob(addr, p, size))
        fatal("writeBlob(0x%x, ...) failed", addr);
};
