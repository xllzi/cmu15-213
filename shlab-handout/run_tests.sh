#!/bin/bash
for i in $(seq -w 1 10); do
    echo "=== test${i} ==="
    ./sdriver.pl -t "trace${i}.txt" -s ./tsh -a "-p" > /tmp/myout.txt
    ./sdriver.pl -t "trace${i}.txt" -s ./tshref -a "-p" > /tmp/refout.txt
    sed -E 's/\([0-9]+\)/(PID)/g' /tmp/myout.txt > /tmp/myclean.txt
    sed -E 's/\([0-9]+\)/(PID)/g' /tmp/refout.txt > /tmp/refclean.txt
    if ! diff /tmp/myclean.txt /tmp/refclean.txt; then
        echo "test${i} FAILED"
        exit 1
    fi
    echo "test${i} PASSED"
done
echo "All 16 tests passed."
