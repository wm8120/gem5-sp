gem5-sp
=========
It contains all necessary sources and scripts for adapting [gem5](http://www.gem5.org) to serving as the frontend of **TLiveSP**.

Install
---------
1. Clone gem5 repository to local
    <pre>
    hg clone [http://repo.gem5.org/gem5](http://repo.gem5.org/gem5)
    </pre>
2. Checkout the stable version
    <pre>
    hg update stable_2014_12_14
    </pre>
3. Copy the cpu model to gem5 tree
    <pre>
    cp -r gem5-sp/src/spcpu/ gem5/src/
    </pre>
4. Patch some files
    <pre>
    patch src/sim/syscall_emul.hh < unlinkat.patch
    </pre>
5. Compile
    <pre>
    scons -j8 build/ARM/gem5.opt CPU_MODELS=AtomicSimpleCPU,LivespCPU
    </pre>

Use
-------
### Periodically checkpointing the simulation

### Restore from one checkpoint and get trace
