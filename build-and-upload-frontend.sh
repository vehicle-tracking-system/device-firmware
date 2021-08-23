#!/bin/bash

# FIRMWARE_PATH -> path to repositiory with tracker firmware
# FRONT_PATH -> path to repositiory with tracker UI

while getopts u:f: flag
do
    case "${flag}" in
        f) FIRMWARE_PATH=${OPTARG};;
        u) FRONT_PATH=${OPTARG};;
    esac
done

cd $FRONT_PATH && \
yarn build && \
cd dist && \

cd js && \
rm *.map && \
cd .. && \

gzip -9 css/* && \
gzip -9 js/* && \

cd $FRONT_PATH && \

FIRMWARE_PATH_DATA=$FIRMWARE_PATH/data/w

if ! [[ -d "$FIRMWARE_PATH_DATA" ]]; then
`mkdir -p $FIRMWARE_PATH_DATA`;
echo "$FIRMWARE_PATH_DATA directory is created"
fi

cp -r dist/* $FIRMWARE_PATH/data/w && \
du -b $FIRMWARE_PATH/data/* && \
read -p "Press any key to upload (or kill me)..." && \
pio run --target uploadfs
