#ifdef _MSC_VER
# define _CRT_SECURE_NO_WARNINGS
#endif

#include "status.h"

#include <string>
#include <iostream>  // eases debugging
#include <algorithm>

namespace {
static constexpr char const * LWG_ACTIVE {"lwg-active.html" };
static constexpr char const * LWG_CLOSED {"lwg-closed.html" };
static constexpr char const * LWG_DEFECTS{"lwg-defects.html"};
}

auto lwg::filename_for_status(std::string stat) -> std::string {
   // Tentative issues are always active
   if(0 == stat.find("Tentatively")) {
      return LWG_ACTIVE;
   }

   stat = remove_qualifier(stat);
   return (stat == "TC1")           ? LWG_DEFECTS
        : (stat == "CD1")           ? LWG_DEFECTS
        : (stat == "TS")            ? LWG_DEFECTS
        : (stat == "C++11")         ? LWG_DEFECTS
        : (stat == "C++14")         ? LWG_DEFECTS
        : (stat == "C++17")         ? LWG_DEFECTS
        : (stat == "C++20")         ? LWG_DEFECTS
        : (stat == "WP")            ? LWG_DEFECTS
        : (stat == "Resolved")      ? LWG_DEFECTS
        : (stat == "DR")            ? LWG_DEFECTS
        : (stat == "TRDec")         ? LWG_DEFECTS
        : (stat == "Dup")           ? LWG_CLOSED
        : (stat == "NAD")           ? LWG_CLOSED
        : (stat == "NAD Future")    ? LWG_CLOSED
        : (stat == "NAD Editorial") ? LWG_CLOSED
        : (stat == "NAD Concepts")  ? LWG_CLOSED
        : (stat == "NAD Arrays")    ? LWG_CLOSED
        : (stat == "Voting")        ? LWG_ACTIVE
        : (stat == "Immediate")     ? LWG_ACTIVE
        : (stat == "Ready")         ? LWG_ACTIVE
        : (stat == "Review")        ? LWG_ACTIVE
        : (stat == "New")           ? LWG_ACTIVE
        : (stat == "Open")          ? LWG_ACTIVE
        : (stat == "EWG")           ? LWG_ACTIVE
        : (stat == "LEWG")          ? LWG_ACTIVE
        : (stat == "Core")          ? LWG_ACTIVE
        : (stat == "SG1")           ? LWG_ACTIVE
        : (stat == "Deferred")      ? LWG_ACTIVE
        : throw std::runtime_error("unknown status " + stat);
}

auto lwg::is_active(std::string const & stat) -> bool {
   return filename_for_status(stat) == LWG_ACTIVE;
}

auto lwg::is_active_not_ready(std::string const & stat) -> bool {
   return is_active(stat)  and  stat != "Ready";
}

auto lwg::is_defect(std::string const & stat) -> bool {
   return filename_for_status(stat) == LWG_DEFECTS;
}

auto lwg::is_closed(std::string const & stat) -> bool {
   return filename_for_status(stat) == LWG_CLOSED;
}

auto lwg::is_tentative(std::string const & stat) -> bool {
   // a more efficient implementation will use some variation of strcmp
   return 0 == stat.find("Tentatively");
}

auto lwg::is_assigned_to_another_group(std::string const & stat) -> bool {
   for( auto s : {"Core", "EWG", "LEWG", "SG1" }) { if(s == stat) return true; }
   return false;
}

auto lwg::is_not_resolved(std::string const & stat) -> bool {
   if (is_assigned_to_another_group(stat)) return true;
   for( auto s : {"Deferred", "New", "Open", "Review"}) { if(s == stat) return true; }
   return false;
}

auto lwg::is_votable(std::string stat) -> bool {
   stat = remove_tentatively(stat);
   for( auto s : {"Immediate", "Voting"}) { if(s == stat) return true; }
   return false;
}

auto lwg::is_ready(std::string stat) -> bool {
   return "Ready" == remove_tentatively(stat);
}

// Functions to "normalize" a status string
auto lwg::remove_pending(std::string stat) -> std::string {
   using size_type = std::string::size_type;
   if(0 == stat.find("Pending")) {
      stat.erase(size_type{0}, size_type{8});
   }
   return stat;
}

auto lwg::remove_tentatively(std::string stat) -> std::string {
   using size_type = std::string::size_type;
   if(0 == stat.find("Tentatively")) {
      stat.erase(size_type{0}, size_type{12});
   }
   return stat;
}

auto lwg::remove_qualifier(std::string const & stat) -> std::string {
   return remove_tentatively(remove_pending(stat));
}

auto lwg::get_status_priority(std::string const & stat) noexcept -> std::ptrdiff_t {
   static char const * const status_priority[] {
      "Voting",
      "Tentatively Voting",
      "Immediate",
      "Ready",
      "Tentatively Ready",
      "Tentatively NAD Editorial",
      "Tentatively NAD Future",
      "Tentatively NAD",
      "Review",
      "New",
      "Open",
      "LEWG",
      "EWG",
      "Core",
      "SG1",
      "Deferred",
      "Tentatively Resolved",
      "Pending DR",
      "Pending WP",
      "Pending Resolved",
      "Pending NAD Future",
      "Pending NAD Editorial",
      "Pending NAD",
      "NAD Future",
      "DR",
      "WP",
      "C++20",
      "C++17",
      "C++14",
      "C++11",
      "CD1",
      "TC1",
      "Resolved",
      "TRDec",
      "NAD Editorial",
      "NAD",
      "Dup",
      "NAD Concepts"
   };


#if !defined(DEBUG_SUPPORT)
   static auto const first = std::begin(status_priority);
   static auto const last  = std::end(status_priority);
   return std::find_if( first, last, [&](char const * str){ return str == stat; } ) - first;
#else
   // Diagnose when unknown status strings are passed
   static auto const first = std::begin(status_priority);
   static auto const last  = std::end(status_priority);
   auto const i = std::find_if( first, last, [&](char const * str){ return str == stat; } );
   if(last == i) {
      std::cout << "Unknown status: " << stat << std::endl;
   }
   return i - first;
#endif
}
