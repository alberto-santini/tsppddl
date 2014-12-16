#!/bin/bash

data_dir="../data/new/"
stdout_dir="../output/stdout/"
stderr_dir="../output/stderr/"
exec_file="../src/tsppddl"
params_file="../program_params.json"

if [ -z $1 ]
then
    echo "I need the instances type as a parameter"
    exit
fi

for file in $(printf "%s%s%s" $data_dir $1 "/*")
do
    filename=$(basename "$file")
    extension="${filename##*.}"
    instance="${filename%.*}"
    
    if [ "$extension" != "json" ]
    then
        echo "Instance file is not json!"
        exit
    fi
    
    mkdir -p "$stdout_dir"
    mkdir -p "$stderr_dir"
    
    stdout_file=$(printf "%s%s%s" $stdout_dir $instance ".stdout")
    stderr_file=$(printf "%s%s%s" $stderr_dir $instance ".stderr")
    
    echo "oarsub -n \"$instance\" -O \"$stdout_file\" -E \"$stderr_file\" -p \"network_address!='drbl10-201-201-21'\" -l /nodes=1/core=1,walltime=4 \"$exec_file $file $params_file branch_and_cut\""
done