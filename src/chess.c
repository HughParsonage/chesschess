#include "chess.h"

#define MAX_MOVES 1023

typedef enum {
  EMPTY,
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING
} Piece;

typedef enum {
  WHITE,
  BLACK
} Color;

typedef struct {
  Piece piece;
  Color color;
} Square;

typedef struct {
  Square board[8][8];
  Color currentTurn;
  unsigned int WhiteKing : 6; // duplicately record the kings' positions
  unsigned int BlackKing : 6;
  bool WhiteMayCastle : 1;
  bool BlackMayCastle: 1;
} Chessboard;

typedef struct {
  int fromRow;
  int fromCol;
  int toRow;
  int toCol;
} Move;

void startingPosition(Chessboard* board) {
  for (int c = 0; c < 8; ++c) {
    for (int r = 0; r < 6; ++r) {
      board->board[r][c].piece = EMPTY;
      board->board[r][c].color = WHITE;
    }
  }
  for (int c = 0; c < 8; ++c) {
    board->board[1][c].piece = PAWN;
    board->board[1][c].color = WHITE;
    board->board[6][c].piece = PAWN;
    board->board[6][c].color = WHITE;
  }
  board->board[0][0].piece = ROOK;
  board->board[0][7].piece = ROOK;
  board->board[7][0].piece = ROOK;
  board->board[7][7].piece = ROOK;

  board->board[0][1].piece = KNIGHT;
  board->board[0][6].piece = KNIGHT;
  board->board[7][1].piece = KNIGHT;
  board->board[7][6].piece = KNIGHT;

  board->board[0][2].piece = BISHOP;
  board->board[0][5].piece = BISHOP;
  board->board[7][2].piece = BISHOP;
  board->board[7][5].piece = BISHOP;

  board->board[0][3].piece = QUEEN;
  board->board[7][3].piece = QUEEN;

  board->board[0][4].piece = KING;
  board->board[7][4].piece = KING;

  for (int c = 0; c < 8; ++c) {
    for (int r = 6; r < 8; ++r) {
      board->board[r][c].color = BLACK;
    }
  }
  board->WhiteKing = 4;
  board->BlackKing = 60;

  board->WhiteMayCastle = 1;
  board->BlackMayCastle = 1;
}

bool isMoveLegal(const Chessboard* board, const Move* move) {
  // Get the piece at the source square
  Square sourceSquare = board->board[move->fromRow][move->fromCol];

  // Check if the source square is empty or the piece color doesn't match the current turn
  if (sourceSquare.piece == EMPTY || sourceSquare.color != board->currentTurn) {
    return false;
  }

  // Get the piece at the destination square
  Square destinationSquare = board->board[move->toRow][move->toCol];

  // Check if the destination square contains a piece of the same color
  if (destinationSquare.piece != EMPTY && destinationSquare.color == board->currentTurn) {
    return false;
  }

  // Check if the move is valid based on the piece type
  switch (sourceSquare.piece) {
  case PAWN:
    // Determine the direction of the pawn's movement based on its color
    int direction = (sourceSquare.color == WHITE) ? 1 : -1;

    // Check if it's a standard pawn move (1 square forward)
    if (move->fromCol == move->toCol && move->toRow == move->fromRow + direction &&
        destinationSquare.piece == EMPTY) {
      return true;
    }

    // Check if it's the pawn's first move (2 squares forward)
    if (move->fromCol == move->toCol && move->toRow == move->fromRow + (2 * direction) &&
        move->fromRow == ((sourceSquare.color == WHITE) ? 1 : 6) &&
        destinationSquare.piece == EMPTY) {
      // Check if the path is clear for the pawn to move two squares forward
      int intermediateRow = move->fromRow + direction;
      if (board->board[intermediateRow][move->fromCol].piece == EMPTY) {
        return true;
      }
    }

    // Check if it's a pawn capture (diagonal move)
    if (abs(move->fromCol - move->toCol) == 1 && move->toRow == move->fromRow + direction &&
        destinationSquare.piece != EMPTY && destinationSquare.color != sourceSquare.color) {
      return true;
    }

    // Add additional checks for special pawn moves like en passant and promotion as needed

    break;
  case KNIGHT:
    // Implement knight move logic here
    // Example: Check if the move is a valid knight move based on the destination square
    break;
  case BISHOP:
    // Implement bishop move logic here
    // Example: Check if the move is a valid bishop move based on the destination square
    break;
  case ROOK:
    // Implement rook move logic here
    // Example: Check if the move is a valid rook move based on the destination square
    break;
  case QUEEN:
    // Implement queen move logic here
    // Example: Check if the move is a valid queen move based on the destination square
    break;
  case KING:
    // Implement king move logic here
    // Example: Check if the move is a valid king move based on the destination square
    break;
  default:
    return false;
  }

  // If none of the checks failed, the move is legal
  return true;
}



