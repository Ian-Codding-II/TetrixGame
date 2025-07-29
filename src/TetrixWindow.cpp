#include "TetrixWindow.h"
#include "TetrixBoard.h"
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QStackedWidget>
#include <QLineEdit>
#include <QFile>
#include <QTextStream>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QDebug>
#include <QResizeEvent>
#include <QVBoxLayout>

TetrixWindow::TetrixWindow(QWidget *parent)
    : QWidget(parent)
{
    // Initialize stacked widget to switch between game and game-over screens
    stackedWidget = new QStackedWidget(this);
    stackedWidget->setObjectName("stackedWidget");

    // Create game widget with island.jpg background
    gameWidget = new QWidget(this);
    gameWidget->setObjectName("gameWidget");
    gameWidget->setAutoFillBackground(true);
    
    // Initialize the game board
    board = new TetrixBoard(gameWidget);
    board->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    board->setMinimumSize(150, 330);

    // Create labels for score, level, lines, and next piece
    scoreLabel = createLabel(tr("SCORE: 0"));
    scoreLabel->setMinimumHeight(30);
    levelLabel = createLabel(tr("LEVEL: 1"));
    levelLabel->setMinimumHeight(30);
    linesLabel = createLabel(tr("LINES: 0"));
    linesLabel->setMinimumHeight(30);
    nextLabel = createLabel(tr("NEXT"));
    nextLabel->setMinimumHeight(30);
    nextLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Create buttons with minimum size and dynamic font scaling
    startButton = new QPushButton(tr("&Start"), gameWidget);
    startButton->setMinimumSize(100, 30);
    quitButton = new QPushButton(tr("&Quit"), gameWidget);
    quitButton->setMinimumSize(100, 30);
    pauseButton = new QPushButton(tr("&Pause"), gameWidget);
    pauseButton->setMinimumSize(100, 30);

    // Disable pause button initially
    pauseButton->setEnabled(false);

    // Connect buttons to slots
    connect(startButton, &QPushButton::clicked, board, &TetrixBoard::start);
    connect(quitButton, &QPushButton::clicked, qApp, &QApplication::quit);
    connect(pauseButton, &QPushButton::clicked, board, &TetrixBoard::pause);

    // Connect board signals to update labels
    connect(board, &TetrixBoard::scoreChanged, this, [this](int score) {
        scoreLabel->setText(tr("SCORE: %1").arg(score));
    });
    connect(board, &TetrixBoard::levelChanged, this, [this](int level) {
        levelLabel->setText(tr("LEVEL: %1").arg(level));
    });
    connect(board, &TetrixBoard::linesRemovedChanged, this, [this](int lines) {
        linesLabel->setText(tr("LINES: %1").arg(lines));
    });

    // Connect pause state signal to manage pause button
    connect(board, &TetrixBoard::pauseStateChanged, this, [this](bool isPaused) {
        pauseButton->setEnabled(isPaused || board->isGameStarted());
        pauseButton->setText(isPaused ? tr("&Resume") : tr("&Pause"));
        qDebug() << "Pause button updated: enabled=" << pauseButton->isEnabled() << ", text=" << pauseButton->text();
    });

    // Set next piece label with dynamic sizing and explicit centering
    board->nextPieceLabel = createLabel("");
    board->nextPieceLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    board->nextPieceLabel->setAlignment(Qt::AlignCenter);
    board->nextPieceLabel->setMinimumSize(60, 60);

    // Create right panel for buttons and labels
    QWidget *rightPanel = new QWidget(gameWidget);
    rightPanel->setObjectName("rightPanel");
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(nextLabel, 0, Qt::AlignCenter);
    rightLayout->addWidget(board->nextPieceLabel, 0, Qt::AlignCenter);
    rightLayout->addWidget(startButton, 0, Qt::AlignCenter);
    rightLayout->addWidget(quitButton, 0, Qt::AlignCenter);
    rightLayout->addWidget(pauseButton, 0, Qt::AlignCenter);
    rightLayout->addWidget(scoreLabel, 0, Qt::AlignCenter);
    rightLayout->addWidget(levelLabel, 0, Qt::AlignCenter);
    rightLayout->addWidget(linesLabel, 0, Qt::AlignCenter);
    rightLayout->setSpacing(2);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightPanel->setLayout(rightLayout);

    // Layout setup for game widget
    QGridLayout *gameLayout = new QGridLayout;
    gameLayout->addWidget(board, 0, 0, 8, 1);
    gameLayout->addWidget(rightPanel, 0, 1, 8, 1);
    gameLayout->setColumnStretch(0, 7);
    gameLayout->setColumnStretch(1, 3);
    gameLayout->setSpacing(0);
    gameLayout->setContentsMargins(0, 0, 0, 0);
    gameWidget->setLayout(gameLayout);

    // Add game widget to stacked widget
    stackedWidget->addWidget(gameWidget);

    // Create game over widget
    gameOverWidget = new QWidget(this);
    gameOverWidget->setObjectName("gameOverWidget");
    QVBoxLayout *gameOverLayout = new QVBoxLayout;
    gameOverMessageLabel = new QLabel("", gameOverWidget);
    gameOverMessageLabel->setAlignment(Qt::AlignCenter);
    gameOverMessageLabel->setMinimumHeight(50);
    nameEdit = new QLineEdit(gameOverWidget);
    nameEdit->setPlaceholderText(tr("Enter your name"));
    nameEdit->setMinimumHeight(30);
    restartButton = new QPushButton(tr("&Restart"), gameOverWidget);
    restartButton->setMinimumSize(100, 30);
    gameOverLayout->addWidget(gameOverMessageLabel);
    gameOverLayout->addWidget(nameEdit);
    gameOverLayout->addWidget(restartButton);
    gameOverLayout->setSpacing(2);
    gameOverLayout->setContentsMargins(0, 0, 0, 0);
    gameOverWidget->setLayout(gameOverLayout);

    // Add game over widget to stacked widget
    stackedWidget->addWidget(gameOverWidget);

    // Debug to confirm widgets in stack
    qDebug() << "Stacked widget contains gameWidget:" << stackedWidget->indexOf(gameWidget);
    qDebug() << "Stacked widget contains gameOverWidget:" << stackedWidget->indexOf(gameOverWidget);

    // Connect game over signal
    connect(board, &TetrixBoard::gameOver, this, [this](int score) {
        currentScore = score;
        int highScore = 0;
        QFile file("highscores.txt");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QStringList scores;
            while (!in.atEnd()) {
                scores << in.readLine();
            }
            file.close();
            if (!scores.isEmpty()) {
                highScore = scores.first().split(",").last().toInt();
            }
        }
        gameOverMessageLabel->setText(tr("Game Over!\nFinal Score: %1\nHigh Score: %2")
                                     .arg(score).arg(highScore));
        disconnect(restartButton, &QPushButton::clicked, this, &TetrixWindow::saveHighScore);
        if (score > highScore) {
            connect(restartButton, &QPushButton::clicked, this, &TetrixWindow::saveHighScore);
        }
        qDebug() << "Switching to gameOverWidget, index:" << stackedWidget->indexOf(gameOverWidget);
        stackedWidget->setCurrentWidget(gameOverWidget);
        QPropertyAnimation *animation = new QPropertyAnimation(gameOverWidget, "windowOpacity");
        animation->setDuration(1000);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // Connect restart button to stop game-over sound and switch back to game
    connect(restartButton, &QPushButton::clicked, this, [this]() {
        board->stopGameOverSound();
        qDebug() << "Switching to gameWidget, index:" << stackedWidget->indexOf(gameWidget);
        stackedWidget->setCurrentWidget(gameWidget);
        board->start();
    });

    // Set main layout
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(stackedWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    // Verify image paths
    QString gameBackgroundPath = "/home/time/introCode/c++/TetrixGame/images/island.jpg";
    QString gameOverBackgroundPath = "/home/time/introCode/c++/TetrixGame/images/gray.jpg";
    if (!QFile::exists(gameBackgroundPath)) {
        qDebug() << "WARNING: Game background image not found:" << gameBackgroundPath;
    }
    if (!QFile::exists(gameOverBackgroundPath)) {
        qDebug() << "WARNING: Game-over background image not found:" << gameOverBackgroundPath;
    }

    // Apply stylesheet to ensure full background coverage and transparency
    setStyleSheet(R"(
        QWidget#gameWidget {
            background-image: url(/home/time/introCode/c++/TetrixGame/images/island.jpg);
            background-position: center;
            background-repeat: no-repeat;
            background-size: cover;
        }
        TetrixBoard {
            background: transparent;
        }
        QWidget#rightPanel {
            background: transparent;
        }
        QWidget#gameOverWidget {
            background-image: url(/home/time/introCode/c++/TetrixGame/images/gray.jpg);
            background-position: center;
            background-repeat: no-repeat;
            background-size: cover;
        }
        QWidget#stackedWidget {
            background: transparent; /* Ensure stacked widget is transparent */
        }
        QWidget {
            background: transparent; /* Main window transparent to show gameWidget */
        }
        QPushButton {
            background: transparent;
            color: #cdd6f4;
            border: none;
            padding: 8px;
            border-radius: 5px;
            font-family: Arial;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: rgb(125, 25, 50);
        }
        QPushButton:pressed {
            background-color: rgb(85, 0, 20);
        }
        QLabel {
            color: #cdd6f4;
            font-family: Arial;
            background: transparent;
            padding: 5px;
            border-radius: 3px;
            text-shadow: 1px 1px 2px #000000;
        }
        QLineEdit {
            color: #cdd6f4;
            background: transparent;
            border: 1px solid #89b4fa;
            padding: 5px;
            border-radius: 3px;
            text-shadow: 1px 1px 2px #000000;
        }
    )");

    // Debug stylesheet and widget properties
    qDebug() << "Window initialized: size=" << size()
             << ", game background path=" << gameBackgroundPath
             << ", game-over background path=" << gameOverBackgroundPath
             << ", main window stylesheet=" << styleSheet()
             << ", gameWidget stylesheet=" << gameWidget->styleSheet()
             << ", rightPanel stylesheet=" << gameWidget->findChild<QWidget*>("rightPanel")->styleSheet()
             << ", gameOverWidget stylesheet=" << gameOverWidget->styleSheet()
             << ", TetrixBoard stylesheet=" << board->styleSheet()
             << ", startButton stylesheet=" << startButton->styleSheet()
             << ", nextLabel stylesheet=" << nextLabel->styleSheet()
             << ", gameWidget autoFillBackground=" << gameWidget->autoFillBackground()
             << ", TetrixBoard autoFillBackground=" << board->autoFillBackground();
}

