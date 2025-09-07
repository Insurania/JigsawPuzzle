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



    QObject::connect(m_menuWidget->newPuzzleButton(), &PuzzleButton::clicked, this, &PuzzleGame::menuNewButtonClicked);
    QObject::connect(m_menuWidget->quitButton(), &PuzzleButton::clicked, this, &PuzzleGame::menuQuitButtonClicked);
    QObject::connect(m_menuWidget->saveButton(), &PuzzleButton::clicked, this, &PuzzleGame::showSaveManager);
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

    QPainterPath backgroundLabelPath = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBoundsBackground), innerBoundsBackground, Jigsaw::TypeOfPiece::TRAPEZOID);
    PuzzleLabel* backgroundLabel = new PuzzleLabel(sizeOuterBoundsBackground, QBrush(QPixmap(":/backgrounds/back3")), backgroundLabelPath,
                                                   m_quitWidget, "您确定要退出吗?", innerBoundsBackground);
    backgroundLabel->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    backgroundLabel->setFont(m_parameters.mainFont);

    QPainterPath backgroundYesButtonPath = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBoundsButtons), QRect(), Jigsaw::TypeOfPiece::TRAPEZOID, 4, true);
    QPainterPath backgroundNoButtonPath = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBoundsButtons), QRect(), Jigsaw::TypeOfPiece::TRAPEZOID, 4, true);

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

    QPainterPath backgroundLabelPath = PuzzlePath::singleJigsawPiecePath(m_parameters.rectWidget, m_parameters.rectWidgetArea, Jigsaw::TypeOfPiece::TRAPEZOID);
    PuzzleLabel* backgroundLabel = new PuzzleLabel(m_parameters.sizeWidget, QBrush(QPixmap(":/backgrounds/back3")), backgroundLabelPath, m_newWidget,
                                                   "", m_parameters.rectWidgetArea);
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
    int numberOfTypes = static_cast<int>(Jigsaw::TypeOfPiece::count) - 1; // 排除CUSTOM类型
    int visibleTypes = 2; // 只显示前两种形状：TRAPEZOID和SIMPLEARC
    // 让两个按钮挨着显示，减少间距
    int smallSpacing = 10; // 使用较小的间距
    int maxLabelPuzzlePieceWidth = visibleTypes != 0 ? (par.widthWidgetPuzzlePiece - par.minBorderWidth * 2 - smallSpacing) / visibleTypes : 0;
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

    int borderPuzzlePieceWidth = smallSpacing; // 使用固定的小间距
    int borderPuzzlePieceHeight = (par.heightWidgetPuzzlePiece - par.minBorderWidth * 2 - labelPuzzlePieceHeight - par.minRadioButtonHeight) / 2;

    // 只显示前两种形状，但创建所有形状的代码
    
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
        
        // 只对前两种形状设置位置和显示
        if (i < visibleTypes) {
            // 计算居中位置，让两个按钮紧凑地显示在中间
            int totalWidth = visibleTypes * labelPuzzlePieceWidth + (visibleTypes - 1) * borderPuzzlePieceWidth;
            int startX = (par.widthWidgetPuzzlePiece - totalWidth) / 2;
            QPoint labelPositionPuzzlePiece(startX + i * (labelPuzzlePieceWidth + borderPuzzlePieceWidth), par.minBorderWidth + borderPuzzlePieceHeight);
            labelPuzzlePiece[i]->move(labelPositionPuzzlePiece);
            labelPuzzlePiece[i]->setToolTip(PuzzlePath::tooltip(PuzzlePath::intToTypeOfPiece(i)));
            labelPuzzlePiece[i]->show(); // 确保前两种形状可见
        } else {
            // 隐藏后面的形状，但保留代码
            labelPuzzlePiece[i]->hide();
        }

        m_radioButtonPuzzlePiece.push_back(new QRadioButton(radioGroupBoxPuzzlePiece));
        
        if (i < visibleTypes) {
            // 计算居中位置，让两个单选按钮紧凑地显示在中间
            int totalWidth = visibleTypes * labelPuzzlePieceWidth + (visibleTypes - 1) * borderPuzzlePieceWidth;
            int startX = (par.widthWidgetPuzzlePiece - totalWidth) / 2;
            QPoint radioPositionPuzzlePiece = QPoint(startX + i * (labelPuzzlePieceWidth + borderPuzzlePieceWidth), par.minBorderWidth + borderPuzzlePieceHeight) + QPoint(labelPuzzlePieceWidth / 2 - 10, labelPuzzlePieceHeight + borderPuzzlePieceHeight);
            m_radioButtonPuzzlePiece[i]->move(radioPositionPuzzlePiece);
            m_radioButtonPuzzlePiece[i]->show(); // 确保前两种形状的单选按钮可见
        } else {
            // 隐藏后面的单选按钮，但保留代码
            m_radioButtonPuzzlePiece[i]->hide();
        }

        QObject::connect(labelPuzzlePiece[i], &PuzzleButton::clicked, m_radioButtonPuzzlePiece[i], &QRadioButton::toggle);
    }
    m_radioButtonPuzzlePiece[PuzzlePath::typeOfPieceToInt(Jigsaw::TypeOfPiece::TRAPEZOID)]->setChecked(true);
    m_ownShapeLabel = labelPuzzlePiece.last();
    // 隐藏自定义形状按钮
    m_ownShapeLabel->hide();
    // 隐藏对应的单选按钮
    m_radioButtonPuzzlePiece.last()->hide();
    
    // 注释掉原有的连接和设置
    // QObject::connect(labelPuzzlePiece.last(), &PuzzleButton::clicked, m_createOwnShapeWidget, &QWidget::show);
    // QObject::connect(labelPuzzlePiece.last(), &PuzzleButton::clicked, m_createOwnShapeWidget, &QWidget::raise);
    // QFont font("Georgia", 32, QFont::Bold);
    // labelPuzzlePiece.last()->setText("?");
    // labelPuzzlePiece.last()->setTextArea(labelPuzzlePiece.last()->rect());
    // labelPuzzlePiece.last()->setFont(font);
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

    QPainterPath path = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), par.sizeButtonOuterBounds), QRect(), Jigsaw::TypeOfPiece::TRAPEZOID, 4, true);

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

    QPainterPath pathOkButton = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeButtonOuterBounds), QRect(), Jigsaw::TypeOfPiece::TRAPEZOID, 4, true);
    QPainterPath pathCancelButton = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeButtonOuterBounds), QRect(), Jigsaw::TypeOfPiece::TRAPEZOID, 4, true);

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

    QPainterPath pathBackgroundLabel = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBoundsBackground), QRect(), Jigsaw::TypeOfPiece::TRAPEZOID, 4, true);
    PuzzleLabel* backgroundLabel = new PuzzleLabel(sizeOuterBoundsBackground, QBrush(QPixmap(":/backgrounds/back3")), pathBackgroundLabel, m_wonWidget, "恭喜您赢了!!!", innerBoundsBackground);
    backgroundLabel->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    backgroundLabel->setFont(m_parameters.mainFont);

    QPainterPath pathOkButton = PuzzlePath::singleJigsawPiecePath(QRect(QPoint(0, 0), sizeOuterBoundsButtons), QRect(), Jigsaw::TypeOfPiece::TRAPEZOID, 4, true);
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
    setupSaveSystem();
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
            if (m_radioButtonEx[i]->isChecked()) {
                image = QPixmap(":/examples/ex" + QString::number(i));
                m_filename = ":/examples/ex" + QString::number(i); // 设置文件名
            }
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

