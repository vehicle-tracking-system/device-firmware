#!/bin/bash
cd /home/dizider/Documents/edu/BP/impl/a/device-front && \
yarn build && \
cd dist && \

cd js && \
rm *.map && \
cd .. && \

gzip -9 css/* && \
gzip -9 js/* && \

cd /home/dizider/Documents/edu/BP/impl/a/device-front && \
cp -r dist/* /home/dizider/CLionProjects/untitled/data/w && \
du -b /home/dizider/CLionProjects/untitled/data/* && \
read -p "Press any key to upload (or kill me)..." && \
pio run --target uploadfs
