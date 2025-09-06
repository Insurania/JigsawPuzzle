#include "puzzle_game.h"
#include "ui/game_menu.h"
#include "qapplication.h"
#include "qdebug.h"
#include <random>
#include <QPainter>
#include <QGroupBox>
#include <QRadioButton>
#include <QFile>
#include <QFileDialog>

void PuzzleGame::createNewMergedPiece(PuzzlePiece *firstPiece)
{
    m_mergedPieces.push_back(QVector<PuzzlePiece*>(1, firstPiece));
    QObject::disconnect(firstPiece, &PuzzlePiece::dragStopped, this, &PuzzleGame::fixPieceIfPossible);
    QObject::connect(firstPiece, &PuzzlePiece::dragStopped, this, &PuzzleGame::fixMergedPieceIfPossible);
    QObject::disconnect(firstPiece, &PuzzlePiece::rotateStopped, this, &PuzzleGame::fixPieceIfPossible);
    QObject::connect(firstPiece, &PuzzlePiece::rotateStopped, this, &PuzzleGame::fixMergedPieceIfPossible);
}

void PuzzleGame::addPuzzlePieceToMergedPiece(PuzzlePiece *piece, int mergedPieceID)
{
    m_mergedPieces[mergedPieceID].push_back(piece);
    QObject::disconnect(piece, &PuzzlePiece::dragStopped, this, &PuzzleGame::fixPieceIfPossible);
    QObject::connect(piece, &PuzzlePiece::dragStopped, this, &PuzzleGame::fixMergedPieceIfPossible);
    QObject::disconnect(piece, &PuzzlePiece::rotateStopped, this, &PuzzleGame::fixPieceIfPossible);
    QObject::connect(piece, &PuzzlePiece::rotateStopped, this, &PuzzleGame::fixMergedPieceIfPossible);
}

void PuzzleGame::combineTwoMergedPieces(int firstMergedPieceID, int secondMergedPieceID)
{
    m_mergedPieces[firstMergedPieceID] += m_mergedPieces[secondMergedPieceID];
    m_mergedPieces.removeAt(secondMergedPieceID);
}

bool PuzzleGame::isPartOfMergedPiece(PuzzlePiece *piece, int &mergedPieceID)
{
    mergedPieceID = -1;
    for (int i = 0; i < m_mergedPieces.size(); ++i) {
        if (m_mergedPieces[i].contains(piece)) {
            mergedPieceID = i;
            return true;
        }
    }
    return false;
}

bool PuzzleGame::areMerged(PuzzlePiece *first, PuzzlePiece *second)
{
    int firstMergedPieceID;
    int secondMergedPieceID;
    if (!isPartOfMergedPiece(first, firstMergedPieceID)) return false;
    if (!isPartOfMergedPiece(second, secondMergedPieceID)) return false;
    if (firstMergedPieceID == -1 && secondMergedPieceID == -1) return false;
    if (firstMergedPieceID == secondMergedPieceID) return true;
    return false;
}

void PuzzleGame::fixPieceIfPossible(int id)
{
    // 只有在游戏真正开始后才记录移动步数
    if (m_gameStarted) {
        updateMovesDisplay();
    }
    
    PuzzlePiece* piece = m_puzzlePieces[id];
    piece->lower();
    m_background->lower();

    int pieceNorthID = id - m_cols;
    int pieceEastID = (id + 1) % m_cols == 0 ? -1 : id + 1;
    int pieceSouthID = (id + m_cols >= m_numberOfPieces) ? -1 : id + m_cols;
    int pieceWestID = id % m_cols == 0 ? -1 : id - 1;

    QVector<PuzzlePiece*> neighbors;
    neighbors.push_back(pieceNorthID >= 0 ? m_puzzlePieces[pieceNorthID] : nullptr);
    neighbors.push_back(pieceEastID >= 0 ? m_puzzlePieces[pieceEastID] : nullptr);
    neighbors.push_back(pieceSouthID >= 0 ? m_puzzlePieces[pieceSouthID] : nullptr);
    neighbors.push_back(pieceWestID >= 0 ? m_puzzlePieces[pieceWestID] : nullptr);

    int mergedPieceID;
    int neighborMergedPieceID;

    for (const auto &neighbor: neighbors) {
        if (isInCorrectPosition(piece, neighbor) && !areMerged(piece, neighbor)) {

            if (isPartOfMergedPiece(piece, mergedPieceID)) {
                for (const auto &pieces : m_mergedPieces[mergedPieceID]) {
                    repositionPiece(pieces, neighbor);
                }
                if (isPartOfMergedPiece(neighbor, neighborMergedPieceID)) {
                    combineTwoMergedPieces(mergedPieceID, neighborMergedPieceID);
                }
                else {
                    addPuzzlePieceToMergedPiece(neighbor, mergedPieceID);
                }
            }
            else
            {
                repositionPiece(piece, neighbor);
                if (isPartOfMergedPiece(neighbor, neighborMergedPieceID)) {
                    addPuzzlePieceToMergedPiece(piece, neighborMergedPieceID);
                }
                else {
                    createNewMergedPiece(piece);
                    isPartOfMergedPiece(piece, mergedPieceID);
                    addPuzzlePieceToMergedPiece(neighbor, mergedPieceID);
                }
            }
        }
    }
    if (!m_mergedPieces.isEmpty() && m_mergedPieces[0].size() == m_numberOfPieces) wonGame();
}

void PuzzleGame::fixMergedPieceIfPossible(int id)
{
    int mergedPieceID;
    isPartOfMergedPiece(m_puzzlePieces[id], mergedPieceID);
    for (const auto &piece: m_mergedPieces[mergedPieceID]) {
        fixPieceIfPossible(piece->id());
    }
}

