LLVM passes for compiling Rust code with Emscripten.  The code could use some
cleanup, but it more or less works.  Currently it appears only
`-remove-overflow-checks` and `-remove-assume` are needed.  Otherwise, upstream
[emscripten-fastcomp][em-fc] (version 1.29.0) works.

[em-fc]: https://github.com/kripken/emscripten-fastcomp/tree/1.29.0

Compile with:

    make RemoveOverflowChecks.so RemoveAssume.so \
        LLVM_PREFIX=.../emscripten-fastcomp/build

Use with:

    .../emscripten-fastcomp/build/bin/opt \
        -load=RemoveOverflowChecks.so -load=RemoveAssume.so \
        -O3 -remove-overflow-checks -remove-assume -globaldce \
        input.ll -S -o output.ll