// 存档系统实现
void PuzzleGame::setupSaveSystem()
{
    m_saveSystem = new SaveSystem(this);
    m_saveManager = new SaveManager(this);
    
    // 连接存档管理器的信号
    connect(m_saveManager, &SaveManager::saveRequested, this, [this](const GameSaveData& gameData, const QString& saveName) {
        if (m_saveSystem->saveGame(gameData, saveName)) {
            m_saveManager->hide();
        }
    });
    
    connect(m_saveManager, &SaveManager::loadRequested, this, [this](const QString& saveName) {
        GameSaveData gameData = m_saveSystem->loadGame(saveName);
        if (!gameData.saveName.isEmpty()) {
            loadGameFromData(gameData);
            m_saveManager->hide();
        }
    });
    
    connect(m_saveManager, &SaveManager::closeRequested, this, [this]() {
        m_saveManager->hide();
    });
    
    // 确保存档管理器初始时隐藏
    m_saveManager->hide();
    
    // 设置存档管理器为模态窗口
    m_saveManager->setWindowModality(Qt::ApplicationModal);
    
    // 设置存档管理器为独立窗口
    m_saveManager->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    
    // 设置存档管理器标题
    m_saveManager->setWindowTitle("存档管理");
    
    // 设置存档管理器大小
    m_saveManager->setFixedSize(600, 500);
    
    // 设置存档管理器居中显示
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - 600) / 2;
        int y = (screenGeometry.height() - 500) / 2;
        m_saveManager->move(x, y);
    }
    
    // 设置存档管理器为可调整大小
    m_saveManager->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
}

