cmake_minimum_required(VERSION 3.6)
project(lwg_issues)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELWITHASSERT "-O2 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -g -DNDEBUG")

add_library(common date.cpp issues.cpp sections.cpp)
add_executable(lists mailing_info.cpp report_generator.cpp lists.cpp)
add_executable(section_data section_data.cpp)
add_executable(toc_diff toc_diff.cpp)
add_executable(set_status set_status.cpp)
add_executable(list_issues list_issues.cpp)
add_executable(test_issue_metadata test_issue_metadata.cpp issue_metadata.cpp)

target_link_libraries(lists common)
target_link_libraries(list_issues common)
