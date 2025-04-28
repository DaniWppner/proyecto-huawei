# Setup ACHyB

## Compile the Linux kernel

1. Get version 4.19.192 of the linux kernel from the main git repository
```bash
git clone git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
cd linux
git checkout -b v4.19.192 tags/v4.19.192
```

2. Launch Docker environment
```bash
docker compose build
docker compose up -d 
docker exec -it CONTAINERID bash
```
If this is the first time building the container, building it will probably take a lot of time (a couple of hours).
You should edit the `LLVM_BUILD_JOBS` variable in `docker-compose.yml` if 6 cpus doesn't fit your hardware.

3. Compile the kernel with wllvm (Whole-program llvm)
```bash
(inside the docker container)
cp achyb/static/kconfig/myallyes_defconfig linux/arch/x86/configs/myallyes_defconfig
cd linux
export LLVM_COMPILER=clang
export JOBS=8
make ARCH=x86_64 CC=wllvm defconfig
make ARCH=x86_64 CC=wllvm -j$JOBS
extract-bc vmlinux
```

## Run the ACHyB static analysis

1. Copy the bitcode file to the achyb directory tree
```bash
mkdir achyb/static/linux
cp linux/vmlinux.bc achyb/static/linux
```

2. Build and run the static analysis
```bash
(inside the docker container)
cd achyb/static
./build.sh build
./run.sh
```
