#!/bin/sh
while read issue_number           
do           
    bin/set_status $issue_number "$@"

# To add a comment use the following (changing the date and text!)
#    sed -i '/<\/discussion/i\
#<note>Tokyo 2024-03-23; Status changed: Voting &rarr; WP.</note>
#' xml/issue$issue_number.xml

done
