#!/bin/bash

error=0
arm-none-eabi-gcc --version
cd Tests
for d in */ ; do
    echo -e "\e[92m In $d\e[39m"
    cd $d
    chmod +x *.sh
    ./test_suite.sh
    val=$?
    if (( $val != 0 ))
    then
        echo -e "\e[31mERROR \e[39m"
        error=$(($error0 + 1))
    else
        echo -e "\e[92mPASSED\e[39m"
    fi
    cd ..
done
cd ..
exit $error
