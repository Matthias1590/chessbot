#include <signal.h>
#include <iostream>
#include <fstream>
#include <optional>
#include <vector>
#include <cstdio>
#include <memory>
#include <thread>
#include <chrono>
#include "main.hpp"

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

Location::Location(int x, int y)
    : x(x), y(y)
{}

char typeToFen(Piece::Type type) {
    switch (type) {
    case Piece::Pawn:
        return 'p';
    case Piece::Knight:
        return 'n';
    case Piece::Bishop:
        return 'b';
    case Piece::Rook:
        return 'r';
    case Piece::Queen:
        return 'q';
    case Piece::King:
        return 'k';
    default:
        std::cerr << "Invalid piece type" << std::endl;
        exit(1);
    }
}

char colorToFen(Piece::Color color, char c) {
    if (color == Piece::White) {
        return toupper(c);
    } else {
        return tolower(c);
    }
}

Piece::Type typeFromFen(char c) {
    switch (tolower(c)) {
    case 'p':
        return Piece::Pawn;
    case 'n':
        return Piece::Knight;
    case 'b':
        return Piece::Bishop;
    case 'r':
        return Piece::Rook;
    case 'q':
        return Piece::Queen;
    case 'k':
        return Piece::King;
    default:
        std::cerr << "Invalid piece type" << std::endl;
        exit(1);
    }
}

Piece::Color colorFromFen(char c) {
    if (isupper(c)) {
        return Piece::White;
    } else {
        return Piece::Black;
    }
}

Piece::Piece(Type type, Color color)
    : type(type), color(color)
{}

void Piece::print() {
    char c = typeToFen(this->type);
    c = colorToFen(this->color, c);
    std::cout << c << " ";
}

Move::Move(Location from, Location to)
    : from(from), to(to), isCastle(false), castleFrom(Location(-1, -1)), castleTo(Location(-1, -1))
{}

Move::Move(Location from, Location to, Location castleFrom, Location castleTo)
    : from(from), to(to), isCastle(true), castleFrom(castleFrom), castleTo(castleTo)
{}

