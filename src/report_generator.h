#ifndef INCLUDE_LWG_REPORT_GENERATOR_H
#define INCLUDE_LWG_REPORT_GENERATOR_H

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <cassert>
#include <functional>
#include "issues.h"  // cannot forward declare the 'section_map' alias, nor the 'LwgIssuesXml' alias
#include <algorithm>

namespace lwg
{

struct issue;
struct mailing_info;

struct report_generator {

   report_generator(mailing_info const & info, section_map & sections, std::vector<issue> const& issues)
      : lwg_issues_xml(info)
      , section_db(sections)
      , issues(issues)
   {
      assert(std::is_sorted(issues.begin(), issues.end(), lwg::order_by_issue_number{}));
      for(const auto& i : issues){
         ++ status_count[i.stat];
         auto tag_string = as_string(i.tags.front());
         auto& tag_cnt = tag_count[tag_string];
         ++ tag_cnt.all;
         if(lwg::is_active(i.stat))
            ++ tag_cnt.active;
         issues_by_doc[i.doc_prefix].push_back(i);
      }
   }

   // Functions to make the 3 standard published issues list documents
   // A precondition for calling any of these functions is that the list of issues is sorted in numerical order, by issue number.
   // While nothing disasterous will happen if this precondition is violated, the published issues list will list items
   // in the wrong order.
   std::string make_active(std::string const & path, std::string const & diff_report) const;
   std::string make_defect(std::string const & path, std::string const & diff_report) const;
   std::string make_closed(std::string const & path, std::string const & diff_report) const;

   // Additional non-standard documents, useful for running LWG meetings
   std::string make_tentative(std::string const & path) const;
      // publish a document listing all tentative issues that may be acted on during a meeting.


   std::string make_unresolved(std::string const & path) const;
      // publish a document listing all non-tentative, non-ready issues that must be reviewed during a meeting.

   std::vector<std::string> make_immediate(std::string const & path) const;
      // publish a document listing all non-tentative, non-ready issues that must be reviewed during a meeting.

   std::vector<std::string> make_ready(std::string const & path) const;
      // publish a document listing all ready issues for a formal vote

   std::vector<std::string> make_editors_issues(std::string const & path) const;

   std::string make_individual_issue(lwg::issue const& iss, std::string const & path) const;

   int count_all_issues_with_tag(std::string const& tag) const { return tag_count.at(tag).all; }

   int count_issues_with_status(std::string const & stat) const { return status_count.at(stat); }

   int count_active_issues_with_tag(std::string const& tag) const {
       return tag_count.at(tag).active;
   }

private:
   enum class print_issue_type { in_list, individual };
   template <typename Pred>
   void print_issues(std::ostream & out, Pred pred) const;

   void print_issue(std::ostream & out, lwg::issue const & iss, print_issue_type type = print_issue_type::in_list) const;

   template<typename Iterator, typename Pred, typename F>
   void make_report(const std::string& filename, Iterator first, Iterator last, Pred pred, F header_printer) const;

   mailing_info const & lwg_issues_xml;
   section_map &        section_db;
   std::vector<issue> const& issues;

   struct issue_count {
      int all = 0;
      int active = 0;
   };

   std::unordered_map<std::string, issue_count> tag_count;
   std::unordered_map<std::string, int> status_count;
   std::unordered_map<std::string, std::vector<std::reference_wrapper<const issue>>> issues_by_doc;
};

struct index_generator {

  index_generator(mailing_info const & info, section_map & sections)
          : lwg_issues_xml(info)
          , section_db(sections)
  {
  }
  void make_sort_by_num(std::vector<issue>& issues, std::string const & filename);

  void make_sort_by_priority(std::vector<issue>& issues, std::string const & filename);

  void make_sort_by_status(std::vector<issue>& issues, std::string const & filename);

  void make_sort_by_status_mod_date(std::vector<issue> & issues, std::string const & filename);

  void make_sort_by_section(std::vector<issue>& issues, std::string const & filename, bool active_only = false);

private:
  mailing_info const & lwg_issues_xml;
  section_map &        section_db;
};

} // close namespace lwg

#endif // INCLUDE_LWG_REPORT_GENERATOR_H
