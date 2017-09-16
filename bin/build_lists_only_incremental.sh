#!/bin/sh
if output=$(git status --porcelain) && [ -z "$output" ]; then
    # Working directory clean
    git rev-parse HEAD > mailing/commit.new
else
    echo Working directory is dirty! Exiting.
    echo For incremental builds to reliably work, please commit your changes.
    exit 1
fi

if [ -f mailing/commit ]; then 
   git diff --name-only "$(cat mailing/commit)" > mailing/changed_files.txt
fi

bin/lists

if [ -f mailing/lwg-active.html ]; then
    test -f mailing/commit && cp mailing/commit mailing/commit.prev
    mv mailing/commit.new mailing/commit
else
    rm -f mailing/changed_files.txt mailing/updated_files.txt
    echo ***********************************
    echo ********** build failure **********
    echo ***********************************
    exit 1
fi
