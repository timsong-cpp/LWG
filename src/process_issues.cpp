#include "report_generator.h"
#include "mailing_info.h"
#include "issues.h"
#include "date.h"
#include <vector>
#include <sstream>
#include <set>
#include <map>
#include <ctime>
#include <cctype>
#include <algorithm>
#include <memory>
#include <iterator>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <dirent.h>
#include "process_issues.h"

auto lwg::read_file_into_string(std::string const & filename) -> std::string {
    // read a text file completely into memory, and return its contents as
    // a 'string' for further manipulation.

    std::ifstream infile{filename.c_str()};
    if (!infile.is_open()) {
        throw std::runtime_error{"Unable to open file " + filename};
    }

    std::istreambuf_iterator<char> first{infile}, last{};
    return std::string {first, last};
}

auto lwg::read_issues(std::string const & issues_path, lwg::section_map & section_db) -> std::vector<lwg::issue> {
   // Open the specified directory, 'issues_path', and iterate all the '.xml' files
   // it contains, parsing each such file as an LWG issue document.  Return the set
   // of issues as a vector.
   //
   // The current implementation relies directly on POSIX headers, but the preferred
   // direction for the future is to switch over to the filesystem TS using directory
   // iterators.

   std::unique_ptr<DIR, int(&)(DIR *)> dir{opendir(issues_path.c_str()), closedir};
   if (!dir) {
      throw std::runtime_error{"Unable to open issues dir"};
   }

   std::vector<lwg::issue> issues{};
   while ( dirent* entry = readdir(dir.get()) ) {
      std::string const issue_file{entry->d_name };
      if (0 == issue_file.find("issue") ) {
         auto const filename = issues_path + issue_file;
         issues.emplace_back(parse_issue_from_file(read_file_into_string(filename), filename, section_db));
      }
   }

   return issues;
}

