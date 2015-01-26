`graphs/*`
------------

The graphs created by the callback `./src/solver/bc/callbacks/print_relaxation_graph_callback.cpp` are saved here. This callback prints out a graph of the solution given by the linear relaxation of the problem at the root node. For each problem, it actually prints many graphs, as every time a constraint is added at the root node a new graph is printed, so that we can evaluate the impact of these constraints.

The graphs are given in `.dot` format (have a look at `src/network/graph_writer.cpp`) and can be converted into `.png` by using `dot`. An example of how to do this is given in `opt/graphs_cmd.txt`.