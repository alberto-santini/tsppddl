#!/bin/bash

# Call it with:
# 1st parameter = instance name
# 2nd parameter = something or 'b'; if 'b' it only prints the graph of best and worst heuristics

results_file="../results/heur/results.txt"
data_dir="../results/heur_solutions"
output_dir="../graphs"
gnuplot_file="/tmp/load_gnuplot_script.p"

if [[ -z "$1" ]]
then
    echo "I need the instance name as a parameter"
    exit
else
  instance_name=$1
fi

if [[ -n "$2" ]]
then
  if [[ "$2" = "b" ]]
  then
    echo "Only printing best and worst series"
    reduce_series=true
  fi
else
  reduce_series=false
fi

data_file="${data_dir}/${instance_name}.txt"
data_file_basename=`basename $data_file`
series_number=`cat ${data_file} | head -n 1 | wc -w | cut -d ' ' -f 8`

echo "Found ${series_number} valid solutions in ${data_file_basename}"

series=($(seq -s ' ' 1 $series_number))
all_series=($(seq -s ' ' 1 $series_number))

if [[ $reduce_series = "true" ]]
then
  min_index_and_cost=(`cat ${results_file} | egrep "^${instance_name}" | tail -n 1 | tr '\t' '\n' | tail -n +2 | sed '/^$/d' | cat -n | sort -n -k 2,2 | head -n 1`)
  min_index=${min_index_and_cost[0]}
  min_cost=${min_index_and_cost[1]}
  
  echo "Min index: ${min_index}, min cost: ${min_cost}"
  
  max_index_and_cost=(`cat ${results_file} | egrep "^${instance_name}" | tail -n 1 | tr '\t' '\n' | tail -n +2 | sed '/^$/d' | cat -n | sort -n -r -k 2,2 | head -n 1`)
  max_index=${max_index_and_cost[0]}
  max_cost=${max_index_and_cost[1]}
  
  echo "Max index: ${max_index}, max cost: ${max_cost}"
  
  series=($min_index $max_index)
fi

echo -n > $gnuplot_file
echo "set terminal png size 2048" >> $gnuplot_file
echo "set output \"${output_dir}/load_${instance_name}.png\"" >> $gnuplot_file
echo -n "plot " >> $gnuplot_file

for i in ${series[@]}
do
  cost=`cat ${results_file} | egrep "^${instance_name}" | tail -n 1 | awk -v f="$((i+1))" {'print \$f'}`
  echo -n "\"${data_file}\" using ${i} title \"Cost ${cost}\" with lines, " >> $gnuplot_file
done

echo >> $gnuplot_file

cat $gnuplot_file

gnuplot $gnuplot_file