#include "TetrixBoard.h"
#include <QPainter>
#include <QKeyEvent>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include <QCoreApplication>
#include <QUrl>
#include <QDir>

TetrixBoard::TetrixBoard(QWidget *parent)
    : QFrame(parent), nextPieceLabel(nullptr), isStarted(false), isPaused(false), isWaitingAfterLine(false), isDropping(false), flashTimer(nullptr), soundDelayTimer(nullptr), flashState(false),
      dropCycleCount(0), lineClearCycleCount(0), gameOverCycleCount(0), backgroundCycleCount(0),
      dropSound(nullptr), lineClearSound(nullptr), gameOverSound(nullptr), backgroundSound(nullptr)
{
    // Remove default frame to avoid extra margins
    setFrameStyle(QFrame::NoFrame);
    setFocusPolicy(Qt::StrongFocus);
    clearBoard();

    // Ensure no maximum size constraint
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    nextPiece.setRandomShape();
    newPiece();

    // Initialize sound effects with absolute paths
    QString soundDir = "/home/time/introCode/c++/TetrixGame/sounds/";
    qDebug() << "Loading sounds from absolute path:" << soundDir;

    QStringList soundFiles = {"drop.wav", "lineclear.wav", "gameover.wav", "background.wav"};
    QSoundEffect **sounds[] = {&dropSound, &lineClearSound, &gameOverSound, &backgroundSound};
    bool isLooping[] = {false, false, true, true};
    int *cycleCounts[] = {&dropCycleCount, &lineClearCycleCount, &gameOverCycleCount, &backgroundCycleCount};

    for (int i = 0; i < 4; ++i) {
        QString filePath = soundDir + soundFiles[i];
        resetSound(sounds[i], filePath, isLooping[i], *cycleCounts[i]);
        soundFilePaths[*sounds[i]] = filePath;
        // Use lambda to adapt signal to slot
        connect(*sounds[i], &QSoundEffect::statusChanged, this, [this, sound = *sounds[i]]() {
            onSoundStatusChanged(sound->status());
        });
    }

    // Initialize flash timer for line-clear animation
    flashTimer = new QTimer(this);
    connect(flashTimer, &QTimer::timeout, this, [this]() {
        flashState = !flashState;
        update();
    });

    // Initialize sound delay timer (not used in simplified playback for testing)
    soundDelayTimer = new QTimer(this);
    soundDelayTimer->setSingleShot(true);
}

TetrixBoard::~TetrixBoard() {
    delete dropSound;
    delete lineClearSound;
    delete gameOverSound;
    delete backgroundSound;
}

void TetrixBoard::onSoundStatusChanged(QSoundEffect::Status status) {
    // Log status changes for debugging
    QSoundEffect *sound = qobject_cast<QSoundEffect*>(sender());
    QString filePath = soundFilePaths.value(sound, "unknown");
    qDebug() << "Sound status changed for" << filePath << ": status=" << status
             << ", isLoaded=" << (sound ? sound->isLoaded() : false)
             << ", isPlaying=" << (sound ? sound->isPlaying() : false);
}

void TetrixBoard::resetSound(QSoundEffect **sound, const QString &filePath, bool isLooping, int &cycleCount) {
    // Delete existing sound instance and create a new one to reset GStreamer pipeline
    if (*sound) {
        (*sound)->stop();
        delete *sound;
    }
    *sound = new QSoundEffect(this);
    (*sound)->setSource(QUrl::fromLocalFile(filePath));
    if (!QFile::exists(filePath)) {
        qDebug() << "ERROR: Sound file not found:" << filePath;
    } else {
        qDebug() << "Sound file loaded:" << filePath;
    }
    // Set volume: 0.7f for drop.wav, 0.3f for background.wav, 0.5f for others
    (*sound)->setVolume(filePath.endsWith("drop.wav") ? 0.7f : (filePath.endsWith("background.wav") ? 0.3f : 0.5f));
    if (isLooping) {
        (*sound)->setLoopCount(QSoundEffect::Infinite);
        qDebug() << "Sound set to loop indefinitely:" << filePath;
    }
    // Check if the sound is loaded
    if (!(*sound)->isLoaded()) {
        qDebug() << "WARNING: QSoundEffect reports not loaded for:" << filePath << "(may still play)";
    } else {
        qDebug() << "QSoundEffect successfully loaded for:" << filePath;
    }
    soundFilePaths[*sound] = filePath; // Update sound file path
    cycleCount = 0; // Reset cycle count
}

