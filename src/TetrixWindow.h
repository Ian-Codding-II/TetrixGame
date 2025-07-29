#ifndef TETRIXWINDOW_H
#define TETRIXWINDOW_H

#include <QWidget>
#include <QStackedWidget>

class TetrixBoard;
class QLabel;
class QPushButton;
class QLineEdit;
class QResizeEvent;

class TetrixWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TetrixWindow(QWidget *parent = nullptr);

private:
    QLabel *createLabel(const QString &text);

private slots:
    void saveHighScore();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    TetrixBoard *board;
    QLabel *scoreLabel;
    QLabel *levelLabel;
    QLabel *linesLabel;
    QLabel *nextLabel;
    QLabel *gameOverMessageLabel;
    QPushButton *startButton;
    QPushButton *quitButton;
    QPushButton *pauseButton;
    QPushButton *restartButton;
    QLineEdit *nameEdit;
    QStackedWidget *stackedWidget;
    QWidget *gameWidget;
    QWidget *gameOverWidget;
    QLabel *gameBackgroundLabel; // Background for game screen
    QLabel *gameOverBackgroundLabel; // Background for game-over screen
    int currentScore;
};

#endif // TETRIXWINDOW_H