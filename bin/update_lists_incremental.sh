#!/bin/sh
# WARNING This batch file assumes a checkout of the gh-pages branch,
# in a directory named issues-gh-pages, at the same level as the master branch checkout
pushd ../issues-gh-pages
git pull
popd

if [ ! -f mailing/commit ] || [ ! -f mailing/updated_files.txt ]; then
   echo ERROR: missing mailing/commit or mailing/updated_files.txt
   exit 1
fi

if cmp -s ../issues-gh-pages/commit mailing/commit.prev; then
   # matching commits, just copy the specified files

   UPDATE_FILE="$(pwd)/mailing/updated_files.txt"
   while read fn; do
       cp mailing/"$(basename $fn)" ../issues-gh-pages
   done < "$UPDATE_FILE"

   cp mailing/commit ../issues-gh-pages

   pushd ../issues-gh-pages
   while read fn; do
       git add "$(basename $fn)"
   done < "$UPDATE_FILE"
   git add commit
   git commit -m"Update"
   git push  "origin" gh-pages:gh-pages
   popd
else
   echo WARNING: Commit mismatch
   echo gh-pages branch is generated from "$(cat ../issues-gh-pages/commit)"
   echo update list is based on changes from commit "$(cat mailing/commit.prev)"
   echo Copying everything. 

   cp -f mailing/lwg-*.html ../issues-gh-pages
   cp -f mailing/unresolved-*.html ../issues-gh-pages
   cp -f mailing/votable-*.html ../issues-gh-pages
   cp -f mailing/issue*.html ../issues-gh-pages
   cp -f mailing/commit ../issues-gh-pages
   pushd ../issues-gh-pages
   git add issue*.html commit lwg-*.html unresolved-*.html votable-*.html 
   git commit -m"Update"
   git push  "origin" gh-pages:gh-pages
   
fi
