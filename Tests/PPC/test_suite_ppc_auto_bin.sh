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
echo -e "\e[94m============================ PPC  TEST AUTO: BINARY ============================\e[39m"
echo ""
test="_test.valid"

error0=0

# Compile converter tool
g++ -mno-ms-bitfields ../bin_to_str.cpp -o ../bin_to_str.bin

# For paging enabled tests, concatenate all test files
for entry in "./tests"/*.S
do
    echo -n "Test: $entry "

    {

        rm -f ./mini_kernel/tests.S
        echo "#include \"ppc-asm.h\"" > ./mini_kernel/tests.S

        echo ".file	\"tests.S\"" >> ./mini_kernel/tests.S
        echo ".text" >> ./mini_kernel/tests.S

        echo "  /* Define kernel entry point */" >> ./mini_kernel/tests.S
        echo ".globl	test_entry" >> ./mini_kernel/tests.S
        echo ".type	test_entry,@function" >> ./mini_kernel/tests.S

        echo "test_entry:" >> ./mini_kernel/tests.S
        echo "/* PR */" >> ./mini_kernel/tests.S
        echo "mflr r0" >> ./mini_kernel/tests.S
        echo "stw r0, 8(sp)" >> ./mini_kernel/tests.S
        echo "stwu sp, -16(sp)" >> ./mini_kernel/tests.S
        echo "/* Enable tracing: 0xFFFFFFF0 */" >> ./mini_kernel/tests.S
        echo ".long 0xFFFFFFF0" >> ./mini_kernel/tests.S
        echo "/* Tests will be cat here */" >> ./mini_kernel/tests.S

        cat "$entry" >> ./mini_kernel/tests.S

        echo "/* EP */" >> ./mini_kernel/tests.S
        echo "/* Disable tracing: 0xFFFFFFF1 */" >> ./mini_kernel/tests.S
        echo ".long 0xFFFFFFF1" >> ./mini_kernel/tests.S
        echo "addi sp, sp, 16" >> ./mini_kernel/tests.S
        echo "lwz r0, 8(sp)" >> ./mini_kernel/tests.S
        echo "mtlr r0" >> ./mini_kernel/tests.S
        echo "blr" >> ./mini_kernel/tests.S

        sync
        cd mini_kernel

        rm *.out

        # Make mini kernel
        make && (make run &)

        sleep 2;
        if [ $env_os = 2 ]
        then
            ps -W | awk '/qemu-system-ppc.exe/,NF=1' | xargs kill -f
        else
            killall qemu-system-ppc
        fi
        sync

        rm ../*.out

        for ff in *.out
        do
            mv $ff ../$ff
        done

        sync

        cd ..

        line1=$(cat "$entry$test" | head -n 1)

        if [ "$line1" = "DATA" ]
        then
            echo "DATA" > filtered_file
        elif [ "$line1" = "INST" ]
        then
            echo "INST" > filtered_file
        elif [ "$line1" = "ALL" ]
        then
            echo "ALL" > filtered_file
        else
            echo -e "\e[31mERROR WRONG FORMAT \e[39m"
            exit -1
        fi


        for ff in *.out
        do
            ../bin_to_str.bin $ff 32 >> filtered_file
        done

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

        rm filtered_file newfile *.out

    } &> /dev/null

    if (( $val != 0 ))
    then
        echo -e "\e[31mERROR \e[39m"
        error0=$(($error0 + 1))
    else
        echo -e "\e[92mPASSED\e[39m"
    fi
done
echo ""
if (( $error0 != 0 ))
then
    echo -e "\e[31m------------------------- PPC  TEST AUTO BINARY FAILED -------------------------\e[39m"

else
    echo -e "\e[92m------------------------- PPC  TEST AUTO BINARY PASSED -------------------------\e[39m"
fi
exit $error0