bool PuzzleGame::isInCorrectPosition(PuzzlePiece *piece, PuzzlePiece *neighbor, int tolerance)
{
    if (!neighbor) return false;

    QPoint pieceGridPoint = m_grid->overlayGridPoint(piece->id());
    QPoint neighborGridPoint = m_grid->overlayGridPoint(neighbor->id());

    QLineF gridLine(pieceGridPoint, neighborGridPoint);
    QLineF positionLine;
    positionLine.setP1(piece->center());
    positionLine.setAngle(gridLine.angle() + piece->angle());
    positionLine.setLength(gridLine.length());

    QLineF positionDifference(positionLine.p2(), neighbor->center());
    double distance = positionDifference.length();

    return distance <= tolerance && piece->angle() == neighbor->angle();
}

void PuzzleGame::repositionPiece(PuzzlePiece *piece, PuzzlePiece *neighbor)
{
    QPoint pieceGridPoint = m_grid->overlayGridPoint(piece->id());
    QPoint neighborGridPoint = m_grid->overlayGridPoint(neighbor->id());
    QLineF line(neighborGridPoint, pieceGridPoint);
    piece->rotateAroundPoint(neighbor->angle(), neighbor->center(), line.length(), line.angle());
}

void PuzzleGame::calculateRowsAndCols(int numberOfPieces, const QPixmap &image)
{
    int maxImageWidth = m_parameters.screenWidth * 2 / 3;
    int maxImageHeight = m_parameters.screenHeight * 4 / 5;
    QSize maxImageSize(maxImageWidth, maxImageHeight);
    QSize imageSize(image.size());
    QSize actualImageSize = imageSize.scaled(maxImageSize, Qt::KeepAspectRatio);
    double imageRatio = 1.0 * actualImageSize.width() / actualImageSize.height();

    // 计算最接近目标碎片数量的行数和列数
    int rows = 1;
    int cols = 1;
    int bestRows = 1;
    int bestCols = 1;
    int bestDifference = abs(numberOfPieces - 1);
    
    // 尝试不同的行数，找到最接近目标数量的组合
    for (int r = 1; r <= numberOfPieces; ++r) {
        int c = qRound(r * imageRatio);
        if (c < 1) c = 1;
        
        int pieces = r * c;
        int difference = abs(pieces - numberOfPieces);
        
        if (difference < bestDifference) {
            bestDifference = difference;
            bestRows = r;
            bestCols = c;
        }
        
        // 如果找到完全匹配的，直接使用
        if (difference == 0) {
            bestRows = r;
            bestCols = c;
            break;
        }
    }
    
    // 确保碎片数量不超过目标数量太多
    if (bestRows * bestCols > numberOfPieces * 1.5) {
        // 如果碎片数量太多，尝试减少行数
        bestRows = qRound(qSqrt(numberOfPieces / imageRatio));
        if (bestRows < 1) bestRows = 1;
        bestCols = qRound(bestRows * imageRatio);
        if (bestCols < 1) bestCols = 1;
    }
    
    m_rows = bestRows;
    m_cols = bestCols;
    m_numberOfPieces = bestRows * bestCols;

    m_pieceWidth = actualImageSize.width() / m_cols;
    m_pieceHeight = actualImageSize.height() / m_rows;
}

void PuzzleGame::setupImage()
{
    QSize overlaySize(m_grid->puzzleTotalSize());
    QSize scaledImageSize(m_cols * m_pieceWidth + 1, m_rows * m_pieceHeight + 1);

    QPixmap overlayPixmap(overlaySize);
    overlayPixmap.fill(Qt::transparent);
    QPixmap scaledImage = m_image.scaled(scaledImageSize);

    QPainter painter(&overlayPixmap);
    painter.drawPixmap(m_grid->symmetricGridPoint(0), scaledImage, QRect(QPoint(0, 0), scaledImageSize));

    m_image = overlayPixmap;
}

void PuzzleGame::generatePuzzlePieces()
{
    for (unsigned int i = 0; i < m_numberOfPieces; ++i) {
        m_puzzlePieces.push_back(new PuzzlePiece(i, m_grid->pieceTotalSize(), QBrush(createImageFragment(i)), m_grid->puzzlePath(i), this));
        m_puzzlePieces.last()->setRotationEnabled(m_rotationAllowed);
         if (m_rotationAllowed) m_puzzlePieces.last()->setAngle(Jigsaw::randomNumber(0, 35) * 10);
        QObject::connect(m_puzzlePieces.last(), &PuzzlePiece::dragStarted, this, &PuzzleGame::raisePieces);
        QObject::connect(m_puzzlePieces.last(), &PuzzlePiece::dragged, this, &PuzzleGame::dragMergedPieces);
        QObject::connect(m_puzzlePieces.last(), &PuzzlePiece::dragStopped, this, &PuzzleGame::fixPieceIfPossible);
        QObject::connect(m_puzzlePieces.last(), &PuzzlePiece::rotateStarted, this, &PuzzleGame::raisePieces);
        QObject::connect(m_puzzlePieces.last(), &PuzzlePiece::rotated, this, &PuzzleGame::rotateMergedPieces);
        QObject::connect(m_puzzlePieces.last(), &PuzzlePiece::rotateStopped, this, &PuzzleGame::fixPieceIfPossible);
    }
}

void PuzzleGame::placePuzzlePieces()
{
    for (auto piece: m_puzzlePieces) {
        double ax, ay;
        do {
            ax = Jigsaw::randomNumber(0, m_parameters.screenWidth - m_grid->pieceTotalWidth());
            ay = Jigsaw::randomNumber(0, m_parameters.screenHeight - m_grid->pieceTotalHeight());
        }
        while (m_parameters.rectFreeArea.contains(QPoint(ax, ay)));
        piece->move(ax, ay);
        piece->raise();
        piece->show();
    }
}

