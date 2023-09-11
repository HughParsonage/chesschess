#include "chess.h"

#define MAX_MOVES 5050
#define LONG_GAME 255

#define OPPCOLOR (sideToMove == WHITE ? BLACK : WHITE)

const char * abcdefgh_ = "abcdefgh";

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
  unsigned int fromRow : 3;
  unsigned int fromCol : 3;
  unsigned int toRow : 3;
  unsigned int toCol : 3;
  Piece toPiece; // relevant for promotions and to signify starting position
} Move;

typedef struct {
  Square board[8][8];
  unsigned int WhiteKing : 6; // duplicately record the kings' positions
  unsigned int BlackKing : 6;
  bool WhiteMayCastle : 1;
  bool BlackMayCastle: 1;
  Move lastMove;
} Chessboard;

unsigned int pos_hash(Chessboard * B) {
  unsigned int o = 0;
  // unsigned int tbl[64][7] = {0};

  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      Piece PP = B->board[r][c].piece;
      // bool is_white = B->board[r][c].color;
      o += 2 * (PP);
    }
  }
  return o;
}



typedef struct {
  unsigned int P : 3;
  unsigned int Q : 3;
  unsigned int R : 3;
  unsigned int N : 3;
  unsigned int B_light : 3;
  unsigned int B_dark : 3;
  unsigned int bishop_pair : 1;
} Material;

typedef struct {
  Chessboard Board;
  Color sideToMove;
  Move Moves[LONG_GAME][2];
  uint16_t white_material[LONG_GAME];
  uint16_t black_material[LONG_GAME];
  unsigned int move : 8;
  unsigned int whiteLostCastlingRights : 8;
  unsigned int blackLostCastlingRights : 8;
  unsigned int last_pawn_move : 8;
} Game;


typedef struct {
  Chessboard Board;
  Color sideToMove;
  Move Moves[MAX_MOVES][2];
  uint16_t white_material[MAX_MOVES];
  uint16_t black_material[MAX_MOVES];
  unsigned int move : 13;
  unsigned int whiteLostCastlingRights : 13;
  unsigned int blackLostCastlingRights : 13;
  unsigned int last_pawn_move : 13;
} LongGame;

unsigned int p2row(unsigned int x) {
  return x >> 3;
}

unsigned int p2col(unsigned int x) {
  return x & 7;
}

unsigned int rowcol2p(int r, int c) {
  return (r << 3) + c;
}

bool is_light_square(int r, int c) {
  return (r + c) & 1;
}

void print_the_piece(Piece P) {
  if (P == QUEEN) Rprintf("QUEEN");
  if (P == PAWN) Rprintf("PAWN");
  if (P == KING) Rprintf("KING");
  if (P == KNIGHT) Rprintf("KNIGHT");
  if (P == BISHOP) Rprintf("BISHOP");
  if (P == EMPTY) Rprintf("EMPTY");
  if (P == ROOK) Rprintf("ROOK");
}

const char * strWHITE = "WHITE";
const char * strBLACK = "BLACK";

#define color2str (sideToMove == WHITE ? strWHITE : strBLACK)


void print_piece(Chessboard * board, int row, int col) {
  print_the_piece(board->board[row][col].piece);
}

void determine_material(Material * M, const Chessboard * board, Color C) {
  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      if (board->board[r][c].piece == EMPTY || board->board[r][c].color != C) {
        continue;
      }
      switch(board->board[r][c].piece) {
      case PAWN:
        M->P++;
        break;
      case QUEEN:
        M->Q++;
        break;
      case ROOK:
        M->R++;
        break;
      case BISHOP:
        if (is_light_square(r, c)) {
          M->B_light++;
        } else {
          M->B_dark++;
        }
        break;
      case KNIGHT:
        M->N++;
      default:
        continue;
      }
    }
  }
  M->bishop_pair = M->B_dark && M->B_light;
}

unsigned int total_material(Material * M) {
  // From AlphaZero because why not, the piece values relative to pawn = 100
  unsigned int p = 100;
  unsigned int n = 305;
  unsigned int b = 333;
  unsigned int bb = 50; // bishop pair
  unsigned int q = 950;
  unsigned int r = 563;

  unsigned int o = 0; // maximum is 60800 (all queens)
  o += p * (M->P);
  o += n * (M->N);
  o += q * (M->Q);
  o += r * (M->R);
  o += b * (M->B_dark);
  o += b * (M->B_light);
  o += bb * (M->bishop_pair);
  return o;
}

void blankBoard(Chessboard * board) {
  for (int c = 0; c < 8; ++c) {
    for (int r = 0; r < 8; ++r) {
      board->board[r][c].piece = EMPTY;
      board->board[r][c].color = WHITE;
    }
  }
}

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

  board->lastMove.fromCol = 0;
  board->lastMove.fromRow = 0;
  board->lastMove.toCol = 0;
  board->lastMove.toRow = 0;
  board->lastMove.toPiece = EMPTY;
}

void initialize_Game(Game * G) {
  if (G == NULL) {
    return;
  }
  G->move = 0;
  G->last_pawn_move = 0;
  startingPosition(&(G->Board));
  G->sideToMove = WHITE;
  memset(G->Moves, 0, LONG_GAME * sizeof(Move));
  memset(G->black_material, 0, LONG_GAME * sizeof(uint16_t));
  memset(G->white_material, 0, LONG_GAME * sizeof(uint16_t));
  Material M;
  determine_material(&M, &(G->Board), WHITE);
  G->white_material[0] = total_material(&M);
  G->black_material[0] = total_material(&M);
  G->whiteLostCastlingRights = LONG_GAME;
  G->blackLostCastlingRights = LONG_GAME;
}

// returns cols[j] = 1 if pawn in column j can take to the right, = -1 can take to left, 0 cannot take enpassant
void colsMayEnPassant(int cols[8], const Chessboard * board) {
  Move lastMove = board->lastMove;
  memset(cols, 0, 8 * sizeof(int));
  if (lastMove.fromRow != 1 && lastMove.fromRow != 6) {
    return;
  }
  if (lastMove.fromRow == 1 && (lastMove.toRow != 3 || lastMove.toCol != lastMove.fromCol)) {
    return;
  }
  if (lastMove.fromRow == 6 && (lastMove.toRow != 4 || lastMove.toCol != lastMove.fromCol)) {
    return;
  }
  if (board->board[lastMove.toRow][lastMove.toCol].piece != PAWN) {
    return;
  }
  if (lastMove.toCol == 0) {
    if (lastMove.toRow == 4) {
      if (board->board[4][1].piece == PAWN && board->board[4][1].color == WHITE) {
        cols[1] = -1;
        return;
      }
    } else {
      if (board->board[3][1].piece == PAWN && board->board[3][1].color == BLACK) {
        cols[1] = -1;
        return;
      }
    }
  }
  if (lastMove.toCol == 7) {
    if (lastMove.toRow == 3) {
      if (board->board[4][6].piece == PAWN && board->board[4][6].color == BLACK) {
        cols[6] = 1;
        return;
      }
    } else {
      if (board->board[3][6].piece == PAWN && board->board[3][6].color == WHITE) {
        cols[6] = 1;
        return;
      }
    }
  }
  int c = lastMove.toCol;
  if (lastMove.toRow == 3) {
    if (board->board[3][c - 1].piece == PAWN && board->board[3][c - 1].color == BLACK) {
      cols[c - 1] = 1;
    }
    if (board->board[3][c + 1].piece == PAWN && board->board[3][c + 1].color == BLACK) {
      cols[c + 1] = -1;
    }
  } else {
    if (board->board[4][c - 1].piece == PAWN && board->board[4][c - 1].color == WHITE) {
      cols[c - 1] = 1;
    }
    if (board->board[4][c + 1].piece == PAWN && board->board[4][c + 1].color == WHITE) {
      cols[c + 1] = -1;
    }
  }
}

