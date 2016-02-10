# Extract n.n... [xx.xxxx...] from HTML headers (e.g. "<h3>1.2.3 Title [my.title]</h3>").
# See h-style-index.bat for invokation.
# TODO: Splice lines together first if headers extend across several lines
s/<h[1-9]>[^0-9]*([0-9.]*)[^[]*\[([^]]*).*/section-prefix \1 [\2]/g
s/([^<]*)<[^>]*>(.*)/\1\2/g
s/([^<]*)<[^>]*>(.*)/\1\2/g
