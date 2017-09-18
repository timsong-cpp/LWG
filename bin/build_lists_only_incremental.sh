#!/bin/sh
if output=$(git status --porcelain) && [ -z "$output" ]; then
    # Working directory clean
    git rev-parse HEAD > mailing/commit.new
else
    echo Working directory is dirty! Later incremental update will not work.
    rm -f mailing/commit.new mailing/commit
fi

if [ -f mailing/commit.prev ]; then
   git diff --name-only "$(cat mailing/commit.prev)" > mailing/changed_files.txt
   git ls-files -o --exclude-standard xml/ >> mailing/changed_files.txt # add any untracked files in xml/ to the list of changed files
else
   rm -f mailing/changed_files.txt
fi

bin/lists

if [ -f mailing/lwg-active.html ]; then
    test -f mailing/commit.new && mv mailing/commit.new mailing/commit
else
    rm -f mailing/changed_files.txt mailing/updated_files.txt mailing/commit.new
    echo ***********************************
    echo ********** build failure **********
    echo ***********************************
    exit 1
fi