bool isMoveLegal(const Chessboard* board, const Move* move) {
  // Get the piece at the source square
  Square sourceSquare = board->board[move->fromRow][move->fromCol];

  // Check if the source square is empty or the piece color doesn't match the current turn
  if (sourceSquare.piece == EMPTY) {
    return false;
  }

  // Get the piece at the destination square
  Square destinationSquare = board->board[move->toRow][move->toCol];

  // Check if the destination square contains a piece of the same color
  if (destinationSquare.piece != EMPTY) {
    return false;
  }

  // Check if the move is valid based on the piece type
  switch (sourceSquare.piece) {
  case PAWN: {
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
  }
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

  case ROOK:
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
  return kingColor == WHITE ? board->WhiteKing : board->BlackKing;
}

bool isKingInCheck(const Chessboard* board, Color kingColor) {
  unsigned int kL = locateKing(board, kingColor);
  unsigned int kingRow = p2row(kL);
  unsigned int kingCol = p2col(kL);

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

bool wouldKingBeInCheck(const Chessboard * board, Color kingColor, int fromRow, int fromCol, int toRow, int toCol, bool enpassant) {
  Square P = board->board[fromRow][fromCol];
  Chessboard tempBoard;
  memcpy(&tempBoard, board, sizeof(Chessboard));
  tempBoard.board[fromRow][fromCol].piece = EMPTY;
  tempBoard.board[toRow][toCol].piece = P.piece;
  tempBoard.board[toRow][toCol].color = P.color;
  // enpassant is unusual in that the captured pawn is not on the destination square
  if (enpassant) {
    tempBoard.board[fromRow][toCol].piece = EMPTY;
  }
  return isKingInCheck(&tempBoard, kingColor);
}

// A piece is 'maybe pinned' if it is pinned though might still be able to move
// (to capture the pinning piece for example), called 'partially pinned' by wiki
bool maybePinned(const Chessboard * board, int row, int col) {
  // corners can't be pinned
  if ((row == 0 || row == 7) && (col == 0 || col == 7)) {
    return false;
  }
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

  Color C = board->board[row][col].color;

  // Determine the color of the pawn
  bool isWhite = (C == WHITE);

  // Determine the direction of movement based on the pawn's color
  int direction = isWhite ? 1 : -1;

  // Check if the pawn can move one square forward
  int toRow = row + direction;
  int toCol = col;

  if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8 && board->board[toRow][toCol].piece == EMPTY) {
    if (!(wouldKingBeInCheck(board, C, row, col, toRow, toCol, false))) {
      addMove(moves, &numMoves, row, col, toRow, toCol);

      // Check if the pawn can move two squares forward from the starting position
      if ((isWhite && row == 6) || (!isWhite && row == 1)) {
        toRow = row + (2 * direction);
        if (toRow >= 0 && toRow < 8 && board->board[toRow][toCol].piece == EMPTY) {
          addMove(moves, &numMoves, row, col, toRow, toCol);
        }
      }
    }
  }

  // Check if the pawn can capture diagonally to the left
  toRow = row + direction;
  toCol = col - 1;
  if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8 && board->board[toRow][toCol].piece != EMPTY && board->board[toRow][toCol].color != board->board[row][col].color) {
    if (!wouldKingBeInCheck(board, C, row, col, toRow, toCol, false)) {
      addMove(moves, &numMoves, row, col, toRow, toCol);
    }
  }

  // Check if the pawn can capture diagonally to the right
  toRow = row + direction;
  toCol = col + 1;
  if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8 &&
      board->board[toRow][toCol].piece != EMPTY &&
      board->board[toRow][toCol].color != board->board[row][col].color) {
    if (!wouldKingBeInCheck(board, C, row, col, toRow, toCol, false)) {
      addMove(moves, &numMoves, row, col, toRow, toCol);
    }
  }

  int cols_maybe_enpassant[8] = {0};
  colsMayEnPassant(cols_maybe_enpassant, board);
  for (int j = 0; j < 8; ++j) {
    if (cols_maybe_enpassant[j]) {
      if (!wouldKingBeInCheck(board, C, row, col, toRow, toCol, true)) {
        addMove(moves, &numMoves, isWhite ? 3 : 4, j, isWhite ? 4 : 3, j + cols_maybe_enpassant[j]);
      }
      break;
    }
  }


  return numMoves;
}

