![Stranded ship](stranded-ship.jpg)
<sub><sub>They did not check the draft very thoroughly... Picture by Mike McBay licensed under the Creative Commons Attribution 2.0 Generic (CC BY 2.0) license</sub></sub>

# TSPPDDL

This is a solver for a generalisation of the Traveling Salesman Problem, namely the **Traveling Salesman Problem with Pickups, Deliveries and Draft Limits**.

It's a problem that stems from maritime applications, where the draught of a ship is a function of the quantity of cargo it currently has on board. Because of its current draught, a ship might or might not be able to visit a certain port.

This software includes:
* An exact branch-and-cut algorithm
* Constructive heuristics
* K-opt recombination heuristics
* Tabu search meta-heuristics (neighbourhood defined by 3-opt moves)
* An implementation of the subgradient method with lagrangean relaxation

It also contains many test instances derived from the TSPLIB and a script to generate new ones. More info is contained in the other `README.md` files present in the subfolders.

## Citation

If you use this software, please cite the following paper:

```bib
@article{malaguti2018travelling,
  title={The {Traveling Salesman Problem} with Pickups, Deliveries, and Draft Limits},
  author={Malaguti, Enrico and Martello, Silvano and Santini, Alberto},
  journal={{Omega}},
  volume=74,
  pages={50--58},
  year=2018,
  doi={10.1016/j.omega.2017.01.005}
}
```

You can also cite this repository via Zenodo.

[![DOI](https://zenodo.org/badge/20017475.svg)](https://zenodo.org/badge/latestdoi/20017475)

```bib
@misc{tsppddl_github,
    title={The Traveling Salesman Problem with Pickups, Deliveries and Draft Limits},
    author={Santini, Alberto},
    date={2022-12-30},
    howpublished={Github repository},
    doi={10.5281/zenodo.7494135},
    url={https://github.com/alberto-santini/tsppddl/}
}
```

## License

This software is distributed under the GNU General Public License v3, as detailed in `LICENSE.txt`.
