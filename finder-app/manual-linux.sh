#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

BASEDIR=/home/wifiez/coursera/linuxSysProg/assignment-2-hishamo76/finder-app/
OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- make -j8 mrproper
    ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- make -j8 defconfig
    ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- make -j4 all
    # make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dtb 
fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir "${OUTDIR}/rootfs"
cd "${OUTDIR}/rootfs"

mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log 

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    echo "distclean...."
    make distclean
    make defconfig
else
    echo "Going into busy box"
    cd busybox
fi

# TODO: Make and install busybox



make distclean
make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}  ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install 


cd ${OUTDIR}/rootfs
echo "Library dependencies"
program_interpreter=$(${CROSS_COMPILE}readelf -a ${OUTDIR}/bin/busybox | grep "program interpreter" | awk -F: '{print $2}' | awk '{print $1}')
shared_libraries=$(${CROSS_COMPILE}readelf -a ${OUTDIR}/bin/busybox | grep "Shared library" | awk -F: '{print $2}' | awk '{print $1}')

echo "${program_interpreter}"
echo "${shared_libraries}"

# TODO: Add library dependencies to rootfs
cp /home/wifiez/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib/ld-linux-aarch64.so.1 /tmp/aeld/rootfs/lib

cp /home/wifiez/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64
cp /home/wifiez/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64
cp /home/wifiez/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64
echo "Finished copying libs"

# TODO: Make device nodes
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1

# TODO: Clean and build the writer utility
if [ -e ${BASEDIR}/writer ]; then
    rm ${BASEDIR}/writer
fi
cd ${BASEDIR}
echo "${CROSS_COMPILE}gcc -o writer writer.c"
${CROSS_COMPILE}gcc -o writer writer.c

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp ${BASEDIR}/writer ${OUTDIR}/rootfs/home

cp ${BASEDIR}/finder.sh ${OUTDIR}/rootfs/home
cp ${BASEDIR}/finder-test.sh ${OUTDIR}/rootfs/home

mkdir -p ${OUTDIR}/rootfs/home/conf
cp -r ${BASEDIR}/conf/ ${OUTDIR}/rootfs/home


cp ${BASEDIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home


# TODO: Chown the root directory
cd "${OUTDIR}/rootfs" 
sudo chown -R root:root *

# TODO: Create initramfs.cpio.gz
find . |  cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
gzip -f ${OUTDIR}/initramfs.cpio
ls ${OUTDIR}
