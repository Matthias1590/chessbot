#!/bin/bash

{
	echo "position startpos moves d2d4"
	sleep 0.1
	echo "isready"
	sleep 0.1
	echo "readyok"
	sleep 0.1
	echo "position startpos moves d2d4"
	sleep 0.1
	echo "go wtime 300000 btime 300000 movestogo 40"
	sleep 0.1
	echo "bestmove e7e6"
	sleep 0.1
	echo "position startpos moves d2d4 e7e6 e2e4"
	sleep 0.1
	echo "go wtime 296183 btime 298154 movestogo 39"
	sleep 0.1
	echo "bestmove g8f6"
	sleep 0.1
	echo "position startpos moves d2d4 e7e6 e2e4 g8f6 e4e5"
	sleep 0.1
	echo "go wtime 292824 btime 291146 movestogo 38"
	sleep 0.1
	echo "bestmove f6d5"
	sleep 0.1
	echo "position startpos moves d2d4 e7e6 e2e4 g8f6 e4e5 f6d5 g1f3"
	sleep 0.1
	echo "go wtime 283537 btime 285578 movestogo 37"
	sleep 0.1
} | valgrind ./chess