std::vector<Move> Piece::getMoves(Board b, Location location, bool checkMoves) {
    std::vector<Move> moves;

    switch (this->type) {
    case Pawn: {
        if (this->color == Piece::White) {
            if (location.y == 6 && !b.at(location.x, location.y - 2).has_value() && !b.at(location.x, location.y - 1).has_value()) {
                moves.push_back(Move(location, Location(location.x, location.y - 2)));
            }
            if (!b.at(location.x, location.y - 1).has_value()) {
                moves.push_back(Move(location, Location(location.x, location.y - 1)));
            }
        } else {
            if (location.y == 1 && !b.at(location.x, location.y + 2).has_value() && !b.at(location.x, location.y + 1).has_value()) {
                moves.push_back(Move(location, Location(location.x, location.y + 2)));
            }
            if (!b.at(location.x, location.y + 1).has_value()) {
                moves.push_back(Move(location, Location(location.x, location.y + 1)));
            }
        }

        int offset = this->color == Piece::White ? -1 : 1;
        // top-left
        int x = location.x - 1;
        int y = location.y + offset;

        if (x >= 0 && y >= 0 && b.at(x, y).has_value() && b.at(x, y).value().color != this->color) {
            moves.push_back(Move(location, Location(x, y)));
        }

        // top-right
        x = location.x + 1;
        y = location.y + offset;

        if (x < 8 && y >= 0 && b.at(x, y).has_value() && b.at(x, y).value().color != this->color) {
            moves.push_back(Move(location, Location(x, y)));
        }
    } break;
    case Knight: {
        int offsets[][2] = {
            {-2, -1}, {-2, 1},
            {-1, -2}, {-1, 2},
            {1, -2}, {1, 2},
            {2, -1}, {2, 1}
        };

        for (int i = 0; i < 8; i++) {
            int x = location.x + offsets[i][0];
            int y = location.y + offsets[i][1];
            if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                if (!b.at(x, y).has_value() || b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
            }
        }
    } break;
    case Bishop: {
        // top-left
        int x = location.x;
        int y = location.y;

        for (int i = 0; i < 7; i++) {
            x--;
            y--;
            if (x < 0 || y < 0) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // top-right
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            x++;
            y--;
            if (x >= 8 || y < 0) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // bottom-left
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            x--;
            y++;
            if (x < 0 || y >= 8) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // bottom-right
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            x++;
            y++;
            if (x >= 8 || y >= 8) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }
    } break;
    case Rook: {
        // left
        int x = location.x;
        int y = location.y;

        for (int i = 0; i < 7; i++) {
            x--;
            if (x < 0) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // up
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            y--;
            if (y < 0) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // right
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            x++;
            if (x >= 8) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // down
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            y++;
            if (y >= 8) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }
    } break;
    case Queen: {
        // top-left
        int x = location.x;
        int y = location.y;

        for (int i = 0; i < 7; i++) {
            x--;
            y--;
            if (x < 0 || y < 0) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // left
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            x--;
            if (x < 0) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // bottom-left
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            x--;
            y++;
            if (x < 0 || y >= 8) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // down
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            y++;
            if (y >= 8) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // bottom-right
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            x++;
            y++;
            if (x >= 8 || y >= 8) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // right
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            x++;
            if (x >= 8) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // top-right
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            x++;
            y--;
            if (x >= 8 || y < 0) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }

        // up
        x = location.x;
        y = location.y;

        for (int i = 0; i < 7; i++) {
            y--;
            if (y < 0) {
                break;
            }
            if (!b.at(x, y).has_value()) {
                moves.push_back(Move(location, Location(x, y)));
            } else {
                if (b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
                break;
            }
        }
    } break;
    case King: {
        int offsets[][2] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            {0, -1}, {0, 1},
            {1, -1}, {1, 0}, {1, 1}
        };

        for (int i = 0; i < 8; i++) {
            int x = location.x + offsets[i][0];
            int y = location.y + offsets[i][1];
            if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                if (!b.at(x, y).has_value() || b.at(x, y).value().color != this->color) {
                    moves.push_back(Move(location, Location(x, y)));
                }
            }
        }

        // castling (only allowed if next to the king in the castle direction is not in check)
        if (!b.inCheck) {
            if (this->color == Piece::White) {
                if (b.whiteCanCastleKingside && !b.at(5, 7).has_value() && !b.at(6, 7).has_value()) {
                    moves.push_back(Move(location, Location(6, 7), Location(7, 7), Location(5, 7)));
                }
                if (b.whiteCanCastleQueenside && !b.at(1, 7).has_value() && !b.at(2, 7).has_value() && !b.at(3, 7).has_value()) {
                    moves.push_back(Move(location, Location(2, 7), Location(0, 7), Location(3, 7)));
                }
            } else {
                if (b.blackCanCastleKingside && !b.at(5, 0).has_value() && !b.at(6, 0).has_value()) {
                    moves.push_back(Move(location, Location(6, 0), Location(7, 0), Location(5, 0)));
                }
                if (b.blackCanCastleQueenside && !b.at(1, 0).has_value() && !b.at(2, 0).has_value() && !b.at(3, 0).has_value()) {
                    moves.push_back(Move(location, Location(2, 0), Location(0, 0), Location(3, 0)));
                }
            }
        }
    } break;
    }

    if (!checkMoves) {
        return moves;
    }

    std::vector<Move> legalMoves;

    for (Move m : moves) {
        Board copy = b;
        copy.applyMove(m);
        bool inCheck = false;

        auto nextMoves = copy.getMoves(false);
        for (Move m2 : nextMoves) {
            if (copy.at(m2.to.x, m2.to.y).has_value() && copy.at(m2.to.x, m2.to.y).value().type == Piece::King) {
                inCheck = true;
                break;
            }
        }

        if (!inCheck) {
            legalMoves.push_back(m);
        }
    }

    return legalMoves;
}

std::optional<Piece>& Board::at(int x, int y) {
    return this->board[x][y];
}

Board::Board(std::string fen) {
    this->whiteCanCastleKingside = true; // todo: take from fen
    this->whiteCanCastleQueenside = true;
    this->blackCanCastleKingside = true;
    this->blackCanCastleQueenside = true;

    this->turn = fen.find(" w ") != std::string::npos ? Piece::White : Piece::Black;
    if (fen.find(" b ") == std::string::npos && fen.find(" w ") == std::string::npos) {
        this->turn = Piece::White;
    }

    int row = 0;
    int col = 0;

    for (char c : fen) {
        if (c == ' ')
            break;
        if (c == '/') {
            row++;
            col = 0;
            continue;
        }
        if (isdigit(c)) {
            col += c - '0';
            continue;
        }
        Piece::Type type = typeFromFen(c);
        Piece::Color color = colorFromFen(c);
        this->at(col, row) = Piece(type, color);
        col++;
    }

    this->updateInCheck();
}

