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


echo ""
echo -e "\e[94m============================ ARM TEST AUTO: SMI ============================\e[39m"
echo ""

test="_test.valid"

# Compile converter tool
g++ -mno-ms-bitfields ../bin_to_str.cpp -o ../bin_to_str.bin


echo ""
echo -e "Compiling SMI client"
echo ""

# Compile SMI client
cd ../../SMILib
make
cd ../Tests/ARM
gcc -std=c99 -O2 test_smi_client.c -L../../SMILib/lib -lsmi -lpthread -lrt -o smi_client

if (( $? != 0 ))
then
    echo -e "\e[31mERROR Could not compile SMI client\e[39m"
    exit 1
fi

echo -e "\e[94m----------------------------- Paging Disabled Tests -----------------------------\e[39m"
echo ""

error0=0

# For paging disabled enabled tests, concatenate all test files
for entry in "./test_pagdis"/*.S
do
    echo -n "Test: $entry "

    {

    # Remove the global test file and create a new one
    rm -f mini_kernel/test_pagdis_glob.S.inc mini_kernel/test_pagen_glob.S.inc
    touch mini_kernel/test_pagdis_glob.S.inc
    touch mini_kernel/test_pagen_glob.S.inc

    cat "$entry" >> mini_kernel/test_pagdis_glob.S.inc

    cd mini_kernel
    sync

    # Make mini kernel
    make && (make run &)
    sleep 1;

    # Run SMI client
    ../smi_client > ../diff_file&

    sleep 1;

    pid=$(pidof qemu-system-arm)

 
    kill -KILL $pid
    sync
    if [ $env_os = 2 ]
    then
        ps -W | awk '/smi_client.exe/,NF=1' | xargs kill -f
    else
        killall smi_client
    fi
    sync

    cd ..

    line1=$(cat "$entry$test" | head -n 1)

    if [ "$line1" = "DATA" ]
    then
        echo "DATA" > filtered_file
        grep -E "^D (LD|ST)" diff_file >> filtered_file
    elif [ "$line1" = "INST" ]
    then
        echo "INST" > filtered_file
        grep -E "^I (LD|ST)" diff_file >> filtered_file
    elif [ "$line1" = "ALL" ]
    then
        echo "ALL" > filtered_file
        grep -E "^(D|I|INV|FL/INV|FL|U|L|P) (LD|ST)" diff_file >> filtered_file
    else
        echo -e "\e[31mERROR WRONG FORMAT |$line1| \e[39m"
        exit -1
    fi

    sync
    cut -d"|" -f1,3,4,6,7,8,9,10,11,12,13,14,15,16 "$entry$test" > newfile
    cut -d"|" -f1,2,3,5,6,7,8,9,10,11,12,13,14,15 filtered_file > filtered_file_new
    sync
    sed -i 's/\s*$//' filtered_file_new
    sync
    mv filtered_file_new filtered_file

    sync
    diff newfile filtered_file >> /dev/null
    val=$?

    } &> /dev/null

    if (( $val != 0 ))
    then
        echo -e "\e[31mERROR \e[39m"
        error0=$(($error0 + 1))
        exit -1
    else
        echo -e "\e[92mPASSED\e[39m"
    fi

    rm filtered_file newfile
done
echo ""
if (( $error0 != 0 ))
then
    echo -e "\e[31m------------------------- Paging Disabled Tests: FAILED -------------------------\e[39m"
else
    echo -e "\e[92m------------------------- Paging Disabled Tests: PASSED -------------------------\e[39m"
fi

echo ""
echo -e "\e[94m---------------------------- Paging Enabled Tests -----------------------------\e[39m"
echo ""
error1=0

# For paging enabled tests, concatenate all test files
for entry in "./test_pagen"/*.S
do
    echo -n "Test: $entry "

    {

    # Remove the global test file and create a new one
    rm -f mini_kernel/test_pagdis_glob.S.inc mini_kernel/test_pagen_glob.S.inc
    touch mini_kernel/test_pagdis_glob.S.inc
    touch mini_kernel/test_pagen_glob.S.inc

    sync

    cat "$entry" >> mini_kernel/test_pagen_glob.S.inc

    cd mini_kernel

    sync
    # Make mini kernel
    make && (make run &)
    sleep 1;

    # Run SMI client
    ../smi_client > ../diff_file&

    sleep 1;

    pid=$(pidof qemu-system-arm)

 
    kill -KILL $pid
    sync
    if [ $env_os = 2 ]
    then
        ps -W | awk '/smi_client.exe/,NF=1' | xargs kill -f
    else
        killall smi_client
    fi
    sync

    cd ..

    line1=$(cat "$entry$test" | head -n 1)

    if [ "$line1" = "DATA" ]
    then
        echo "DATA" > filtered_file
        grep -E "^D (LD|ST)" diff_file >> filtered_file
    elif [ "$line1" = "INST" ]
    then
        echo "INST" > filtered_file
        grep -E "^I (LD|ST)" diff_file >> filtered_file
    elif [ "$line1" = "ALL" ]
    then
        echo "ALL" > filtered_file
        grep -E "^(D|I|INV|FL/INV|FL|U|L|P) (LD|ST)" diff_file >> filtered_file
    else
        echo -e "\e[31mERROR WRONG FORMAT |$line1| \e[39m"
        exit -1
    fi

    sync
    cut -d"|" -f1,3,4,6,7,8,9,10,11,12,13,14,15,16 "$entry$test" > newfile
    cut -d"|" -f1,2,3,5,6,7,8,9,10,11,12,13,14,15 filtered_file > filtered_file_new
    sync
    sed -i 's/\s*$//' filtered_file_new
    sync
    mv filtered_file_new filtered_file

    sync
    diff newfile filtered_file >> /dev/null
    val=$?

    } &> /dev/null

    if (( $val != 0 ))
    then
        echo -e "\e[31mERROR \e[39m"
        error1=$(($error1 + 1))
        exit -1
    else
        echo -e "\e[92mPASSED \e[39m"
    fi

    rm filtered_file newfile
done
echo ""
if (( $error1 != 0 ))
then
    echo -e "\e[31m------------------------- Paging Enabled Tests FAILED -------------------------\e[39m"
else
    echo -e "\e[92m------------------------- Paging Enabled Tests PASSED -------------------------\e[39m"
fi

echo ""

if (( $error0 != 0  ||  $error1 != 0 ))
then
    echo -e "\e[31m------------------------- ARM TEST AUTO SMI FAILED -------------------------\e[39m"

else
    echo -e "\e[92m------------------------- ARM TEST AUTO SMI PASSED -------------------------\e[39m"
fi
error=$(($error0 + $error1))
exit $error
