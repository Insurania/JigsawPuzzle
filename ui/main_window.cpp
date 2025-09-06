#include "main_window.h"
#include "ui_main_window.h"
#include <QApplication>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_puzzleWidget(new PuzzleGame(this))
{
    ui->setupUi(this);
    setCentralWidget(m_puzzleWidget);
    setWindowTitle("拼图游戏");
    
    // 将窗口居中显示
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
    
    // 让PuzzleWidget填满整个窗口
    m_puzzleWidget->setGeometry(0, 0, width(), height());
}

MainWindow::~MainWindow()
{
    delete ui;
}