bool canPieceAttackSquare(const Chessboard* board, int pieceRow, int pieceCol, int toRow, int toCol) {
  Square pieceSquare = board->board[pieceRow][pieceCol];
  Piece pieceType = pieceSquare.piece;
  Color pieceColor = pieceSquare.color;

  // Check if the target square is out of bounds
  if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) {
    return false;
  }

  // Check if the piece can attack the target square based on its type
  switch (pieceType) {
  case PAWN: {
    // Determine the direction of the pawn's movement based on its color
    int direction = (pieceColor == WHITE) ? 1 : -1;

    // Check if the pawn can attack the target square diagonally
    if (toCol == pieceCol + 1 || toCol == pieceCol - 1) {
      if (toRow == pieceRow + direction) {
        return true;
      }
    }
    break;
  }

  case KNIGHT: {
    // Check if the target square is a valid knight's move away from the piece
    int rowDiff = abs(toRow - pieceRow);
    int colDiff = abs(toCol - pieceCol);
    if ((rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2)) {
      return true;
    }
    break;
  }

  case BISHOP: {
    // Check if the target square is on the same diagonal as the piece
    if (abs(toRow - pieceRow) == abs(toCol - pieceCol)) {
    // Determine the direction of movement along the diagonal
    int rowDirection = (toRow > pieceRow) ? 1 : -1;
    int colDirection = (toCol > pieceCol) ? 1 : -1;

    int row = pieceRow + rowDirection;
    int col = pieceCol + colDirection;

    // Check if there are any pieces obstructing the path
    while (row != toRow && col != toCol) {
      if (board->board[row][col].piece != EMPTY) {
        return false; // Obstruction in the path
      }
      row += rowDirection;
      col += colDirection;
    }

    return true; // No obstructions, the bishop can attack the target square
  }
    break;
  }

  case ROOK: {
    // Check if the target square is in the same row or column as the piece
    if (toRow == pieceRow || toCol == pieceCol) {
    // Determine the direction of movement along the row or column
    int rowDirection = (toRow > pieceRow) ? 1 : (toRow < pieceRow) ? -1 : 0;
    int colDirection = (toCol > pieceCol) ? 1 : (toCol < pieceCol) ? -1 : 0;

    int row = pieceRow + rowDirection;
    int col = pieceCol + colDirection;

    // Check if there are any pieces obstructing the path
    while (row != toRow || col != toCol) {
      if (board->board[row][col].piece != EMPTY) {
        return false; // Obstruction in the path
      }
      row += rowDirection;
      col += colDirection;
    }

    return true; // No obstructions, the rook can attack the target square
  }
    break;
  }

  case QUEEN: {
    // Check if the target square is on the same diagonal, row, or column as the piece
    if (toRow == pieceRow || toCol == pieceCol ||
        abs(toRow - pieceRow) == abs(toCol - pieceCol)) {
    // Determine the direction of movement
    int rowDirection = (toRow > pieceRow) ? 1 : (toRow < pieceRow) ? -1 : 0;
    int colDirection = (toCol > pieceCol) ? 1 : (toCol < pieceCol) ? -1 : 0;

    int row = pieceRow + rowDirection;
    int col = pieceCol + colDirection;

    // Check if there are any pieces obstructing the path
    while (row != toRow || col != toCol) {
      if (board->board[row][col].piece != EMPTY) {
        return false; // Obstruction in the path
      }
      row += rowDirection;
      col += colDirection;
    }

    return true; // No obstructions, the queen can attack the target square
  }
    break;
  }

  case KING: {
    // Check if the target square is adjacent to the king
    int rowDiff = abs(toRow - pieceRow);
    int colDiff = abs(toCol - pieceCol);
    if ((rowDiff == 1 && colDiff == 0) || (rowDiff == 0 && colDiff == 1) || (rowDiff == 1 && colDiff == 1)) {
      return true;
    }
    break;
  }

  default:
    break;
  }

  return false; // The piece cannot attack the target square
}

