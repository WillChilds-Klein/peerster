#!/bin/bash

make clean &&
(qmake-qt4 || qmake) && 
make clean && 
make && 
./peerster
