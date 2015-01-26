`src/*`
---------

Here goes the source code.

* `heuristics` contains the base machinery that makes the constructive and k-opt heuristics work.
* `network` contains the building blocks of everything (nodes, arcs, graphs, paths) plus the part that writes out the `.dot` representation of the graph.
* `parser` contains the code that parses both the instances and the program params.
* `program` contains the code responsible to invoke the appropriate subsystem to be launched and the data that gets carried across various parts of the programme, in order to collect statistics (running times, etc.).
* `solver` contains the various solvers: branch-and-cut (`bc`), heuristics (`heuristics`), metaheuristics (`metaheuristics`), subgradient method (`subgradient`).
* `main.cpp` sets the random seed and launches the programme.