#!/bin/bash
sudo gcc -o $1-bin $1.c `pkg-config --libs --cflags gtk+-3.0` -export-dynamic -D _GNU_SOURCE && sudo ./$1-bin
