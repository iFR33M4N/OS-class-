#!/bin/bash

sudo insmod redirectOpen.ko redirection="dummyfile.txt" target="hiddenfile.txt"
