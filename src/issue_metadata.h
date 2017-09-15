#ifndef INCLUDE_LWG_ISSUE_METADATA_H
#define INCLUDE_LWG_ISSUE_METADATA_H

#include "issues.h"
#include <string>
#include <vector>
#include <algorithm>

namespace lwg {

struct issue_metadata {
    int num;
    std::string stat;
    std::string first_tag;
    std::vector<std::string> duplicates;
    friend bool operator==(issue const & issue, issue_metadata const & md) {
        return md.num == issue.num && md.stat == issue.stat && md.first_tag == as_string(issue.tags.front()) &&
               std::equal(md.duplicates.begin(),md.duplicates.end(), issue.duplicates.begin(), issue.duplicates.end());
    }
    friend bool operator==(issue_metadata const & md, issue const & issue) {
        return issue == md;
    }
    friend bool operator!=(issue const & issue, issue_metadata const & md) {
        return !(issue == md);
    }
    friend bool operator!=(issue_metadata const & md, issue const & issue) {
        return issue != md;
    }
};


auto read_issue_metadata_from_toc(std::string const & s) -> std::vector<issue_metadata>;



}

#endif //INCLUDE_LWG_ISSUE_METADATA_H
