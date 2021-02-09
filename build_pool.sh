#!/bin/sh
flag="-pthread -std=c++11 -g"
g++ $flag example.cpp my_thread.cpp -o example_pool
g++ $flag ./serial_example.cpp ./my_thread.cpp -o example_serial
g++ $flag ./timer_example.cpp ./my_thread.cpp -o example_timer
