import os
import sys
import chess.pgn
import io

def contains_en_passant_or_castling(file_path):
    with open(file_path, 'r') as file:
        pgn = file.read()
    
    pgn_io = io.StringIO(pgn)
    game = chess.pgn.read_game(pgn_io)

    if game is None:
        return False

    for move in game.mainline_moves():
        uci_move = move.uci()
        # Check for castling moves
        if uci_move in ["e1g1", "e1c1", "e8g8", "e8c8"]:
            return True
        # Check for en passant moves
        from_square = chess.parse_square(uci_move[:2])
        to_square = chess.parse_square(uci_move[2:4])
        piece = game.board().piece_at(from_square)
        if piece and piece.piece_type == chess.PAWN and abs(chess.square_file(from_square) - chess.square_file(to_square)) == 1 and game.board().piece_at(to_square) is None:
            return True
    return False

def delete_files_with_en_passant_or_castling(folder_path):
    for file_name in os.listdir(folder_path):
        if file_name.endswith('.pgn'):
            file_path = os.path.join(folder_path, file_name)
            if contains_en_passant_or_castling(file_path):
                os.remove(file_path)
                print(f"Deleted {file_name}")

def main():
    if len(sys.argv) != 2:
        print("Usage: python delete_pgn_files.py <FOLDER_PATH>")
        return
    
    folder_path = sys.argv[1]

    if not os.path.isdir(folder_path):
        print(f"The path {folder_path} is not a directory.")
        return
    
    delete_files_with_en_passant_or_castling(folder_path)

if __name__ == "__main__":
    main()
