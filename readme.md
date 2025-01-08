# Setup ACHyB
(These are the steps that I had to do)

1. Download the LLVM binaries
```bash
wget https://releases.llvm.org/9.0.0/clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz
tar -xvf clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz
rm clang+llvm-9.0.0-x86_64-pc-linux-gnu.tar.xz
```

2. Add the full path of ```./clang+llvm-9.0.0-x86_64-pc-linux-gnu/bin``` to $PATH. Then confirm that clang version 9.0.0 is available with
```bash
clang --version
```
 If you get ```clang: error while loading shared libraries: libz3.so.4.8: cannot open shared object file: No such file or directory``` , try [installing z3.4.8](#z3lib-missing-version)

3. Build the [kernel binary](#setup-kernel-binary) 

4. Inside the ```/static``` directory of the project:
```bash
./build.sh build
```
# Setup Kernel Binary
(These are the steps that I had to do)

0. Download linux kernel (git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git)


/achyb/cve-analyzer/kernel/linux -> linus repo
/linux -> githubhuyang repo
/caso-a-mano/linux -> stable repo

1. Install apt dependencies ```sudo apt install make flex bison libelf-dev libssl-dev```

2. Install wllvm ```pip install wllvm```





# z3lib missing version

 - Install the z3lib version that clang needs
```bash
wget https://github.com/Z3Prover/z3/releases/download/z3-4.8.17/z3-4.8.17-x64-glibc-2.31.zip
unzip z3-4.8.17-x64-glibc-2.31.zip
sudo cp z3-4.8.17-x64-glibc-2.31/bin/libz3.so /usr/lib/libz3.so.4.8.17
sudo ln -s /usr/lib/libz3.so.4.8.17 /usr/lib/libz3.so.4.8
```
