#ifndef GAME_MENU_H
#define GAME_MENU_H


#include "puzzle_button.h"
#include "components/puzzle_path.h"
#include <QWidget>

/*
 * This class simply provides a widget with signals, when you enter and leave it. It is not essential for the jigsaw puzzle.
 *
 * @Julia: Nur interessant für Qt, beachte diese Klasse einfach nicht.
 */

class GameMenu : public QWidget
{
    Q_OBJECT
private:
    PuzzleButton* m_newPuzzleButton;
    PuzzleButton* m_quitButton;
    PuzzleButton* m_saveButton;
public:
    explicit GameMenu(QSize size, QWidget *parent = nullptr);

    PuzzleButton *newPuzzleButton() const;
    PuzzleButton *quitButton() const;
    PuzzleButton *saveButton() const;

signals:
    void enterMenu();
    void leaveMenu();

protected:
    virtual void enterEvent(QEnterEvent* event) override;
    virtual void leaveEvent(QEvent* event) override;

};

#endif // GAME_MENU_H
