#include <iostream>
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
    : from(from), to(to)
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
    this->turn = Piece::Color::White;

    int row = 0;
    int col = 0;

    for (char c : fen) {
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

void Board::applyMove(Move m) {
    this->turn = this->turn == Piece::White ? Piece::Black : Piece::White;
    this->at(m.to.x, m.to.y) = this->at(m.from.x, m.from.y);
    this->at(m.from.x, m.from.y) = std::nullopt;
    
    // pawn promotion
    if ((m.to.y == 0 || m.to.y == 7) && this->at(m.to.x, m.to.y).value().type == Piece::Pawn) {
        auto color = this->at(m.to.x, m.to.y).value().color;
        this->at(m.to.x, m.to.y) = Piece(Piece::Queen, color);
    }
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

Move Board::getBestMove() {
    auto moves = this->getMoves(true);
    return moves[random() % moves.size()];
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

int main() {
    std::cout << "\n";
    srand(time(NULL));

    Board b = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");  // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR

    std::cout << "[INITIAL]" << std::endl;
    b.print();
    std::cout << "\n";

    while (b.getMoves(true).size() > 0) {
        auto move = b.getBestMove();
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
