#include "save_manager.h"
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QDateTime>
#include <QListWidgetItem>

SaveManager::SaveManager(QWidget *parent)
    : QWidget(parent)
    , m_saveSystem(new SaveSystem(this))
{
    qDebug() << "SaveManager 构造函数开始";
    setupUI();
    refreshSaveList();
    hide(); // 初始时隐藏窗口
    
    // 连接信号
    connect(m_saveSystem, &SaveSystem::saveCompleted, this, [this](const QString& saveName) {
        QMessageBox::information(this, "保存成功", "游戏已保存为: " + saveName);
        refreshSaveList();
    });
    
    connect(m_saveSystem, &SaveSystem::saveError, this, [this](const QString& error) {
        QMessageBox::critical(this, "保存失败", error);
    });
    
    connect(m_saveSystem, &SaveSystem::loadError, this, [this](const QString& error) {
        QMessageBox::critical(this, "加载失败", error);
    });
    
    qDebug() << "SaveManager 构造函数完成";
}

SaveManager::~SaveManager()
{
}

void SaveManager::setupUI()
{
    qDebug() << "SaveManager setupUI 开始";
    setWindowTitle("存档管理");
    setFixedSize(600, 500);
    
    // 居中显示
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    qDebug() << "SaveManager 窗口位置设置为:" << x << y;
    
    m_mainLayout = new QVBoxLayout(this);
    qDebug() << "SaveManager 主布局创建完成";
    
    // 存档名称输入
    m_saveNameLayout = new QHBoxLayout();
    m_saveNameLabel = new QLabel("存档名称:");
    m_saveNameEdit = new QLineEdit();
    m_saveNameEdit->setPlaceholderText("输入存档名称...");
    m_saveNameLayout->addWidget(m_saveNameLabel);
    m_saveNameLayout->addWidget(m_saveNameEdit);
    m_mainLayout->addLayout(m_saveNameLayout);
    
    qDebug() << "SaveManager 存档名称输入创建完成";
    
    // 按钮布局
    m_buttonLayout = new QHBoxLayout();
    m_saveButton = new QPushButton("保存游戏");
    m_loadButton = new QPushButton("加载游戏");
    m_deleteButton = new QPushButton("删除存档");
    m_refreshButton = new QPushButton("刷新");
    m_closeButton = new QPushButton("关闭");
    
    m_buttonLayout->addWidget(m_saveButton);
    m_buttonLayout->addWidget(m_loadButton);
    m_buttonLayout->addWidget(m_deleteButton);
    m_buttonLayout->addWidget(m_refreshButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_closeButton);
    m_mainLayout->addLayout(m_buttonLayout);
    
    qDebug() << "SaveManager 按钮布局创建完成";
    
    // 存档列表
    m_saveList = new QListWidget();
    m_saveList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_mainLayout->addWidget(m_saveList);
    
    qDebug() << "SaveManager 存档列表创建完成";
    
    // 存档信息
    m_saveInfoLabel = new QLabel("选择存档查看详细信息");
    m_saveInfoLabel->setWordWrap(true);
    m_saveInfoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 10px; border: 1px solid #ccc; }");
    m_mainLayout->addWidget(m_saveInfoLabel);
    
    qDebug() << "SaveManager UI 组件创建完成";
    
    // 连接信号
    connect(m_saveButton, &QPushButton::clicked, this, &SaveManager::onSaveButtonClicked);
    connect(m_loadButton, &QPushButton::clicked, this, &SaveManager::onLoadButtonClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &SaveManager::onDeleteButtonClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &SaveManager::refreshSaveList);
    connect(m_closeButton, &QPushButton::clicked, this, &SaveManager::closeRequested);
    
    qDebug() << "SaveManager 按钮信号连接完成";
    
    connect(m_saveList, &QListWidget::itemSelectionChanged, this, &SaveManager::onSaveListSelectionChanged);
    connect(m_saveNameEdit, &QLineEdit::textChanged, this, &SaveManager::onSaveNameChanged);
    
    qDebug() << "SaveManager 列表和输入框信号连接完成";
    
    // 初始状态
    m_loadButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    
    qDebug() << "SaveManager setupUI 完成，按钮状态已设置";
}

void SaveManager::setCurrentGameData(const GameSaveData& gameData)
{
    m_currentGameData = gameData;
    qDebug() << "设置当前游戏数据，碎片数量:" << gameData.pieces.size() << "行数:" << gameData.rows << "列数:" << gameData.cols;
}

GameSaveData SaveManager::getSelectedSaveData()
{
    QListWidgetItem* currentItem = m_saveList->currentItem();
    if (currentItem) {
        QString displayText = currentItem->text();
        QString saveName = displayText.split(' ').first(); // 获取存档名称部分
        qDebug() << "获取选中的存档数据，存档名称:" << saveName;
        return m_saveSystem->loadGame(saveName);
    }
    return GameSaveData();
}

