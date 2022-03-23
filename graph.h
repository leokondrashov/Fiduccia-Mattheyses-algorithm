#ifndef GRAPH_H
#define GRAPH_H

#include <algorithm>
#include <list>
#include <iostream>
#include <vector>

class Graph {
public:
    Graph(const char *file);
    void dump(std::ostream& out = std::cout) const;
    void dump(const char* file) const;
    void print_partitionment(std::ostream& out) const;
    void print_partitionment(const char* file) const;

    const auto& get_cells() const { return cells; }
    const auto& ith_cell_nets(unsigned i) const { return cells[i]; }
    unsigned get_cell_count() const { return cells.size(); }

    const auto& get_nets() const { return nets; }
    const auto& ith_net_cells(unsigned i) const { return nets[i]; }
    unsigned get_net_count() const { return nets.size(); }
    
    const auto& get_partitionment() const { return partitionment; }
    auto get_ith_cell_partition(unsigned i) const { return partitionment[i]; }
    void set_partitionment(const std::vector<bool>& new_partitionment);
    void set_partitionment(std::vector<bool>&& new_partitionment);
    unsigned get_partitionment_cost() const;
    int get_disbalance() const { return disbalance; };

    void move_cell(unsigned i);
    void update_disbalance();

    int get_net_cells_partition(unsigned net, bool partition) const;
    bool is_net_cut(unsigned net) const;

    unsigned get_max_degree() const;

private:
    std::vector<std::list<unsigned>> cells;
    std::vector<std::list<unsigned>> nets;
    std::vector<bool> partitionment;
    int disbalance;
};

#endif // GRAPH_H