QPixmap PuzzleGame::createImageFragment(int pieceID)
{
    QSize puzzlePieceSize(m_grid->pieceTotalSize());
    QPixmap imageFragment(puzzlePieceSize);
    QPainter fragmentPainter(&imageFragment);

    fragmentPainter.drawPixmap(QPoint(0, 0), m_image, QRect(m_grid->overlayGridPoint(pieceID),puzzlePieceSize));
    return imageFragment;
}

void PuzzleGame::setMenuWidget()
{
    // 修改为底边栏布局，宽度适中，高度为120像素，居中显示
    QSize menuSize(800, 120);
    m_menuWidget = new GameMenu(menuSize, this);
    // 固定在窗口底部居中显示，适应窗口大小
    int xPos = (this->width() - menuSize.width()) / 2;  // 水平居中
    int yPos = this->height() - menuSize.height();       // 底部
    m_menuWidget->setGeometry(QRect(QPoint(xPos, yPos), menuSize));
    m_menuWidget->raise();

    // 不再需要定时器和动画效果
    // m_moveMenuInTimer = new QTimer(this);
    // m_moveMenuOutTimer = new QTimer(this);

    // QObject::connect(m_moveMenuInTimer, &QTimer::timeout, this, &PuzzleGame::moveMenuInTimerTimeout);
    // QObject::connect(m_moveMenuOutTimer, &QTimer::timeout, this, &PuzzleGame::moveMenuOutTimerTimeout);

    // 不再需要鼠标进入离开事件
    // QObject::connect(m_menuWidget, &GameMenu::enterMenu, this, &PuzzleGame::enterMenu);
    // QObject::connect(m_menuWidget, &GameMenu::leaveMenu, this, &PuzzleGame::leaveMenu);

    QObject::connect(m_menuWidget->newPuzzleButton(), &PuzzleButton::clicked, this, &PuzzleGame::menuNewButtonClicked);
    QObject::connect(m_menuWidget->quitButton(), &PuzzleButton::clicked, this, &PuzzleGame::menuQuitButtonClicked);
}

void PuzzleGame::setQuitWidget()
{
    QSize sizeOuterBoundsBackground(1000, 600);
    QSize sizeInnerBoundsBackground(sizeOuterBoundsBackground - QSize(400, 400));
    QRect innerBoundsBackground(QPoint(200, 200), sizeInnerBoundsBackground);
    QSize sizeOuterBoundsButtons(130, 100);
    QSize sizeInnerBoundsButtons(70, 60);
    
    // 直接获取屏幕尺寸，计算界面在屏幕中的居中位置
    QScreen *screen = QApplication::primaryScreen();
    int xPos, yPos;
    
    if (screen) {
        QRect screenGeometry = screen->geometry();
        xPos = (screenGeometry.width() - sizeOuterBoundsBackground.width()) / 2;  // 在屏幕中水平居中
        yPos = (screenGeometry.height() - sizeOuterBoundsBackground.height()) / 2;  // 在屏幕中垂直居中
    } else {
        // 如果无法获取屏幕信息，使用默认居中
        xPos = (1920 - sizeOuterBoundsBackground.width()) / 2;
        yPos = (1080 - sizeOuterBoundsBackground.height()) / 2;
    }
    
    QPoint positionWidget(xPos, yPos);

    m_quitWidget = new QWidget(this);
    m_quitWidget->setGeometry(QRect(positionWidget, sizeOuterBoundsBackground));

    QPainterPath backgroundLabelPath = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBoundsBackground), innerBoundsBackground, Jigsaw::TypeOfPiece::STANDARD);
    PuzzleLabel* backgroundLabel = new PuzzleLabel(sizeOuterBoundsBackground, QBrush(QPixmap(":/backgrounds/back3")), backgroundLabelPath,
                                                   m_quitWidget, "您确定要退出吗?", innerBoundsBackground);
    backgroundLabel->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    backgroundLabel->setFont(m_parameters.mainFont);

    QPainterPath backgroundYesButtonPath = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBoundsButtons), QRect(), Jigsaw::TypeOfPiece::STANDARD, 4, true);
    QPainterPath backgroundNoButtonPath = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBoundsButtons), QRect(), Jigsaw::TypeOfPiece::STANDARD, 4, true);

    PuzzleButton* yesButton = new PuzzleButton(sizeOuterBoundsButtons, QBrush(QPixmap(":/backgrounds/back2")), backgroundYesButtonPath, m_quitWidget, "是");
    yesButton->move(innerBoundsBackground.left() + 10, innerBoundsBackground.bottom() - 10 - yesButton->height());
    yesButton->animate();
    yesButton->setFont(m_parameters.mainFont);
    QObject::connect(yesButton, &PuzzleButton::clicked, this, &PuzzleGame::quitWidgetYesClicked);

    PuzzleButton* noButton = new PuzzleButton(sizeOuterBoundsButtons, QBrush(QPixmap(":/backgrounds/back2")), backgroundNoButtonPath, m_quitWidget, "否");
    noButton->move(innerBoundsBackground.right() - 10 - noButton->width(), innerBoundsBackground.bottom() - 10 - noButton->height());
    noButton->animate();
    noButton->setFont(m_parameters.mainFont);
    QObject::connect(noButton, &PuzzleButton::clicked, m_quitWidget, &QWidget::hide);

    m_quitWidget->hide();
}