void SaveManager::onSaveButtonClicked()
{
    QString saveName = m_saveNameEdit->text().trimmed();
    if (saveName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入存档名称");
        return;
    }
    
    qDebug() << "保存按钮点击，存档名称:" << saveName;
    
    if (m_saveSystem->saveExists(saveName)) {
        int ret = QMessageBox::question(this, "确认覆盖", 
                                       "存档 \"" + saveName + "\" 已存在，是否覆盖？",
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    m_currentGameData.saveName = saveName;
    m_currentGameData.saveTime = QDateTime::currentDateTime();
    
    qDebug() << "发送保存请求，碎片数量:" << m_currentGameData.pieces.size();
    emit saveRequested(m_currentGameData, saveName);
}

void SaveManager::onLoadButtonClicked()
{
    QListWidgetItem* currentItem = m_saveList->currentItem();
    if (currentItem) {
        QString displayText = currentItem->text();
        QString saveName = displayText.split(' ').first(); // 获取存档名称部分
        qDebug() << "加载按钮点击，存档名称:" << saveName;
        emit loadRequested(saveName);
    }
}

void SaveManager::onDeleteButtonClicked()
{
    QListWidgetItem* currentItem = m_saveList->currentItem();
    if (currentItem) {
        QString displayText = currentItem->text();
        QString saveName = displayText.split(' ').first(); // 获取存档名称部分
        
        qDebug() << "删除按钮点击，存档名称:" << saveName;
        
        int ret = QMessageBox::question(this, "确认删除", 
                                       "确定要删除存档 \"" + saveName + "\" 吗？",
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            if (m_saveSystem->deleteSave(saveName)) {
                QMessageBox::information(this, "删除成功", "存档已删除");
                refreshSaveList();
            } else {
                QMessageBox::critical(this, "删除失败", "无法删除存档: " + saveName);
            }
        }
    }
}

void SaveManager::onSaveListSelectionChanged()
{
    bool hasSelection = m_saveList->currentItem() != nullptr;
    m_loadButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    
    qDebug() << "存档列表选择改变，有选择:" << hasSelection;
    
    if (hasSelection) {
        updateSaveInfo();
    } else {
        clearSaveInfo();
    }
}

void SaveManager::onSaveNameChanged()
{
    QString saveName = m_saveNameEdit->text().trimmed();
    bool exists = m_saveSystem->saveExists(saveName);
    
    qDebug() << "存档名称改变:" << saveName << "已存在:" << exists;
    
    if (exists) {
        m_saveButton->setText("覆盖保存");
        m_saveButton->setStyleSheet("QPushButton { background-color: #ffcc00; }");
    } else {
        m_saveButton->setText("保存游戏");
        m_saveButton->setStyleSheet("");
    }
}

void SaveManager::refreshSaveList()
{
    m_saveList->clear();
    
    QVector<QString> saveList = m_saveSystem->getSaveList();
    qDebug() << "刷新存档列表，找到存档数量:" << saveList.size();
    
    for (const QString& saveName : saveList) {
        GameSaveData info = m_saveSystem->getSaveInfo(saveName);
        QString displayText = QString("%1 (%2)")
                             .arg(saveName)
                             .arg(info.saveTime.toString("yyyy-MM-dd hh:mm"));
        m_saveList->addItem(displayText);
        qDebug() << "添加存档到列表:" << displayText;
    }
    
    if (saveList.isEmpty()) {
        m_saveList->addItem("暂无存档");
        qDebug() << "没有找到存档文件";
    }
}

void SaveManager::updateSaveInfo()
{
    QListWidgetItem* currentItem = m_saveList->currentItem();
    if (!currentItem) {
        clearSaveInfo();
        return;
    }
    
    QString displayText = currentItem->text();
    QString saveName = displayText.split(' ').first(); // 获取存档名称部分
    qDebug() << "更新存档信息，存档名称:" << saveName;
    
    GameSaveData info = m_saveSystem->getSaveInfo(saveName);
    
    QString infoText = QString(
        "<b>存档名称:</b> %1<br>"
        "<b>保存时间:</b> %2<br>"
        "<b>拼图尺寸:</b> %3 x %4 (%5 块)<br>"
        "<b>拼图形状:</b> %6<br>"
        "<b>游戏时间:</b> %7<br>"
        "<b>移动步数:</b> %8<br>"
        "<b>允许旋转:</b> %9"
    ).arg(info.saveName)
     .arg(info.saveTime.toString("yyyy-MM-dd hh:mm:ss"))
     .arg(info.rows)
     .arg(info.cols)
     .arg(info.numberOfPieces)
     .arg(static_cast<int>(info.typeOfPiece))
     .arg(QString("%1:%2:%3")
          .arg(info.gameTime / 3600, 2, 10, QChar('0'))
          .arg((info.gameTime % 3600) / 60, 2, 10, QChar('0'))
          .arg(info.gameTime % 60, 2, 10, QChar('0')))
     .arg(info.moveCount)
     .arg(info.rotationAllowed ? "是" : "否");
    
    m_saveInfoLabel->setText(infoText);
    qDebug() << "存档信息已更新";
}

void SaveManager::clearSaveInfo()
{
    m_saveInfoLabel->setText("选择存档查看详细信息");
    qDebug() << "清空存档信息显示";
}
