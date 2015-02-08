TSPPDDL
============

Notice
------

This repository has been moved to the [Operational Research Group at the University of Bologna](https://github.com/OR-Bologna/tsppddl).

Intro
-----

This is a solver for a generalisation of the Travelling Salesman Problem, namely the TSP with Pickups, Deliveries and Draught Limits.

It's a problem that stems from maritime applications, where the draught of a ship is a function of the quantity of cargo it currently has on board. Because of its current draught, a ship might or might not be able to visit a certain port.

This software includes:

* An exact branch-and-cut algorithm
* Constructive heuristics
* K-opt recombination heuristics
* Tabu search meta-heuristics (neighbourhood defined by 3-opt moves)
* An implementation of the subgradient method with lagrangean relaxation

It also contains many test instances derived from the TSPLIB and a script to generate new ones. More info is contained in the other `README.md` files present in the subfolders.