#!/bin/bash

for i in 10 25 50
do
    for j in {1..10}
    do
        for k in 100 200 300
        do
            ./tsppddl data/bayg29_$((i))_$((j)).json $((k))
        done
    done
done