void TetrixBoard::playSoundWithDelay(QSoundEffect *sound, int &cycleCount, const QString &filePath, bool isLooping) {
    // Simplified playback for testing: play sound directly without delay
    if (cycleCount >= MaxSoundCycles || sound->status() == QSoundEffect::Error) {
        qDebug() << "Sound reached cycle limit or error, reinitializing, cycle count:" << cycleCount << ", file:" << filePath;
        resetSound(&sound, filePath, isLooping, cycleCount);
    }
    if (sound->isPlaying()) {
        sound->stop();
        qDebug() << "Sound was playing, stopped before restart, file:" << filePath;
    }
    // Play sound immediately and log details
    sound->play();
    cycleCount++;
    qDebug() << "Attempting to play sound, cycle count:" << cycleCount << ", file:" << filePath
             << ", status:" << sound->status() << ", isLoaded:" << sound->isLoaded()
             << ", isPlaying:" << sound->isPlaying();
}

void TetrixBoard::setNextPieceLabel(QLabel *label) {
    nextPieceLabel = label;
}

QSize TetrixBoard::sizeHint() const {
    // Suggest a size based on parent's dimensions, maintaining 10x22 aspect ratio
    QSize parentSize = parentWidget() ? parentWidget()->size() : QSize(600, 450);
    int scaleFactor = qMin(parentSize.width() / BoardWidth, parentSize.height() / BoardHeight);
    if (scaleFactor < 15) scaleFactor = 15; // Minimum square size
    qDebug() << "Board sizeHint: parentSize=" << parentSize << ", scaleFactor=" << scaleFactor
             << ", suggested size=" << QSize(BoardWidth * scaleFactor, BoardHeight * scaleFactor);
    return QSize(BoardWidth * scaleFactor, BoardHeight * scaleFactor);
}

QSize TetrixBoard::minimumSizeHint() const {
    // Minimum size for visibility (10x22 grid at 15px per square)
    qDebug() << "Board minimumSizeHint: width=" << BoardWidth * 15 << ", height=" << BoardHeight * 15;
    return QSize(BoardWidth * 15, BoardHeight * 15);
}

void TetrixBoard::start() {
    if (isPaused) {
        return;
    }

    isStarted = true;
    isWaitingAfterLine = false;
    isDropping = false; // Reset dropping state
    numLinesRemoved = 0;
    numPiecesDropped = 0;
    score = 0;
    level = 1;
    clearBoard();
    flashingLines.clear();

    emit linesRemovedChanged(numLinesRemoved);
    emit scoreChanged(score);
    emit levelChanged(level);
    emit pauseStateChanged(false); // Ensure pause state is reset

    newPiece();
    timer.start(1000 / level, this);
    playSoundWithDelay(backgroundSound, backgroundCycleCount, "/home/time/introCode/c++/TetrixGame/sounds/background.wav", true);
}

void TetrixBoard::pause() {
    if (!isStarted) {
        qDebug() << "Pause ignored: game not started";
        return;
    }

    isPaused = !isPaused;
    qDebug() << "Pause state changed: isPaused=" << isPaused;
    emit pauseStateChanged(isPaused); // Notify UI of pause state change

    if (isPaused) {
        timer.stop();
        flashTimer->stop();
        backgroundSound->stop();
        backgroundCycleCount++;
        qDebug() << "Game paused, background music stopped, cycle count:" << backgroundCycleCount
                 << ", status:" << backgroundSound->status() << ", isPlaying:" << backgroundSound->isPlaying();
    } else {
        timer.start(1000 / level, this);
        if (!flashingLines.isEmpty()) {
            flashTimer->start(200);
            qDebug() << "Resumed with active line flash";
        }
        playSoundWithDelay(backgroundSound, backgroundCycleCount, "/home/time/introCode/c++/TetrixGame/sounds/background.wav", true);
    }
    update();
}

