#include <assert.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

#include "gain_container.h"
#include "graph.h"

std::unique_ptr<char[]> create_output_file_name(const char* input) {
    unsigned in_len = strlen(input);
    std::unique_ptr<char[]> name(new char[in_len + sizeof(".part.2")]);
    strcpy(name.get(), input);
    strcpy(name.get() + in_len, ".part.2");

    return name;
}

int main(int argc, char **argv) {
    char *input_filename = nullptr;
    char *dump_filename = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--dump") == 0) {
            assert(i + 1 < argc);
            dump_filename = argv[i + 1];
            ++i;
        } else {
            input_filename = argv[i];
        }
    }

    if (input_filename == nullptr) {
        std::cout << "Missing input filename" << '\n';
        std::cout << "Usage: ./test input_filename --dump dump_file.dot" << '\n';
        exit(1);
    }

    Graph g(input_filename);
    GainContainer gc(g.get_max_degree(), g.get_cell_count());

    g.move_cell(0);
    g.move_cell(1);

    if (dump_filename)
        g.dump(dump_filename);
    auto output_filename = create_output_file_name(input_filename);
    g.print_partitionment(output_filename.get());
    std::cout << "\nmax degree: " << g.get_max_degree() << '\n';
    std::cout << "partitionment cost: " << g.get_partitionment_cost() << '\n';
    
    gc.initialize_gain(g);
    std::cout << "\n\n";
    gc.dump(std::cout);

    std::cout << "\n";
    std::cout << "after changing gain for 2nd on -2:\n";
    //gc.update_gain(2, -2);
    gc.dump(std::cout);
}
