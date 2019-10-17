#!/bin/bash
if [ "$(uname)" == "Darwin" ]; then
    env_os=0
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    env_os=1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    env_os=2
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ]; then
    env_os=2
fi

echo -e "\e[94m============================= PPC  TEST AUTO: FULL =============================\e[39m"

echo "Compiling..."
# {
# Modify output to binary
sed -i 's/QEM_TRACE_PRINT 1/QEM_TRACE_PRINT 0/g' ../../QEMTrace/qem_trace_config.h

# Compile qemu
mkdir -p ../../Qemu/build
cd ../../Qemu/build
if [ $env_os = 2 ]
then
    ../configure --disable-kvm --target-list=ppc-softmmu --python=/mingw64/bin/python3 --disable-capstone
else
    ../configure --disable-kvm --target-list=ppc-softmmu
fi
make

# Launch test
cd ../../Tests/PPC
#} &> /dev/null

./test_suite_ppc_auto_bin.sh
error=$?

echo "Compiling..."
# {
# Modify output to printf
sed -i 's/QEM_TRACE_PRINT 0/QEM_TRACE_PRINT 1/g' ../../QEMTrace/qem_trace_config.h

# Compile qemu
cd ../../Qemu/build
make

# Launch test
cd ../../Tests/PPC
#} &> /dev/null

./test_suite_ppc_auto_str.sh
error=$(($? + $error))

echo ""
echo -e "\e[94m==================================== RESULT ====================================\e[39m"
if (( $error != 0 ))
then
    echo -e "\e[31mPPC Test suite FAILED: $error errors\e[39m"
else
    echo -e "\e[92mPPC Test suite PASSED: $error errors\e[39m"
fi

exit $error
