#ifndef INCLUDE_LWG_STATUS_H
#define INCLUDE_LWG_STATUS_H

// standard headers
#include <string>
#include <cstddef>

namespace lwg
{
auto filename_for_status(std::string stat) -> std::string;

auto get_status_priority(std::string const & stat) noexcept -> std::ptrdiff_t;

// this predicate API should probably switch to 'std::experimental::string_view'
auto is_active(std::string const & stat) -> bool;
auto is_active_not_ready(std::string const & stat) -> bool;
auto is_defect(std::string const & stat) -> bool;
auto is_closed(std::string const & stat) -> bool;
auto is_tentative(std::string const & stat) -> bool;
auto is_not_resolved(std::string const & stat) -> bool;
auto is_assigned_to_another_group(std::string const & stat) -> bool;
auto is_votable(std::string stat) -> bool;
auto is_ready(std::string stat) -> bool;

// Functions to "normalize" a status string
// Might profitable switch to 'experimental/string_view'
auto remove_pending(std::string stat) -> std::string;
auto remove_tentatively(std::string stat) -> std::string;
auto remove_qualifier(std::string const & stat) -> std::string;


} // close namespace lwg

#endif // INCLUDE_LWG_STATUS_H