void PuzzleGame::setNewWidget()
{

    m_newWidget = new QWidget(this);
    
    // 直接获取屏幕尺寸，计算界面在屏幕中的居中位置
    QScreen *screen = QApplication::primaryScreen();
    QSize widgetSize = m_parameters.sizeWidget;
    int xPos, yPos;
    
    if (screen) {
        QRect screenGeometry = screen->geometry();
        xPos = (screenGeometry.width() - widgetSize.width()) / 2;  // 在屏幕中水平居中
        yPos = (screenGeometry.height() - widgetSize.height()) / 2;  // 在屏幕中垂直居中
    } else {
        // 如果无法获取屏幕信息，使用默认居中
        xPos = (1920 - widgetSize.width()) / 2;
        yPos = (1080 - widgetSize.height()) / 2;
    }
    
    m_newWidget->setGeometry(QRect(QPoint(xPos, yPos), widgetSize));

    QPainterPath backgroundLabelPath = PuzzlePath::singleJigsawPiecePath(m_parameters.rectWidget, m_parameters.rectWidgetArea, Jigsaw::TypeOfPiece::STANDARD);
    PuzzleLabel* backgroundLabel = new PuzzleLabel(m_parameters.sizeWidget, QBrush(QPixmap(":/backgrounds/back3")), backgroundLabelPath, m_newWidget,
                                                   "创建新拼图!", m_parameters.rectWidgetArea);
    backgroundLabel->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    backgroundLabel->setFont(m_parameters.mainFont);

    QWidget* widgetArea = new QWidget(m_newWidget);
    widgetArea->setGeometry(m_parameters.rectWidgetArea);

    QWidget* widgetCaption = new QWidget(widgetArea);
    widgetCaption->setGeometry(m_parameters.rectWidgetCaption);

    setNewWidgetExWidget(widgetArea, m_parameters);
    setNewWidgetPuzzlePieceWidget(widgetArea, m_parameters);
    setNewWidgetNumberOfPiecesWidget(widgetArea, m_parameters);
    setNewWidgetButtonsWidget(widgetArea, m_parameters);

    m_newWidget->hide();
}

void PuzzleGame::setNewWidgetExWidget(QWidget *parent, const Jigsaw::Parameters &par)
{
    QWidget* widgetEx = new QWidget(parent);
    widgetEx->setGeometry(par.rectWidgetEx);
    QGroupBox* radioGroupBoxEx = new QGroupBox(widgetEx);
    radioGroupBoxEx->setGeometry(QRect(QPoint(0, 0), par.sizeWidgetEx));
    QVector<PuzzleButton*> labelEx;
    int numberOfEx = 0;
    while (QFile::exists(":/examples/ex" + QString::number(numberOfEx))) {
        ++numberOfEx;
    }
    ++numberOfEx;
    int maxLabelExWidth = (par.widthWidgetEx - (numberOfEx + 1) * par.minBorderWidth) / numberOfEx;
    int maxLabelExHeight = par.heightWidgetEx - par.minBorderWidth * 2 - par.minRadioButtonHeight;
    int labelExWidth, labelExHeight;
    if (maxLabelExHeight * 16 / 9 > maxLabelExWidth) {
        labelExWidth = maxLabelExWidth;
        labelExHeight = labelExWidth * 9 / 16;
    }
    else {
        labelExHeight = maxLabelExHeight;
        labelExWidth = labelExHeight * 16 / 9;
    }
    bool noPreviewEx = labelExWidth < par.minLabelExWidth || labelExHeight < par.minLabelExHeight;
    QSize sizeLabelEx(labelExWidth, labelExHeight);

    int borderExWidth = numberOfEx != 1 ? (par.widthWidgetEx - par.minBorderWidth * 2 - (numberOfEx) * labelExWidth) / (numberOfEx - 1) : 0;
    int borderExHeight = (par.heightWidgetEx - par.minBorderWidth * 2 - labelExHeight - par.minRadioButtonHeight) / 2;

    bool ownImage = false;

    for (int i = 0; i < numberOfEx; ++i) {
        QString filename = ":/examples/ex" + QString::number(i);
        if (!QFile::exists(filename)) ownImage = true;
        labelEx.push_back(new PuzzleButton(widgetEx));
        if (noPreviewEx) {
            labelEx[i]->setText(ownImage ? "... 或打开您自己的图片!" : "示例 " + QString::number(i + 1));
        }
        else {
            if (!ownImage) {
                QPixmap preview(filename);
                preview = preview.scaled(sizeLabelEx);
                labelEx[i]->setPixmap(preview);
                labelEx[i]->animate();
            }
            else {
                QPixmap preview(sizeLabelEx);
                QFont font("Georgia", 32, QFont::Bold);
                preview.fill(Qt::darkGray);
                m_ownImageLabel = new PuzzleButton(widgetEx);
                m_ownImageLabel->setPixmap(preview);
                m_ownImageLabel->setText("?");
                m_ownImageLabel->setFont(font);
                m_ownImageLabel->animate();
                labelEx[i] = m_ownImageLabel;
                QObject::connect(m_ownImageLabel, &PuzzleButton::clicked, this, &PuzzleGame::newWidgetOwnImageClicked);
            }
        }
        QPoint labelPositionEx(par.minBorderWidth + i * labelExWidth + i * borderExWidth, par.minBorderWidth + borderExHeight);
        labelEx[i]->move(labelPositionEx);

        m_radioButtonEx.push_back(new QRadioButton(radioGroupBoxEx));
        QPoint radioPositionEx = labelPositionEx + QPoint(labelExWidth / 2 - 10, labelExHeight + borderExHeight);
        m_radioButtonEx[i]->move(radioPositionEx);

        QObject::connect(labelEx[i], &PuzzleButton::clicked, m_radioButtonEx[i], &QRadioButton::toggle);
    }

    m_radioButtonEx.last()->setCheckable(false);
    m_radioButtonEx[0]->setChecked(true);
}

