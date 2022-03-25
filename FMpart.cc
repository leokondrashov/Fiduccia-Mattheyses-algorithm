#include <assert.h>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "gain_container.h"
#include "graph.h"

#ifndef NDEBUG
bool verbose_debug = false;
#define ON_DEBUG(op) if (verbose_debug) { op }
#else
#define ON_DEBUG(op) ;
#endif // NDEBUG

std::vector<bool> static_initial_partitionment(unsigned num_cells) {
    std::vector<bool> partitionment(num_cells);

    for (unsigned i = 0; i < num_cells / 2; ++i)
        partitionment[i] = true;

    return partitionment;
}

std::vector<bool> random_initial_partitionment(unsigned num_cells) {
    std::vector<bool> partitionment(num_cells);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution rand(0.5);

    for (unsigned i = 0; i < num_cells; ++i)
        partitionment[i] = rand(gen);

    return partitionment;
}

void print_usage() {
    std::cout << "Usage: ./FMpart input_filename [--dump_file dump_file.dot] [-m]"
        << "[--disbalance DISBALANCE] [--initial (static|random)]"
        << '\n';
}

std::vector<bool> initial_partitionment(unsigned num_cells, const char *type) {
    if (strcmp(type, "static") == 0)
        return static_initial_partitionment(num_cells);
    if (strcmp(type, "random") == 0)
        return random_initial_partitionment(num_cells);

    std::cout << "Unknown initial type " << type << '\n';
    print_usage();
    exit(1);
    return std::vector<bool>();
}

void update_gain(const Graph& g, GainContainer *gc, const Move& m) {
    for (const auto net: g.ith_cell_nets(m.cell)) {
        if (g.get_net_cells_partition(net, m.to) == 0) { // adding net's first cell to dest
            for (const auto cell: g.ith_net_cells(net))
                gc->update_gain(cell, 1);
        }
        if (g.get_net_cells_partition(net, m.from) == 1) { // removing net's last cell
            for (const auto cell: g.ith_net_cells(net))
                gc->update_gain(cell, -1);
        }

        if (g.get_net_cells_partition(net, m.from) == 2) { // leaving one behind
            for (const auto cell: g.ith_net_cells(net))
                if (g.get_ith_cell_partition(cell) == m.from) // updating m.cell as well
                    gc->update_gain(cell, 1);
        }
        if (g.get_net_cells_partition(net, m.to) == 1) { // adding second cell to dest
            for (const auto cell: g.ith_net_cells(net))
                if (g.get_ith_cell_partition(cell) == m.to)
                    gc->update_gain(cell, -1);
        }
    }
}

unsigned FMpass(Graph *g, GainContainer *gc, unsigned possible_disbalance) {
    gc->initialize_gain(*g);

    unsigned solution_cost = g->get_partitionment_cost();
    unsigned best_solution = (unsigned) -1;

    int cur_disbalance = g->get_disbalance();
    unsigned best_disbalance = possible_disbalance + 1;

    std::vector<bool> best_partitionment;

    while (!gc->empty()) {
        Move m = gc->best_move(g->get_disbalance(), possible_disbalance);

        solution_cost -= m.gain;
        gc->lock_cell(m.cell);
        update_gain(*g, gc, m);

        g->move_cell(m.cell);
        cur_disbalance = g->get_disbalance();
        assert(solution_cost == g->get_partitionment_cost());

        if (abs(cur_disbalance) <= possible_disbalance &&
                (solution_cost < best_solution ||
                 (solution_cost == best_solution && // out of equally good we choose with
                  abs(cur_disbalance) < best_disbalance))) { // better balance
            best_partitionment = g->get_partitionment();
            best_solution = solution_cost;
            best_disbalance = abs(cur_disbalance);
        }

        ON_DEBUG(
        std::cout << m.cell << " moved, solution cost = " << solution_cost <<
           ", disbalance = " << cur_disbalance << '\n';
        gc->dump(std::cout);
        getchar();
        )
    }

    g->set_partitionment(std::move(best_partitionment));

    return best_solution;
}

struct Parameters {
    unsigned disbalance = 2;
    bool modified = false;
    const char *dump = nullptr;
    const char *init_part = "static";
};

void FM(const char *input, const char *output, const Parameters& p) {
    Graph g(input);
    GainContainer gc(g.get_max_degree(), g.get_cell_count(), p.modified);

    g.set_partitionment(initial_partitionment(g.get_cell_count(), p.init_part));
    unsigned current_cost = g.get_partitionment_cost();
    unsigned old_cost = 0;

    std::cout << "Initial: cost=" << current_cost << ", disbalance=" <<
        g.get_disbalance() << '\n';

    unsigned iteration_count = 0;

    std::clock_t start_time = std::clock();

    do {
        ON_DEBUG(
        if (p.dump)
            g.dump(p.dump);
        )

        old_cost = current_cost;
        current_cost = FMpass(&g, &gc, p.disbalance);
        ++iteration_count;

        auto elapsed_time = (double) (std::clock() - start_time) / CLOCKS_PER_SEC;

        std::cout << "Heartbeat: iteration=" << iteration_count <<
            ", cost=" << current_cost << ", disbalance=" << g.get_disbalance() <<
            ", time=" << elapsed_time << '\n';

        ON_DEBUG(
        if (p.dump)
            system(("dotty " + std::string(p.dump)).c_str());
        )
    } while (current_cost < old_cost);

    std::clock_t end_time = std::clock();

    std::cout << "Results: time=" << (double) (end_time - start_time) / CLOCKS_PER_SEC <<
        ", iterations=" << iteration_count <<
        ", cost=" << current_cost <<
        ", disbalance=" << g.get_disbalance() << '\n';

    g.print_partitionment(output);

    if (p.dump)
        g.dump(p.dump);
}

void check_argc(int i, int argc) {
    if (i >= argc) {
        std::cout << "Not enough arguments\n";
        print_usage();
        exit(1);
    }
}

int main(int argc, char **argv) {
    char *input_filename = nullptr;
    Parameters p;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--dump") == 0) {
            check_argc(i + 1, argc);
            p.dump = argv[i + 1];
            ++i;
        } else if (strcmp(argv[i], "--disbalance") == 0) {
            check_argc(i + 1, argc);
            p.disbalance = atoi(argv[i + 1]);
            ++i;
        } else if (strcmp(argv[i], "--initial") == 0) {
            check_argc(i + 1, argc);
            p.init_part = argv[i + 1];
            ++i;
        } else if (strcmp(argv[i], "-m") == 0) {
            p.modified = true;
#ifndef NDEBUG
        } else if (strcmp(argv[i], "--verbose_debug") == 0) {
            verbose_debug = true;
#endif // NDEBUG
        } else if (argv[i][0] == '-') {
            std::cout << "Unknown argument " << argv[i] << '\n';
            print_usage();
            exit(1);
        } else {
            input_filename = argv[i];
        }
    }

    if (input_filename == nullptr) {
        std::cout << "Missing input filename" << '\n';
        print_usage();
        exit(1);
    }

    std::string output_filename = std::string(input_filename) + ".part.2";
    FM(input_filename, output_filename.c_str(), p);
}
