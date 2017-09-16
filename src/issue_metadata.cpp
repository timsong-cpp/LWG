#include "issue_metadata.h"
#include "process_issues.h"
#include <stdexcept>
#include <utility>
#include <sstream>
#include <string>


// return the next <element...></element> pair and the index immediately afterwards, starting at index i.
static auto next_element(std::string const & element, std::string const & s, std::string::size_type i = 0)
    -> std::pair<std::string::size_type, std::string>{
    const auto ele_start = "<" + element;
    const auto ele_end = "</" + element + ">";
    auto r = s.find(ele_start, i);
    if(r == std::string::npos)
        return {r, ""};
    auto rend = s.find(">", r + ele_start.length());
    if(rend  == std::string::npos)
        throw std::runtime_error("Unable to parse issues: cannot find > for <" + element + "> element.");
    auto j = s.find(ele_end, rend + 1);
    if(j == std::string::npos)
        throw std::runtime_error("Unable to parse issues: cannot find expected " + ele_end + " element.");
    return {j + ele_end.length(), s.substr(r, j + ele_end.length() - r)};
}

// return the content of the next <element ...></element> pair and the index immediately afterwards, starting at index i.
static auto next_element_content(std::string const & element, std::string const & s, std::string::size_type i = 0)
    -> std::pair<std::string::size_type, std::string> {
    auto p = next_element(element, s, i);
    if(p.first == std::string::npos) return p;
    auto b = p.second.find('>') + 1;
    return {p.first, p.second.substr(b, p.second.rfind('<') - b)};
}

// consume the next <td> element, which must contain a section tag, and returns the text of that section tag,
// not including the square brackets.
static auto find_next_section_tag(std::string const & s, std::string::size_type i)
    -> std::pair<std::string::size_type, std::string> {
    auto td = next_element_content("td", s, i);
    auto b = td.second.find('[');
    if(b == std::string::npos) {
        throw std::runtime_error("unable to parse issue: can't find starting bracket for section tag");
    }
    auto e = td.second.find(']', b+1);
    if(e == std::string::npos) {
        throw std::runtime_error("unable to parse issue: can't find ending bracket for section tag");
    }
    return {td.first, td.second.substr(b+1, e-b-1)};
}

auto lwg::read_issue_metadata_from_toc(std::string const & filename) -> std::vector<issue_metadata> {
    auto s = lwg::read_file_into_string(filename);
    // Skip the title row
    auto i = next_element("tr", s).first;
    // Read all issues in table
    std::vector<issue_metadata> issues;
    std::string row;
    for(std::tie(i, row) = next_element_content("tr", s, i);
        i != std::string::npos;
        std::tie(i, row) = next_element_content("tr", s, i)) {

        std::string num_str, stat, first_tag, title, resolution, priority_str;

        // read the elements.
        std::string::size_type j;
        std::tie(j, num_str) = next_element_content("a", row);
        std::tie(j, stat) = next_element_content("a", row, j);
        std::tie(j, first_tag) = find_next_section_tag(row, j);
        std::tie(j, title) = next_element_content("td", row, j);
        std::tie(j, resolution) = next_element_content("td", row, j);
        std::tie(j, priority_str) = next_element_content("td", row, j);

        // parse the numerical ones.
        const int num = std::stoi(num_str);
        bool has_resolution = resolution.find("Yes") != std::string::npos;
        const int priority = priority_str.empty() ? 99 : std::stoi(priority_str);

        // duplicates
        std::vector<std::string> dups;
        std::string cur_dup;

        for(std::tie(j, cur_dup) = next_element("a", row, j);
            j != std::string::npos;
            std::tie(j, cur_dup) = next_element("a", row, j)) {
            dups.push_back(std::move(cur_dup));
        }

        issues.push_back({num, std::move(stat), std::move(first_tag), std::move(title),
                          has_resolution, priority, std::move(dups)});
    }

    return issues;
}
