#!/bin/sh
while read issue_number           
do           
    bin/set_status $issue_number "$@"
done
