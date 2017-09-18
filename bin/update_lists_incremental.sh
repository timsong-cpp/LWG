#!/bin/bash
# WARNING This batch file assumes a checkout of the gh-pages branch,
# in a directory named issues-gh-pages, at the same level as the master branch checkout
if [ ! -f mailing/commit ] || [ ! -f mailing/updated_files.txt ]; then
   echo ERROR: missing mailing/commit or mailing/updated_files.txt
   echo Maybe you built the lists with a dirty working directory?
   exit 1
fi

pushd ../issues-gh-pages
git pull
popd

if cmp -s ../issues-gh-pages/commit mailing/commit.prev; then
   # matching commits, just copy the specified files

   cat mailing/updated_files.txt | sed 's/\r//' | xargs cp -t ../issues-gh-pages

   cp mailing/commit mailing/updated_files.txt ../issues-gh-pages

   pushd ../issues-gh-pages

   cat updated_files.txt | sed 's!.*/!!' | xargs git add
   rm updated_files.txt
   git add commit
   git commit -m"Update"
   git push  "origin" gh-pages:gh-pages
   popd
   mv mailing/commit mailing/commit.prev
   cp mailing/lwg-toc.html mailing/lwg-toc.prev.html
else
   echo WARNING: Commit mismatch
   echo gh-pages branch is generated from "$(cat ../issues-gh-pages/commit)"
   echo update list is based on changes from commit "$(cat mailing/commit.prev)"
   echo COPYING EVERYTHING

   cp -f mailing/lwg-*.html ../issues-gh-pages
   cp -f mailing/unresolved-*.html ../issues-gh-pages
   cp -f mailing/votable-*.html ../issues-gh-pages
   cp -f mailing/issue*.html ../issues-gh-pages
   cp -f mailing/commit ../issues-gh-pages
   pushd ../issues-gh-pages
   git add issue*.html commit lwg-*.html unresolved-*.html votable-*.html
   git commit -m"Update"
   git push  "origin" gh-pages:gh-pages
   popd
   mv mailing/commit mailing/commit.prev
   cp mailing/lwg-toc.html mailing/lwg-toc.prev.html
fi