int locateKing(const Chessboard * board, Color kingColor) {
  // 0-63, normally at positions 4, 60
  int p = 0;
  if (kingColor == WHITE && board->board[0][4].piece == KING && board->board[0][4].color == WHITE) {
    return 4;
  }
  if (kingColor == BLACK && board->board[7][4].piece == KING && board->board[7][4].color == BLACK) {
    return 60;
  }
  if (kingColor == WHITE) {
    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        if (board->board[r][c].piece == KING && board->board[r][c].color == WHITE) {
          return 8 * r + c;
        }
      }
    }
    return 4; // unable to find
  }
  for (int r = 7; r >= 0; --r) {
    for (int c = 0; c < 8; ++c) {
      if (board->board[r][c].piece == KING && board->board[r][c].color == BLACK) {
        return 8 * r + c;
      }
    }
  }
  return 60;
}


bool isKingInCheck(const Chessboard* board, Color kingColor) {
  // Find the position of the king
  int kingRow = -1, kingCol = -1; // contemplate no king
  int kl = locateKing(board, kingColor);
  kingRow = kl >> 3;
  kingCol = kl & 7;

  // Check if any opponent piece can attack the king
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      Square square = board->board[row][col];

      // Check if the square contains an opponent's piece
      if (square.color != kingColor && square.piece != EMPTY) {
        // Check if the opponent's piece can attack the king's position
        if (canPieceAttackSquare(board, row, col, kingRow, kingCol)) {
          return true;
        }
      }
    }
  }

  // If no opponent piece can attack the king, it's not in check
  return false;
}

bool maybePinned(const Chessboard * board, int row, int col) {
  Chessboard tempBoard;
  memcpy(&tempBoard, board, sizeof(Chessboard));
  tempBoard.board[row][col].piece = EMPTY;
  return isKingInCheck(&tempBoard, board->board[row][col].color);
}


// Helper function to add a move to the moves array
void addMove(Move* moves, int* numMoves, int sourceRow, int sourceCol, int toRow, int toCol) {
  moves[*numMoves].fromRow = sourceRow;
  moves[*numMoves].fromCol = sourceCol;
  moves[*numMoves].toRow = toRow;
  moves[*numMoves].toCol = toCol;
  (*numMoves)++;
}

