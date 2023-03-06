#!/bin/bash
# Send email for LWG issue prioritization poll.
# To batch send use a command like:
# git pull
# for i in `seq 3858 3864` ; do bin/priority_poll $i || break ; sleep 2 ; done

To="C++ Library Working Group <lib@lists.isocpp.org>"
From=${EMAIL}

usage()
{
  echo "Usage: $0 [--help] [--to=<address>] [--from=<address>] ISSUE-NUMBER" ;
  exit $1
}


while [[ "$1" == -* ]]
do
  case "$1" in
    --help | -h) usage ;;
    --to=*) To="${1#--to=}" ;;
    --to) To="$2" ; shift ;;
    --from=*) From="${1#--from=}" ;;
    --from) From="$2" ; shift ;;
    *) echo "$0: Invalid argument: $1" >&2 ; exit 1 ;;
  esac
  shift
done

[ $# -eq 1 ] || usage 1 >&2

: ${From:?'address not set, use --from or $EMAIL'}

issue=$1
xml=$(printf "xml/issue%04d.xml" $issue)
if [ ! -f $xml ]
then
  echo "$0: No such issue: $issue" >&2
  exit 1
fi

if [ $(xpath -q -e '/issue/priority/text()' $xml) != 99 ]
then
  echo "$0: Priority already set: $issue" >&2
  exit 1
elif [ "$(xpath -q -e '/issue/@status' $xml)" != ' status="New"' ]
then
  echo "$0: Status is not \"New\": $issue" >&2
  exit 1
fi

# Convert HTML to plain text (specifically, replace entities).
# html2txt() { w3m -dump -T text/html -cols 1000 ; }
# title=$(xpath -q -s '' -e '/issue/title//text()' $xml | html2txt)

# Use xpath to select text content of <title> element, with entities replaced.
title=$(xpath -q -e 'normalize-space(/issue/title)' $xml)

draft=`mktemp /tmp/draft.prio.mail.XXXXXX` || exit

cat > $draft <<EOT
From: $From
Subject: Issue $issue: $title

It's Monday, so it must be bug prioritization time!
Thanks to everyone who participates.

https://cplusplus.github.io/LWG/issue$issue

Priority levels:
P0 - Has a proposed resolution and that resolution is clearly correct. Requires unanimity among the people doing prioritization. Move the issue to "Tentatively Ready" or "Ready" (whichever is appropriate for the group doing the review); we don't want to spend any more time discussing this issue. [Shortened: Approve, and move on.]
P1 - Showstopper bug; don't ship a standard w/o resolving this.
P2 - Important bug.
P3 - Normal bug.
P4 - Less important bug.
NAD - Not A Defect. If this has consensus, move the issue to "Tentatively NAD" and close as NAD after the next plenary.

The most common priority should be P3.

Please add your comments to this thread. Comments which are insightful or summarize the group's consensus might be added to the issue, without attribution, possibly paraphrased. If you would prefer not to have your comments shared in public, please make that clear.

EOT

mutt -H $draft -- "$To" </dev/null || exit 1
echo "Sent: Issue $issue: $title"

rm $draft
