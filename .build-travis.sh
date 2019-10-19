#!/bin/bash

cd Tests/PPC
./test_suite_ppc_auto_smi.sh 
exit 0

error=0
cd Tests
for d in */ ; do
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
done
cd ..
exit $error