int generateKnightMoves(const Chessboard* board, int row, int col, Move* moves) {
  int numMoves = 0;
  const Color c = board->board[row][col].color;

  // think of a clockface 1,2 4,5, 7,8, 10,11 are the moves in general
  // const int hrs[8] = {1, 2, 4, 5, 7, 8, 10, 11};
  const int toRows[8] = {1, 2, 2, 1, -1, -2, -2, -1};
  const int toCols[8] = {2, 1, -1, -2, -2, -1, 1, 2};
  for (int h = 0; h < 8; ++h) {
    int toRow = row + toRows[h];
    int toCol = col + toCols[h];
    if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) {
      continue; // outside the board
    }
    // same color => not possible
    if (board->board[toRow][toCol].color == board->board[row][col].color &&
        board->board[toRow][toCol].piece != EMPTY) {
      continue;
    }
    if (wouldKingBeInCheck(board, c, row, col, toRow, toCol, false)) {
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
    // if the king lies on the same row/col then the bishop is totally pinned
    unsigned int K_row = p2row(c == WHITE ? board->WhiteKing : board->BlackKing);
    if (K_row == row) {
      return 0;
    }
    unsigned int K_col = p2col(c == WHITE ? board->WhiteKing : board->BlackKing);
    if (K_col == col) {
      return 0;
    }

    // check each diagonal -- if king in check when moving one square along a diagonal
    // it must be in check along both
    for (int diag = 0; diag < 4; ++diag) {
      if (!diags_avbl[diag]) {
        continue;
      }
      int toRow = (diag < 2) ? (row + 1) : (row - 1);
      int toCol = (diag == 2 || diag == 3) ? (col - 1) : (col + 1);
      if (toRow >= 8 || toRow < 0 || toCol >= 8 || toCol < 0) {
        continue;
      }
      if (board->board[toRow][toCol].piece != EMPTY && board->board[toRow][toCol].color == c) {
        diags_avbl[diag] = 0;
        continue;
      }

      Chessboard tempBoard;
      memcpy(&tempBoard, board, sizeof(Chessboard));
      tempBoard.board[toRow][toCol].piece = BISHOP;
      tempBoard.board[toRow][toCol].color = c;
      tempBoard.board[row][col].piece = EMPTY;

      if (board->board[toRow][toCol].piece != EMPTY) {
        // Capture the adjacent piece
        if (isKingInCheck(&tempBoard, c)) {
          diags_avbl[diag] = 0;
          diags_avbl[(diag + 2) & 3] = 0; // opposite direction must be unavailable too
        } else {
          // Add the move to the array here because we immediately disqualify
          // the diagnoal
          addMove(moves, &numMoves, row, col, toRow, toCol);
          diags_avbl[diag] = 0; //
        }
      } else {
        if (isKingInCheck(&tempBoard, c)) {
          diags_avbl[diag] = 0;
          diags_avbl[(diag + 2) & 3] = 0; // opposite direction must be unavailable too
        } else {
          // do nothing, we will add the move in the main loop
        }
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
  bool dirs_avbl[4] = {1, 1, 1, 1}; // N, S, E, W
  int dirs_oppos[4] = {1, 0, 3, 2};
  int toRows[4] = {1, -1, 0, 0};
  int toCols[4] = {0, 0, 1, -1};

  const Color c = board->board[row][col].color;
  bool maybe_pinned = maybePinned(board, row, col);
  bool in_check = isKingInCheck(board, c);

  if (in_check) {
    for (int dir = 0; dir < 4; ++dir) {
      for (int d = 1; d < 8; ++d) {
        if (!dirs_avbl[dir]) {
          break;
        }
        int toRow = row + d * toRows[dir];
        int toCol = col + d * toCols[dir];

        if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) {
          dirs_avbl[dir] = 0;
          break;
        }
        if (board->board[toRow][toCol].piece != EMPTY) {
          dirs_avbl[dir] = 0;
          if (board->board[toRow][toCol].color == c) {
            break;
          }
        }
        if (wouldKingBeInCheck(board, c, row, col, toRow, toCol, false)) {
          continue;
        } else {
          addMove(moves, &numMoves, row, col, toRow, toCol);
        }
      }
    }
    return numMoves;
  }

  if (maybe_pinned) {
    // if the king and the rook lie on the same diagonal then it is totally pinned
    unsigned int itsKing = (c == WHITE) ? board->WhiteKing : board->BlackKing;
    if (liesOnSameDiag(itsKing, (row << 3) + col)) {
      return 0;
    }
    unsigned int K_row = itsKing >> 3;
    if (K_row == row) {
      dirs_avbl[0] = 0;
      dirs_avbl[1] = 0;
    } else {
      dirs_avbl[2] = 0;
      dirs_avbl[3] = 0;
    }
    for (int dir = 0; dir < 4; ++dir) {
      if (!dirs_avbl[dir]) {
        continue;
      }
      int toRow = row + (dir == 3) - 2 * (dir == 4);
      int toCol = col + (dir == 0) - 2 * (dir == 1);
      if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) {
        continue;
      }
      if (board->board[toRow][toCol].piece != EMPTY && board->board[toRow][toCol].color == c) {
        dirs_avbl[dir] = 0;
        continue;
      }
      Chessboard tempBoard;
      memcpy(&tempBoard, board, sizeof(Chessboard));
      tempBoard.board[toRow][toCol].piece = ROOK;
      tempBoard.board[toRow][toCol].color = c;
      tempBoard.board[row][col].piece = EMPTY;

      if (board->board[toRow][toCol].piece != EMPTY) {
        // Capture the adjacent piece
        if (isKingInCheck(&tempBoard, c)) {
          dirs_avbl[dir] = 0;
          dirs_avbl[dirs_oppos[dir]] = 0;
        } else {
          addMove(moves, &numMoves, row, col, toRow, toCol);
          dirs_avbl[dir] = 0;
        }
      } else {
        if (!isKingInCheck(&tempBoard, c)) {
          addMove(moves, &numMoves, row, col, toRow, toCol);
        }
      }
    }


  }
  if (dirs_avbl[0]) {
    for (int d = 1; d < 8; ++d) {
      if (col + d >= 8) {
        break;
      }
      int toCol = col + d;
      int toRow = row;
      Chessboard tempBoard;
      memcpy(&tempBoard, board, sizeof(Chessboard));
      tempBoard.board[toRow][toCol].piece = ROOK;
      tempBoard.board[toRow][toCol].color = c;
      tempBoard.board[row][col].piece = EMPTY;
      if (!isKingInCheck(&tempBoard, c)) {
        addMove(moves, &numMoves, row, col, row, col + d);
      }
      if (board->board[row][toCol].piece != EMPTY) {
        break;
      }
    }
  }
  if (dirs_avbl[1]) {
    for (int d = 1; d < 8; ++d) {
      if (col - d < 0) {
        break;
      }
      int toCol = col - d;
      int toRow = row;
      Chessboard tempBoard;
      memcpy(&tempBoard, board, sizeof(Chessboard));
      tempBoard.board[toRow][toCol].piece = ROOK;
      tempBoard.board[toRow][toCol].color = c;
      tempBoard.board[row][col].piece = EMPTY;
      if (!isKingInCheck(&tempBoard, c)) {
        addMove(moves, &numMoves, row, col, row, col + d);
      }
      if (board->board[row][toCol].piece != EMPTY) {
        break;
      }
    }
  }
  if (dirs_avbl[2]) {
    for (int d = 1; d < 8; ++d) {
      if (row + d >= 8) {
        break;
      }
      int toCol = col;
      int toRow = row + d;
      Chessboard tempBoard;
      memcpy(&tempBoard, board, sizeof(Chessboard));
      tempBoard.board[toRow][toCol].piece = ROOK;
      tempBoard.board[toRow][toCol].color = c;
      tempBoard.board[row][col].piece = EMPTY;
      if (!isKingInCheck(&tempBoard, c)) {
        addMove(moves, &numMoves, row, col, row, col + d);
      }
      if (board->board[row][toCol].piece != EMPTY) {
        break;
      }
    }
  }
  if (dirs_avbl[3]) {
    for (int d = 1; d < 8; ++d) {
      if (row - d < 0) {
        break;
      }
      int toCol = col;
      int toRow = row - d;
      Chessboard tempBoard;
      memcpy(&tempBoard, board, sizeof(Chessboard));
      tempBoard.board[toRow][toCol].piece = ROOK;
      tempBoard.board[toRow][toCol].color = c;
      tempBoard.board[row][col].piece = EMPTY;
      if (!isKingInCheck(&tempBoard, c)) {
        addMove(moves, &numMoves, row, col, row, col + d);
      }
      if (board->board[row][toCol].piece != EMPTY) {
        break;
      }
    }
  }

  return numMoves;
}

int generateQueenMoves(const Chessboard* board, int row, int col, Move* moves) {
  int numMoves = 0;
  const Color c = board->board[row][col].color;
  bool maybe_pinned = maybePinned(board, row, col);

  // N clockwise to NW
  bool dirs_avbl[8] = {1, 1, 1, 1, 1, 1, 1, 1};
  int toCols[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  int toRows[8] = {1, 1, 0, -1, -1, -1, 0, 1};

  if (maybe_pinned) {
    // unsigned int K_row = p2row(c == WHITE ? board->WhiteKing : board->BlackKing);
    for (int dir = 0; dir < 8; ++dir) {
      if (!dirs_avbl[dir]) {
        continue;
      }
      int toRow = row + toRows[dir];
      int toCol = col + toCols[dir];
      if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) {
        continue;
      }
      if (board->board[toRow][toCol].piece != EMPTY && board->board[toRow][toCol].color == c) {
        dirs_avbl[dir] = 0;
        continue;
      }
      Chessboard tempBoard;
      memcpy(&tempBoard, board, sizeof(Chessboard));
      tempBoard.board[toRow][toCol].piece = QUEEN;
      tempBoard.board[toRow][toCol].color = c;
      tempBoard.board[row][col].piece = EMPTY;
      if (board->board[toRow][toCol].piece != EMPTY) {
        if (isKingInCheck(board, c)) {
          dirs_avbl[dir] = 0;
          dirs_avbl[(dir + 4) & 7] = 0;
        } else {
          addMove(moves, &numMoves, row, col, toRow, toCol);
          dirs_avbl[dir] = 0;
        }
      } else {
        if (isKingInCheck(board, c)) {
          dirs_avbl[dir] = 0;
          dirs_avbl[(dir + 4) & 7] = 0;
        }
      }
    }
  }
  for (int dir = 0; dir < 8; ++dir) {
    if (!dirs_avbl[dir]) {
      continue;
    }
    for (int d = 0; d < 8; ++d) {
      int toRow = row + d * toRows[dir];
      int toCol = col + d * toCols[dir];
      if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) {
        break;
      }
      if (board->board[toRow][toCol].piece == EMPTY) {
        addMove(moves, &numMoves, row, col, toRow, toCol);
        continue;
      }
      if (board->board[toRow][toCol].color == c) {
        dirs_avbl[dir] = 0;
        break;
      }
      addMove(moves, &numMoves, row, col, toRow, toCol);
      break;
    }
  }
  return numMoves;
}

