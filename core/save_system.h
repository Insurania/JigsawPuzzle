#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QPointF>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include "jigsaw_types.h"

/*
 * 存档系统类
 * 负责保存和加载游戏状态
 */

struct PuzzlePieceSaveData {
    int id;                    // 碎片ID
    QPointF position;          // 位置
    double angle;              // 旋转角度
    bool isFixed;              // 是否已固定
    int mergedPieceID;         // 合并的碎片组ID
    
    QJsonObject toJson() const;
    static PuzzlePieceSaveData fromJson(const QJsonObject& json);
};

struct GameSaveData {
    QString saveName;          // 存档名称
    QDateTime saveTime;        // 保存时间
    QString imagePath;         // 图片路径
    int rows;                  // 行数
    int cols;                  // 列数
    int numberOfPieces;        // 碎片总数
    Jigsaw::TypeOfPiece typeOfPiece; // 拼图形状类型
    bool rotationAllowed;      // 是否允许旋转
    int gameTime;              // 游戏时间（秒）
    int moveCount;             // 移动步数
    bool gameStarted;          // 游戏是否已开始
    QVector<PuzzlePieceSaveData> pieces; // 碎片数据
    QVector<QVector<int>> mergedPieces;  // 合并的碎片组
    
    QJsonObject toJson() const;
    static GameSaveData fromJson(const QJsonObject& json);
};

class SaveSystem : public QObject
{
    Q_OBJECT

public:
    explicit SaveSystem(QObject *parent = nullptr);
    
    // 保存游戏
    bool saveGame(const GameSaveData& gameData, const QString& saveName = "");
    
    // 加载游戏
    GameSaveData loadGame(const QString& saveName);
    
    // 获取所有存档列表
    QVector<QString> getSaveList();
    
    // 删除存档
    bool deleteSave(const QString& saveName);
    
    // 检查存档是否存在
    bool saveExists(const QString& saveName);
    
    // 获取存档信息（不加载完整数据）
    GameSaveData getSaveInfo(const QString& saveName);

signals:
    void saveCompleted(const QString& saveName);
    void loadCompleted(const QString& saveName);
    void saveError(const QString& error);
    void loadError(const QString& error);

private:
    QString m_saveDirectory;
    
    // 获取存档文件路径
    QString getSaveFilePath(const QString& saveName);
    
    // 确保保存目录存在
    bool ensureSaveDirectory();
    
    // 生成默认存档名称
    QString generateDefaultSaveName();
};

#endif // SAVE_SYSTEM_H
