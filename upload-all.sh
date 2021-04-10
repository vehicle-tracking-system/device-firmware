#!/bin/bash

pio run -t uploadfs && \
pio run -t upload && \
pio device monitor
