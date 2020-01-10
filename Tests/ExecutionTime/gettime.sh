#!/bin/bash

rm times.out
for i in {0..20} 
do
    (make run | grep Elapsed >> times.out)& 
    sleep 5
    killall qemu-system-ppc
done