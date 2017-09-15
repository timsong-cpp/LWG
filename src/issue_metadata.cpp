#include "issue_metadata.h"
#include <stdexcept>
#include <utility>
#include <sstream>

auto lwg::read_issue_metadata_from_toc(std::string const & s) -> std::vector<issue_metadata> {
    // Like lists.cpp's read_issues_from_toc, but parses more information.

    // return the index immediately after the next <td> </td> pair, starting at i.
    auto skip_td = [](std::string const & s, std::string::size_type i, std::string::size_type until) {
        auto r = s.find("<td", i);
        if(r >= until)
            throw std::runtime_error("Unable to parse issues: cannot find expected <td> element.");
        auto j = s.find("</td>", r + 4);
        if(j >= until)
            throw std::runtime_error("Unable to parse issues: cannot find expected <td> element.");
        return j + 5;
    };

    // returns the text of the next <a ...>text</a> element, starting at i, updating it to point after the link.
    // text must not contain >s
    auto find_next_link_text = [](std::string const & s, std::string::size_type& i){
        auto e = s.find("</a>", i);
        auto j = s.rfind(">", e);
        if (j == std::string::npos) {
            throw std::runtime_error{"unable to parse issue: can't find beginning bracket"};
        }
        i = e + 4; // 4 chars in </a>
        return s.substr(j+1, e-j-1);
    };

    // consume the next <td> element that contains a section tag, and returns the text of that section tag,
    // not including the square brackets.
    auto find_next_section_tag = [&](std::string const & s, std::string::size_type& i, std::string::size_type until) {
        auto td_end = skip_td(s, i, until);
        auto b = s.find("[", i);
        if(b >= td_end) {
            throw std::runtime_error("unable to parse issue: can't find starting bracket for section tag");
        }
        auto e = s.find("]", b);
        if(e >= td_end) {
            throw std::runtime_error("unable to parse issue: can't find ending bracket for section tag");
        }
        i = td_end;
        return s.substr(b+1, e-b-1);
    };

    // returns the full text of the next <a> element, i.e., the whole <a href="...">text</a> sequence,
    // in the substring [i, until) of s.
    // On success, update i to point past the link. On failure, set i to until.
    // We return a pair, because we can't use optional :(

    auto try_find_next_full_link = [](std::string const& s, std::string::size_type& i, std::string::size_type until) -> std::pair<bool, std::string> {
        auto b = s.find("<a", i);
        if (b == std::string::npos || b >= until) {
            i = until;
            return {false, ""};
        }
        auto e = s.find("</a>", b+2);
        if (e == std::string::npos || e >= until) {
            i = until;
            return {false, ""};
        }
        i = e + 4; // 4 chars in </a>
        return { true, s.substr(b, i - b)};
    };

    // Skip the title row
    auto i = s.find("<tr>");
    if (std::string::npos == i) {
        throw std::runtime_error{"Unable to find the first (title) row"};
    }
    i += 4;
    // Read all issues in table
    std::vector<issue_metadata> issues;
    for(;;) {
        i = s.find("<tr>", i);
        if (i == std::string::npos) {
            break;
        }
        const auto until = s.find("</tr>", i + 4); // end of row

        // issue number
        auto num_str = find_next_link_text(s, i);
        std::istringstream instr{num_str};
        int num;
        instr >> num;
        if (instr.fail()) {
            throw std::runtime_error{"unable to parse issue number: " + num_str};
        }

        // status
        auto stat = find_next_link_text(s, i);

        // section
        auto first_tag = find_next_section_tag(s, i, until);

        // skip title, PR, and priority
        for(int x = 0; x < 3; ++x) i = skip_td(s, i, until);

        // duplicates
        std::vector<std::string> dups;
        for(auto next_dup = try_find_next_full_link(s, i, until); next_dup.first;
            next_dup = try_find_next_full_link(s, i, until)) {
            dups.push_back(std::move(next_dup.second));
        }

        issues.push_back({num, stat, first_tag, std::move(dups)});
    }

    return issues;
}
