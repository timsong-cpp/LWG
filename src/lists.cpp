// This program reads all the issues in the issues directory passed as the first command line argument.
// If all documents are successfully parsed, it will generate the standard LWG Issues List documents
// for an ISO SC22 WG21 mailing.

// Based on code originally donated by Howard Hinnant
// Since modified by Alisdair Meredith

// Note that this program requires a reasonably conformant C++11 compiler, supporting at least:
//    auto
//    lambda expressions
//    brace-initialization
//    range-based for loops
//    raw string literals
//    constexpr
//    new function syntax (late specified return type)
//    noexcept
//    new type-alias syntax (using my_name = type)

// Likewise, the following C++11 library facilities are used:
//    to_string
//    consistent overloading of 'char const *' and 'std::string'

// Following the planned removal of (deprecated) bool post-increment operator, we now
// require the following C++14 library facilities:
//    exchange

// The following C++14 features are being considered for future cleanup/refactoring:
//    polymorphic lambdas
//    string UDLs

// The following TS features are also desirable
//    filesystem
//    string_view

// Its coding style assumes a standard library optimized with move-semantics
// The only known compiler to support all of this today is the experimental gcc trunk (4.6)

// TODO
// .  Grouping of issues in "Clause X" by TR/TS
// .  Sort the Revision comments in the same order as the 'Status' reports, rather than alphabetically
// .  Lots of tidy and cleanup after merging the revision-generating tool
// .  Refactor more common text
// .  Split 'format' function and usage to that the issues vector can pass by const-ref in the common cases
// .  Document the purpose amd contract on each function
// Waiting on external fix for preserving file-dates
// .  sort-by-last-modified-date should offer some filter or separation to see only the issues modified since the last meeting

// Missing standard facilities that we work around
// . Date
// . Filesystem navigation

// Missing standard library facilities that would probably not change this program
// . XML parser

// standard headers
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// platform headers - requires a Posix compatible platform
// The hope is to replace all of this with the filesystem TS
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

// solution specific headers
#include "date.h"
#include "issues.h"
#include "mailing_info.h"
#include "report_generator.h"
#include "sections.h"
#include "process_issues.h"
#include "issue_metadata.h"


#if 0
// Revisit this after AJM works out linker issues on his Mac
#if 1
// workaround until <experimental/filesystem> is widely available:
##include <boost/filesystem.hpp>
namespace std {
namespace experimental {
namespace filesystem {
using namespace boost::filesystem;
}
}
}
#else
#include <experimental/filesystem>
#endif
#endif



// Issue-list specific functionality for the rest of this file
// ===========================================================

struct list_issues {
   std::vector<int> const & issues;
};


auto operator<<( std::ostream & out, list_issues const & x) -> std::ostream & {
   auto list_separator = "";
   for (auto number : x.issues ) {
      out << list_separator << "<iref ref=\"" << number << "\"/>";
      list_separator = ", ";
   }
   return out;
}

struct discover_new_issues {
   std::vector<lwg::issue_metadata> const & old_issues;
   std::vector<lwg::issue> const & new_issues;
};


auto operator<<( std::ostream & out, discover_new_issues const & x) -> std::ostream & {
   auto const & old_issues = x.old_issues;
   auto const & new_issues = x.new_issues;

   struct status_order {
      // predicate for 'map'

      using status_string = std::string;
      auto operator()(status_string const & x, status_string const & y) const noexcept -> bool {
         return lwg::get_status_priority(x) < lwg::get_status_priority(y);
      }
   };

   std::map<std::string, std::vector<int>, status_order> added_issues;
   for (auto const & i : new_issues ) {
      auto j = std::lower_bound(old_issues.cbegin(), old_issues.cend(), i.num, lwg::order_by_issue_number{});
      if(j == old_issues.end() or j-> num != i.num ) {
         added_issues[i.stat].push_back(i.num);
      }
   }

   for (auto const & i : added_issues ) {
      auto const item_count = std::get<1>(i).size();
      if(1 == item_count) {
         out << "<li>Added the following " << std::get<0>(i) << " issue: <iref ref=\"" << std::get<1>(i).front() << "\"/>.</li>\n";
      }
      else {
         out << "<li>Added the following " << item_count << " " << std::get<0>(i) << " issues: " << list_issues{std::get<1>(i)} << ".</li>\n";
      }
   }
   
   if (added_issues.empty()) {
      out << "<li>No issues added.</li>\n";
   }

   return out;
}