GameSaveData PuzzleGame::createCurrentGameData()
{
    GameSaveData data;
    data.saveName = "";
    data.saveTime = QDateTime::currentDateTime();
    data.imagePath = m_filename.isEmpty() ? ":/examples/ex0" : m_filename; // 确保有默认图片路径
    data.rows = m_rows;
    data.cols = m_cols;
    data.numberOfPieces = m_numberOfPieces;
    data.typeOfPiece = m_typeOfPiece;
    data.rotationAllowed = m_rotationAllowed;
    data.gameTime = m_gameTime;
    data.moveCount = m_moveCount;
    data.gameStarted = m_gameStarted;
    
    // 保存所有碎片的状态
    for (PuzzlePiece* piece : m_puzzlePieces) {
        PuzzlePieceSaveData pieceData;
        pieceData.id = piece->id();
        pieceData.position = piece->originalPosition();
        pieceData.angle = piece->angle();
        pieceData.isFixed = false; // 暂时设为false，实际状态会在加载时重新计算
        pieceData.mergedPieceID = -1; // 需要从合并组中获取
        
        data.pieces.append(pieceData);
    }
    
    // 如果没有碎片数据，创建默认数据
    if (data.pieces.isEmpty()) {
        for (int i = 0; i < data.numberOfPieces; ++i) {
            PuzzlePieceSaveData pieceData;
            pieceData.id = i;
            pieceData.position = QPointF(100 + i * 10, 100 + i * 10); // 默认位置
            pieceData.angle = 0;
            pieceData.isFixed = false;
            pieceData.mergedPieceID = -1;
            data.pieces.append(pieceData);
        }
    }
    
    // 保存合并的碎片组（将PuzzlePiece*转换为ID）
    for (const QVector<PuzzlePiece*>& mergedGroup : m_mergedPieces) {
        QVector<int> groupIds;
        for (PuzzlePiece* piece : mergedGroup) {
            groupIds.append(piece->id());
        }
        data.mergedPieces.append(groupIds);
    }
    
    // 如果没有合并组数据，创建空数据
    if (data.mergedPieces.isEmpty()) {
        // 保持空，表示没有合并的碎片
    }
    
    return data;
}