void PuzzleGame::setNewWidgetPuzzlePieceWidget(QWidget *parent, const Jigsaw::Parameters &par)
{
    QWidget* widgetPuzzlePiece = new QWidget(parent);
    widgetPuzzlePiece->setGeometry(par.rectWidgetPuzzlePiece);
    QGroupBox* radioGroupBoxPuzzlePiece = new QGroupBox(widgetPuzzlePiece);
    radioGroupBoxPuzzlePiece->setGeometry(QRect(QPoint(0, 0), par.sizeWidgetPuzzlePiece));
    QVector<PuzzleButton*> labelPuzzlePiece;
    int numberOfTypes = static_cast<int>(Jigsaw::TypeOfPiece::count);
    int maxLabelPuzzlePieceWidth = numberOfTypes != 0 ? (par.widthWidgetPuzzlePiece - (numberOfTypes + 1) * par.minBorderWidth) / numberOfTypes : 0;
    int maxLabelPuzzlePieceHeight = par.heightWidgetPuzzlePiece - par.minBorderWidth * 2 - par.minRadioButtonHeight;
    int labelPuzzlePieceWidth, labelPuzzlePieceHeight;
    if (maxLabelPuzzlePieceHeight > maxLabelPuzzlePieceWidth) {
        labelPuzzlePieceHeight = maxLabelPuzzlePieceWidth;
        labelPuzzlePieceWidth = maxLabelPuzzlePieceWidth;
    }
    else {
        labelPuzzlePieceHeight = maxLabelPuzzlePieceHeight;
        labelPuzzlePieceWidth = maxLabelPuzzlePieceHeight;
    }
    bool noPreviewPuzzlePiece = labelPuzzlePieceWidth < par.minLabelPuzzlePieceWidth || labelPuzzlePieceHeight < par.minLabelPuzzlePieceHeight;
    QSize sizeLabelPuzzlePiece(labelPuzzlePieceWidth, labelPuzzlePieceHeight);

    int borderPuzzlePieceWidth = numberOfTypes != 1 ? (par.widthWidgetPuzzlePiece - par.minBorderWidth * 2 - (numberOfTypes) * labelPuzzlePieceWidth) / (numberOfTypes - 1) : 0;
    int borderPuzzlePieceHeight = (par.heightWidgetPuzzlePiece - par.minBorderWidth * 2 - labelPuzzlePieceHeight - par.minRadioButtonHeight) / 2;

    for (int i = 0; i < numberOfTypes; ++i) {
        labelPuzzlePiece.push_back(new PuzzleButton(widgetPuzzlePiece));
        if (noPreviewPuzzlePiece) {
            labelPuzzlePiece[i]->setText("类型 " + QString::number(i + 1));
        }
        else {
            QPainterPath path = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeLabelPuzzlePiece), QRect(), PuzzlePath::intToTypeOfPiece(i == numberOfTypes - 1 ? 0 : i), 4, true);
            labelPuzzlePiece[i]->setJigsawPath(path, sizeLabelPuzzlePiece, QBrush(Qt::lightGray));
            labelPuzzlePiece[i]->animate();
        }
        QPoint labelPositionPuzzlePiece(par.minBorderWidth + i * labelPuzzlePieceWidth + i * borderPuzzlePieceWidth, par.minBorderWidth + borderPuzzlePieceHeight);
        labelPuzzlePiece[i]->move(labelPositionPuzzlePiece);
        labelPuzzlePiece[i]->setToolTip(PuzzlePath::tooltip(PuzzlePath::intToTypeOfPiece(i)));

        m_radioButtonPuzzlePiece.push_back(new QRadioButton(radioGroupBoxPuzzlePiece));
        QPoint radioPositionPuzzlePiece = labelPositionPuzzlePiece + QPoint(labelPuzzlePieceWidth / 2 - 10, labelPuzzlePieceHeight + borderPuzzlePieceHeight);
        m_radioButtonPuzzlePiece[i]->move(radioPositionPuzzlePiece);

        QObject::connect(labelPuzzlePiece[i], &PuzzleButton::clicked, m_radioButtonPuzzlePiece[i], &QRadioButton::toggle);
    }
    m_radioButtonPuzzlePiece[PuzzlePath::typeOfPieceToInt(Jigsaw::TypeOfPiece::STANDARD)]->setChecked(true);
    m_ownShapeLabel = labelPuzzlePiece.last();
    QObject::connect(labelPuzzlePiece.last(), &PuzzleButton::clicked, m_createOwnShapeWidget, &QWidget::show);
    QObject::connect(labelPuzzlePiece.last(), &PuzzleButton::clicked, m_createOwnShapeWidget, &QWidget::raise);
    QFont font("Georgia", 32, QFont::Bold);
    labelPuzzlePiece.last()->setText("?");
    labelPuzzlePiece.last()->setTextArea(labelPuzzlePiece.last()->rect());
    labelPuzzlePiece.last()->setFont(font);
}

