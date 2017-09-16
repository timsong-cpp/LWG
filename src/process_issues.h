#ifndef INCLUDE_LWG_PROCESS_ISSUES_H
#define INCLUDE_LWG_PROCESS_ISSUES_H
#include<string>
#include<vector>
#include "sections.h"
#include "issues.h"

namespace lwg {

auto read_file_into_string(std::string const & filename) -> std::string;
auto read_issues(std::string const & issues_path, lwg::section_map & section_db) -> std::vector<lwg::issue>;
void prepare_issues(std::vector<lwg::issue> & issues, lwg::section_map & section_db);

}
#endif //INCLUDE_LWG_PROCESS_ISSUES_H
