#!/usr/bin/python

import json
import sys
import re

with open(sys.argv[1]) as f:
    for num,data in json.load(f).items():
        if re.match(r'^(P\d\d\d\dR\d+|N\d\d\d\d)$', num):
            print("{} {}".format(num, data['title']))
