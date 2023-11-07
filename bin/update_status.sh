#!/bin/sh
while read issue_number           
do           
    bin/set_status $issue_number "$@"

# To add a comment use the following (changing the date and text!)
    sed -i '/<\/discussion/i\
<note>Kona 2023-11-07; move to Ready</note>\
' xml/issue$issue_number.xml

done