struct discover_changed_issues {
   std::vector<lwg::issue_metadata> const & old_issues;
   std::vector<lwg::issue> const & new_issues;
};

auto operator << (std::ostream & out, discover_changed_issues x) -> std::ostream & {
   auto const & old_issues = x.old_issues;
   auto const & new_issues = x.new_issues;

   struct status_transition_order {
      using status_string = std::string;
      using from_status_to_status = std::tuple<status_string, status_string>;

      auto operator()(from_status_to_status const & x, from_status_to_status const & y) const noexcept -> bool {
         auto const xp2 = lwg::get_status_priority(std::get<1>(x));
         auto const yp2 = lwg::get_status_priority(std::get<1>(y));
         return xp2 < yp2  or  (!(yp2 < xp2)  and  lwg::get_status_priority(std::get<0>(x)) < lwg::get_status_priority(std::get<0>(y)));
      }
   };

   std::map<std::tuple<std::string, std::string>, std::vector<int>, status_transition_order> changed_issues;
   for (auto const & i : new_issues ) {
      auto j = std::lower_bound(old_issues.begin(), old_issues.end(), i.num, lwg::order_by_issue_number{});
      if (j != old_issues.end()  and i.num == j->num and j->stat != i.stat) {
         changed_issues[std::make_tuple(j->stat, i.stat)].push_back(i.num);
      }
   }

   for (auto const & i : changed_issues ) {
      auto const item_count = std::get<1>(i).size();
      if(1 == item_count) {
         out << "<li>Changed the following issue to " << std::get<1>(std::get<0>(i))
             << " (from " << std::get<0>(std::get<0>(i)) << "): <iref ref=\"" << std::get<1>(i).front() << "\"/>.</li>\n";
      }
      else {
         out << "<li>Changed the following " << item_count << " issues to " << std::get<1>(std::get<0>(i))
             << " (from " << std::get<0>(std::get<0>(i)) << "): " << list_issues{std::get<1>(i)} << ".</li>\n";
      }
   }

   if (changed_issues.empty()) {
      out << "<li>No issues changed.</li>\n";
   }

   return out;
}

template<class IssueOrMD>
void count_issues(std::vector<IssueOrMD> const & issues, int & n_open, int & n_reassigned, int & n_closed) {
   n_open = 0;
   n_reassigned = 0;
   n_closed = 0;

   for(auto const & elem : issues) {
      if (lwg::is_assigned_to_another_group(elem.stat)) {
      	++n_reassigned;
      }
      else if (lwg::is_active(elem.stat)) {
         ++n_open;
      }
      else {
         ++n_closed;
      }
   }
}


struct write_summary {
   std::vector<lwg::issue_metadata > const & old_issues;
   std::vector<lwg::issue > const & new_issues;
};