void TetrixBoard::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);

    QPainter painter(this);
    QRect rect = contentsRect();
    qDebug() << "Board paintEvent: contentsRect=" << rect << ", widget size=" << size();

    // Calculate square size based on available space, maintaining aspect ratio
    int squareSize = qMin(rect.width() / BoardWidth, rect.height() / BoardHeight);
    if (squareSize < 15) squareSize = 15; // Minimum square size
    int boardWidthPx = BoardWidth * squareSize;
    int boardHeightPx = BoardHeight * squareSize;

    // Center the grid
    int boardLeft = rect.left() + (rect.width() - boardWidthPx) / 2;
    int boardTop = rect.top() + (rect.height() - boardHeightPx) / 2;
    qDebug() << "Board rendering: squareSize=" << squareSize << ", boardWidthPx=" << boardWidthPx
             << ", boardHeightPx=" << boardHeightPx << ", boardLeft=" << boardLeft << ", boardTop=" << boardTop;

    // Draw grid background
    painter.setPen(QPen(QColor("#585b70"), 1)); // Subtle gray color for grid lines
    for (int i = 0; i <= BoardHeight; ++i) {
        // Draw horizontal lines
        painter.drawLine(boardLeft, boardTop + i * squareSize, boardLeft + boardWidthPx, boardTop + i * squareSize);
    }
    for (int j = 0; j <= BoardWidth; ++j) {
        // Draw vertical lines
        painter.drawLine(boardLeft + j * squareSize, boardTop, boardLeft + j * squareSize, boardTop + boardHeightPx);
    }

    // Draw custom outline exactly around the 10x22 grid
    painter.setPen(QPen(QColor("#89b4fa"), 4));
    painter.drawRect(boardLeft, boardTop, boardWidthPx, boardHeightPx);

    // Draw the 10x22 grid
    for (int i = 0; i < BoardHeight; ++i) {
        for (int j = 0; j < BoardWidth; ++j) {
            TetrixShape shape = shapeAt(j, BoardHeight - i - 1);
            if (shape != TetrixShape::NoShape) {
                if (flashingLines.contains(BoardHeight - i - 1) && flashState) {
                    drawSquare(painter, boardLeft + j * squareSize, boardTop + i * squareSize, TetrixShape::ZShape, squareSize);
                } else {
                    drawSquare(painter, boardLeft + j * squareSize, boardTop + i * squareSize, shape, squareSize);
                }
            }
        }
    }

    // Draw the current piece
    if (currentPiece.shape() != TetrixShape::NoShape) {
        for (int i = 0; i < 4; ++i) {
            int x = curX + currentPiece.x(i);
            int y = curY - currentPiece.y(i);
            drawSquare(painter, boardLeft + x * squareSize, boardTop + (BoardHeight - y - 1) * squareSize, currentPiece.shape(), squareSize);
        }
    }

    // Draw "PAUSED" text when game is paused
    if (isPaused) {
        painter.setPen(QColor("#cdd6f4"));
        painter.setFont(QFont("Arial", qMax(16, squareSize / 2), QFont::Bold)); // Scale PAUSED text
        painter.drawText(rect, Qt::AlignCenter, tr("PAUSED"));
    }
}

void TetrixBoard::keyPressEvent(QKeyEvent *event) {
    // Ignore input if game is not started, paused, no piece exists, waiting after line clear, or dropping
    if (!isStarted || isPaused || currentPiece.shape() == TetrixShape::NoShape || isWaitingAfterLine || isDropping) {
        qDebug() << "Key press ignored: isStarted=" << isStarted
                 << ", isPaused=" << isPaused
                 << ", hasPiece=" << (currentPiece.shape() != TetrixShape::NoShape)
                 << ", isWaitingAfterLine=" << isWaitingAfterLine
                 << ", isDropping=" << isDropping
                 << ", key=" << event->key();
        QFrame::keyPressEvent(event);
        return;
    }

    // Process key input for game control
    qDebug() << "Processing key press: key=" << event->key();
    switch (event->key()) {
    case Qt::Key_Left:
        tryMove(currentPiece, curX - 1, curY);
        break;
    case Qt::Key_Right:
        tryMove(currentPiece, curX + 1, curY);
        break;
    case Qt::Key_Down:
        oneLineDown();
        break;
    case Qt::Key_Up:
        tryMove(currentPiece.rotatedRight(), curX, curY);
        break;
    case Qt::Key_Space:
        qDebug() << "Space bar pressed, initiating dropDown";
        dropDown();
        break;
    case Qt::Key_D:
        oneLineDown();
        break;
    default:
        QFrame::keyPressEvent(event);
    }
}

