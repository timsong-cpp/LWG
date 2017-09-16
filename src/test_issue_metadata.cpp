#include "issue_metadata.h"
#include "issues.h"
#include "sections.h"
#include "process_issues.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <memory>

std::ostream& operator<<(std::ostream& os, lwg::issue_metadata const& metadata) {
    os << metadata.num << '\t' << metadata.stat << '\t' << metadata.first_tag << '\t'
       << std::quoted(metadata.title) << '\t' << metadata.priority << '\t' << metadata.has_resolution
       << "\t[";
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

    auto sort_by_num =  [](auto const& l, auto const& r) { return l.num < r.num; };
    std::sort(mdv.begin(), mdv.end(), sort_by_num);
    std::ifstream sec_str("meta-data/section.data");
    auto section_db = lwg::read_section_db(sec_str);
    auto issues = lwg::read_issues("xml/", section_db).first;
    lwg::prepare_issues(issues, section_db);
    if(!std::equal(issues.begin(), issues.end(), mdv.begin(), mdv.end())){
        std::cerr << "Comparison failure!\n";
        auto first_mismatch = std::mismatch(issues.begin(), issues.end(), mdv.begin(), mdv.end());
        std::cerr << "First mismatch at: " << *first_mismatch.second;
    }
}