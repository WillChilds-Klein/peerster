#!/bin/bash

DIR=$(pwd)
ALT='../peerster-alt3'

rm $ALT/*

cp *.cc $ALT && cp *.hh $ALT
cp *.txt $ALT
cp peerster.pro $ALT
cp Makefile $ALT
cp build.sh $ALT
cp clean-build.sh $ALT

cd $ALT && ./clean-build.sh && ./peerster

cd $DIR
