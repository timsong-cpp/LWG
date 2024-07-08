#!/bin/sh

# Sweep all issues from Voting -> WP

if [ -z "$1" ]
then
  echo "Usage: $0 '<Meeting Name> YYYY-MM-DD'" >&2
  exit 1
fi

date=$(date +%F)
meeting=$1

# We could use awk -F'"' '/status="voting"/ { print $2}' xml/issue*.xml here,
# but the wildcard matches thousands of files, so let git grep loop over them.
git grep 'status="Voting"' xml | awk -F'"' '{print $2}' | while read inum
do
  bin/set_status $inum WP
  sed -i "s/^<note>$date /<note>$meeting; /" xml/issue$inum.xml
done
