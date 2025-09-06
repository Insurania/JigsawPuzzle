#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "core/puzzle_game.h"
#include <QMainWindow>

/*
 * @Julia: Nur interessant für Qt, beachte diese Klasse einfach nicht.
 */

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    PuzzleGame* m_puzzleWidget;
};
#endif // MAIN_WINDOW_H