auto operator << (std::ostream & out, write_summary const & x) -> std::ostream & {
   auto const & old_issues = x.old_issues;
   auto const & new_issues = x.new_issues;

   int n_open_new = 0;
   int n_open_old = 0;
   int n_reassigned_new = 0;
   int n_reassigned_old = 0;
   int n_closed_new = 0;
   int n_closed_old = 0;
   count_issues(old_issues, n_open_old, n_reassigned_old, n_closed_old);
   count_issues(new_issues, n_open_new, n_reassigned_new, n_closed_new);
   auto write_change = [&out](int n_new, int n_old){
      out << (n_new >= n_old ? "up by " : "down by ")
          << std::abs(n_new - n_old);
   };

   out << "<li>" << n_open_new << " open issues, ";
   write_change(n_open_new, n_open_old);
   out << ".</li>\n";

   out << "<li>" << n_closed_new << " closed issues, ";
   write_change(n_closed_new, n_closed_old);
   out << ".</li>\n";

   out << "<li>" << n_reassigned_new << " reassigned issues, ";
   write_change(n_reassigned_new, n_reassigned_old);
   out << ".</li>\n";

   int n_total_new = n_open_new + n_reassigned_new + n_closed_new;
   int n_total_old = n_open_old + n_reassigned_old + n_closed_old;
   out << "<li>" << n_total_new << " issues total, ";
   write_change(n_total_new, n_total_old);
   out << ".</li>\n";

   return out;
}


void print_current_revisions( std::ostream & out
                            , std::vector<lwg::issue_metadata > const & old_issues
                            , std::vector<lwg::issue > const & new_issues
                            ) {
   out << "<ul>\n"
          "<li><b>Summary:</b><ul>\n"
       << write_summary{old_issues, new_issues}
       << "</ul></li>\n"
          "<li><b>Details:</b><ul>\n"
       << discover_new_issues{old_issues, new_issues}
       << discover_changed_issues{old_issues, new_issues}
       << "</ul></li>\n"
          "</ul>\n";
}

// ============================================================================================================

void check_is_directory(std::string const & directory) {
  struct stat sb;
  if (stat(directory.c_str(), &sb) != 0 || !S_ISDIR(sb.st_mode))
    throw std::runtime_error(directory + " is not an existing directory");
}

