#ifndef TETRIXBOARD_H
#define TETRIXBOARD_H

#include <QFrame>
#include <QBasicTimer>
#include <QSoundEffect>
#include <QVector>
#include <QTimer>
#include <QMap>
#include "TetrixPiece.h"

class QLabel;

class TetrixBoard : public QFrame {
    Q_OBJECT

public:
    explicit TetrixBoard(QWidget *parent = nullptr);
    ~TetrixBoard();

    void setNextPieceLabel(QLabel *label);
    QLabel *nextPieceLabel;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    // Public method to stop game-over sound
    void stopGameOverSound() {
        if (gameOverSound) {
            gameOverSound->stop();
            gameOverCycleCount++;
            qDebug() << "Game-over sound stopped, cycle count:" << gameOverCycleCount << ", status:" << gameOverSound->status() << ", isPlaying:" << gameOverSound->isPlaying();
            if (gameOverCycleCount >= MaxSoundCycles) {
                resetSound(&gameOverSound, "/home/time/introCode/c++/TetrixGame/sounds/gameover.wav", true, gameOverCycleCount);
            }
        }
    }

    // Getter for game started state
    bool isGameStarted() const { return isStarted; }

public slots:
    void start();
    void pause();

signals:
    void scoreChanged(int score);
    void levelChanged(int level);
    void linesRemovedChanged(int linesRemoved);
    void gameOver(int score);
    void pauseStateChanged(bool isPaused); // Signal to notify pause state changes

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private slots:
    void onSoundStatusChanged(QSoundEffect::Status status);

private:
    enum { BoardWidth = 10, BoardHeight = 22, MaxSoundCycles = 5 }; // 10x22 grid, max 5 sound cycles

    TetrixShape &shapeAt(int x, int y) { return board[y * BoardWidth + x]; }
    const TetrixShape &shapeAt(int x, int y) const { return board[y * BoardWidth + x]; }
    void clearBoard();
    void dropDown();
    void oneLineDown();
    void pieceDropped(int dropHeight);
    void removeFullLines();
    void newPiece();
    void showNextPiece();
    bool tryMove(const TetrixPiece &newPiece, int newX, int newY);
    void drawSquare(QPainter &painter, int x, int y, TetrixShape shape, int squareSize);
    void resetSound(QSoundEffect **sound, const QString &filePath, bool isLooping, int &cycleCount);
    void playSoundWithDelay(QSoundEffect *sound, int &cycleCount, const QString &filePath, bool isLooping);

    QBasicTimer timer;
    QTimer *flashTimer; // Timer for line-clear animation
    QTimer *soundDelayTimer; // Timer for sound playback delay
    TetrixPiece currentPiece;
    TetrixPiece nextPiece;
    bool isStarted;
    bool isPaused;
    bool isWaitingAfterLine;
    bool isDropping; // Flag to prevent multiple dropDown calls
    bool flashState; // Toggle for flashing effect
    int curX;
    int curY;
    int numLinesRemoved;
    int numPiecesDropped;
    int score;
    int level;
    TetrixShape board[BoardWidth * BoardHeight];
    QSoundEffect *dropSound;
    QSoundEffect *lineClearSound;
    QSoundEffect *gameOverSound;
    QSoundEffect *backgroundSound; // Background music
    QVector<int> flashingLines; // Lines to animate
    // Counters for tracking play/stop cycles
    int dropCycleCount;
    int lineClearCycleCount;
    int gameOverCycleCount;
    int backgroundCycleCount;
    QMap<QSoundEffect*, QString> soundFilePaths; // Map to track sound file paths
};

#endif // TETRIXBOARD_H