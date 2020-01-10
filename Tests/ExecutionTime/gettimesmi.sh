#!/bin/bash

rm times.out
for i in {0..20} 
do
    make run&
    #(make run | grep Elapsed >> times.out)& 
    sleep 1
    

    # Run SMI client
    ./smi_client

    sleep 5


    killall qemu-system-ppc
    killall smi_client

done