static void format_issue_as_html(lwg::issue & is,
                          std::vector<lwg::issue>::iterator first_issue,
                          std::vector<lwg::issue>::iterator last_issue,
                          lwg::section_map & section_db) {
   // Reformt the issue text for the specified 'is' as valid HTML, replacing all the issue-list
   // specific XML markup as appropriate:
   //   tag             replacement
   //   ---             -----------
   //   iref            internal reference to another issue, replace with an anchor tag to that issue
   //   sref            section-tag reference, replace with formatted tag and section-number
   //   discussion      <p><b>Discussion:</b></p>CONTENTS
   //   resolution      <p><b>Proposed resolution:</b></p>CONTENTS
   //   rationale       <p><b>Rationale:</b></p>CONTENTS
   //   duplicate       tags are erased, leaving just CONTENTS
   //   note            <p><i>[NOTE CONTENTS]</i></p>
   //   !--             comments are simply erased
   //
   // In addition, as duplicate issues are discovered, the duplicates are marked up
   // in the supplied range [first_issue,last_issue).  Similarly, if an unexpected
   // (unknown) section is discovered, it will be inserted into the supplied
   // section index, 'section_db'.
   //
   // The behavior is undefined unless the issues in the supplied vector range are sorted by issue-number.
   //
   // Essentially, this function is a tiny xml-parser driven by a stack of open tags, that pops as tags
   // are closed.
   //
   // Returns true if the issue contains a <iref> to any issue in changec_numbers.

   auto fix_tags = [&](std::string &s) {
   int issue_num = is.num;     // current issue number for the issue being formatted
   std::vector<std::string> tag_stack;   // stack of open XML tags as we parse
   std::ostringstream er;      // stream to format error messages

   // cannot rewrite as range-based for-loop as the string 's' is modified within the loop
   for (std::string::size_type i{0}; i < s.size(); ++i) {
      if (s[i] == '<') {
         auto j = s.find('>', i);
         if (j == std::string::npos) {
            er.clear();
            er.str("");
            er << "missing '>' in issue " << issue_num;
            throw std::runtime_error{er.str()};
         }

         std::string tag;
         {
            std::istringstream iword{s.substr(i + 1, j - i - 1)};
            iword >> tag;
         }

         if (tag.empty()) {
             er.clear();
             er.str("");
             er << "unexpected <> in issue " << issue_num;
             throw std::runtime_error{er.str()};
         }

         if (tag[0] == '/') { // closing tag
             tag.erase(tag.begin());
             if (tag == "issue"  or  tag == "revision") {
                s.erase(i, j-i + 1);
                --i;
                return;
             }

             if (tag_stack.empty()  or  tag != tag_stack.back()) {
                er.clear();
                er.str("");
                er << "mismatched tags in issue " << issue_num;
                if (tag_stack.empty()) {
                   er << ".  Had no open tag.";
                }
                else {
                   er << ".  Open tag was " << tag_stack.back() << ".";
                }
                er << "  Closing tag was " << tag;
                throw std::runtime_error{er.str()};
             }

             tag_stack.pop_back();
             if (tag == "discussion") {
                 s.erase(i, j-i + 1);
                 --i;
             }
             else if (tag == "resolution") {
                 s.erase(i, j-i + 1);
                 --i;
             }
             else if (tag == "rationale") {
                 s.erase(i, j-i + 1);
                 --i;
             }
             else if (tag == "duplicate") {
                 s.erase(i, j-i + 1);
                 --i;
             }
             else if (tag == "note") {
                 s.replace(i, j-i + 1, "]</i></p>\n");
                 i += 9;
             }
             else {
                 i = j;
             }

             continue;
         }

         if (s[j-1] == '/') { // self-contained tag: sref, iref

            // format section references
            if (tag == "sref") {
               static const
               auto report_missing_quote = [](std::ostringstream & er, unsigned num) {
                  er.clear();
                  er.str("");
                  er << "missing '\"' in sref in issue " << num;
                  throw std::runtime_error{er.str()};
               };

               std::string r;
               auto k = s.find('\"', i+5);
               if (k >= j) {
                  report_missing_quote(er, issue_num);
               }

               auto l = s.find('\"', k+1);
               if (l >= j) {
                  report_missing_quote(er, issue_num);
               }

               ++k;
               lwg::section_tag tag;
               tag.prefix = is.doc_prefix;
               r = s.substr(k, l-k);
               tag.name = r.substr(1, r.size() - 2);

               // heuristic: if the name is not found using the doc_prefix, try
               // using no prefix (i.e. the C++ standard itself)
               if (!tag.prefix.empty() && section_db.find(tag) == section_db.end())
               {
                 //std::cout << "issue:" << is.num << " tag" << tag << '\n';
                 lwg::section_tag fallback_tag;
                 fallback_tag.name = tag.name;
                 if (section_db.find(fallback_tag) != section_db.end())
                 {
                   //std::cout << "bingo\n";
                   tag = fallback_tag;
                   //std::cout << "section_tag=\"" << tag.prefix << "\", \"" << tag.name << "\"\n";
                 }
               }

               j -= i - 1;
               r = format_section_tag_as_link(section_db, tag);
               s.replace(i, j, r);
               i += r.size() - 1;
               continue;
            }

            // format issue references
            else if (tag == "iref") {
               static const
               auto report_missing_quote = [](std::ostringstream & er, unsigned num) {
                  er.clear();
                  er.str("");
                  er << "missing '\"' in iref in issue " << num;
                  throw std::runtime_error{er.str()};
               };

               auto k = s.find('\"', i+5);
               if (k >= j) {
                  report_missing_quote(er, issue_num);
               }
               auto l = s.find('\"', k+1);
               if (l >= j) {
                  report_missing_quote(er, issue_num);
               }

               ++k;
               std::string r{s.substr(k, l - k)};
               std::istringstream temp{r};
               int num;
               temp >> num;
               if (temp.fail()) {
                  er.clear();
                  er.str("");
                  er << "bad number in iref in issue " << issue_num;
                  throw std::runtime_error{er.str()};
               }

               auto n = std::lower_bound(first_issue, last_issue, num, lwg::order_by_issue_number{});
               if (n == last_issue  or  n->num != num) {
                  er.clear();
                  er.str("");
                  er << "could not find issue " << num << " for iref in issue " << issue_num;
                  throw std::runtime_error{er.str()};
               }

               if (!tag_stack.empty()  and  tag_stack.back() == "duplicate") {
                  n->duplicates.insert(make_html_anchor(is));
                  is.duplicates.insert(lwg::make_html_anchor(*n));
                  r.clear();
               }
               else {
                  r = lwg::make_html_anchor(*n);
               }

               j -= i - 1;
               s.replace(i, j, r);
               i += r.size() - 1;
               continue;
            }
            i = j;
            continue;  // don't worry about this <tag/>
         }

         tag_stack.push_back(tag);
         if (tag == "discussion") {
             s.replace(i, j-i + 1, "<p><b>Discussion:</b></p>");
             i += 24;
         }
         else if (tag == "resolution") {
             s.replace(i, j-i + 1, "<p><b>Proposed resolution:</b></p>");
             i += 33;
         }
         else if (tag == "rationale") {
             s.replace(i, j-i + 1, "<p><b>Rationale:</b></p>");
             i += 23;
         }
         else if (tag == "duplicate") {
             s.erase(i, j-i + 1);
             --i;
         }
         else if (tag == "note") {
             s.replace(i, j-i + 1, "<p><i>[");
             i += 6;
         }
         else if (tag == "!--") {
             tag_stack.pop_back();
             j = s.find("-->", i);
             j += 3;
             s.erase(i, j-i);
             --i;
         }
         else {
             i = j;
         }
      }
   }
   };

   fix_tags(is.text);
   fix_tags(is.resolution);
}

void lwg::prepare_issues(std::vector<lwg::issue> & issues, lwg::section_map & section_db) {
   // Initially sort the issues by issue number, so each issue can be correctly 'format'ted
   sort(issues.begin(), issues.end(), lwg::order_by_issue_number{});

   // Then we format the issues, which should be the last time we need to touch the issues themselves
   // We may turn this into a two-stage process, analysing duplicates and then applying the links
   // This will allow us to better express constness when the issues are used purely for reference.
   // Currently, the 'format' function takes a reference-to-non-const-vector-of-issues purely to
   // mark up information related to duplicates, so processing duplicates in a separate pass may
   // clarify the code.
   for (auto & i : issues) {
      format_issue_as_html(i, issues.begin(), issues.end(), section_db);
   }

   // Issues will be routinely re-sorted in later code, but contents should be fixed after formatting.
   // This suggests we may want to be storing some kind of issue handle in the functions that keep
   // re-sorting issues, and so minimize the churn on the larger objects.
}