QLabel *TetrixWindow::createLabel(const QString &text) {
    QLabel *label = new QLabel(text, this);
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    return label;
}

void TetrixWindow::saveHighScore() {
    QFile file("highscores.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << nameEdit->text() << "," << currentScore << "\n";
        file.close();
        qDebug() << "High score saved:" << nameEdit->text() << "," << currentScore;
    } else {
        qDebug() << "Failed to open highscores.txt for writing";
    }
}

void TetrixWindow::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    int fontSize = qMax(10, height() / 30);
    QFont buttonFont("Arial", fontSize, QFont::Bold);
    startButton->setFont(buttonFont);
    quitButton->setFont(buttonFont);
    pauseButton->setFont(buttonFont);
    QFont labelFont("Arial", qMax(12, height() / 35));
    scoreLabel->setFont(labelFont);
    levelLabel->setFont(labelFont);
    linesLabel->setFont(labelFont);
    nextLabel->setFont(labelFont);
    board->nextPieceLabel->setFont(labelFont);
    qDebug() << "Window resized: new size=" << event->size()
             << ", button font size=" << fontSize
             << ", label font size=" << labelFont.pointSize()
             << ", nextLabel geometry=" << nextLabel->geometry()
             << ", nextPieceLabel geometry=" << board->nextPieceLabel->geometry()
             << ", gameWidget geometry=" << gameWidget->geometry()
             << ", rightPanel geometry=" << gameWidget->findChild<QWidget*>("rightPanel")->geometry()
             << ", TetrixBoard geometry=" << board->geometry();
}