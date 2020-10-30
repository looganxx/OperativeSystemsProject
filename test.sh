#!/bin/bash

for letter in {{a..z},{A..X}}; do
  ./client $letter 1 &
done

wait
killall -SIGUSR1 objserver

for letter in {{a..z},{A..D}}; do
  ./client $letter 2 &
done

killall -SIGUSR1 objserver

for letter in {E..X}; do
  ./client $letter 3 &
done

wait
killall -SIGUSR1 objserver