int canCastle(const Chessboard* board, Color C) {
  // 0 castling not possible either side
  // 1 castling possible kingside but not queenside
  // 2 castling possible queenside but not kingside
  // 3 castling possible either side

  if (isKingInCheck(board, C)) {
    return 0;
  }
  bool kingside = true;
  bool queenside = true;
  if (C == WHITE) {
    if (!(board->WhiteMayCastle)) {
      return 0;
    }
    if (board->board[0][0].piece != ROOK || board->board[0][0].color != C) {
      queenside = false;
    }
    if (board->board[0][7].piece != ROOK || board->board[0][7].color != C) {
      kingside = false;
    }


    // check obstructions
    // loop through bottom row, but not the rooks
    for (int c = 1; c < 7; ++c) {
      if (c == 4) {
        if (board->board[0][4].piece != KING || board->board[0][4].color != C) {
          // should be an error
          return 0;
        }
        continue; // king
      }
      // don't check each column if already determined to be impossible
      if (c < 4 && !queenside) {
        continue;
      }
      if (c > 4 && !kingside) {
        break;
      }
      if (board->board[0][c].piece != EMPTY) {
        if (c < 4) {
          queenside = false;
        } else {
          kingside = false;
        }
        continue;
      }
      if (c == 1) {
        // queenside possible if in check on 1
        continue;
      }
      Chessboard tempBoard;
      memcpy(&tempBoard, board, sizeof(Chessboard));
      tempBoard.board[0][c].piece = KING;
      tempBoard.board[0][c].color = C;
      tempBoard.board[0][4].piece = EMPTY;
      if (isKingInCheck(&tempBoard, C)) {
        if (c < 4) {
          queenside = false;
        } else {
          kingside = false;
        }
      }
    }


  } else {
    if (!(board->BlackMayCastle)) {
      return 0;
    }
    if (board->board[7][0].piece != ROOK || board->board[7][0].color != C) {
      queenside = false;
    }
    if (board->board[7][7].piece != ROOK || board->board[7][7].color != C) {
      kingside = false;
    }


    // check obstructions
    // loop through bottom row, but not the castles
    for (int c = 1; c < 7; ++c) {
      if (c == 4) {
        if (board->board[7][4].piece != KING || board->board[7][4].color != C) {
          // should be an error
          return 0;
        }
        continue; // king
      }
      // don't check each column if already determined to be impossible
      if (c < 4 && !queenside) {
        continue;
      }
      if (c > 4 && !kingside) {
        break;
      }
      if (board->board[7][c].piece != EMPTY) {
        if (c < 4) {
          queenside = false;
        } else {
          kingside = false;
        }
        continue;
      }
      if (c == 1) {
        continue;
      }
      Chessboard tempBoard;
      memcpy(&tempBoard, board, sizeof(Chessboard));
      tempBoard.board[7][c].piece = KING;
      tempBoard.board[7][c].color = C;
      tempBoard.board[7][4].piece = EMPTY;
      if (isKingInCheck(&tempBoard, C)) {
        if (c < 4) {
          queenside = false;
        } else {
          kingside = false;
        }
      }
    }
  }
  if (kingside && queenside) {
    return 3;
  }
  if (kingside) {
    return 1;
  }
  if (queenside) {
    return 2;
  }
  return 0;
}

int generateKingMoves(const Chessboard* board, int row, int col, Move* moves) {
  int numMoves = 0;
  const Color c = board->board[row][col].color;

  // N clockwise to NW
  int toCols[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  int toRows[8] = {1, 1, 0, -1, -1, -1, 0, 1};

  for (int dir = 0; dir < 8; ++dir) {
    int toRow = row + toRows[dir];
    int toCol = col + toCols[dir];
    if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) {
      continue;
    }
    if (board->board[toRow][toCol].piece != EMPTY && board->board[toRow][toCol].color == c) {
      continue;
    }
    Chessboard tempBoard;
    memcpy(&tempBoard, board, sizeof(Chessboard));
    tempBoard.board[toRow][toCol].piece = KING;
    tempBoard.board[toRow][toCol].color = c;
    tempBoard.board[row][col].piece = EMPTY;
    if (c == WHITE) {
      tempBoard.WhiteKing = rowcol2p(toRow, toCol);
    } else {
      tempBoard.BlackKing = rowcol2p(toRow, toCol);
    }
    if (isKingInCheck(&tempBoard, c)) {
      continue;
    }
    addMove(moves, &numMoves, row, col, toRow, toCol);
  }
  // Castling
  int can_castle = canCastle(board, c);
  switch(can_castle) {
  case 0:
    break;
  case 1:
    addMove(moves, &numMoves, row, col, row, 6);
    break;
  case 2:
    addMove(moves, &numMoves, row, col, row, 2);
    break;
  default: {
      addMove(moves, &numMoves,  row, col, row, 6);
      addMove(moves, &numMoves,  row, col, row, 2);
    }
  }
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
  for (int j = 0; j < numPossibleMoves; ++j) {
    // print_the_piece(possibleMoves[j].toPiece);
    // const char * letters = "abcdefgh";
    // Rprintf(": %c%d->%c%d\n",
    //         letters[possibleMoves[j].fromCol], possibleMoves[j].fromRow + 1,
    //         letters[possibleMoves[j].toCol], possibleMoves[j].toRow + 1);

  }

  return numPossibleMoves == 0 && isKingInCheck(board, sideToMove);
}

