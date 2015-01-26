`opt/*`
---------

This folder contains misc things related to the project:

* `data` contains the original TSPLIB instances used by *Battarra et Al.*
* `data_translator.rb` is the script that creates our own data, starting from those in `data`. Despite its name, it doesn't just translate from their ugly format into shiny JSON, but it also transform their instances into instances suitable for our problem. Some of the data is generated randomly, so one can obtain brand new data at every run of the script.
* `graphs_cmd.txt` is an example of shell command that can be used to transform graphs from the `.dot` format to `.png`.
* `launch_jobs.sh` is script to launch the jobs on the cluster.
