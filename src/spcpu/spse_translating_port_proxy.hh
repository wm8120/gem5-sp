#ifndef __SP_TRANSLATING_PROXY
#define __SP_TRANSLATING_PROXY

#include "mem/page_table.hh"
#include "mem/port_proxy.hh"
#include "mem/se_translating_port_proxy.hh"
#include "sim/probe/probe.hh"

class Process;

class SPSETranslatingPortProxy: public SETranslatingPortProxy
{
    public:
        SPSETranslatingPortProxy(MasterPort& port, Process* p, AllocType alloc);
        virtual ~SPSETranslatingPortProxy() { };

        virtual void writeBlob(Addr addr, uint8_t *p, int size) const;

        void setProbePointArg(ProbePointArg<std::pair<const uint8_t*, int >>* pp);

    private:
        ProbePointArg<std::pair<const uint8_t*, int>> *ppSysemu;
};

#endif //__SP_TRANSLATING_PROXY
