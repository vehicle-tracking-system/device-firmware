#!/bin/bash
cd /home/dizider/Documents/edu/BP/impl/a/device-front && \
#  rm -rf data/w/css data/w/f data/w/imgs data/w/js data/w/webfonts data/w/index.html
#  cd front
#  rm -rf dist && \
  yarn build && \
  cd dist && \

  cd js && \
  rm *.map && \
  cd .. && \

#  cd fonts && \
#  for f in *.woff; do mv "$f" "$(echo "$f" | sed -E 's/roboto-latin-[[:digit:]]{3}/ro/')"; done && \
#  cd .. && \
#  mv fonts f && \
#  sed -i -E 's/..\/fonts\/roboto-latin-[[:digit:]]{3}/\/f\/ro/g' css/*.css && \

  gzip -9 css/* && \
#  gzip -9 f/* && \
#  gzip -9 webfonts/* && \
  gzip -9 js/* && \
#  rm **/*LICENSE* && \

  cd /home/dizider/Documents/edu/BP/impl/a/device-front && \
  cp -r dist/* /home/dizider/CLionProjects/untitled/data/w && \
  du -b /home/dizider/CLionProjects/untitled/data/* && \
  read -p "Press any key to upload (or kill me)..." && \
  pio run --target uploadfs