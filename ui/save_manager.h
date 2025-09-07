#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QDateTime>
#include "../core/save_system.h"

/*
 * 存档管理界面
 * 提供保存、加载、删除存档的功能
 */

class SaveManager : public QWidget
{
    Q_OBJECT

public:
    explicit SaveManager(QWidget *parent = nullptr);
    ~SaveManager();

    // 设置当前游戏数据
    void setCurrentGameData(const GameSaveData& gameData);
    
    // 获取选中的存档数据
    GameSaveData getSelectedSaveData();
    
    // 刷新存档列表
    void refreshSaveList();

signals:
    void saveRequested(const GameSaveData& gameData, const QString& saveName);
    void loadRequested(const QString& saveName);
    void closeRequested();

private slots:
    void onSaveButtonClicked();
    void onLoadButtonClicked();
    void onDeleteButtonClicked();
    void onSaveListSelectionChanged();
    void onSaveNameChanged();

private:
    void setupUI();
    void updateSaveInfo();
    void clearSaveInfo();
    
    SaveSystem* m_saveSystem;
    GameSaveData m_currentGameData;
    
    // UI 组件
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;
    QHBoxLayout* m_saveNameLayout;
    
    QListWidget* m_saveList;
    QLineEdit* m_saveNameEdit;
    QPushButton* m_saveButton;
    QPushButton* m_loadButton;
    QPushButton* m_deleteButton;
    QPushButton* m_closeButton;
    QPushButton* m_refreshButton;
    
    QLabel* m_saveInfoLabel;
    QLabel* m_saveNameLabel;
};

#endif // SAVE_MANAGER_H
