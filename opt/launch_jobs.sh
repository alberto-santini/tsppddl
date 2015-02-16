#!/bin/bash

# Example usage:
# ./launch_jobs.sh tsppd
# params_file=../params/go_only.json ./launch_jobs.sh tsppddl

data_dir="../data/new/"
stdout_dir="../output/stdout/"
stderr_dir="../output/stderr/"
exec_file="../build/tsppddl"

if [[ -z "$params_file" ]]
then
    echo "Using default params file"
    params_file="../params/default.json"
else
    echo "Using params file $params_file"
fi

excluded_instances=()

contains_element () {
    for element in "${@:2}"
    do
      	[[ "$element" == *"$1"* ]] && return 0
    done

    return 1
}

check_the_user_didnt_forget_excluded_instances() {
    if [[ ${#excluded_instances[@]} -eq 0 ]]
    then
        echo "No excluded instances, proceeding..."
    else
        read -p "Warning: I have a list of excluded instances: proceed? y/n " -n 1 -r
        echo

        if [[ "$REPLY" =~ ^[Yy]$ ]]
        then
            echo "Ok, proceeding..."
        else
            echo "Ok, aborting..."
            exit
        fi
    fi
}

create_output_dirs() {
    mkdir -p "$stdout_dir"
    mkdir -p "$stderr_dir"
}

check_command_line_options() {
    if [[ -z "$1" ]]
    then
        echo "I need the instances type as a parameter"
        exit
    fi
}

schedule_jobs() {
    for file in $(printf "%s%s%s" "$data_dir" "$1" "/*")
    do
        filename=$(basename "$file")
        extension="${filename##*.}"
        instance="${filename%.*}"

        if [[ "$extension" != "json" ]]
        then
            echo "Instance file is not json!"
            exit
        fi

        if contains_element "$filename" "${excluded_instances[@]}"
        then
           	echo "Skipping $filename"
        else
            stdout_file=$(printf "%s%s%s" "$stdout_dir" "$instance" ".stdout")
            stderr_file=$(printf "%s%s%s" "$stderr_dir" "$instance" ".stderr")

            oarsub -n "$1 $instance" -O "$stdout_file" -E "$stderr_file" -p "network_address!='drbl10-201-201-21'" -l /nodes=1/core=1,walltime=24 "$exec_file $file $params_file branch_and_cut"
        fi
    done
}

check_the_user_didnt_forget_excluded_instances
check_command_line_options "$*"
create_output_dirs
schedule_jobs "$*"