int isntValidBoard(const Chessboard * board, Color colorToMove) {
  // 0 is valid
  // 1 white king absent
  // 2 black king absent
  // 3 both kings absent
  // 4 white king does not match duplicate record
  // 5 black king does not match duplicate record
  // 6 other king is in check
  int kingRows[2] = {-1, -1};
  int kingCols[2] = {-1, -1};
  bool white_king_set = false;
  bool black_king_set = false;
  if (board->board[0][4].piece == KING && board->board[0][4].color == WHITE) {
    kingRows[0] = 0;
    kingCols[0] = 4;
    white_king_set = true;
  }
  if (board->board[7][4].piece == KING && board->board[7][4].color == BLACK) {
    kingRows[1] = 7;
    kingCols[1] = 4;
    black_king_set = true;
  }
  if (!white_king_set) {
    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        if (board->board[r][c].piece == KING && board->board[r][c].color == WHITE) {
          kingRows[0] = r;
          kingCols[0] = c;
        }
      }
    }
  }
  if (!black_king_set) {
    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        if (board->board[r][c].piece == KING && board->board[r][c].color == BLACK) {
          kingRows[1] = r;
          kingCols[1] = c;
        }
      }
    }
  }
  if (kingRows[0] < 0 || kingRows[1] < 0 || kingCols[0] < 0 || kingCols[1] < 0) {
    // one or both kings are absent
    return (kingRows[0] < 0 && kingRows[1] < 0) ? 3 : (kingRows[0] < 0 ? 1 : 2);
  }
  if (rowcol2p(kingRows[0], kingCols[0]) != board->WhiteKing) {
    return 4;
  }
  if (rowcol2p(kingRows[1], kingCols[1]) != board->BlackKing) {
    return 5;
  }

  if (isKingInCheck(board, colorToMove == WHITE ? BLACK : WHITE)) {
    return 6;
  }




  return 0;
}

// positions only, not algebraic notation
unsigned int string2p(const char * x) {
  const char * letters = "abcdefgh";

  if (isupper(x[0])) {
    if (strlen(x) < 3) {
      error("strlen(x = %s) < 3", x);
    }
    for (int c = 0; c < 8; ++c) {
      if (x[1] == letters[c]) {

        unsigned int oooo =  rowcol2p(x[2] - '1', c);
        return oooo;
      }
    }
  } else {
    for (int c = 0; c < 8; ++c) {
      if (x[0] == letters[c]) {
        return rowcol2p(x[1] - '1', c);
      }
    }
  }
  error("(string2p)Could not determine position from string '%s'", x);
}

Piece string2Piece(const char * x) {
  switch(x[0]) {
  case 'K':
    return KING;
  case 'Q':
    return QUEEN;
  case 'B':
    return BISHOP;
  case 'N':
    return KNIGHT;
  case 'R':
    return ROOK;
  default:
    return PAWN;
  }
}

unsigned int PawnBoard2p(const Color C, const Chessboard * board, unsigned int to_p, int fromRow, int fromCol, bool capture) {
  int to_row = p2row(to_p);
  int to_col = p2col(to_p);
  if (fromRow > 0 && fromCol >= 0) {
    return rowcol2p(fromRow, fromCol);
  }
  switch(C) {
  case WHITE:
    switch(to_row) {
    case 2:
      if (capture) {
        if (fromRow >= 0) {
          // just need to determine column, which will be the only one with a pawn
          if (to_col > 0 &&
              board->board[fromRow][to_col - 1].piece == PAWN &&
              board->board[fromRow][to_col - 1].color == WHITE) {
            return rowcol2p(fromRow, to_col - 1);
          }
          if (to_col < 7 &&
              board->board[fromRow][to_col + 1].piece == PAWN &&
              board->board[fromRow][to_col + 1].color == WHITE) {
            return rowcol2p(fromRow, to_col + 1);
          }
        } else {
          // column is determined
          return rowcol2p(to_row - 1, fromRow);
        }
      } else {

      }
    }
    break;
  case BLACK:
    return 0;
  }
  return 55;
}

unsigned int PieceBoard2p(const Piece P, const Color C, const Chessboard * board, unsigned int to_p) {

  int n = 0;
  uint16_t o[62] = {0};

  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      if (board->board[r][c].piece == P &&
          board->board[r][c].color == C) {
        o[n++] = rowcol2p(r, c);
      }
    }
  }
  if (n == 0) {
    error("Could not find piece on board.");
  }
  if (n == 1) {
    return o[0];
  }
  int to_row = p2row(to_p);
  int to_col = p2col(to_p);
  for (int k = 0; k < n; ++k) {
    unsigned int from_p = o[k];
    int from_row = p2row(from_p);
    int from_col = p2col(from_p);
    if (canPieceAttackSquare(board, from_row, from_col, to_row, to_col)) {
      return from_p;
    }

  }


  if (P == BISHOP) {
    if (liesOnSameDiag(to_p, o[0])) {

    }
  }
  // error("Unable to determine Piece2Board.");

  return 55;

}

bool is_1to8(char x) {
  return x >= '1' && x <= '8';
}

bool is_abcdefgh(char x) {
  return x >= 'a' && x <= 'h';
}

bool is_algebraic_position(char x) {
  return is_1to8(x) || is_abcdefgh(x);
}

bool is_terminal_char(char x) {
  return x == '+' || x == '!' || x == '?' || x == '#' || x == '=';
}

void validate_algebraic_string(const char * x) {
  int j = 0;
  if (isupper(x[0])) {
    if (x[0] != 'R' && x[0] != 'N' && x[0] != 'B' && x[0] != 'Q' && x[0] != 'K') {
      error("First char of x was uppercase but not a valid piecename.");
    }
    j = 1;
  } else {
    // pawn cannot move to outer rows
    if (x[1] <= '1' || x[1] >= '8') {
      error("Invalid row for pawn move.");
    }
  }
  if (!is_algebraic_position(x[j++])) {
    error("First character not a position.");
  }

  while (x[j] != '\0') {
    if (!is_algebraic_position(x[j])) {
      if (is_terminal_char(x[j]))  {
        return;
      }
      error("Invalid string '%s' at position %d.", x, j);
    }
    ++j;
    if (x[j] == 'x') {
      ++j;
      break;
    }
  }
  while (x[j] != '\0') {
    if (!is_algebraic_position(x[j])) {
      if (is_terminal_char(x[j]))  {
        return;
      }
      error("Invalid string '%s' at position %d.", x, j);
    }
    ++j;
  }


}

