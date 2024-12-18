# Setup ACHyB

1. Download the LLVM binaries
```
wget https://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz
tar -xvf clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz
rm clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz
```
Now you can add the uncompressed folder to the PATH like the README says.

THIS DOESNT SEEM TO BE NECESSARY? WTH?

2. Download the source code for version 2.5.0 of CUDD (Colorado University Decision Diagram Package) for some files that are missing in the source code of CUDD included in ACHyB
```
wget https://github.com/ivmai/cudd/archive/refs/tags/cudd-2.5.0.tar.gz
tar -xvf cudd-2.5.0.tar.gz
rm cudd-2.5.0.tar.gz
```


