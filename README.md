# Fiduccia-Mattheyses algorithm
Fiduccia-Mattheyses algorithm for hypergraph partitioning.

Task for VLSI CAD cource at MIPT.

## Algorithm overview
The task is partition hypergraph: spread vertices into disjoint sets (partitions) such that amount of hyperegdes which spans over more than one
partitions is minimized (cut size).

Finding optimal solution is NP-full task.

This code implements [Fiduccia-Mattheyses algorithm](https://en.wikipedia.org/wiki/Fiduccia%E2%80%93Mattheyses_algorithm) - heuristic for finding 
solution for hypergraph partitioning problem.

## Building and running
Built with `make` command for linux.

Running program:
```
./FMpart FILE [--dump DUMP_FILE]
```

Input file is in [hMetis](http://glaros.dtc.umn.edu/gkhome/fetch/sw/hmetis/manual.pdf) format of __unweighted__ hypergraph. Same stands for output file: 
name is the name of input file appended with `.part.2` and contains partition key for every vertex.

Dump file can be used for representation of partitionment in `.dot` format (not recommended for large graphs).
