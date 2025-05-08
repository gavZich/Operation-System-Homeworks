import sys
import io
import chess.pgn

def parse_moves(pgn_moves):
    # Read the game from the PGN string
    pgn_stream = io.StringIO(pgn_moves)
    game = chess.pgn.read_game(pgn_stream)
    
    if not game:
        print("Failed to parse PGN", file=sys.stderr)
        return []

    # Traverse the game to collect UCI moves
    node = game
    uci_moves = []
    while node.variations:
        next_node = node.variation(0)
        uci_moves.append(next_node.move.uci())
        node = next_node

    return uci_moves

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py 'pgn_moves'", file=sys.stderr)
        sys.exit(1)

    pgn_moves = sys.argv[1]
    uci_moves = parse_moves(pgn_moves)
    
    if uci_moves:
        print(' '.join(uci_moves))
    else:
        print("No valid moves found.", file=sys.stderr)
