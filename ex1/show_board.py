# show_board.py

import chess
import sys

board = chess.Board()

# Get a list of moves
moves = sys.argv[1].split()
for move in moves:
    board.push_uci(move)

print(board)