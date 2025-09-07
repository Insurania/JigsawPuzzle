#include "save_system.h"
#include "jigsaw_types.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <algorithm>

// PuzzlePieceSaveData 实现
QJsonObject PuzzlePieceSaveData::toJson() const
{
    QJsonObject obj;
    obj["id"] = id;
    obj["positionX"] = position.x();
    obj["positionY"] = position.y();
    obj["angle"] = angle;
    obj["isFixed"] = isFixed;
    obj["mergedPieceID"] = mergedPieceID;
    return obj;
}

PuzzlePieceSaveData PuzzlePieceSaveData::fromJson(const QJsonObject& json)
{
    PuzzlePieceSaveData data;
    data.id = json["id"].toInt();
    data.position = QPointF(json["positionX"].toDouble(), json["positionY"].toDouble());
    data.angle = json["angle"].toDouble();
    data.isFixed = json["isFixed"].toBool();
    data.mergedPieceID = json["mergedPieceID"].toInt();
    return data;
}

// GameSaveData 实现
QJsonObject GameSaveData::toJson() const
{
    QJsonObject obj;
    obj["saveName"] = saveName;
    obj["saveTime"] = saveTime.toString(Qt::ISODate);
    obj["imagePath"] = imagePath;
    obj["rows"] = rows;
    obj["cols"] = cols;
    obj["numberOfPieces"] = numberOfPieces;
    obj["typeOfPiece"] = static_cast<int>(typeOfPiece);
    obj["rotationAllowed"] = rotationAllowed;
    obj["gameTime"] = gameTime;
    obj["moveCount"] = moveCount;
    obj["gameStarted"] = gameStarted;
    
    // 保存碎片数据
    QJsonArray piecesArray;
    for (const auto& piece : pieces) {
        piecesArray.append(piece.toJson());
    }
    obj["pieces"] = piecesArray;
    
    // 保存合并的碎片组
    QJsonArray mergedArray;
    for (const auto& mergedGroup : mergedPieces) {
        QJsonArray groupArray;
        for (int id : mergedGroup) {
            groupArray.append(id);
        }
        mergedArray.append(groupArray);
    }
    obj["mergedPieces"] = mergedArray;
    
    return obj;
}

GameSaveData GameSaveData::fromJson(const QJsonObject& json)
{
    GameSaveData data;
    data.saveName = json["saveName"].toString();
    data.saveTime = QDateTime::fromString(json["saveTime"].toString(), Qt::ISODate);
    data.imagePath = json["imagePath"].toString();
    data.rows = json["rows"].toInt();
    data.cols = json["cols"].toInt();
    data.numberOfPieces = json["numberOfPieces"].toInt();
    data.typeOfPiece = static_cast<Jigsaw::TypeOfPiece>(json["typeOfPiece"].toInt());
    data.rotationAllowed = json["rotationAllowed"].toBool();
    data.gameTime = json["gameTime"].toInt();
    data.moveCount = json["moveCount"].toInt();
    data.gameStarted = json["gameStarted"].toBool();
    
    // 加载碎片数据
    QJsonArray piecesArray = json["pieces"].toArray();
    for (const auto& value : piecesArray) {
        data.pieces.append(PuzzlePieceSaveData::fromJson(value.toObject()));
    }
    
    // 加载合并的碎片组
    QJsonArray mergedArray = json["mergedPieces"].toArray();
    for (const auto& value : mergedArray) {
        QJsonArray groupArray = value.toArray();
        QVector<int> group;
        for (const auto& idValue : groupArray) {
            group.append(idValue.toInt());
        }
        data.mergedPieces.append(group);
    }
    
    return data;
}

// SaveSystem 实现
SaveSystem::SaveSystem(QObject *parent)
    : QObject(parent)
{
    // 设置保存目录为用户的文档目录下的拼图游戏文件夹
    m_saveDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/JigsawPuzzle/Saves";
    qDebug() << "SaveSystem 初始化，保存目录:" << m_saveDirectory;
    ensureSaveDirectory();
}

bool SaveSystem::saveGame(const GameSaveData& gameData, const QString& saveName)
{
    try {
        if (!ensureSaveDirectory()) {
            qDebug() << "无法创建保存目录";
            emit saveError("无法创建保存目录");
            return false;
        }
        
        QString fileName = saveName.isEmpty() ? generateDefaultSaveName() : saveName;
        QString filePath = getSaveFilePath(fileName);
        
        QJsonObject jsonObj = gameData.toJson();
        jsonObj["saveName"] = fileName; // 确保存档名称正确
        
        QJsonDocument doc(jsonObj);
        QFile file(filePath);
        
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "无法打开保存文件:" << filePath << "错误:" << file.errorString();
            emit saveError("无法打开保存文件: " + file.errorString());
            return false;
        }
        
        QByteArray jsonData = doc.toJson();
        qint64 bytesWritten = file.write(jsonData);
        file.close();
        
        if (bytesWritten != jsonData.size()) {
            qDebug() << "保存文件写入不完整:" << bytesWritten << "/" << jsonData.size();
            emit saveError("保存文件写入不完整");
            return false;
        }
        
        qDebug() << "游戏已保存到:" << filePath << "碎片数量:" << gameData.pieces.size();
        emit saveCompleted(fileName);
        return true;
        
    } catch (const std::exception& e) {
        qDebug() << "保存游戏时发生错误:" << e.what();
        emit saveError("保存游戏时发生错误: " + QString(e.what()));
        return false;
    }
}