void PuzzleGame::setNewWidgetNumberOfPiecesWidget(QWidget *parent, const Jigsaw::Parameters &par)
{
    QWidget* widgetNumberOfPieces = new QWidget(parent);
    widgetNumberOfPieces->setGeometry(par.rectWidgetNumberOfPieces);

    QLabel* caption = new QLabel("拼图块数量:", widgetNumberOfPieces);
    caption->setFont(m_parameters.mainFont);
    caption->setGeometry(QRect(QPoint(par.minBorderWidth, par.minBorderWidth), QSize(par.widthWidgetNumberOfPieces / 2 - par.minBorderWidth * 2, par.heightWidgetNumberOfPieces / 4 - par.minBorderWidth * 2)));

    m_rotationAllowedCheckBox = new QCheckBox("允许旋转", widgetNumberOfPieces);
    m_rotationAllowedCheckBox->setFont(m_parameters.mainFont);
    m_rotationAllowedCheckBox->setGeometry(QRect(QPoint(par.minBorderWidth + par.widthWidgetNumberOfPieces / 2, par.minBorderWidth), QSize(par.widthWidgetNumberOfPieces / 2 - par.minBorderWidth * 2, par.heightWidgetNumberOfPieces / 4 - par.minBorderWidth * 2)));

    QPainterPath path = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), par.sizeButtonOuterBounds), QRect(), Jigsaw::TypeOfPiece::STANDARD, 4, true);

    m_sliderButton = new PuzzleSlider(par.sizeButtonOuterBounds, QBrush(QPixmap(":/backgrounds/back2")), path, QBrush(QPixmap(":/backgrounds/back1")), widgetNumberOfPieces, 10, 10, 50);
    m_sliderButton->setGeometry(QRect(QPoint(0, caption->height() + par.minBorderWidth), QSize(par.widthWidgetNumberOfPieces - par.minBorderWidth * 4, par.heightWidgetNumberOfPieces / 8)));
    m_sliderButton->setFont(m_parameters.mainFont);
    m_sliderButton->animate();
}

void PuzzleGame::setNewWidgetButtonsWidget(QWidget *parent, const Jigsaw::Parameters &par)
{
    QWidget* widgetButtons = new QWidget(parent);
    widgetButtons->setGeometry(par.rectWidgetButtons);

    QSize sizeButtonOuterBounds(120, 120);

    QPainterPath pathOkButton = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeButtonOuterBounds), QRect(), Jigsaw::TypeOfPiece::STANDARD, 4, true);
    QPainterPath pathCancelButton = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeButtonOuterBounds), QRect(), Jigsaw::TypeOfPiece::STANDARD, 4, true);

    PuzzleButton* okButton = new PuzzleButton(sizeButtonOuterBounds, QBrush(QPixmap(":/backgrounds/back2")), pathOkButton, widgetButtons, "确定");
    okButton->animate();
    okButton->setFont(m_parameters.mainFont);
    okButton->move(par.minBorderWidth, par.minBorderWidth);

    PuzzleButton* cancelButton = new PuzzleButton(sizeButtonOuterBounds, QBrush(QPixmap(":/backgrounds/back2")), pathCancelButton, widgetButtons, "取消");
    cancelButton->animate();
    cancelButton->setFont(m_parameters.mainFont);
    cancelButton->move(par.rectWidgetButtons.right() - par.minBorderWidth - cancelButton->width(), par.minBorderWidth);

    QObject::connect(cancelButton, &PuzzleButton::clicked, m_newWidget, &QWidget::hide);
    QObject::connect(okButton, &PuzzleButton::clicked, this, &PuzzleGame::newWidgetOkClicked);
}

void PuzzleGame::setWonWidget()
{
    m_wonWidget = new QWidget(this);
    QSize sizeOuterBoundsBackground(1000, 600);
    QSize sizeInnerBoundsBackground(sizeOuterBoundsBackground - QSize(400, 400));
    QRect innerBoundsBackground(QPoint(200, 200), sizeInnerBoundsBackground);
    QSize sizeOuterBoundsButtons(130, 100);
    QSize sizeInnerBoundsButtons(70, 60);
    
    // 直接获取屏幕尺寸，计算界面在屏幕中的居中位置
    QScreen *screen = QApplication::primaryScreen();
    int xPos, yPos;
    
    if (screen) {
        QRect screenGeometry = screen->geometry();
        xPos = (screenGeometry.width() - sizeOuterBoundsBackground.width()) / 2;  // 在屏幕中水平居中
        yPos = (screenGeometry.height() - sizeOuterBoundsBackground.height()) / 2;  // 在屏幕中垂直居中
    } else {
        // 如果无法获取屏幕信息，使用默认居中
        xPos = (1920 - sizeOuterBoundsBackground.width()) / 2;
        yPos = (1080 - sizeOuterBoundsBackground.height()) / 2;
    }
    
    QPoint positionWidget(xPos, yPos);

    m_wonWidget = new QWidget(this);
    m_wonWidget->setGeometry(QRect(positionWidget, sizeOuterBoundsBackground));

    QPainterPath pathBackgroundLabel = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBoundsBackground), QRect(), Jigsaw::TypeOfPiece::STANDARD, 4, true);
    PuzzleLabel* backgroundLabel = new PuzzleLabel(sizeOuterBoundsBackground, QBrush(QPixmap(":/backgrounds/back3")), pathBackgroundLabel, m_wonWidget, "恭喜您赢了!!!", innerBoundsBackground);
    backgroundLabel->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    backgroundLabel->setFont(m_parameters.mainFont);

    QPainterPath pathOkButton = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBoundsButtons), QRect(), Jigsaw::TypeOfPiece::STANDARD, 4, true);
    PuzzleButton* okButton = new PuzzleButton(sizeOuterBoundsButtons, QBrush(QPixmap(":/backgrounds/back2")), pathOkButton, m_wonWidget, "确定");
    okButton->move(innerBoundsBackground.center() + QPoint(-okButton->width() / 2, okButton->height() / 2));
    okButton->setFont(m_parameters.mainFont);
    okButton->animate();
    QObject::connect(okButton, &PuzzleButton::clicked, m_wonWidget, &QWidget::hide);

    m_wonWidget->hide();
}

