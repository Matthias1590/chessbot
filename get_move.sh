#!/bin/bash

STOCKFISH_PATH="./stockfish/src/stockfish"  # Set this to your Stockfish binary
FEN="$1"

if [ -z "$FEN" ]; then
  echo "Usage: $0 \"<FEN>\""
  exit 1
fi

{
  echo "uci"
  sleep 0.1
  echo "position fen $FEN"
  sleep 0.1
  echo "go depth 20"
  sleep 0.1
  echo "quit"
} | "$STOCKFISH_PATH" | grep -m 1 "bestmove" | awk '{print $2}'