GameSaveData SaveSystem::loadGame(const QString& saveName)
{
    GameSaveData emptyData;
    
    try {
        QString filePath = getSaveFilePath(saveName);
        QFile file(filePath);
        
        if (!file.exists()) {
            qDebug() << "存档文件不存在:" << filePath;
            emit loadError("存档文件不存在: " + saveName);
            return emptyData;
        }
        
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "无法打开存档文件:" << filePath << "错误:" << file.errorString();
            emit loadError("无法打开存档文件: " + file.errorString());
            return emptyData;
        }
        
        QByteArray data = file.readAll();
        file.close();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error != QJsonParseError::NoError) {
            qDebug() << "存档文件格式错误:" << error.errorString();
            emit loadError("存档文件格式错误: " + error.errorString());
            return emptyData;
        }
        
        GameSaveData gameData = GameSaveData::fromJson(doc.object());
        qDebug() << "游戏已从存档加载:" << saveName << "碎片数量:" << gameData.pieces.size();
        emit loadCompleted(saveName);
        return gameData;
        
    } catch (const std::exception& e) {
        qDebug() << "加载游戏时发生错误:" << e.what();
        emit loadError("加载游戏时发生错误: " + QString(e.what()));
        return emptyData;
    }
}

QVector<QString> SaveSystem::getSaveList()
{
    QVector<QString> saveList;
    
    if (!ensureSaveDirectory()) {
        qDebug() << "无法创建保存目录:" << m_saveDirectory;
        return saveList;
    }
    
    QDir dir(m_saveDirectory);
    QStringList filters;
    filters << "*.json";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
    
    qDebug() << "找到存档文件数量:" << fileList.size();
    
    for (const QFileInfo& fileInfo : fileList) {
        QString fileName = fileInfo.baseName();
        saveList.append(fileName);
        qDebug() << "存档文件:" << fileName;
    }
    
    // 按修改时间排序（最新的在前）
    std::sort(saveList.begin(), saveList.end(), [this](const QString& a, const QString& b) {
        QFileInfo fileA(getSaveFilePath(a));
        QFileInfo fileB(getSaveFilePath(b));
        return fileA.lastModified() > fileB.lastModified();
    });
    
    return saveList;
}

bool SaveSystem::deleteSave(const QString& saveName)
{
    QString filePath = getSaveFilePath(saveName);
    QFile file(filePath);
    
    if (!file.exists()) {
        qDebug() << "存档文件不存在:" << filePath;
        return false;
    }
    
    bool success = file.remove();
    if (success) {
        qDebug() << "存档已删除:" << filePath;
    } else {
        qDebug() << "删除存档失败:" << filePath << "错误:" << file.errorString();
    }
    
    return success;
}

bool SaveSystem::saveExists(const QString& saveName)
{
    QString filePath = getSaveFilePath(saveName);
    QFile file(filePath);
    bool exists = file.exists();
    qDebug() << "检查存档是否存在:" << saveName << "路径:" << filePath << "存在:" << exists;
    return exists;
}

GameSaveData SaveSystem::getSaveInfo(const QString& saveName)
{
    GameSaveData data = loadGame(saveName);
    // 只返回基本信息，不包含碎片数据
    data.pieces.clear();
    data.mergedPieces.clear();
    qDebug() << "获取存档信息:" << saveName << "行数:" << data.rows << "列数:" << data.cols;
    return data;
}

QString SaveSystem::getSaveFilePath(const QString& saveName)
{
    QString filePath = m_saveDirectory + "/" + saveName + ".json";
    qDebug() << "获取存档文件路径:" << filePath;
    return filePath;
}

bool SaveSystem::ensureSaveDirectory()
{
    QDir dir;
    if (!dir.exists(m_saveDirectory)) {
        bool success = dir.mkpath(m_saveDirectory);
        if (success) {
            qDebug() << "创建保存目录成功:" << m_saveDirectory;
        } else {
            qDebug() << "创建保存目录失败:" << m_saveDirectory;
        }
        return success;
    }
    qDebug() << "保存目录已存在:" << m_saveDirectory;
    return true;
}

QString SaveSystem::generateDefaultSaveName()
{
    QDateTime now = QDateTime::currentDateTime();
    QString name = "Save_" + now.toString("yyyy-MM-dd_hh-mm-ss");
    qDebug() << "生成默认存档名称:" << name;
    return name;
}
