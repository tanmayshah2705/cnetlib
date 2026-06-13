#!/bin/bash

# Create temp folder and copy html into it
mkdir -p /tmp/cnetlib-docs/
cp -r docs/html/. /tmp/cnetlib-docs/

# Switch to gh-pages
git checkout gh-pages

# Remove old docs
git rm -rf .
git clean -fdx

# Copy new docs
cp -r /tmp/cnetlib-docs/. .

# Commit and push
git add .
git commit -m "Update documentation"
git push origin gh-pages

# Go back to main
git checkout main

# Cleanup temp folder
rm -rf /tmp/cnetlib-docs/

echo "Documentation changes updated on github"