// Function to generate pawn moves
int generatePawnMoves(const Chessboard* board, int row, int col, Move* moves) {
  int numMoves = 0;

  // Determine the color of the pawn
  bool isWhite = (board->board[row][col].color == WHITE);

  // Determine the direction of movement based on the pawn's color
  int direction = isWhite ? -1 : 1;

  // Check if the pawn can move one square forward
  int toRow = row + direction;
  int toCol = col;
  if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8 && board->board[toRow][toCol].piece == EMPTY) {
    addMove(moves, &numMoves, row, col, toRow, toCol);

    // Check if the pawn can move two squares forward from the starting position
    if ((isWhite && row == 6) || (!isWhite && row == 1)) {
      toRow = row + (2 * direction);
      if (toRow >= 0 && toRow < 8 && board->board[toRow][toCol].piece == EMPTY) {
        addMove(moves, &numMoves, row, col, toRow, toCol);
      }
    }
  }

  // Check if the pawn can capture diagonally to the left
  toRow = row + direction;
  toCol = col - 1;
  if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8 && board->board[toRow][toCol].piece != EMPTY && board->board[toRow][toCol].color != board->board[row][col].color) {
    addMove(moves, &numMoves, row, col, toRow, toCol);
  }

  // Check if the pawn can capture diagonally to the right
  toRow = row + direction;
  toCol = col + 1;
  if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8 && board->board[toRow][toCol].piece != EMPTY && board->board[toRow][toCol].piece != board->board[row][col].color) {
    addMove(moves, &numMoves, row, col, toRow, toCol);
  }

  return numMoves;
}

int generateKnightMoves(const Chessboard* board, int row, int col, Move* moves) {
  int numMoves = 0;
  if (maybePinned(board, row, col)) {
    // A pinned knight cannot move
    return 0;
  }

  // think of a clockface 1,2 4,5, 7,8, 10,11 are the moves in general
  const int hrs[8] = {1, 2, 4, 5, 7, 8, 10, 11};
  const int toRows[8] = {1, 2, 2, 1, -1, -2, -2, -1};
  const int toCols[8] = {2, 1, -1, -2, -2, -1, 1, 2};
  for (int h = 0; h < 8; ++h) {
    int toRow = row + toRows[h];
    int toCol = col + toCols[h];
    if (toRow < 0 || toRow >= 8 || toCol < 0 || toRow >= 8) {
      continue; // outside the board
    }
    // same color => not possible
    if (board->board[toRow][toCol].color == board->board[row][col].color) {
      continue;
    }
    addMove(moves, &numMoves, row, col, toRow, toCol);
  }
  return numMoves;
}

int generateBishopMoves(const Chessboard* board, int row, int col, Move* moves) {
  int numMoves = 0;
  bool diags_avbl[4] = {1, 1, 1, 1}; // NE, SE, SW, NW
  const Color c = board->board[row][col].color;
  bool maybe_pinned = maybePinned(board, row, col);
  // determine the diagnonals along which the bishop is pinned
  if (maybe_pinned) {
    // if the king lies on the same row or column then the bishop is totally
    // pinned




    Chessboard tempBoard;
    memcpy(&tempBoard, board, sizeof(Chessboard));
    if (row + 1 < 8) {
      if (col + 1 < 8) {
        if (board->board[row + 1][col + 1].piece != EMPTY) {
          if (board->board[row + 1][col + 1].color == c) {
            diags_avbl[0] = 0;
          } else {
            // Capture the adjacent piece
            tempBoard.board[row + 1][col + 1].piece = BISHOP;
            tempBoard.board[row + 1][col + 1].color = c;
            tempBoard.board[row][col].piece = EMPTY;
            if (isKingInCheck(&tempBoard, c)) {
              diags_avbl[0] = 0;
              diags_avbl[2] = 0; // opposite direction must be unavailable too
            } else {
              addMove(moves, &numMoves, row, col, row + 1, col + 1);
              diags_avbl[0] = 0; //
            }
          }
        } else {
          if (isKingInCheck(&tempBoard, c)) {
            diags_avbl[0] = 0;
            diags_avbl[2] = 0; // opposite direction must be unavailable too
          } else {
            addMove(moves, &numMoves, row, col, row + 1, col + 1);
            diags_avbl[0] = 0; //
          }
        }
      } else {

      }
    }
  }

  for (int d = 1; d < 8; ++d) {
    // first test whether we have reached beyond the edges of the board
    if (row + d >= 8) {
      diags_avbl[0] = 0; // NE
      diags_avbl[1] = 0; // SE
    }
    if (row - d < 0) {
      diags_avbl[2] = 0; // SW
      diags_avbl[3] = 0; // NW
    }
    if (col + d >= 8) {
      diags_avbl[0] = 0; // NE
      diags_avbl[3] = 0; // NW
    }
    if (col - d < 0) {
      diags_avbl[1] = 0; // SE
      diags_avbl[2] = 0; // SW
    }
    // Now check each diagonal; a diag is unavbl once a piece is in the way
    // if the blocking piece is of the opposite color, it can be captured so
    // counts as an extra move, before disqualifying the diagonal.

    // NE
    if (diags_avbl[0]) {
      if (board->board[row + d][col + d].piece != EMPTY) {
        if (board->board[row + d][col + d].color != c) {
          addMove(moves, &numMoves, row, col, row + d, col + d);
        }
        diags_avbl[0] = 0;
        continue;
      }
      addMove(moves, &numMoves, row, col, row + d, col + d);
      continue;
    }
    // SE
    if (diags_avbl[1]) {
      if (board->board[row - d][col + d].piece != EMPTY) {
        if (board->board[row - d][col + d].color != c) {
          addMove(moves, &numMoves, row, col, row - d, col + d);
        }
        diags_avbl[1] = 0;
        continue;
      }
      addMove(moves, &numMoves, row, col, row - d, col + d);
      continue;
    }
    // SW
    if (diags_avbl[2]) {
      if (board->board[row - d][col - d].piece != EMPTY) {
        if (board->board[row - d][col - d].color != c) {
          addMove(moves, &numMoves, row, col, row - d, col - d);
        }
        diags_avbl[2] = 0;
        continue;
      }
      addMove(moves, &numMoves, row, col, row - d, col - d);
      continue;
    }
    // NW
    if (diags_avbl[3]) {
      if (board->board[row + d][col - d].piece != EMPTY) {
        if (board->board[row + d][col - d].color != c) {
          addMove(moves, &numMoves, row, col, row + d, col - d);
        }
        diags_avbl[3] = 0;
        continue;
      }
      addMove(moves, &numMoves, row, col, row + d, col - d);
      continue;
    }
  }
  return numMoves;
}

