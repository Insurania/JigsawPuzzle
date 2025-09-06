#include "game_menu.h"

PuzzleButton *GameMenu::newPuzzleButton() const
{
    return m_newPuzzleButton;
}

PuzzleButton *GameMenu::quitButton() const
{
    return m_quitButton;
}

void GameMenu::enterEvent(QEnterEvent *event)
{
    raise();
    emit enterMenu();
}

void GameMenu::leaveEvent(QEvent *event)
{
    emit leaveMenu();
}

GameMenu::GameMenu(QSize size, QWidget *parent)
    : QWidget{parent}
{
    QPixmap backgroundMenu(":/backgrounds/back3");
    QPixmap backgroundButtons(":/backgrounds/back2");

    QLabel* backgroundLabel = new QLabel(this);
    backgroundLabel->setGeometry(QRect(QPoint(0, 0), size));
    backgroundLabel->setPixmap(backgroundMenu);
    backgroundLabel->setScaledContents(true);  // 确保背景图片适应底边栏尺寸

    QSize sizeInnerBounds(70, 60);
    QSize sizeOuterBounds(130, 100);

    QPainterPath pathNewButton = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBounds), QRect(), Jigsaw::TypeOfPiece::STANDARD, 4, true);
    QPainterPath pathQuitButton = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBounds), QRect(), Jigsaw::TypeOfPiece::STANDARD, 4, true);

    // 将按钮放置在底边栏中，水平排列
    m_newPuzzleButton = new PuzzleButton(sizeOuterBounds, QBrush(backgroundButtons), pathNewButton, this, "新建");
    m_newPuzzleButton->setFont(QFont("Georgia", 16, QFont::Bold));
    m_newPuzzleButton->animate();
    // 放置在底边栏左侧，垂直居中
    m_newPuzzleButton->move(100, (size.height() - sizeOuterBounds.height()) / 2);

    m_quitButton = new PuzzleButton(sizeOuterBounds, QBrush(backgroundButtons), pathQuitButton, this, "退出");
    m_quitButton->setFont(QFont("Georgia", 16, QFont::Bold));
    m_quitButton->animate();
    // 放置在底边栏右侧，垂直居中
    m_quitButton->move(size.width() - sizeOuterBounds.width() - 100, (size.height() - sizeOuterBounds.height()) / 2);
}
