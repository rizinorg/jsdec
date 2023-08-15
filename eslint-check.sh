#!/bin/sh

find js/ -type f -name "*.js" | xargs eslint
