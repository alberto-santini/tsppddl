#!/bin/bash

for i in 10 25 50
do
    for j in {1..10}
    do
        ./tsppddl "data/bayg29_$((i))_$((j)).json"
    done
done