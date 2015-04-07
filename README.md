gem5-sp
=========
It contains all necessary sources and scripts for adapting [gem5](http://www.gem5.org) to serving as the frontend of **TLiveSP**.

Install
---------
1. Clone gem5 repository to local
    <pre>
    hg clone [http://repo.gem5.org/](http://repo.gem5.org/)
    </pre>
2. Checkout the stable version
    <pre>
    hg update stable_2014_12_14
    </pre>
3. Copy the cpu model to gem5 tree and compile
    <pre>
    cp -r gem5-sp/src/spcpu/ gem5/src/
    scons -j8 build/ARM/gem5.opt CPU_MODELS=AtomicSimpleCPU,LivespCPU
    </pre>

Use
-------
### Periodically checkpointing the simulation

### Restore from one checkpoint and get trace