void Board::print() {
    std::cout << getFen() << std::endl;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto p = this->at(col, row);
            if (p.has_value()) {
                p.value().print();
            } else {
                std::cout << ". ";
            }
        }
        std::cout << std::endl;
    }
}

void Board::updateInCheck() {
    this->inCheck = false;
    auto moves = this->getMoves(false);
    for (Move m : moves) {
        if (this->at(m.to.x, m.to.y).has_value() && this->at(m.to.x, m.to.y).value().type == Piece::King) {
            this->inCheck = true;
            break;
        }
    }
}

bool DEBUG = false;

void Board::applyMove(Move m) {
    // if its a king move and it moves 2 squares, turn it into a castle move
    if (this->at(m.from.x, m.from.y).value().type == Piece::King && abs(m.to.x - m.from.x) == 2) {
        m.isCastle = true;
        m.castleFrom = m.from.x < m.to.x ? Location(7, m.from.y) : Location(0, m.from.y);
        m.castleTo = m.from.x < m.to.x ? Location(5, m.from.y) : Location(3, m.from.y);
    }

    // if from is king, cant castle anymore
    if (this->at(m.from.x, m.from.y).value().type == Piece::King) {
        if (this->at(m.from.x, m.from.y).value().color == Piece::White) {
            this->whiteCanCastleKingside = false;
            this->whiteCanCastleQueenside = false;
        } else {
            this->blackCanCastleKingside = false;
            this->blackCanCastleQueenside = false;
        }
    }
    // if from or to is our rook, cant castle with that rook anymore
    if (this->at(m.from.x, m.from.y).value().type == Piece::Rook) {
        if (this->at(m.from.x, m.from.y).value().color == Piece::White) {
            if (m.from.x == 0) {
                this->whiteCanCastleQueenside = false;
            } else if (m.from.x == 7) {
                this->whiteCanCastleKingside = false;
            }
        } else {
            if (m.from.x == 0) {
                this->blackCanCastleQueenside = false;
            } else if (m.from.x == 7) {
                this->blackCanCastleKingside = false;
            }
        }
    }

    this->turn = this->turn == Piece::White ? Piece::Black : Piece::White;
    this->at(m.to.x, m.to.y) = this->at(m.from.x, m.from.y);
    this->at(m.from.x, m.from.y) = std::nullopt;
    if (m.isCastle) {
        if (DEBUG)
            std::cout << "Castling" << std::endl;
        this->at(m.castleTo.x, m.castleTo.y) = this->at(m.castleFrom.x, m.castleFrom.y);
        this->at(m.castleFrom.x, m.castleFrom.y) = std::nullopt;
    }
    
    // pawn promotion
    if ((m.to.y == 0 || m.to.y == 7) && this->at(m.to.x, m.to.y).value().type == Piece::Pawn) {
        auto color = this->at(m.to.x, m.to.y).value().color;
        this->at(m.to.x, m.to.y) = Piece(Piece::Queen, color);
    }

    this->updateInCheck();
}

std::vector<Move> Board::getMoves(bool checkMoves) {
    std::vector<Move> moves;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto p = this->at(col, row);
            if (p.has_value() && p.value().color == this->turn) {
                std::vector<Move> pieceMoves = p.value().getMoves(*this, Location(col, row), checkMoves);
                moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
    }

    return moves;
}

bool Board::hasKings() {
    bool whiteKing = false;
    bool blackKing = false;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            auto p = this->at(col, row);
            if (p.has_value() && p.value().type == Piece::King) {
                if (p.value().color == Piece::White) {
                    whiteKing = true;
                } else {
                    blackKing = true;
                }
            }
        }
    }

    return whiteKing && blackKing;
}

std::string Move::display() {
    std::string s = "";

    s += (char)('a' + this->from.x);
    s += std::to_string(8 - this->from.y);
    s += (char)('a' + this->to.x);
    s += std::to_string(8 - this->to.y);

    return s;
}

