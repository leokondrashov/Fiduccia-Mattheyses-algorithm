#include <assert.h>
#include <fstream>
#include <list>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>

#include "graph.h"

Graph::Graph(const char *file) {
    std::ifstream in(file);

    int cell_num = 0, net_num = 0, fmt = 0;

    std::string line;
    std::getline(in, line);
    std::istringstream str_stream(line);
    str_stream >> net_num >> cell_num >> fmt;
    assert(fmt == 0); // weighted nets and cells are not supported

    cells.resize(cell_num);
    partitionment.resize(cell_num);
    nets.resize(net_num);
    disbalance = cell_num;

    int net_idx = 0;
    while (getline(in, line)) {
        if (line[0] == '%') // comment line in hgr file
            continue;

        int cell = 0;
        str_stream.str(line);
        str_stream.clear();
        while (str_stream >> cell) {
            --cell; // internally, cells numbered from 0
            nets[net_idx].push_back(cell);
            cells[cell].push_back(net_idx);
        }
        ++net_idx;
    }

    in.close();
}

void Graph::dump(const char* file) const {
    std::ofstream out(file);
    dump(out);
    out.close();
}

// dumps hypergraph as digraph:
// cells and nets are nodes and incidency relation is edges
// direction of edges purely for graphical structure:
// column of nets internal to partition 0 ->
// column of cells of partition 0 ->
// column nets that are to be cut ->
// column of cells of partition 1 ->
// column of nets internal to partition 1
void Graph::dump(std::ostream& out) const {
    out << "digraph hyper {\n";
    out << "rankdir = LR;\n";
    out << "ranksep = 3;\n";

    // nodes
    // internal nets to partition 0
    out << "{ rank = min; \n";
    for (unsigned i = 0; i < nets.size(); ++i) {
        if (get_net_cells_partition(i, 1) == 0) {
            out << "\tn" << i << " [shape=box label=\"net " << i << "\"];\n";
        }
    }
    out << "}\n";

    // cells of partition 0
    out << "{ rank = same; \n";
    for (unsigned i = 0; i < cells.size(); ++i) {
        if (!partitionment[i]) {
            out << "\tc" << i << " [label=\"cell " << i << "\"];\n";
        }
    }
    out << "}\n";
    
    // cut nets
    out << "{ rank = same; \n";
    for (unsigned i = 0; i < nets.size(); ++i) {
        if (is_net_cut(i)) {
            out << "\tn" << i << " [shape=box label=\"net " << i << "\" color=blue];\n";
        }
    }
    out << "}\n";

    // cells of partition 1
    out << "{ rank = same; \n";
    for (unsigned i = 0; i < cells.size(); ++i) {
        if (partitionment[i]) {
            out << "\tc" << i << " [label=\"cell " << i << "\" color=red];\n";
        }
    }
    out << "}\n";

    // internal nets to partition 1
    out << "{ rank = max; \n";
    for (unsigned i = 0; i < nets.size(); ++i) {
        if (get_net_cells_partition(i, 0) == 0)
            out << "\tn" << i << " [shape=box label=\"net " << i << "\"];\n";
    }
    out << "}\n";

    // nets internal to partition 0
    for (unsigned i = 0; i < nets.size(); ++i)
        if (get_net_cells_partition(i, 1) == 0)
            for (const auto cell: nets[i])
                out << "\tn" << i << " -> c" << cell << ";\n";

    // cells of partition 0
    for (unsigned i = 0; i < cells.size(); ++i)
        if (!partitionment[i])
            for (const auto net: cells[i])
                if (is_net_cut(net))
                    out << "\tc" << i << " -> n" << net << ";\n";
    
    // cut nets
    for (unsigned i = 0; i < nets.size(); ++i)
        if (is_net_cut(i))
            for (const auto cell: nets[i])
                if (partitionment[cell])
                    out << "\tn" << i << " -> c" << cell << ";\n";

    // cells of partition 1
    for (unsigned i = 0; i < cells.size(); ++i)
        if (partitionment[i])
            for (const auto net: cells[i])
                if (!is_net_cut(net))
                    out << "\tc" << i << " -> n" << net << ";\n";

    out << "}\n";
}

void Graph::print_partitionment(const char* file) const {
    std::ofstream out(file);
    print_partitionment(out);
    out.close();
}

void Graph::print_partitionment(std::ostream& out) const {
    for (const auto part: partitionment)
        out << (int) part << '\n';
}

void Graph::set_partitionment(const std::vector<bool>& new_partitionment) {
    partitionment = new_partitionment;
    update_disbalance();
}

void Graph::set_partitionment(std::vector<bool>&& new_partitionment) {
    partitionment = new_partitionment;
    update_disbalance();
}

unsigned Graph::get_partitionment_cost() const {
    int cost = 0;
    for (unsigned i = 0; i < nets.size(); ++i) {
        if (is_net_cut(i))
            ++cost;
    }

    return cost;
} 

void Graph::move_cell(unsigned i) {
    partitionment[i] = !partitionment[i];
    disbalance += partitionment[i] ? -2 : 2; // decrease if we move to partition 1
                                             // increase if we move to partition 0
                                             // can be negative
}

void Graph::update_disbalance() {
    unsigned part0 = std::count_if(partitionment.begin(), partitionment.end(),
            [](bool x) { return !x; });
    unsigned part1 = std::count_if(partitionment.begin(), partitionment.end(),
            [](bool x) { return x; });

    disbalance = (int) (part0 - part1);
}

int Graph::get_net_cells_partition(unsigned net, bool partition) const {
    return std::count_if(nets[net].begin(), nets[net].end(), 
            [partition, this](int cell) { return partitionment[cell] == partition; });
}

bool Graph::is_net_cut(unsigned net) const {
    return get_net_cells_partition(net, 0) && get_net_cells_partition(net, 1);
}    

unsigned Graph::get_max_degree() const {
    unsigned degree = 0;
    for (const auto& cell: cells)
        degree = std::max<unsigned>(degree, cell.size());

    return degree;
}
