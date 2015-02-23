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

void SPSETranslatingPortProxy::setProbePointArg(ProbePointArg<std::pair<const uint8_t*,int>>* pp)
{
    ppSysemu = pp;
}

void SPSETranslatingPortProxy::writeBlob(Addr addr, uint8_t *p, int size) const {
    if (ppSysemu == NULL)
        fatal("ProbePointArg of SPSETranslatingPortProxy didn't initialize!");
    ppSysemu->notify(std::make_pair(p, size));

    if (!tryWriteBlob(addr, p, size))
        fatal("writeBlob(0x%x, ...) failed", addr);
};