Move Board::getBestMove(int depth, bool maximize) {
    int bestScore = maximize ? -1000000 : 1000000;
    Move bestMove = Move(Location(-1, -1), Location(-1, -1));
    std::vector<Move> moves = this->getMoves(true);

    for (Move m : moves) {
        Board copy = *this;
        copy.applyMove(m);
        int score = copy.eval(depth, !maximize);
        if (maximize) {
            if (score > bestScore) {
                bestScore = score;
                bestMove = m;
            }
        } else {
            if (score < bestScore) {
                bestScore = score;
                bestMove = m;
            }
        }
    }

    return bestMove;
}

std::string Board::getFen() {
    std::string fen = "";

    for (int row = 0; row < 8; row++) {
        int empty = 0;
        for (int col = 0; col < 8; col++) {
            auto p = this->at(col, row);
            if (p.has_value()) {
                if (empty > 0) {
                    fen += std::to_string(empty);
                    empty = 0;
                }
                char c = typeToFen(p.value().type);
                fen += colorToFen(p.value().color, c);
            } else {
                empty++;
            }
        }
        if (empty > 0) {
            fen += std::to_string(empty);
        }
        if (row < 7) {
            fen += "/";
        }
    }

    return fen + " " + (this->turn == Piece::White ? "w" : "b") + " - - 0 1";
}

int Board::getStockfishScore() {
    std::string fen = this->getFen();
    std::string command = "./get_move.sh " + fen;
    std::string move = exec(command.c_str());

    int score = std::stoi(move);
    if (this->turn == Piece::White) {
        score = -score;
    }
    return score;
}

Move Board::getStockfishMove() {
    // run ./get_move.sh {current_fen}
    std::string fen = this->getFen();
    std::string command = "./get_move.sh " + fen;
    std::string move = exec(command.c_str());

    // parse move
    std::cout << "Command: " << command << std::endl;
    std::cout << "Output: " << move << std::endl;
    int x = move[0] - 'a';
    int y = 8 - (move[1] - '0');
    int x2 = move[2] - 'a';
    int y2 = 8 - (move[3] - '0');

    return Move(Location(x, y), Location(x2, y2));
}

int Board::evalMat(Piece::Color color) {
    int score = 0;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            auto p = this->at(x, y);
            if (!p.has_value()) {
                continue;
            }
            if (p.value().color != color) {
                continue;
            }
            score += p.value().value();
        }
    }

    return score;
}

int Board::evalMob(Piece::Color color) {
    int score = 0;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            auto p = this->at(x, y);
            if (!p.has_value()) {
                continue;
            }
            if (p.value().color != color) {
                continue;
            }

            auto moves = p.value().getMoves(*this, Location(x, y), true);
            score += moves.size();
        }
    }

    return score;
}

Location Board::getKingLoc(Piece::Color color) {
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            auto p = this->at(x, y);
            if (!p.has_value()) {
                continue;
            }
            if (p.value().type == Piece::King && p.value().color == color) {
                return Location(x, y);
            }
        }
    }

    return Location(-1, -1);
}

int Board::evalKingSafety(Piece::Color color) {
    int score = 0;
    int dir = color == Piece::White ? -1 : 1;

    auto kingLoc = this->getKingLoc(color);
    for (int x = kingLoc.x; x < kingLoc.x + 2; x++) {
        if (x < 0 || x >= 8) {
            continue;
        }
        auto pawnFound = false;
        for (int y = kingLoc.y; y >= 0 && y < 8; y += dir) {
            auto p = this->at(x, y);
            if (!p.has_value()) {
                continue;
            }
            if (p.value().color == color && p.value().type == Piece::Pawn) {
                pawnFound = true;
                break;
            }
        }
        if (!pawnFound) {
            score -= 10;
        }
    }

    return score;
}

int PIECE_SQUARE_TABLE_PAWN[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5, 5, 10, 25, 25, 10, 5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, -5, -10, 0, 0, -10, -5, 5,
    5, 10, 10, -20, -20, 10, 10, 5,
    0, 0, 0, 0, 0, 0, 0, 0
};

int PIECE_SQUARE_TABLE_KNIGHT[] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 0, 0, 0, -20, -40,
    -30, 0, 10, 15, 15, 10, 0, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 0, 15, 20, 20, 15, 0, -30,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

int PIECE_SQUARE_TABLE_BISHOP[] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 5, 10, 10, 5, 5, -10,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -10, 10, 10, 10, 10, 10, 10, -10,
    -10, 5, 0, 0, 0, 0, 5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

