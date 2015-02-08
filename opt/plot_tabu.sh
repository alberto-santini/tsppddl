#!/bin/bash

data_dir="../results/tabu_search_progress"
output_dir="../graphs"
gnuplot_file="/tmp/gnuplot_script.p"
i=1

if [[ -z "$1" ]]
then
    echo "I need the instance name as a parameter"
    exit
else
  instance_name=$1
fi

echo -n > $gnuplot_file
echo "set terminal png" >> $gnuplot_file
echo "set output \"${output_dir}/${instance_name}.png\"" >> $gnuplot_file
echo -n "plot " >> $gnuplot_file

for data_file in ${data_dir}/${instance_name}*.dat
do
  echo -n "\"${data_file}\" using 1:2 title \"Heur $i\" with lines, " >> $gnuplot_file
  i=$((i+1))
done

echo >> $gnuplot_file

cat $gnuplot_file

gnuplot $gnuplot_file