Move string2move(const char * x, int n, const Chessboard * board, Color sideToMove) {
  Move M;
  if (x[0] == 'O' && x[1] == '-' && x[2] == 'O') {
    M.fromRow = (sideToMove == WHITE) ? 0 : 7;
    M.fromCol = 4;
    M.toRow = M.fromRow;
    M.toCol = (n == 3) ? 7 : 0;

    if (board->board[M.fromRow][M.fromCol].piece != KING) {
      error("move was '%s' for %s but King not in starting position", x, color2str);
    }
    if (board->board[M.toRow][M.toCol].piece != ROOK) {
      error("move was '%s' for %s but Rook not in starting position", x, color2str);
    }

    M.toPiece = KING;
    return M;
  }

  validate_algebraic_string(x);
  // unsigned int p = string2p(x);


  switch(n) {
  case 2: {
    M.toRow = x[1] - '1';
    M.toCol = x[0] - 'a';
    M.fromCol = M.toCol;
    M.toPiece = PAWN;
    // pawn has simply moved
    if (sideToMove == WHITE) {
      if (board->board[M.toRow - 1][M.toCol].piece == PAWN &&
          board->board[M.toRow - 1][M.toCol].color == WHITE) {
        M.fromRow = M.toRow - 1;
        return M;
      }
      if (board->board[M.toRow - 2][M.toCol].piece == PAWN &&
          board->board[M.toRow - 2][M.toCol].color == WHITE &&
          board->board[M.toRow - 1][M.toCol].piece == EMPTY) {
        M.fromRow = M.toRow - 2;
        return M;
      }
      error("String '%s' appears to be a pawn move but not valid.", x);

    } else {
      if (board->board[M.toRow + 1][M.toCol].piece == PAWN &&
          board->board[M.toRow + 1][M.toCol].color == BLACK) {
        M.fromRow = M.toRow + 1;
        return M;
      }
      if (board->board[M.toRow + 2][M.toCol].piece == PAWN &&
          board->board[M.toRow + 2][M.toCol].color == BLACK &&
          board->board[M.toRow + 1][M.toCol].piece == EMPTY) {
        M.fromRow = M.toRow + 2;
        return M;
      }
      error("String '%s' appears to be a pawn move but not valid.", x);
    }
  }
  case 3:
    if (!isupper(x[0])) {
      // should be a pawn something (check, mate, blunder etc)
      if (!is_terminal_char(x[2]))  {
        error("String '%s' was length 3 but did not have expected terminal char", x);
      }
      M.toPiece = PAWN;
      M.toRow = x[0] - '1';
      M.toCol = x[1] - 'a';
      M.fromCol = M.toCol;
      if (x[2] == '+' || x[2] == '#') {
        // check king in Check
        bool king_in_check = false;
        int lh_target = M.toCol - 1;
        int rh_target = M.toCol + 1;
        int rr_target = (sideToMove == WHITE) ? (M.toRow + 1) : (M.toRow - 1);
        if (lh_target >= 0 &&
            board->board[rr_target][lh_target].piece == KING &&
            board->board[rr_target][lh_target].color != sideToMove) {
          king_in_check = true;
        }
        if (rh_target <= 7 &&
            board->board[rr_target][lh_target].piece == KING &&
            board->board[rr_target][lh_target].color != sideToMove) {
          king_in_check = true;
        }
        //
        if (!king_in_check) {
          Chessboard tempBoard;
          memcpy(&tempBoard, board, sizeof(Chessboard));
          tempBoard.board[M.toRow][M.toCol].piece = PAWN;
          tempBoard.board[M.toRow][M.toCol].color = sideToMove;


          if (board->board[M.toCol][M.toRow - 1].piece == PAWN &&
              board->board[M.toCol][M.toRow - 1].color == sideToMove)  {
            tempBoard.board[M.toCol][M.toRow - 1].piece = EMPTY;
            if (isKingInCheck(&tempBoard, OPPCOLOR)) {
              king_in_check = true;
            }
          } else if (board->board[M.toCol][M.toRow - 2].piece == PAWN &&
            board->board[M.toCol][M.toRow - 2].color == sideToMove &&
            board->board[M.toCol][M.toRow - 1].piece == EMPTY)  {
            tempBoard.board[M.toCol][M.toRow - 2].piece = PAWN;
            if (isKingInCheck(&tempBoard, OPPCOLOR)) {
              king_in_check = true;
            }
          } else {
            error("Unexpected move '%s'.", x);
          }



        }
      }

      if (sideToMove == WHITE) {
        if (board->board[M.toCol][M.toRow - 1].piece == PAWN &&
            board->board[M.toCol][M.toRow - 1].color == WHITE) {
          M.fromRow = M.toRow - 1;
          return M;
        }
        if (board->board[M.toCol][M.toRow - 2].piece == PAWN &&
            board->board[M.toCol][M.toRow - 2].color == WHITE &&
            board->board[M.toCol][M.toRow - 1].piece == EMPTY) {
          M.fromRow = M.toRow - 2;
          return M;
        }
        error("String '%s' appears to be a pawn move but not valid.", x);

      } else {
        if (board->board[M.toCol][M.toRow + 1].piece == PAWN &&
            board->board[M.toCol][M.toRow + 1].color == WHITE) {
          M.fromRow = M.toRow + 1;
          return M;
        }
        if (board->board[M.toCol][M.toRow + 2].piece == PAWN &&
            board->board[M.toCol][M.toRow + 2].color == WHITE &&
            board->board[M.toCol][M.toRow + 1].piece == EMPTY) {
          M.fromRow = M.toRow + 2;
          return M;
        }
        error("String '%s' appears to be a pawn move but not valid.", x);
      }

    } else {
      // piece
      Piece P = string2Piece(x);
      unsigned int p = string2p(x);
      M.toRow = p2row(p);
      M.toCol = p2col(p);
      unsigned int q = PieceBoard2p(P, sideToMove, board, p);
      M.fromRow = p2row(q);
      M.fromCol = p2col(q);
      M.toPiece = string2Piece(x);



    }
  }

  return M;
}

static void hasRookMoved(int moves[2], Game * G, Color sideToMove) {
  moves[0] = 0;
  moves[1] = 0;
  for (int m = 1; m < (G->move); ++m) {
    if (moves[0] && moves[1]) {
      break;
    }
    if (G->Moves[m][sideToMove == BLACK].toPiece == ROOK) {
      if (moves[0] == 0 && G->Moves[m][sideToMove == BLACK].fromCol == 0) {
        moves[0] = m;
      }
      if (moves[1] == 0 && G->Moves[m][sideToMove == BLACK].fromCol == 7) {
        moves[1] = m;
      }
    }
  }
}

void verify_castling(Game * G, bool queenside, Color sideToMove) {
  int c_castle = canCastle(&(G->Board), sideToMove);
  int rook_moves[2] = {0};
  hasRookMoved(rook_moves, G, sideToMove);

  if (sideToMove == WHITE) {
    if (rook_moves[!queenside]) {
      error("On move %d castling was attempted, but WHITE's ROOK moved on moved %d",
            G->move + 1, rook_moves[!queenside]);
    }
    if (G->whiteLostCastlingRights != LONG_GAME) {
      error("On move %d castling was attempted, but WHITE lost castling rights on move %d",
            G->move + 1, G->whiteLostCastlingRights);
    }

    if (c_castle == 3) {
      return;
    }
    if (isKingInCheck(&(G->Board), WHITE)) {
      error("On move %d castling was attempted, but king is in check.", G->move + 1);
    }

    if (queenside) {
      if (G->Board.board[0][0].piece != ROOK || G->Board.board[0][0].color != WHITE) {
        error("On move %d castling was attempted, but WHITE rook not on square a1.", G->move + 1);
      }
      for (int r = 1; r < 4; ++r) {
        if (G->Board.board[0][r].piece != EMPTY) {
          error("On move %d castling was attempted, but piece exists on square %c1.", G->move + 1, abcdefgh_[r]);
        }
      }
      for (int r = 2; r < 3; ++r) {
        if (wouldKingBeInCheck(&(G->Board), WHITE, 0, 4, 0, r, false)) {
          error("On move %d castling was attempted, but king would be in check exists on square %c1.", G->move + 1, abcdefgh_[r]);
        }
      }
    } else {
      if (G->Board.board[0][7].piece != ROOK || G->Board.board[0][7].color != WHITE) {
        error("On move %d castling was attempted, but WHITE rook not on square a1.", G->move + 1);
      }
      for (int r = 5; r < 7; ++r) {
        if (G->Board.board[0][r].piece != EMPTY) {
          error("On move %d castling was attempted, but piece exists on square %c1.", G->move + 1, abcdefgh_[r]);
        }
      }
      for (int r = 5; r < 7; ++r) {
        if (wouldKingBeInCheck(&(G->Board), WHITE, 0, 4, 0, r, false)) {
          error("On move %d castling was attempted, but king would be in check exists on square %c1.", G->move + 1, abcdefgh_[r]);
        }
      }
    }
  } else {
    if (rook_moves[!queenside]) {
      error("On move %d castling was attempted, but BLACK's ROOK moved on moved %d",
            G->move + 1, rook_moves[!queenside]);
    }
    if (G->blackLostCastlingRights != LONG_GAME) {
      error("On move %d castling was attempted, but BLACK lost castling rights on move %d",
            G->blackLostCastlingRights, G->move + 1);
    }
    if (c_castle == 3) {
      return;
    }
    if (queenside) {
      if (G->Board.board[7][0].piece != ROOK || G->Board.board[7][0].color != WHITE) {
        error("On move %d castling was attempted, but BLACK rook not on square a8.", G->move + 1);
      }
      for (int r = 1; r < 4; ++r) {
        if (G->Board.board[7][r].piece != EMPTY) {
          error("On move %d castling was attempted, but piece exists on square %c8.", G->move + 1, abcdefgh_[r]);
        }
      }
      for (int r = 2; r < 3; ++r) {
        if (wouldKingBeInCheck(&(G->Board), WHITE, 7, 4, 7, r, false)) {
          error("On move %d castling was attempted, but king would be in check exists on square %c8.", G->move + 1, abcdefgh_[r]);
        }
      }
    } else {
      if (G->Board.board[7][7].piece != ROOK || G->Board.board[7][7].color != WHITE) {
        error("On move %d castling was attempted, but BLACK rook not on square h8.", G->move + 1);
      }
      for (int r = 5; r < 7; ++r) {
        if (G->Board.board[7][r].piece != EMPTY) {
          error("On move %d castling was attempted, but piece exists on square %c8.", G->move + 1, abcdefgh_[r]);
        }
      }
      for (int r = 5; r < 7; ++r) {
        if (wouldKingBeInCheck(&(G->Board), WHITE, 7, 4, 7, r, false)) {
          error("On move %d castling was attempted, but king would be in check exists on square %c8.", G->move + 1, abcdefgh_[r]);
        }
      }
    }
  }
}

