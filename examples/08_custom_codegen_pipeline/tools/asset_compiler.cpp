#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Entry {
    std::string key;
    std::string value;
};

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: asset_compiler <input_file> <output_dir>\n";
        return 1;
    }

    fs::path input_path = argv[1];
    fs::path output_dir = argv[2];

    std::ifstream in(input_path);
    if (!in.is_open()) {
        std::cerr << "Error: cannot open input file " << input_path << "\n";
        return 1;
    }

    std::vector<Entry> entries;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        auto eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            entries.push_back({key, val});
        }
    }
    in.close();

    fs::create_directories(output_dir);
    fs::path header_path = output_dir / "ui_strings.hpp";
    fs::path source_path = output_dir / "ui_strings.cpp";

    // Write header
    std::ofstream h(header_path);
    h << "#pragma once\n\n";
    h << "#include <string_view>\n\n";
    h << "namespace strings {\n\n";
    for (const auto &e : entries) {
        h << "std::string_view " << e.key << "() noexcept;\n";
    }
    h << "\n} // namespace strings\n";
    h.close();

    // Write source
    std::ofstream s(source_path);
    s << "#include \"ui_strings.hpp\"\n\n";
    s << "namespace strings {\n\n";
    for (const auto &e : entries) {
        s << "std::string_view " << e.key << "() noexcept {\n";
        s << "    return \"" << e.value << "\";\n";
        s << "}\n\n";
    }
    s << "} // namespace strings\n";
    s.close();

    std::cout << "[asset_compiler] Successfully generated " << header_path.string() << " and " << source_path.string()
              << " (" << entries.size() << " strings)\n";
    return 0;
}
