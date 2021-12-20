#/bin/bash
echo "bundle exec ..."

origin_url=`git remote get-url origin`
pages_branch='gh-pages'

cd deploy
git checkout $pages_branch
git rm -rf *

cd ..
find src/ -name "*.md" | xargs -i cp {} _posts/
bundle exec jekyll b -d "deploy" --config _config.yml

cd deploy
git add -A
date +%Y-%m-%d-%H-%M-%S | xargs git commit -m
git push -f $origin_url $pages_branch

cd ..
git add .
git commit -m 'update'
git push origin master
