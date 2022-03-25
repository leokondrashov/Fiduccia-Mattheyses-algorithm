#ifndef GAIN_CONTAINER_H
#define GAIN_CONTAINER_H

#include <list>
#include <ostream>
#include <vector>

#include "graph.h"

struct Move {
    int gain;
    unsigned cell;
    bool from, to;
};

class GainContainer {
public:
    GainContainer(unsigned max_gain, unsigned num_cells);
    
    void update_gain(unsigned cell, int value);
    void initialize_gain(const Graph& g);
    void lock_cell(unsigned i);
    bool empty() const { return num_locked == num_cells; }
    bool empty_bucket(bool partition) const { return current_max_gain[partition] < (int) -MAX_GAIN; }
    
    Move best_move(int disbalance, int max_disbalance) const;

    struct CellInfo {
        int gain;
        std::list<unsigned>::iterator it;
        bool locked;
        bool partition;
    };

    void dump(std::ostream& out) const;

private:
    void update_max_gain(int max_gain, bool partition);

    std::vector<std::list<unsigned>> buckets[2];
    std::vector<CellInfo> cells;
    const unsigned MAX_GAIN;
    int current_max_gain[2];

    unsigned num_cells;
    unsigned num_locked = 0;
};

#endif // GAIN_CONTAINER_H