void PuzzleGame::wonGame()
{
    // 停止游戏计时器
    stopGameTimer();
    
    for (const auto &puzzlePiece : m_puzzlePieces) {
        puzzlePiece->hide();
    }
    ImageEffects* effect = new ImageEffects(m_background, QPixmap(m_filename), ImageEffects::TypeOfEffect::GROW, this, 3000);
    QObject::connect(effect, &ImageEffects::effectFinished, m_wonWidget, &QWidget::show);
    effect->run();
}

void PuzzleGame::setCreateOwnShapeWidget()
{
    m_createOwnShapeWidget = new QWidget(this);
    
    // 直接获取屏幕尺寸，计算界面在屏幕中的居中位置
    QScreen *screen = QApplication::primaryScreen();
    int xPos, yPos;
    QSize widgetSize(m_parameters.screenWidth, m_parameters.screenHeight);
    
    if (screen) {
        QRect screenGeometry = screen->geometry();
        xPos = (screenGeometry.width() - widgetSize.width()) / 2;  // 在屏幕中水平居中
        yPos = (screenGeometry.height() - widgetSize.height()) / 2;  // 在屏幕中垂直居中
    } else {
        // 如果无法获取屏幕信息，使用默认居中
        xPos = (1920 - widgetSize.width()) / 2;
        yPos = (1080 - widgetSize.height()) / 2;
    }
    
    m_createOwnShapeWidget->setGeometry(QRect(QPoint(xPos, yPos), widgetSize));
    QLabel* background = new QLabel(m_createOwnShapeWidget);
    background->setGeometry(0, 0, m_parameters.screenWidth, m_parameters.screenHeight);
    QPixmap backgroundPixmap(QSize(m_parameters.screenWidth, m_parameters.screenHeight));
    backgroundPixmap.fill(Qt::lightGray);
    background->setPixmap(backgroundPixmap);
    m_customJigsawPathCreator = new PathCreator(m_createOwnShapeWidget);
    m_customJigsawPathCreator->setGeometry(0, 0, m_parameters.screenWidth, m_parameters.screenHeight);
    m_customJigsawPathCreator->raise();

    QObject::connect(m_customJigsawPathCreator, &PathCreator::apply, this, &PuzzleGame::customJigsawPathCreatorApplyClicked);

    m_createOwnShapeWidget->hide();
}

PuzzleGame::PuzzleGame(QWidget *parent)
    : QWidget{parent}
    , m_background(new QLabel(this))
    , m_gameTime(0)
    , m_moveCount(0)
    , m_gameStarted(false)
{
    // 让PuzzleWidget填满整个父窗口
    setGeometry(0, 0, parent->width(), parent->height());
    
    // 背景也填满整个PuzzleWidget
    m_background->setGeometry(0, 0, width(), height());
    m_background->setScaledContents(true);
    m_background->setPixmap(QPixmap(":/backgrounds/back1"));

    // 初始化统计组件
    setupStatsWidget();

    setCreateOwnShapeWidget();
    setMenuWidget();
    setNewWidget();
    setQuitWidget();
    setWonWidget();
}

void PuzzleGame::loadImage(const QPixmap &image)
{
    m_image = image;
}

void PuzzleGame::setupPuzzle()
{
    m_grid = new PuzzleGrid(m_rows, m_cols, m_pieceWidth, m_pieceHeight, m_typeOfPiece, this, m_customJigsawPath);
    setupImage();
    generatePuzzlePieces();
    placePuzzlePieces();
    
    // 启动游戏计时器
    startGameTimer();
}

void PuzzleGame::menuNewButtonClicked()
{
    m_newWidget->show();
    m_newWidget->raise();
}

void PuzzleGame::menuQuitButtonClicked()
{
    m_quitWidget->show();
    m_quitWidget->raise();
}

void PuzzleGame::enterMenu()
{
    if (m_menuWidget->pos().y() > 960) {  // 如果菜单在屏幕下方（隐藏状态）
        m_moveMenuOutTimer->stop();
        m_moveMenuInTimer->start(10);
    }
}

void PuzzleGame::leaveMenu()
{
    if (m_menuWidget->pos().y() < 1080) {  // 如果菜单在屏幕上方（显示状态）
        m_moveMenuOutTimer->start(10);
        m_moveMenuInTimer->stop();
    }
}

void PuzzleGame::moveMenuInTimerTimeout()
{
    if (m_menuWidget->pos().y() <= 960) {  // 移动到屏幕底部显示位置
        m_moveMenuInTimer->stop();
        return;
    }
    m_menuWidget->move(m_menuWidget->pos() + QPoint(0, -2));  // 向上移动
}

void PuzzleGame::moveMenuOutTimerTimeout()
{
    if (m_menuWidget->pos().y() >= 1080) {  // 移动到屏幕下方隐藏位置
        m_moveMenuOutTimer->stop();
        return;
    }
    m_menuWidget->move(m_menuWidget->pos() + QPoint(0, 2));  // 向下移动
}

void PuzzleGame::quitWidgetYesClicked()
{
    QApplication::quit();
}

void PuzzleGame::newWidgetOkClicked()
{
    if (!m_radioButtonEx.last()->isChecked()) {
        QPixmap image;
        for (int i = 0; i < m_radioButtonEx.size(); ++i) {
            if (m_radioButtonEx[i]->isChecked()) image = QPixmap(":/examples/ex" + QString::number(i));
        }
        loadImage(image);
    }
    else {
        loadImage(QPixmap(m_filename));
    }

    for (int i = 0; i < m_radioButtonPuzzlePiece.size(); ++i) {
        if (m_radioButtonPuzzlePiece[i]->isChecked()) m_typeOfPiece = PuzzlePath::intToTypeOfPiece(i);
    }

    qDeleteAll(m_puzzlePieces);
    m_puzzlePieces.clear();
    m_mergedPieces.clear();

    // 重置游戏统计信息
    resetGameStats();

    calculateRowsAndCols(m_sliderButton->val(), m_image);
    m_rotationAllowed = m_rotationAllowedCheckBox->isChecked();

    setupPuzzle();
    m_newWidget->lower();
    m_newWidget->hide();

    m_background->setPixmap(QPixmap(":/backgrounds/back1"));
}

