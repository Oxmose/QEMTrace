#! /bin/bash
echo "Communicator tests"

echo "Test nominal features"
make commNominalTest
if [[ $? != 0 ]]; then
    echo "Test nominal features"
    exit -1
fi
echo "Test null buffer use"
make commNullBufferTest
if [[ $? != 0 ]]; then
    echo "Test null buffer use"
    exit -1
fi
echo "Test unconnected comm use"
make commUnconnectExcTest
if [[ $? != 0 ]]; then
    echo "Test unconnected comm use"
    exit -1
fi
echo "Test uninit comm use"
make commUninitExcText
if [[ $? != 0 ]]; then
    echo "Test uninit comm use"
    exit -1
fi
echo "Test wrong direction use"
make commWrongDirectionExcTest
if [[ $? != 0 ]]; then
    echo "Test wrong direction use"
    exit -1
fi
echo "Test timeout"
make commTimeoutTest
if [[ $? != 0 ]]; then
    echo "Test timeout"
    exit -1
fi
