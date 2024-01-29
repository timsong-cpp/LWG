#!/usr/bin/python

import json
import sys
import re

latest_rev = {}

with open(sys.argv[1]) as f:
    docs = json.load(f)
    for num,data in docs.items():
        m = re.fullmatch(r'(P\d\d\d\d)R(\d+)|N\d\d\d\d', num)
        if m:
            print("{} {}".format(num, data['title']))
            if num.startswith('P'):
                pnum = m[1]
                rev = int(m[2])
                if rev > latest_rev.get(pnum, -1):
                    latest_rev[pnum] = rev
    for pnum, rev in latest_rev.items():
        pnumrev = "{}R{}".format(pnum, rev)
        print("{} {}".format(pnum, docs[pnumrev]['title']))