void PuzzleGame::newWidgetOwnImageClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "打开图片", m_filename.isEmpty() ? "C:/": m_filename.section('/', 0, -2), "图片文件 (*.png *.jpg *.bmp)");
    if (!fileName.isEmpty() && fileName != "" && !QPixmap(fileName).isNull()) {
        m_filename = fileName;
        m_radioButtonEx.last()->setCheckable(true);
        QPixmap preview(fileName);
        preview = preview.scaled(QSize(m_ownImageLabel->width(), m_ownImageLabel->height()));
        m_ownImageLabel->setPixmap(preview);
        m_ownImageLabel->setText("");
        loadImage(QPixmap(fileName));
    }
}

void PuzzleGame::customJigsawPathCreatorApplyClicked(const CustomPuzzlePath &customJigsawPath)
{
    m_customJigsawPath = customJigsawPath;

//    QPainterPath customPath = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), m_ownShapeLabel->size()), QRect(QPoint(0, 0),
//                                                                m_ownShapeLabel->size() / 2), TypeOfPiece::CUSTOM, 4, false, customJigsawPath);
//    m_ownShapeLabel->setJigsawPath(customPath, m_ownShapeLabel->size());

    m_createOwnShapeWidget->hide();
}

void PuzzleGame::dragMergedPieces(int id, const QPointF &draggedBy)
{
    int mergedPieceID;
    if (isPartOfMergedPiece(m_puzzlePieces[id], mergedPieceID)) {
        for (const auto &piece : m_mergedPieces[mergedPieceID]) {
            if (piece != m_puzzlePieces[id]) piece->move(piece->originalPosition() + draggedBy);
        }
    }
}

void PuzzleGame::rotateMergedPieces(int id, int angle, const QPointF &rotatingPoint)
{
    int mergedPieceID;
    if (isPartOfMergedPiece(m_puzzlePieces[id], mergedPieceID)) {
        for (const auto &piece : m_mergedPieces[mergedPieceID]) {
            if (piece != m_puzzlePieces[id]) {
                QPoint pieceGridPoint = m_grid->overlayGridPoint(id);
                QPoint neighborGridPoint = m_grid->overlayGridPoint(piece->id());
                QLineF line(pieceGridPoint, neighborGridPoint);
                piece->rotateAroundPoint(angle, rotatingPoint, line.length(), line.angle());
            }
        }
    }
}

void PuzzleGame::raisePieces(int id)
{
    int mergedPieceID;
    if (isPartOfMergedPiece(m_puzzlePieces[id], mergedPieceID)) {
        for (const auto &piece : m_mergedPieces[mergedPieceID]) {
            piece->raise();
        }
    }
    else {
        m_puzzlePieces[id]->raise();
    }
}

void PuzzleGame::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // 当窗口大小改变时，调整背景大小
    if (m_background) {
        m_background->setGeometry(0, 0, width(), height());
    }
    
    // 调整底边栏位置
    if (m_menuWidget) {
        QSize menuSize = m_menuWidget->size();
        int xPos = (width() - menuSize.width()) / 2;  // 水平居中
        int yPos = height() - menuSize.height();       // 底部
        m_menuWidget->setGeometry(QRect(QPoint(xPos, yPos), menuSize));
    }
    
    // 调整统计组件位置
    if (m_statsWidget) {
        m_statsWidget->setGeometry(10, 10, 200, 80);
    }
}

void PuzzleGame::setupStatsWidget()
{
    m_statsWidget = new QWidget(this);
    m_statsWidget->setGeometry(10, 10, 200, 80);
    m_statsWidget->setStyleSheet("QWidget { background-color: rgba(0, 0, 0, 0.7); border-radius: 10px; }");
    
    QVBoxLayout* layout = new QVBoxLayout(m_statsWidget);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(5);
    
    // 时间标签
    m_timeLabel = new QLabel("时间: 00:00", m_statsWidget);
    m_timeLabel->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; }");
    layout->addWidget(m_timeLabel);
    
    // 步数标签
    m_movesLabel = new QLabel("步数: 0", m_statsWidget);
    m_movesLabel->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; }");
    layout->addWidget(m_movesLabel);
    
    // 游戏计时器
    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(1000); // 每秒更新一次
    connect(m_gameTimer, &QTimer::timeout, this, &PuzzleGame::updateTimeDisplay);
    
    m_statsWidget->raise();
}

void PuzzleGame::updateTimeDisplay()
{
    m_gameTime++;
    int minutes = m_gameTime / 60;
    int seconds = m_gameTime % 60;
    m_timeLabel->setText(QString("时间: %1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));
}

void PuzzleGame::updateMovesDisplay()
{
    m_moveCount++;
    m_movesLabel->setText(QString("步数: %1").arg(m_moveCount));
}

void PuzzleGame::startGameTimer()
{
    m_gameStarted = true;  // 标记游戏已开始
    m_gameTimer->start();
}

void PuzzleGame::stopGameTimer()
{
    m_gameTimer->stop();
}

void PuzzleGame::resetGameStats()
{
    m_gameTime = 0;
    m_moveCount = 0;
    m_gameStarted = false;  // 重置游戏开始标志
    updateTimeDisplay();
    updateMovesDisplay();
}