void PuzzleGame::loadGameFromData(const GameSaveData& gameData)
{
    // 停止当前游戏
    stopGameTimer();
    
    // 清理当前游戏状态
    for (PuzzlePiece* piece : m_puzzlePieces) {
        piece->deleteLater();
    }
    m_puzzlePieces.clear();
    m_mergedPieces.clear();
    
    if (m_grid) {
        m_grid->deleteLater();
        m_grid = nullptr;
    }
    
    // 设置游戏参数
    m_rows = gameData.rows;
    m_cols = gameData.cols;
    m_numberOfPieces = gameData.numberOfPieces;
    m_typeOfPiece = gameData.typeOfPiece;
    m_rotationAllowed = gameData.rotationAllowed;
    m_gameTime = gameData.gameTime;
    m_moveCount = gameData.moveCount;
    m_gameStarted = gameData.gameStarted;
    m_filename = gameData.imagePath;
    
    // 加载图片
    if (!m_filename.isEmpty()) {
        QPixmap image(m_filename);
        if (!image.isNull()) {
            loadImage(image);
        } else {
            // 如果无法加载图片，尝试从资源加载示例图片
            QPixmap fallbackImage(":/examples/ex0");
            if (!fallbackImage.isNull()) {
                loadImage(fallbackImage);
            }
        }
    } else {
        // 如果没有图片文件，尝试从资源加载示例图片
        QPixmap image(":/examples/ex0");
        if (!image.isNull()) {
            loadImage(image);
        }
    }
    
    // 重新创建拼图
    if (!m_image.isNull()) {
        // 创建网格和图片，但不放置碎片
        m_grid = new PuzzleGrid(m_rows, m_cols, m_pieceWidth, m_pieceHeight, m_typeOfPiece, this, m_customJigsawPath);
        setupImage();
        generatePuzzlePieces();
    } else {
        // 如果图片加载失败，尝试使用默认图片重新创建拼图
        qDebug() << "图片加载失败，尝试使用默认图片重新创建拼图";
        QPixmap defaultImage(":/examples/ex0");
        if (!defaultImage.isNull()) {
            loadImage(defaultImage);
            m_grid = new PuzzleGrid(m_rows, m_cols, m_pieceWidth, m_pieceHeight, m_typeOfPiece, this, m_customJigsawPath);
            setupImage();
            generatePuzzlePieces();
        } else {
            qDebug() << "无法加载默认图片，拼图创建失败";
            return;
        }
    }
    
    // 恢复碎片状态（不调用placePuzzlePieces）
    for (const PuzzlePieceSaveData& pieceData : gameData.pieces) {
        if (pieceData.id < m_puzzlePieces.size()) {
            PuzzlePiece* piece = m_puzzlePieces[pieceData.id];
            piece->move(pieceData.position);
            piece->setAngle(pieceData.angle);
            piece->show();
            piece->raise();
            // 注意：isFixed 状态会在 fixPieceIfPossible 中自动处理
        }
    }
    
    // 确保所有碎片都可见
    for (PuzzlePiece* piece : m_puzzlePieces) {
        piece->show();
        piece->raise();
    }
    
    // 如果没有碎片数据，使用默认位置
    if (gameData.pieces.isEmpty()) {
        for (int i = 0; i < m_puzzlePieces.size(); ++i) {
            PuzzlePiece* piece = m_puzzlePieces[i];
            piece->move(100.0 + i * 10.0, 100.0 + i * 10.0);
            piece->setAngle(0);
            piece->show();
            piece->raise();
        }
    }
    
    // 恢复合并的碎片组（将ID转换回PuzzlePiece*）
    m_mergedPieces.clear();
    for (const QVector<int>& groupIds : gameData.mergedPieces) {
        QVector<PuzzlePiece*> group;
        for (int id : groupIds) {
            if (id < m_puzzlePieces.size()) {
                group.append(m_puzzlePieces[id]);
            }
        }
        if (!group.isEmpty()) {
            m_mergedPieces.append(group);
        }
    }
    
    // 如果没有合并组数据，保持空
    if (gameData.mergedPieces.isEmpty()) {
        // 保持空，表示没有合并的碎片
    }
    
    // 更新显示
    updateTimeDisplay();
    updateMovesDisplay();
    
    // 如果游戏已经开始且未完成，继续计时
    if (m_gameStarted && m_mergedPieces.isEmpty()) {
        startGameTimer();
    } else if (m_mergedPieces.size() == 1 && m_mergedPieces[0].size() == m_numberOfPieces) {
        // 如果游戏已完成，显示胜利界面
        m_wonWidget->show();
        m_wonWidget->raise();
    } else {
        // 如果游戏未开始，不启动计时器
        stopGameTimer();
    }
}

void PuzzleGame::showSaveManager()
{
    // 设置当前游戏数据
    GameSaveData currentData = createCurrentGameData();
    m_saveManager->setCurrentGameData(currentData);
    
    // 显示存档管理器
    m_saveManager->show();
    m_saveManager->raise();
    m_saveManager->activateWindow();
    
    // 刷新存档列表
    m_saveManager->refreshSaveList();
}
