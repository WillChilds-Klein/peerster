#!/bin/bash

(qmake-qt4 || qmake) && make clean && make
