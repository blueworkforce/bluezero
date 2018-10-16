#!/bin/sh
DEST=v1
set -x
set -e
git checkout gh-pages
git fetch --all
git reset --hard origin/gh-pages
rm -rf ${DEST} && mkdir ${DEST}
cp -r docs/* ${DEST}/
git add .
git commit --amend -m 'update docs'
git push -f
git checkout master
