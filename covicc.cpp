#include <iostream>
#include <fstream>
#include <string>
#include <vector>

// Function to write a string as length-prefixed bytecode
void write_length_prefixed_string(std::ofstream& output, const std::string& str) {
    uint8_t length = static_cast<uint8_t>(str.size());
    output.put(length);
    output.write(str.c_str(), length);
}

void write_big_endian_uint32(std::ofstream& output, uint32_t value) {
    output.put((value >> 24) & 0xFF);
    output.put((value >> 16) & 0xFF);
    output.put((value >> 8) & 0xFF);
    output.put(value & 0xFF);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./covicc <input.covi> <output.fac>" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];

    // Parse the input file
    std::ifstream input(input_file);
    if (!input.is_open()) {
        std::cerr << "Error: Cannot open input file: " << input_file << std::endl;
        return 1;
    }

    std::ofstream output(output_file, std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Error: Cannot open output file: " << output_file << std::endl;
        return 1;
    }

    // Write the magic number in big-endian format
    uint32_t magic = 0xFAACBEED;
    write_big_endian_uint32(output, magic);

    // Write the main method header
    output.put(0x40); // OP_MAIN opcode
    write_length_prefixed_string(output, "Covi"); // Object name
    write_length_prefixed_string(output, "main"); // Method name
    output.put(0x00); // No arguments

    // Process the input file
    std::string line;
    while (std::getline(input, line)) {
        if (line.find("System.out.printnl") != std::string::npos) {
            output.put(0x30); // OP_PRINTNLIO opcode
            write_length_prefixed_string(output, "System"); // Object name
            write_length_prefixed_string(output, "out"); // Group name
            write_length_prefixed_string(output, "printnl"); // Method name

            // Extract the argument string
            size_t start = line.find("\"") + 1;
            size_t end = line.rfind("\"");
            std::string argument = line.substr(start, end - start);
            write_length_prefixed_string(output, argument); // Argument
        }
    }

    // Write the HALT instruction
    output.put(0xFF); // OP_HALT opcode

    input.close();
    output.close();

    std::cout << "[covicc] Compilation successful: " << output_file << std::endl;
    return 0;
}