int generateRookMoves(const Chessboard* board, int row, int col, Move* moves) {
  int numMoves = 0;


  return numMoves;
}

int generateMoves(const Chessboard* board, Color sideToMove, Move* moves) {
  int numMoves = 0;

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      Square square = board->board[row][col];
      if (square.piece != EMPTY && square.color == sideToMove) {
        switch (square.piece) {
        case PAWN:
          numMoves += generatePawnMoves(board, row, col, moves + numMoves);
          break;
        case KNIGHT:
          numMoves += generateKnightMoves(board, row, col, moves + numMoves);
          break;
        case BISHOP:
          numMoves += generateBishopMoves(board, row, col, moves + numMoves);
          break;
        case ROOK:
          numMoves += generateRookMoves(board, row, col, moves + numMoves);
          break;
        case QUEEN:
          numMoves += generateQueenMoves(board, row, col, moves + numMoves);
          break;
        case KING:
          numMoves += generateKingMoves(board, row, col, moves + numMoves);
          break;
        default:
          break;
        }
      }
    }
  }

  return numMoves;
}



bool isCheckmate(const Chessboard* board, Color sideToMove) {
  // Generate all possible moves for the side to move
  Move possibleMoves[MAX_MOVES];
  int numPossibleMoves = generateMoves(board, sideToMove, possibleMoves);

  // Iterate through each possible move
  for (int i = 0; i < numPossibleMoves; i++) {
    // Make a copy of the current chessboard
    Chessboard tempBoard;
    memcpy(&tempBoard, board, sizeof(Chessboard));

    // Make the current move on the temporary board
    makeMove(&tempBoard, &possibleMoves[i]);

    // Check if the opponent's king is in check after the move
    if (!isKingInCheck(&tempBoard, opponentColor(sideToMove))) {
      // If the opponent's king is not in check, the move is not checkmate
      return false;
    }
  }

  // If all possible moves result in the opponent's king being in check, it's checkmate
  return true;
}

