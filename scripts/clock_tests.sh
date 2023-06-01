#!/bin/bash

# this script runs `make test` $1 times and reports the average time
# it takes to run the tests, in milliseconds


TEST_CMD ?= "make test"

# run the tests $1 times
AVG=0
ITERS=$1
for i in $(seq 1 $ITERS);
do
    echo "Running test $i of $ITERS"
    # get milliseconds since epoch
    START=$(date +%s%3N)
    $TEST_CMD
    END=$(date +%s%3N)
    # get the difference in milliseconds
    DIFF=$(( $END - $START ))
    AVG=$(( $AVG + $DIFF ))
done

# divide by the number of iterations to get the average
AVG=$(( $AVG / $ITERS ))
echo "Average time to run tests: $AVG milliseconds"