void TetrixBoard::timerEvent(QTimerEvent *event) {
    if (event->timerId() == timer.timerId()) {
        if (isWaitingAfterLine) {
            qDebug() << "Processing line clear: flashingLines=" << flashingLines;
            isWaitingAfterLine = false;
            for (int i : qAsConst(flashingLines)) {
                for (int k = i; k < BoardHeight - 1; ++k) {
                    for (int j = 0; j < BoardWidth; ++j) {
                        shapeAt(j, k) = shapeAt(j, k + 1);
                    }
                }
                for (int j = 0; j < BoardWidth; ++j) {
                    shapeAt(j, BoardHeight - 1) = TetrixShape::NoShape;
                }
            }
            flashingLines.clear();
            flashTimer->stop();
            qDebug() << "Lines cleared, animation stopped";
            newPiece();
            timer.start(1000 / level, this);
        } else {
            oneLineDown();
        }
    } else {
        QFrame::timerEvent(event);
    }
}

void TetrixBoard::clearBoard() {
    for (int i = 0; i < BoardHeight * BoardWidth; ++i) {
        board[i] = TetrixShape::NoShape;
    }
}

void TetrixBoard::dropDown() {
    // Prevent multiple drops
    if (isDropping) {
        qDebug() << "DropDown ignored: already dropping";
        return;
    }
    isDropping = true;
    qDebug() << "Starting dropDown: curX=" << curX << ", curY=" << curY;

    int dropHeight = 0;
    int newY = curY;
    while (newY > 0) {
        if (!tryMove(currentPiece, curX, newY - 1)) {
            break;
        }
        --newY;
        ++dropHeight;
    }
    qDebug() << "Piece dropped: dropHeight=" << dropHeight << ", curX=" << curX << ", curY=" << newY;
    pieceDropped(dropHeight);
    isDropping = false; // Reset after drop completes
}

void TetrixBoard::oneLineDown() {
    if (!tryMove(currentPiece, curX, curY - 1)) {
        pieceDropped(0);
    }
}

void TetrixBoard::pieceDropped(int dropHeight) {
    // Place piece on board
    for (int i = 0; i < 4; ++i) {
        int x = curX + currentPiece.x(i);
        int y = curY - currentPiece.y(i);
        // Safety check to prevent out-of-bounds access
        if (x < 0 || x >= BoardWidth || y < 0 || y >= BoardHeight) {
            qDebug() << "ERROR: Invalid board access in pieceDropped: x=" << x << ", y=" << y;
            return;
        }
        shapeAt(x, y) = currentPiece.shape();
    }

    playSoundWithDelay(dropSound, dropCycleCount, "/home/time/introCode/c++/TetrixGame/sounds/drop.wav", false);

    ++numPiecesDropped;
    if (numPiecesDropped % 25 == 0) {
        ++level;
        timer.start(1000 / level, this);
        emit levelChanged(level);
        qDebug() << "Level increased to:" << level;
    }

    score += dropHeight + 7;
    emit scoreChanged(score);
    qDebug() << "Score updated: score=" << score;
    removeFullLines();

    if (!isWaitingAfterLine) {
        newPiece();
    }
}

void TetrixBoard::removeFullLines() {
    int numFullLines = 0;
    flashingLines.clear();

    for (int i = BoardHeight - 1; i >= 0; --i) {
        bool lineIsFull = true;
        for (int j = 0; j < BoardWidth; ++j) {
            if (shapeAt(j, i) == TetrixShape::NoShape) {
                lineIsFull = false;
                break;
            }
        }
        if (lineIsFull) {
            ++numFullLines;
            flashingLines.append(i);
        }
    }

    if (numFullLines > 0) {
        playSoundWithDelay(lineClearSound, lineClearCycleCount, "/home/time/introCode/c++/TetrixGame/sounds/lineclear.wav", false);
        numLinesRemoved += numFullLines;
        score += 10 * numFullLines;
        emit linesRemovedChanged(numLinesRemoved);
        emit scoreChanged(score);
        isWaitingAfterLine = true;
        flashTimer->start(200);
        timer.start(1000, this);
        qDebug() << "Starting line flash animation for" << numFullLines << "lines";
    }
}

