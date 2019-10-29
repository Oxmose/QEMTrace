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

echo -e "\e[94m============================= I386 TEST AUTO: FULL =============================\e[39m"

echo "Compiling..."
{
# Modify output to binary
sed -i 's/QEM_TRACE_TYPE QEM_TRACE_PRINT/QEM_TRACE_TYPE QEM_TRACE_FILE/g' ../../QEMTrace/qem_trace_config.h
sed -i 's/QEM_TRACE_TYPE QEM_TRACE_SMI/QEM_TRACE_TYPE QEM_TRACE_FILE/g' ../../QEMTrace/qem_trace_config.h

# Compile qemu
mkdir -p ../../Qemu/build
cd ../../Qemu/build
if [ $env_os = 2 ]
then
    ../configure --disable-kvm --target-list=i386-softmmu --python=/mingw64/bin/python3 --disable-capstone
else
    ../configure --disable-kvm --target-list=i386-softmmu
fi
make

# Launch test
cd ../../Tests/i386
} &> /dev/null

./test_suite_i386_auto_bin.sh
error=$?

echo "Compiling..."
{
# Modify output to printf
sed -i 's/QEM_TRACE_TYPE QEM_TRACE_FILE/QEM_TRACE_TYPE QEM_TRACE_PRINT/g' ../../QEMTrace/qem_trace_config.h

# Compile qemu
cd ../../Qemu/build
make

# Launch test
cd ../../Tests/i386
} &> /dev/null

./test_suite_i386_auto_str.sh
error=$(($? + $error))

echo "Compiling..."
{
# Modify output to printf
sed -i 's/QEM_TRACE_TYPE QEM_TRACE_PRINT/QEM_TRACE_TYPE QEM_TRACE_SMI/g' ../../QEMTrace/qem_trace_config.h

# Compile qemu
cd ../../Qemu/build
make

# Launch test
cd ../../Tests/i386
} &> /dev/null

./test_suite_i386_auto_smi.sh
error=$(($? + $error))


echo ""
echo -e "\e[94m==================================== RESULT ====================================\e[39m"
if (( $error != 0 ))
then
    echo -e "\e[31mi386 Test suite FAILED: $error errors\e[39m"
else
    echo -e "\e[92mi386 Test suite PASSED: $error errors\e[39m"
fi

exit $error
