#ifndef INCLUDE_LWG_PROCESS_ISSUES_H
#define INCLUDE_LWG_PROCESS_ISSUES_H
#include<string>
#include<vector>
#include "sections.h"
#include "issues.h"
#include "issue_metadata.h"

namespace lwg {

auto read_file_into_string(std::string const & filename) -> std::string;
auto read_issues(std::string const & issues_path, lwg::section_map & section_db,
                 std::vector<std::string> const& changed_paths = {}) -> std::pair<std::vector<lwg::issue>, std::vector<int>>;
auto prepare_issues(std::vector<lwg::issue> & issues, lwg::section_map & section_db,
                    std::vector<int> const & changed_issue_numbers = {}) -> std::vector<int>;
auto find_location_changes(std::vector<lwg::issue> const & issues, std::vector<lwg::issue_metadata> const & meta,
                                   std::vector<int> const & issue_numbers) -> std::vector<int>;
}
#endif //INCLUDE_LWG_PROCESS_ISSUES_H
