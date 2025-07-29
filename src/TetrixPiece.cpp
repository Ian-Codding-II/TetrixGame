#include "TetrixPiece.h"
#include <random>

void TetrixPiece::setShape(TetrixShape shape) {
    static const int coordsTable[8][4][2] = {
        { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }, // NoShape
        { { 0, -1 }, { 0, 0 }, { -1, 0 }, { -1, 1 } }, // ZShape
        { { 0, -1 }, { 0, 0 }, { 1, 0 }, { 1, 1 } }, // SShape
        { { 0, -1 }, { 0, 0 }, { 0, 1 }, { 0, 2 } }, // LineShape
        { { -1, 0 }, { 0, 0 }, { 1, 0 }, { 0, 1 } }, // TShape
        { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } }, // SquareShape
        { { -1, -1 }, { 0, -1 }, { 0, 0 }, { 0, 1 } }, // LShape
        { { 1, -1 }, { 0, -1 }, { 0, 0 }, { 0, 1 } } // MirroredLShape
    };

    for (int i = 0; i < 4; ++i) {
        coords[i][0] = coordsTable[static_cast<int>(shape)][i][0];
        coords[i][1] = coordsTable[static_cast<int>(shape)][i][1];
    }
    pieceShape = shape;
}

void TetrixPiece::setRandomShape() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1, 7);
    setShape(static_cast<TetrixShape>(dis(gen)));
}

int TetrixPiece::minX() const {
    int min = coords[0][0];
    for (int i = 1; i < 4; ++i) {
        min = std::min(min, coords[i][0]);
    }
    return min;
}

int TetrixPiece::maxX() const {
    int max = coords[0][0];
    for (int i = 1; i < 4; ++i) {
        max = std::max(max, coords[i][0]);
    }
    return max;
}

int TetrixPiece::minY() const {
    int min = coords[0][1];
    for (int i = 1; i < 4; ++i) {
        min = std::min(min, coords[i][1]);
    }
    return min;
}

int TetrixPiece::maxY() const {
    int max = coords[0][1];
    for (int i = 1; i < 4; ++i) {
        max = std::max(max, coords[i][1]);
    }
    return max;
}

TetrixPiece TetrixPiece::rotatedLeft() const {
    if (pieceShape == TetrixShape::SquareShape) {
        return *this;
    }
    TetrixPiece result;
    result.pieceShape = pieceShape;
    for (int i = 0; i < 4; ++i) {
        result.setX(i, y(i));
        result.setY(i, -x(i));
    }
    return result;
}

TetrixPiece TetrixPiece::rotatedRight() const {
    if (pieceShape == TetrixShape::SquareShape) {
        return *this;
    }
    TetrixPiece result;
    result.pieceShape = pieceShape;
    for (int i = 0; i < 4; ++i) {
        result.setX(i, -y(i));
        result.setY(i, x(i));
    }
    return result;
}