void apply_castling(Game * G, bool queenside, Color sideToMove) {
  verify_castling(G, queenside, sideToMove);
  switch(sideToMove) {
  case WHITE:
    if (queenside) {

    } else {

    }
    break;
  case BLACK:
    if (queenside) {

    } else {

    }
    break;
  }
}

void apply_move2game(Game * G, Move M, Color sideToMove) {
  unsigned int move = G->move;

  ++move;
  if (move >= LONG_GAME) {
    error("move = %d >= LONG_GAME = %d", move, LONG_GAME);
  }
  switch(M.toPiece) {
  case KING:
    if (sideToMove == WHITE) {
      G->Board.WhiteKing = rowcol2p(M.toRow, M.toCol);
      if (G->whiteLostCastlingRights == LONG_GAME) {
        G->whiteLostCastlingRights = move;
      }
      G->Board.lastMove = M;
      G->Board.board[M.fromRow][M.fromCol].piece = EMPTY;
      G->Board.board[M.toRow][M.toCol].piece = KING;
      G->Board.board[M.toRow][M.toCol].color = WHITE;
      G->Moves[move][0] = M;
    } else {
      G->Board.BlackKing = rowcol2p(M.toRow, M.toCol);
      if (G->blackLostCastlingRights == LONG_GAME) {
        G->blackLostCastlingRights = move;
      }
      G->Board.lastMove = M;
      G->Board.board[M.fromRow][M.fromCol].piece = EMPTY;
      G->Board.board[M.toRow][M.toCol].piece = KING;
      G->Board.board[M.toRow][M.toCol].color = BLACK;
      G->Moves[move][1] = M;
    }
    break;
  default:
    if (sideToMove == WHITE) {
      G->Board.lastMove = M;
      bool is_enpassant =
        M.toPiece == PAWN &&
        M.toRow == 6 &&
        M.toCol != M.fromCol &&
        G->Board.board[M.toRow][M.toCol].piece == EMPTY &&
        G->Board.board[M.fromRow][M.toCol].piece == PAWN &&
        G->Board.board[M.fromRow][M.toCol].color == BLACK;

      G->Board.board[M.fromRow][M.fromCol].piece = EMPTY;
      G->Board.board[M.toRow][M.toCol].piece = M.toPiece;
      G->Board.board[M.toRow][M.toCol].color = WHITE;
      if (is_enpassant) {
        G->Board.board[M.fromRow][M.toCol].piece = EMPTY;
      }
      G->Moves[move][0] = M;


    } else {
      G->Board.lastMove = M;
      bool is_enpassant =
        M.toPiece == PAWN &&
        M.toRow == 4 &&
        M.toCol != M.fromCol &&
        G->Board.board[M.toRow][M.toCol].piece == EMPTY &&
        G->Board.board[M.fromRow][M.toCol].piece == PAWN &&
        G->Board.board[M.fromRow][M.toCol].color == WHITE;
      G->Board.board[M.fromRow][M.fromCol].piece = EMPTY;
      G->Board.board[M.toRow][M.toCol].piece = M.toPiece;
      G->Board.board[M.toRow][M.toCol].color = BLACK;
      if (is_enpassant) {
        G->Board.board[M.fromRow][M.toCol].piece = EMPTY;
      }
      G->Moves[move][1] = M;
    }
  }
  Material Mat;
  determine_material(&Mat, &(G->Board), WHITE);
  G->white_material[move] = total_material(&Mat);
  determine_material(&Mat, &(G->Board), BLACK);
  G->black_material[move] = total_material(&Mat);
  if (M.toPiece == PAWN) {
    G->last_pawn_move = move;
  }
  G->sideToMove = OPPCOLOR;
  G->move += (sideToMove == BLACK);
}

