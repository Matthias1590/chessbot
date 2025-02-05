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

    Move(Location from, Location to);
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

    std::vector<Move> getMoves(Board b, Location location, bool checkMoves);
};

class Board {
public:
    std::optional<Piece> board[8][8];
    Piece::Color turn;

    std::optional<Piece>& at(int x, int y);

    Board(std::string fen);

    void print();
    void applyMove(Move m);
    std::vector<Move> getMoves(bool checkMoves);
    Move getBestMove();
    bool hasKings();
    Move getStockfishMove();
    std::string getFen();
};
