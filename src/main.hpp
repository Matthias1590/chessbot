#pragma once

#include <vector>

class Location {
public:
    int x, y;

    Location(int x, int y);
};

class Move {
public:
    Location from, to;
    bool isCastle;
    Location castleFrom, castleTo;

    Move(Location from, Location to);
    Move(Location from, Location to, Location castleFrom, Location castleTo);

    std::string display();
};

class Board;

class Piece {
public:
    enum Type { Pawn, Knight, Bishop, Rook, Queen, King };
    enum Color { White, Black };

    Type type;
    Color color;

public:
    Piece(Type type, Color color);

    void print();
    int value();

    std::vector<Move> getMoves(Board b, Location location, bool checkMoves);
};

class Board {
public:
    std::optional<Piece> board[8][8];
    Piece::Color turn;
    bool inCheck;
    bool whiteCanCastleKingside;
    bool whiteCanCastleQueenside;
    bool blackCanCastleKingside;
    bool blackCanCastleQueenside;

    std::optional<Piece>& at(int x, int y);

    Board(std::string fen);

    void print();
    void applyMove(Move m);
    std::vector<Move> getMoves(bool checkMoves);
    Move getBestMove(int depth, bool maximize);
    bool hasKings();
    Move getStockfishMove();
    std::string getFen();
    int eval(int depth, bool maximize);
    int evalMat(Piece::Color color);
    int evalMob(Piece::Color color);
    int evalKingSafety(Piece::Color color);
    Location getKingLoc(Piece::Color color);
    int evalPieceSquares(Piece::Color color);
    int getStockfishScore();
    void updateInCheck();
};