void setup_board(Chessboard * board, SEXP x, SEXP y, SEXP Start, Color sideToMove, SEXP LastMove) {
  const int start = asInteger(Start);
  if (start == 0) {
    blankBoard(board);
    board->WhiteMayCastle = 0;
    board->BlackMayCastle = 0;
  } else if (start == 1) {
    startingPosition(board);
    if (!length(x) && !length(y) && !length(LastMove)) {
      return; // just the starting position
    }
  } else {
    error("Invalid start.");
  }
  int n = length(x);
  const SEXP * xp = STRING_PTR(x);
  for (int i = 0; i < n; ++i) {
    if (xp[i] == NA_STRING) {
      continue;
    }
    int ni = length(xp[i]);
    const char * xi = CHAR(xp[i]);
    if (ni == 0) {
      continue;
    }
    unsigned int p = string2p(xi);
    int r = p2row(p);
    int c = p2col(p);
    board->board[r][c].color = WHITE;
    if (islower(xi[0])) {
      board->board[r][c].piece = PAWN;
      continue;
    }
    switch(xi[0]) {
    case 'K': {
      board->board[r][c].piece = KING;
      board->WhiteKing = p;
    }
      break;
    case 'Q':
      board->board[r][c].piece = QUEEN;
      break;
    case 'B':
      board->board[r][c].piece = BISHOP;
      break;
    case 'N':
      board->board[r][c].piece = KNIGHT;
      break;
    case 'R':
      board->board[r][c].piece = ROOK;
      break;
    default:
      error("Could not determine piece from string '%s'.", xi);
    }
  }
  // Now black
  n = length(y);
  const SEXP * yp = STRING_PTR(y);
  for (int i = 0; i < n; ++i) {
    if (yp[i] == NA_STRING) {
      continue;
    }
    int ni = length(yp[i]);
    const char * xi = CHAR(yp[i]);
    if (ni == 0) {
      continue;
    }
    unsigned int p = string2p(xi);
    int r = p2row(p);
    int c = p2col(p);
    board->board[r][c].color = BLACK;
    if (islower(xi[0])) {
      board->board[r][c].piece = PAWN;
      continue;
    }
    switch(xi[0]) {
    case 'K': {
      board->board[r][c].piece = KING;
      board->BlackKing = p;
    }
      break;
    case 'Q':
      board->board[r][c].piece = QUEEN;
      break;
    case 'R':
      board->board[r][c].piece = ROOK;
      break;
    case 'B':
      board->board[r][c].piece = BISHOP;
      break;
    case 'N':
      board->board[r][c].piece = KNIGHT;
      break;
    default:
      error("Could not determine piece from string '%s'.", xi);
    }
  }
  if (!isString(LastMove)) {
    error("LastMove was type '%s' not STRSXP.", type2char(TYPEOF(LastMove)));
  }
  int len_last_move = length(STRING_ELT(LastMove, 0));
  if (len_last_move != 4 && len_last_move != 5) {
    error("len_last_move = %d, but must be length 4 or 5. e.g. Qa1a7", len_last_move);
  }
  const char * last_move = CHAR(STRING_ELT(LastMove, 0));
  if (len_last_move == 4) {
    board->lastMove.toPiece = PAWN;
    if (last_move[0] < 'a' || last_move[0] > 'h') {
      error("last_move[0] was not in a-h");
    }
    board->lastMove.fromCol = last_move[0] - 'a';
    if (last_move[1] < '1' || last_move[1] > '8') {
      error("last_move[1] was not in 1-8");
    }
    board->lastMove.fromRow = last_move[1] - '1';
    if (last_move[2] < 'a' || last_move[2] > 'h') {
      error("last_move[2] was not in a-h");
    }
    board->lastMove.toCol = last_move[2] - 'a';
    if (last_move[3] < '1' || last_move[3] > '8') {
      error("last_move[3] was not in 1-8");
    }
    board->lastMove.toRow = last_move[3] - '1';

  } else {
    board->lastMove.toPiece = string2Piece(last_move);
    if (last_move[1] < 'a' || last_move[1] > 'h') {
      error("last_move[1] was not in a-h");
    }
    board->lastMove.fromCol = last_move[1] - 'a';
    if (last_move[2] < '1' || last_move[2] > '8') {
      error("last_move[2] was not in 1-8");
    }
    board->lastMove.fromRow = last_move[2] - '1';
    if (last_move[3] < 'a' || last_move[3] > 'h') {
      error("last_move[3] was not in a-h");
    }
    board->lastMove.toCol = last_move[3] - 'a';
    if (last_move[4] < '1' || last_move[4] > '8') {
      error("last_move[4] was not in 1-8");
    }
    board->lastMove.toRow = last_move[4] - '1';
  }


  if (isntValidBoard(board, sideToMove)) {
    error("isntValidBoard[%d]", isntValidBoard(board, sideToMove));
  }
}

SEXP C_isCheckmate(SEXP x, SEXP y, SEXP Start, SEXP WhiteToMove, SEXP LastMove) {
  bool white_to_move = asLogical(WhiteToMove);
  Color sideToMove = white_to_move ? WHITE : BLACK;
  Chessboard Board;
  setup_board(&Board, x, y, Start, sideToMove, LastMove);
  return ScalarLogical(isCheckmate(&Board, sideToMove));
}



bool threefold_repetition(Game * G) {
  unsigned int move = G->move;
  unsigned int w_material = G->white_material[move];
  unsigned int b_material = G->black_material[move];

  int n_repetitions = 1;
  while (--move > G->last_pawn_move) {
    if (w_material != G->white_material[move] || b_material != G->black_material[move]) {
      break;
    }



  }
  return n_repetitions >= 3;
}

bool insufficient_material(Material * M_white, Material * M_black) {
  // if any pawns, queens, or rooks on the board
  if (M_white->P || M_black->P || M_white->Q || M_black->Q || M_white->R || M_black->R) {
    return false;
  }
  // bishop pair is not insufficient
  if ((M_white->B_dark && M_white->B_light) || (M_black->B_dark && M_black->B_light)) {
    return false;
  }

  // knight and king v lone king
  if (M_white->B_dark == 0 && M_white->B_light == 0 && M_white->N <= 1 && M_black->B_dark == 0 && M_black->B_light == 0 && M_black->N == 0) {
    return true;
  }
  if (M_black->B_dark == 0 && M_black->B_light == 0 && M_black->N <= 1 && M_white->B_dark == 0 && M_white->B_light == 0 && M_white->N == 0) {
    return true;
  }
  if (M_white->B_dark == 1 && M_black->B_dark == 1 && M_white->N == 0 && M_black->N == 0) {
    return true;
  }
  if (M_white->B_light == 1 && M_black->B_light == 1 && M_white->N == 0 && M_black->N == 0) {
    return true;
  }

  return false; // unusual position
}

bool hasInsufficientMaterial(const Chessboard * board) {
  Material W;
  memset(&W, 0, sizeof(W));
  determine_material(&W, board, WHITE);
  Material B;
  memset(&B, 0, sizeof(B));
  determine_material(&B, board, BLACK);
  return insufficient_material(&W, &B);
}

bool isDraw(const Chessboard * board, Color sideToMove) {
  if (hasInsufficientMaterial(board)) {
    return true;
  }
  Move possibleMoves[MAX_MOVES];
  int numPossibleMoves = generateMoves(board, sideToMove, possibleMoves);
  if (numPossibleMoves == 0 && !isKingInCheck(board, sideToMove)) {
    // stalemate
    return true;
  }
  return false;
}

SEXP C_canEnPassant(SEXP x, SEXP y, SEXP Start, SEXP WhiteToMove, SEXP LastMove) {
  bool white_to_move = asLogical(WhiteToMove);
  Color sideToMove = white_to_move ? WHITE : BLACK;
  Chessboard Board;
  setup_board(&Board, x, y, Start, sideToMove, LastMove);
  int cols_to_enpassant[8] = {0};
  colsMayEnPassant(cols_to_enpassant, &Board);
  for (int j = 0; j < 8; ++j) {
    if (cols_to_enpassant[j]) {
      return(ScalarInteger(cols_to_enpassant[j] * (j + 1)));
    }
  }
  return ScalarInteger(0);
}

SEXP C_game2outcome(SEXP x, SEXP y) {
  Game G;
  initialize_Game(&G);
  int n = length(x);
  if (n != length(y) && (n - 1) != length(y)) {
    error("Lengths of x and y do not agree. length(x) = %d, length(y) = %d", length(x), length(y));
  }
  const SEXP * xp = STRING_PTR(x);
  const SEXP * yp = STRING_PTR(y);
  for (int i = 0; i < n; ++i) {
    const char * xpi = CHAR(xp[i]);
    int npi = length(xp[i]);
    if (xpi[0] == 'O' && xpi[1] == '-') {
      apply_castling(&G, npi > 3, WHITE);
      continue;
    }

    Move M_i = string2move(xpi, npi, &(G.Board), WHITE);
    apply_move2game(&G, M_i, WHITE);
    if (i < length(y)) {
      M_i = string2move(CHAR(yp[i]), length(yp[i]), &(G.Board), BLACK);
      apply_move2game(&G, M_i, BLACK);
    }

  }
  if (isCheckmate(&(G.Board), WHITE)) {
    return ScalarInteger(-1);
  }

  if (isCheckmate(&(G.Board), BLACK)) {
    return ScalarInteger(1);
  }

  return ScalarInteger(0);
}







