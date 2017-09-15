#include "issue_metadata.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iterator>

std::ostream& operator<<(std::ostream& os, lwg::issue_metadata const& metadata) {
    os << metadata.num << '\t' << metadata.stat << '\t' << metadata.first_tag << "\t[";
    const char* sep = "";
    for(const auto& x : metadata.duplicates){
        os << sep << std::quoted(x, '|');
        sep = ", ";
    }
    os << "]\n";
    return os;
}

auto read_file_into_string(std::string const & filename) -> std::string {
    // read a text file completely into memory, and return its contents as
    // a 'string' for further manipulation.

    std::ifstream infile{filename.c_str()};
    if (!infile.is_open()) {
        throw std::runtime_error{"Unable to open file " + filename};
    }

    std::istreambuf_iterator<char> first{infile}, last{};
    return std::string {first, last};
}

int main() {
    auto mdv = lwg::read_issue_metadata_from_toc(read_file_into_string("mailing/lwg-toc.html"));
    for(const auto& md : mdv){
        std::cout << md ;
    }
}