#!/bin/bash
gcc detect_write.c -o detect_write -lpthread 
./detect_write 0 1 300
