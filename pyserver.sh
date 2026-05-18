#!/bin/sh

set -xe

nohup python -m http.server 6969 > server.log 2>&1 &