int PIECE_SQUARE_TABLE_ROOK[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    -5, 0, 0, 0, 0, 0, 0, -5,
    0, 0, 0, 5, 5, 0, 0, 0
};

int PIECE_SQUARE_TABLE_QUEEN[] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 0, 0, 0, 0, 0, -10,
    -10, 0, 5, 5, 5, 5, 0, -10,
    -5, 0, 5, 5, 5, 5, 0, -5,
    0, 0, 5, 5, 5, 5, 0, -5,
    -10, 5, 5, 5, 5, 5, 0, -10,
    -10, 0, 5, 0, 0, 0, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
};

int PIECE_SQUARE_TABLE_KING_MIDGAME[] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
    20, 20,   0,   0,   0,   0,  20,  20,
    20, 30,  10,   0,   0,  10,  30,  20,
};

int Board::evalPieceSquares(Piece::Color color) {
    int score = 0;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            auto p = this->at(x, y);
            if (!p.has_value()) {
                continue;
            }
            if (p.value().color != color) {
                continue;
            }

            int index = y * 8 + x;
            switch (p.value().type) {
            case Piece::Pawn:
                score += color == Piece::White ? PIECE_SQUARE_TABLE_PAWN[index] : PIECE_SQUARE_TABLE_PAWN[63 - index];
                break;
            case Piece::Knight:
                score += color == Piece::White ? PIECE_SQUARE_TABLE_KNIGHT[index] : PIECE_SQUARE_TABLE_KNIGHT[63 - index];
                break;
            case Piece::Bishop:
                score += color == Piece::White ? PIECE_SQUARE_TABLE_BISHOP[index] : PIECE_SQUARE_TABLE_BISHOP[63 - index];
                break;
            case Piece::Rook:
                score += color == Piece::White ? PIECE_SQUARE_TABLE_ROOK[index] : PIECE_SQUARE_TABLE_ROOK[63 - index];
                break;
            case Piece::Queen:
                score += color == Piece::White ? PIECE_SQUARE_TABLE_QUEEN[index] : PIECE_SQUARE_TABLE_QUEEN[63 - index];
                break;
            case Piece::King:
                score += color == Piece::White ? PIECE_SQUARE_TABLE_KING_MIDGAME[index] : PIECE_SQUARE_TABLE_KING_MIDGAME[63 - index];
                break;
            }
        }
    }

    return score;
}

int Board::eval(int depth, bool maximize) {
    if (depth <= 0) {  //  && !this->inCheck
        int score = 0;

        score += Board::evalMat(Piece::Color::White) * 1.6;
        score -= Board::evalMat(Piece::Color::Black) * 1.6;
        // score += Board::evalMob(Piece::Color::White) * 2;
        // score -= Board::evalMob(Piece::Color::Black) * 2;
        score += Board::evalKingSafety(Piece::Color::White) * 1.3;
        score -= Board::evalKingSafety(Piece::Color::Black) * 1.3;
        score += Board::evalPieceSquares(Piece::Color::White) * 1.8;
        score -= Board::evalPieceSquares(Piece::Color::Black) * 1.8;

        return score;
    }

    int bestScore = maximize ? -1000000 : 1000000;
    std::vector<Move> moves = this->getMoves(true);

    for (Move m : moves) {
        Board copy = *this;
        copy.applyMove(m);
        int score = copy.eval(depth - 1, !maximize);
        if (maximize) {
            bestScore = std::max(bestScore, score);
        } else {
            bestScore = std::min(bestScore, score);
        }
    }

    return bestScore;
}

int Piece::value() {
    switch (this->type) {
    case Piece::Type::Pawn:
        return 100;
    case Piece::Type::Knight:
        return 300;
    case Piece::Type::Bishop:
        return 340;
    case Piece::Type::Rook:
        return 520;
    case Piece::Type::Queen:
        return 930;
    case Piece::Type::King:
        return 20000;
    default:
        return 0;
    }
}

std::vector<std::string> split(std::string str, char delim) {
    std::vector<std::string> parts;
    std::string part = "";
    for (char c : str) {
        if (c == delim) {
            parts.push_back(part);
            part = "";
        } else {
            part += c;
        }
    }
    parts.push_back(part);
    return parts;
}

std::ofstream out;

