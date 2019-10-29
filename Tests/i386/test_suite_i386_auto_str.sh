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
echo -e "\e[94m============================ i386 TEST AUTO: STRING ============================\e[39m"
echo ""
echo -e "\e[94m----------------------------- Paging Enabled Tests -----------------------------\e[39m"
echo ""
test="_test.valid"

error0=0

# For paging enabled tests, concatenate all test files
for entry in "./test_pagen"/*.asm
do
    echo -n "Test: $entry "

    {

    # Remove the global test file and create a new one
    rm -f mini_kernel/test_pagdis_glob.asm mini_kernel/test_pagen_glob.asm
    touch mini_kernel/test_pagdis_glob.asm
    touch mini_kernel/test_pagen_glob.asm

    cat "$entry" >> mini_kernel/test_pagen_glob.asm

    cd mini_kernel
    sync

    # Make mini kernel
    make && (make run > ../diff_file &)

    sleep 2
    pid=$(pidof qemu-system-i386)

 
    kill -KILL $pid
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
        grep -E "^(D|I) (LD|ST)" diff_file >> filtered_file
    else
        echo -e "\e[31mERROR WRONG FORMAT |$line1| \e[39m"
        exit -1
    fi
    sync
    cut -d"|" -f1,2,3,4,6,7,8,9,10,11,12,13,14,15,16 "$entry$test" > newfile
    cut -d"|" -f1,2,3,4,6,7,8,9,10,11,12,13,14,15,16 filtered_file > filtered_file_new
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
    else
        echo -e "\e[92mPASSED\e[39m"
    fi

    rm filtered_file newfile diff_file
done
echo ""
if (( $error0 != 0 ))
then
    echo -e "\e[31m------------------------- Paging Enabled Tests: FAILED -------------------------\e[39m"
else
    echo -e "\e[92m------------------------- Paging Enabled Tests: PASSED -------------------------\e[39m"
fi

echo ""
echo -e "\e[94m---------------------------- Paging Disabled Tests -----------------------------\e[39m"
echo ""
error1=0

# For paging enabled tests, concatenate all test files
for entry in "./test_pagdis"/*.asm
do
    echo -n "Test: $entry "

    {

    # Remove the global test file and create a new one
    rm -f mini_kernel/test_pagdis_glob.asm mini_kernel/test_pagen_glob.asm
    touch mini_kernel/test_pagdis_glob.asm
    touch mini_kernel/test_pagen_glob.asm

    sync

    cat "$entry" >> mini_kernel/test_pagdis_glob.asm

    cd mini_kernel

    sync
    # Make mini kernel
    make && (make run > ../diff_file &)

    sleep 2

    pid=$(pidof qemu-system-i386)
 
    kill -KILL $pid

    sync
    if [ $env_os = 2 ]
    then
        ps -W | awk '/qemu-system-i386.exe/,NF=1' | xargs kill -f
    else
        killall qemu-system-i386
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
        grep -E "^(D|I) (LD|ST)" diff_file >> filtered_file
    else
        echo -e "\e[31mERROR WRONG FORMAT \e[39m"
        exit -1
    fi
    sync

    cut -d"|" -f1,2,3,4,6,7,8,9,10,11,12,13,14,15,16 "$entry$test" > newfile
    cut -d"|" -f1,2,3,4,6,7,8,9,10,11,12,13,14,15,16 filtered_file > filtered_file_new
    sync
    sed -i 's/\s*$//' filtered_file_new
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

    rm filtered_file newfile diff_file
done
echo ""
if (( $error1 != 0 ))
then
    echo -e "\e[31m------------------------- Paging Disabled Tests FAILED -------------------------\e[39m"
else
    echo -e "\e[92m------------------------- Paging Disabled Tests PASSED -------------------------\e[39m"
fi

echo ""

if (( $error0 != 0  ||  $error1 != 0 ))
then
    echo -e "\e[31m------------------------- i386 TEST AUTO STRING FAILED -------------------------\e[39m"

else
    echo -e "\e[92m------------------------- i386 TEST AUTO STRING PASSED -------------------------\e[39m"
fi
error=$(($error0 + $error1))
exit $error
