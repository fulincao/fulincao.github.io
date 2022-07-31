---
title: Jekyll博客环境搭建
date: 2022-07-22
categories: [环境问题]
tags: [jekyll]     # TAG names should always be lowercase
---

- 安装ruby
  1. sudo apt update
  2. sudo apt install build-essential
  3. sudo apt install ruby ruby-dev

- 安装 jekyll
  1. gem sources --add http://gems.ruby-china.com/ --remove https://rubygems.org/ 
  2. gem -u
  3. gem sources -l
  4. sudo gem install jekyll
  5. sudo gem install bundler
  6. cd `BLOG_DIR` && bundle install
  7. build jekyll
    - bundle exec jekyll s # bundle 执行
    - bundle exec jekyll b -d "deploy" --config _config.yml && cd deploy && jekyll s # 发布到某个目录再执行
