#ifndef PUZZLE_GAME_H
#define PUZZLE_GAME_H

#include "tools/path_creator.h"
#include "ui/game_menu.h"
#include "components/puzzle_path.h"
#include "puzzle_grid.h"
#include "ui/puzzle_label.h"
#include "components/puzzle_piece.h"
#include "ui/puzzle_button.h"
#include "ui/puzzle_slider.h"
#include "tools/image_effects.h"
#include <QRadioButton>
#include <QCheckBox>
#include <QWidget>
#include <QTimer>
#include <QPainterPath>
#include <QLabel>

// @Julia: Das ist die Hauptklasse. Ignoriere das struct und scrolle runter bis zur class PuzzleWidget. Viel Spaß!



/*
 * The PuzzleWidget class is the core of the jigsaw puzzle game. You can load your own image or choose one of
 * the examples to turn it into a jigsaw puzzle. You can choose the shape and the number of your pieces and
 * whether you want them to be rotatable or not. In the GUI, the pieces can be dragged (or rotated) by clicking
 * them. When you put two pieces close enough together, they are merged and will be dragged (or rotated) together
 * from here on. The game is finished, when all pieces are merged.
 */

class PuzzleGame : public QWidget
{
    Q_OBJECT

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QLabel* m_background;
    QVector<PuzzlePiece*> m_puzzlePieces;

    QVector<QVector<PuzzlePiece*>> m_mergedPieces;

    void createNewMergedPiece(PuzzlePiece* firstPiece);
    void addPuzzlePieceToMergedPiece(PuzzlePiece* piece, int mergedPieceID);
    void combineTwoMergedPieces(int firstMergedPieceID, int secondMergedPieceID);
    bool isPartOfMergedPiece(PuzzlePiece* piece, int &mergedPieceID);
    bool areMerged(PuzzlePiece* first, PuzzlePiece* second);

    bool isInCorrectPosition(PuzzlePiece* piece, PuzzlePiece *neighbor, int tolerance = 5);
    void repositionPiece(PuzzlePiece* piece, PuzzlePiece* neighbor);

    int m_numberOfPieces;
    bool m_rotationAllowed;

    void calculateRowsAndCols(int numberOfPieces, const QPixmap &image);

    int m_rows;
    int m_cols;
    QPixmap m_image;
    Jigsaw::TypeOfPiece m_typeOfPiece;
    CustomPuzzlePath m_customJigsawPath;

    int m_pieceWidth;
    int m_pieceHeight;

    PuzzleGrid *m_grid;

    void setupImage();

    void generatePuzzlePieces();
    void placePuzzlePieces();

    QPixmap createImageFragment(int pieceID);

    //Menu Widgets

    Jigsaw::Parameters m_parameters;

    GameMenu* m_menuWidget;

    void setMenuWidget();

    QTimer* m_moveMenuInTimer;
    QTimer* m_moveMenuOutTimer;

    QWidget* m_quitWidget;

    void setQuitWidget();

    QWidget* m_newWidget;

    QVector<QRadioButton*> m_radioButtonEx;
    QVector<QRadioButton*> m_radioButtonPuzzlePiece;
    QCheckBox* m_rotationAllowedCheckBox;
    PuzzleButton* m_ownImageLabel;
    PuzzleButton* m_ownShapeLabel;
    QString m_filename;
    PuzzleSlider* m_sliderButton;

    void setNewWidget();
    void setNewWidgetExWidget(QWidget* parent, const Jigsaw::Parameters &par);
    void setNewWidgetPuzzlePieceWidget(QWidget* parent, const Jigsaw::Parameters &par);
    void setNewWidgetNumberOfPiecesWidget(QWidget* parent, const Jigsaw::Parameters &par);
    void setNewWidgetButtonsWidget(QWidget* parent, const Jigsaw::Parameters &par);

    QWidget* m_wonWidget;

    void setWonWidget();
    void wonGame();

    QWidget* m_createOwnShapeWidget;
    PathCreator* m_customJigsawPathCreator;

    void setCreateOwnShapeWidget();

    void loadImage(const QPixmap &image);
    void setupPuzzle();

    // 计时和计步相关
    QWidget* m_statsWidget;
    QLabel* m_timeLabel;
    QLabel* m_movesLabel;
    QTimer* m_gameTimer;
    int m_gameTime;
    int m_moveCount;
    bool m_gameStarted;  // 游戏是否已经开始
    
    void setupStatsWidget();
    void updateTimeDisplay();
    void updateMovesDisplay();
    void startGameTimer();
    void stopGameTimer();
    void resetGameStats();

public:
    explicit PuzzleGame(QWidget *parent = nullptr);

private slots:
    void menuNewButtonClicked();
    void menuQuitButtonClicked();

    void enterMenu();
    void leaveMenu();
    void moveMenuInTimerTimeout();
    void moveMenuOutTimerTimeout();

    void quitWidgetYesClicked();
    void newWidgetOkClicked();
    void newWidgetOwnImageClicked();
    void customJigsawPathCreatorApplyClicked(const CustomPuzzlePath &customJigsawPath);

    void dragMergedPieces(int id, const QPointF &draggedBy);
    void rotateMergedPieces(int id, int angle, const QPointF &rotatingPoint);
    void raisePieces(int id);
    void fixPieceIfPossible(int id);
    void fixMergedPieceIfPossible(int id);

signals:

};

#endif // PUZZLE_GAME_H
