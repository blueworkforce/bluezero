#!/bin/sh
set -x
set -e
git checkout gh-pages
rm *.html *.png *.css *.js ; rm -rf search
cp -r docs/* .
git add .
git commit --amend -m 'update docs'
git push -f
git checkout master