void TetrixBoard::newPiece() {
    currentPiece = nextPiece;
    nextPiece.setRandomShape();
    showNextPiece();

    curX = BoardWidth / 2 + 1;
    curY = BoardHeight - 1 + currentPiece.minY();
    qDebug() << "New piece created: curX=" << curX << ", curY=" << curY << ", shape=" << static_cast<int>(currentPiece.shape());

    if (!tryMove(currentPiece, curX, curY)) {
        currentPiece.setShape(TetrixShape::NoShape);
        timer.stop();
        isStarted = false;
        isDropping = false; // Reset dropping state
        backgroundSound->stop();
        backgroundCycleCount++;
        qDebug() << "Background music stopped, cycle count:" << backgroundCycleCount
                 << ", status:" << backgroundSound->status() << ", isPlaying:" << backgroundSound->isPlaying();
        playSoundWithDelay(gameOverSound, gameOverCycleCount, "/home/time/introCode/c++/TetrixGame/sounds/gameover.wav", true);
        emit gameOver(score);
        emit pauseStateChanged(false); // Disable pause button on game over
        qDebug() << "Game over, final score:" << score;
    }
}

void TetrixBoard::showNextPiece() {
    if (!nextPieceLabel) {
        return;
    }

    // Calculate bounding box of the next piece
    int dx = nextPiece.maxX() - nextPiece.minX() + 1;
    int dy = nextPiece.maxY() - nextPiece.minY() + 1;

    // Scale based on parent's available space
    QSize parentSize = parentWidget() ? parentWidget()->size() : QSize(600, 450);
    int squareSize = qMin(parentSize.width() / BoardWidth, parentSize.height() / BoardHeight);
    if (squareSize < 15) squareSize = 15; // Minimum square size

    // Create pixmap exactly matching the piece's bounding box
    QPixmap pixmap(dx * squareSize, dy * squareSize);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    for (int i = 0; i < 4; ++i) {
        int x = nextPiece.x(i) - nextPiece.minX();
        int y = nextPiece.y(i) - nextPiece.minY();
        drawSquare(painter, x * squareSize, y * squareSize, nextPiece.shape(), squareSize);
    }

    // Set pixmap and enforce exact size to avoid dead space
    nextPieceLabel->setPixmap(pixmap);
    nextPieceLabel->setFixedSize(dx * squareSize, dy * squareSize);
    qDebug() << "Next piece updated: piece extent dx=" << dx << ", dy=" << dy
             << ", squareSize=" << squareSize << ", pixmap size=" << pixmap.size()
             << ", nextPieceLabel size=" << nextPieceLabel->size();
}

bool TetrixBoard::tryMove(const TetrixPiece &newPiece, int newX, int newY) {
    // Check if move is valid
    for (int i = 0; i < 4; ++i) {
        int x = newX + newPiece.x(i);
        int y = newY - newPiece.y(i);
        // Stricter bounds checking
        if (x < 0 || x >= BoardWidth || y < 0 || y >= BoardHeight) {
            qDebug() << "Move rejected: out of bounds, x=" << x << ", y=" << y;
            return false;
        }
        if (shapeAt(x, y) != TetrixShape::NoShape) {
            qDebug() << "Move rejected: collision at x=" << x << ", y=" << y;
            return false;
        }
    }

    currentPiece = newPiece;
    curX = newX;
    curY = newY;
    qDebug() << "Move accepted: newX=" << newX << ", newY=" << newY;
    update();
    return true;
}

void TetrixBoard::drawSquare(QPainter &painter, int x, int y, TetrixShape shape, int squareSize) {
    static const QColor colors[] = {
        Qt::black, Qt::red, Qt::green, Qt::blue, Qt::cyan, Qt::magenta, Qt::yellow, Qt::gray
    };
    QColor color = colors[static_cast<int>(shape)];

    painter.fillRect(x + 1, y + 1, squareSize - 2, squareSize - 2, color);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(x, y, squareSize - 1, squareSize - 1);
}