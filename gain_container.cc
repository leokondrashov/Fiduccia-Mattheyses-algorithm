#include <assert.h>
#include <list>
#include <ostream>
#include <vector>

#include "gain_container.h"
#include "graph.h"

#ifndef NDEBUG
#define dassert(cond) if (!(cond)) { dump(std::cout); assert(cond); };
#else
#define dassert(cond)
#endif // NDEBUG

GainContainer::GainContainer(unsigned max_gain, unsigned num_cells, bool lifo) :
    MAX_GAIN(max_gain), num_cells(num_cells) {
    buckets[0].resize(max_gain * 2 + 1);
    buckets[1].resize(max_gain * 2 + 1);

    cells.resize(num_cells);

    this->lifo = lifo;
}

void GainContainer::update_gain(unsigned cell, int value) {
    if (cells[cell].locked)
        return;

    CellInfo& info = cells[cell];

    int new_gain = info.gain + value;

    gain_bucket(info.partition, info.gain).erase(cells[cell].it);
    gain_bucket(info.partition, new_gain).push_front(cell);
    info.it = gain_bucket(info.partition, new_gain).begin(); // iterator to the last element
    assert(*(info.it) == cell);

    if (new_gain > current_max_gain[info.partition]) // increasing max gain
        current_max_gain[info.partition] = new_gain;
    else if (info.gain == current_max_gain[info.partition]) // possibly, decreasing max_gain
        update_max_gain(current_max_gain[info.partition], info.partition);

    info.gain = new_gain;
}

void GainContainer::initialize_gain(const Graph& g) {
    for (auto& bucket: buckets[0])
        bucket.clear();
    for (auto& bucket: buckets[1])
        bucket.clear();

    for (unsigned i = 0; i < cells.size(); ++i) {
        CellInfo& info = cells[i];
        info.gain = 0;
        info.locked = false;
        info.partition = g.get_ith_cell_partition(i);

        for (auto net: g.ith_cell_nets(i)) {
            if (g.get_net_cells_partition(net, info.partition) == 1) // F(n)
                ++info.gain;

            if (g.get_net_cells_partition(net, !info.partition) == 0) // T(n)
                --info.gain;
        }

        gain_bucket(info.partition, info.gain).push_front(i);
        info.it = gain_bucket(info.partition, info.gain).begin();
        assert(*(info.it) == i);
    }
    
    update_max_gain(MAX_GAIN, 0);
    update_max_gain(MAX_GAIN, 1);

    num_locked = 0;
} 

void GainContainer::lock_cell(unsigned i) {
    dassert(!cells[i].locked);
    CellInfo& info = cells[i];

    info.locked = true;
    gain_bucket(info.partition, info.gain).erase(info.it);
    ++num_locked;
    
    update_max_gain(current_max_gain[info.partition], info.partition);
}

Move GainContainer::best_move(int disbalance, int max_disbalance) const {
    Move m = { .gain = (int) -MAX_GAIN - 1 };

    // handles the case if current disbalance is unsatisfactory:
    // chooses bigger partition to move cell from
    bool is_part0_available = disbalance > -(max_disbalance - 1) || empty_bucket(1);
    bool is_part1_available = disbalance < (max_disbalance - 1) || empty_bucket(0);
    
    if (is_part0_available && m.gain < current_max_gain[0]) {
        m.gain = current_max_gain[0];
        m.from = 0;
        m.to = 1;
        if (lifo)
            m.cell = gain_bucket(0, m.gain).front();
        else
            m.cell = gain_bucket(0, m.gain).back();
    }
    if (is_part1_available && m.gain < current_max_gain[1]) {
        m.gain = current_max_gain[1];
        m.from = 1;
        m.to = 0;
        if (lifo)
            m.cell = gain_bucket(1, m.gain).front();
        else
            m.cell = gain_bucket(1, m.gain).back();
    }

    dassert(m.gain > (int) -MAX_GAIN);

    return m;
}

// descending search for next max gain
// -MAX_GAIN - 1 if not found: renders this partition useless
void GainContainer::update_max_gain(int max_gain, bool partition) {
    while (max_gain >= (int) -MAX_GAIN && gain_bucket(partition, max_gain).empty())
        --max_gain;

    current_max_gain[partition] = max_gain;
}

void GainContainer::dump(std::ostream& out) const {
    out << "gain container:\n";
    out << "\tmax gain: " << current_max_gain[0] << " " << current_max_gain[1] << '\n';
    for (int partition = 0; partition < 2; ++partition) {
        out << "\t[" << partition << "]:\n";
        for (int i = -MAX_GAIN; i < (int) MAX_GAIN; ++i) {
            out << "\t\t[" << i << "]:";
            for (auto cell: gain_bucket(partition, i))
                out << ' ' << cell;
            out << '\n';
        }
    }

    out << "\tfree list:\n";
    for (unsigned i = 0; i < cells.size(); ++i) {
        out << "\t\t[" << i << "]: partition=" << cells[i].partition << " ";
        if (cells[i].locked)
            out << "locked\n";
        else
            out << "gain=" << cells[i].gain << " *it=" << *(cells[i].it) << '\n';
    }
}