int main(int argc, char* argv[]) {
   try {
      std::string path;
      std::cout << "Preparing new LWG issues lists..." << std::endl;
      if (argc == 2) {
         path = argv[1];
      }
      else {
         char cwd[1024];
         if (getcwd(cwd, sizeof(cwd)) == 0) {
            std::cout << "unable to getcwd\n";
            return 1;
         }
         path = cwd;
      }

      if (path.back() != '/') { path += '/'; }
      check_is_directory(path);
	  
      const std::string target_path{path + "mailing/"};
      check_is_directory(target_path);
	  

      lwg::section_map section_db =[&path]() {
         auto filename = path + "meta-data/section.data";
         std::ifstream infile{filename};
         if (!infile.is_open()) {
            throw std::runtime_error{"Can't open section.data at " + path + "meta-data"};
         }
         std::cout << "Reading section-tag index from: " << filename << std::endl;

         return lwg::read_section_db(infile);
      }();
#if defined (DEBUG_LOGGING)
      // dump the contents of the section index
      for (auto const & elem : section_db ) {
         std::string temp = elem.first;
         temp.erase(temp.end()-1);
         temp.erase(temp.begin());
         std::cout << temp << ' ' << elem.second << '\n';
      }
#endif
 
      auto const old_issues = lwg::read_issue_metadata_from_toc(path + "meta-data/lwg-toc.old.html");

      auto const issues_path = path + "xml/";

      lwg::mailing_info lwg_issues_xml = [&issues_path](){
         std::string filename{issues_path + "lwg-issues.xml"};
         std::ifstream infile{filename};
         if (!infile.is_open()) {
            throw std::runtime_error{"Unable to open " + filename};
         }

         return lwg::mailing_info{infile};
      }();

      //lwg::mailing_info lwg_issues_xml{issues_path};


      std::cout << "Reading issues from: " << issues_path << std::endl;
      auto issues = read_issues(issues_path, section_db);
      prepare_issues(issues, section_db);

      lwg::report_generator generator{lwg_issues_xml, section_db, issues};

      // issues must be sorted by number before making the mailing list documents
      //sort(issues.begin(), issues.end(), order_by_issue_number{});

      // Collect a report on all issues that have changed status
      // This will be added to the revision history of the 3 standard documents
      std::ostringstream os_diff_report;
      print_current_revisions(os_diff_report, old_issues, issues);
      auto const diff_report = os_diff_report.str();

      std::vector<lwg::issue> unresolved_issues;
      std::vector<lwg::issue> votable_issues;

      std::copy_if(issues.begin(), issues.end(), std::back_inserter(unresolved_issues),
                   [](lwg::issue const &iss) { return lwg::is_not_resolved(iss.stat); });
      std::copy_if(issues.begin(), issues.end(), std::back_inserter(votable_issues),
                   [](lwg::issue const &iss) { return lwg::is_votable(iss.stat); });

      // If votable list is empty, we are between meetings and should list Ready issues instead
      // Otherwise, issues moved to Ready during a meeting will remain 'unresolved' by that meeting
      auto ready_inserter = votable_issues.empty()
                            ? std::back_inserter(votable_issues)
                            : std::back_inserter(unresolved_issues);
      std::copy_if(issues.begin(), issues.end(), ready_inserter,
                   [](lwg::issue const &iss) { return lwg::is_ready(iss.stat); });

      // First generate the primary 3 standard issues lists
      generator.make_active(target_path, diff_report);
      generator.make_defect(target_path, diff_report);
      generator.make_closed(target_path, diff_report);

      // unofficial documents.
      // These are all subclassifications of active issues.
      generator.make_tentative(target_path);
      generator.make_unresolved(target_path);
      generator.make_immediate(target_path);
      generator.make_ready(target_path);
      generator.make_editors_issues(target_path);

      // individual issues
      for (const auto &iss : issues) {
          generator.make_individual_issue(iss, target_path);
      }

      lwg::index_generator toc_generator{lwg_issues_xml, section_db};
      // Now we have a parsed and formatted set of issues, we can write the standard set of HTML documents
      // Note that each of these functions is going to re-sort the 'issues' vector for its own purposes

      toc_generator.make_sort_by_num            (issues, {target_path + "lwg-toc.html"});
      toc_generator.make_sort_by_status         (issues, {target_path + "lwg-status.html"});
      toc_generator.make_sort_by_status_mod_date(issues, {target_path + "lwg-status-date.html"});  // this report is useless, as git checkouts touch filestamps
      toc_generator.make_sort_by_section        (issues, {target_path + "lwg-index.html"});

      // Note that this additional document is very similar to unresolved-index.html below
      toc_generator.make_sort_by_section        (issues, {target_path + "lwg-index-open.html"}, true);

      // Make a similar set of index documents for the issues that are 'live' during a meeting
      // Note that these documents want to reference each other, rather than lwg- equivalents,
      // although it may not be worth attempting fix-ups as the per-issue level
      // During meetings, it would be good to list newly-Ready issues here

      toc_generator.make_sort_by_num            (unresolved_issues, {target_path + "unresolved-toc.html"});
      toc_generator.make_sort_by_status         (unresolved_issues, {target_path + "unresolved-status.html"});
      toc_generator.make_sort_by_status_mod_date(unresolved_issues, {target_path + "unresolved-status-date.html"});
      toc_generator.make_sort_by_section        (unresolved_issues, {target_path + "unresolved-index.html"});
      toc_generator.make_sort_by_priority       (unresolved_issues, {target_path + "unresolved-prioritized.html"});

      // Make another set of index documents for the issues that are up for a vote during a meeting
      // Note that these documents want to reference each other, rather than lwg- equivalents,
      // although it may not be worth attempting fix-ups as the per-issue level
      // Between meetings, it would be good to list Ready issues here
      toc_generator.make_sort_by_num            (votable_issues, {target_path + "votable-toc.html"});
      toc_generator.make_sort_by_status         (votable_issues, {target_path + "votable-status.html"});
      toc_generator.make_sort_by_status_mod_date(votable_issues, {target_path + "votable-status-date.html"});
      toc_generator.make_sort_by_section        (votable_issues, {target_path + "votable-index.html"});

      std::cout << "Made all documents\n";
   }
   catch(std::exception const & ex) {
      std::cout << ex.what() << std::endl;
      return 1;
   }
}
