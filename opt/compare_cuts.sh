#!/bin/bash

if [[ -z "$1" ]]
then
    echo "I need the cuts file name as a parameter"
    exit
else
  cuts_file=$1
fi

if [[ -z "$2" ]]
then
    echo "I need the x variables file name as a parameter"
    exit
else
  x_vars_file=$2
fi

reading_vars="true"
ruby_script="/tmp/script.rb"

while read l
do    
    if [[ $reading_vars = "true" ]]
    then
        # echo "Copying the integer solution variables into the script"
        
        cat "$x_vars_file" > $ruby_script
        reading_vars="false"
    else
        # echo "Copying the cut into the script"
        
        {
            echo "if $l"
            echo "  exit 0"
            echo "else"
            echo "  puts \"This cut was added but a feasible integer solution violates it\""
            echo "  exit 1"
            echo "end"
        } >> $ruby_script
        
        # echo "Executing the script"
        
        ruby $ruby_script
        
        if [[ "$?" = "1" ]]
        then
            echo "Wrong cut found:"
            echo "$l"
        fi
        
        reading_vars="true"
    fi
done < "$cuts_file"