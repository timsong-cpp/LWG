#!/bin/sh

# Read issue numbers from standard input (one per line) and update their status
# to the first positional argument given to this script.
# e.g. printf "123\n456\n" | bin/update_status.sh WP

# The set_status command will add a <note> when making a "major" status change
# i.e. when moving it from one of the open/defects/closed lists to another.

# When adding a <note>, any arguments after the first will be used by the
# `set_status` executable as an optional description in the <note>, e.g.
# echo 123 | bin/update_status.sh NAD "LWG telecon"
# will add: <note>YYYY-MM-DD LWG telecon. Status changed: New -> NAD</note>

# To add a note manually (e.g. because the status change isn't "major")
# use the bin/add_note.sh script.

while read issue_number           
do           
  bin/set_status $issue_number "$@"

  # To add a custom comment use something like the following
  # (after changing the text!)
  # bin/add_note.sh $issue_number --prefix=Croydon "Status changed: New &rarr; Immediate"
done