void ragequit(int signum) {
    if (signum == SIGSEGV) {
        out << "RAGEQUIT" << std::endl;
        std::cerr << "RAGEQUIT" << std::endl;
        exit(1);
    }
}

int main(int argc, char **argv) {
    signal(SIGSEGV, ragequit);

    // log to file
    out.open("log.txt");

    out << "called with " << argc << " args" << std::endl;
    for (int i = 0; i < argc; i++) {
        out << "arg " << i << ": " << argv[i] << std::endl;
    }

    if (argc == 1) {
        Board board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
        while (1) {
            // read input
            std::string input;
            std::getline(std::cin, input);
            out << "Received '" << input << "'\n";
            if (input == "quit") {
                out << "Quitting" << std::endl;
                break;
            } else if (input == "uci") {
                out << "Responding with 'uciok'" << std::endl;
                std::cout << "uciok" << std::endl;
                continue;
            } else if (input == "isready") {
                out << "Responding with 'readyok'" << std::endl;
                std::cout << "readyok" << std::endl;
                continue;
            } else if (input == "ucinewgame") {
                continue;
            } else if (input == "position startpos") {
                continue;
            } else if (input.starts_with("position startpos moves")) {
                std::string moves = input.substr(24);
                board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
                std::vector<std::string> moveList = split(moves, ' ');
                for (std::string move : moveList) {
                    Location from = Location(move[0] - 'a', 8 - (move[1] - '0'));
                    Location to = Location(move[2] - 'a', 8 - (move[3] - '0'));
                    board.applyMove(Move(from, to));
                }
                continue;
            } else if (input.starts_with("go ")) {  // go wtime 300000 btime 300000 movestogo 40
                auto startTime = std::chrono::high_resolution_clock::now();
                auto bm = board.getBestMove(2, board.turn == Piece::White);
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
                if (duration < 300) {
                    bm = board.getBestMove(3, board.turn == Piece::White);
                }
                out << "Responding with 'bestmove " << bm.display() << "'\n";
                std::cout << "bestmove " << bm.display() << std::endl;
                out << "New board fen is " << board.getFen() << std::endl;
                continue;
            }

            // write input to log.txt
            out << "Couldnt handle '" << input << "'" << std::endl;
            break;
        }
        return 0;
    }

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <fen>" << std::endl;
        return 1;
    }

    srand(time(NULL));

    Board b = Board(argv[1]);  // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR

    // std::cout << "[INITIAL]" << std::endl;
    // b.print();
    // std::cout << "\n";

    bool maximizing = false;
    if (std::string(argv[1]).find(" w ") != std::string::npos) {
        maximizing = true;
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    auto bm = b.getBestMove(2, b.turn == Piece::White);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    if (duration < 300) {
        bm = b.getBestMove(3, b.turn == Piece::White);
    }

    std::cout << bm.display() << std::endl;
    DEBUG = true;
    b.applyMove(bm);
    b.print();
    DEBUG = false;
    bm = b.getBestMove(3, b.turn == Piece::White);
    std::cout << bm.display() << std::endl;
    DEBUG = true;
    b.applyMove(bm);
    b.print();
    while (1) {
        DEBUG = false;
        bm = b.getBestMove(3, b.turn == Piece::White);
        std::cout << bm.display() << std::endl;
        DEBUG = true;
        b.applyMove(bm);
        b.print();
    }
    return 0;

    std::cout << "us\tstockfish" << std::endl;
    std::cout << b.eval(5, maximizing) << "\t" << b.getStockfishScore() << std::endl;

    // for (int i = 0; ; i++) {
    //     auto score = b.eval(i, maximizing);
    //     std::cout << "depth " << i << std::endl;
    //     std::cout << "score " << score << std::endl;
    // }
    return 0;

    while (b.getMoves(true).size() > 0) {
        auto move = b.getBestMove(3, b.turn == Piece::White);
        b.applyMove(move);
        std::cout << "[CHECKMATE] Move: " << move.from.x << ", " << move.from.y << " to " << move.to.x << ", " << move.to.y << std::endl;
        b.print();
        std::cout << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // std::cin.get(); // 
        move = b.getStockfishMove();
        b.applyMove(move);
        std::cout << "[STOCKFISH] Move: " << move.from.x << ", " << move.from.y << " to " << move.to.x << ", " << move.to.y << std::endl;
        b.print();
        std::cout << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // std::cin.get(); // 
    }

    return 0;
}
