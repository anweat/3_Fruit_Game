#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>

/**
 * @brief 登录界面组件
 * 
 * 提供玩家登录功能
 */
class LoginWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     */
    LoginWidget(QWidget* parent = nullptr);
    
    /**
     * @brief 获取当前登录的玩家ID
     */
    QString getCurrentPlayerId() const { return currentPlayerId_; }
    
    /**
     * @brief 获取当前登录的玩家名称
     */
    QString getCurrentPlayerName() const { return currentPlayerName_; }
    
    /**
     * @brief 获取当前玩家分数
     */
    int getCurrentPlayerScore() const { return currentPlayerScore_; }
    
    /**
     * @brief 检查玩家是否已登录
     */
    bool isLoggedIn() const { return !currentPlayerId_.isEmpty() && currentPlayerId_ != "guest"; }
    
    /**
     * @brief 重置登录状态（返回登录界面）
     */
    void resetLoginState();

signals:
    /**
     * @brief 登录成功信号
     */
    void loginSucceeded(const QString& playerId, const QString& playerName);

private slots:
    /**
     * @brief 登录按钮点击处理
     */
    void onLoginClicked();
    
    /**
     * @brief 游客模式按钮点击处理
     */
    void onGuestClicked();

private:
    /**
     * @brief 初始化UI
     */
    void setupUi();
    
    QLineEdit* playerIdInput_;       // 玩家ID输入框
    QLineEdit* playerNameInput_;     // 玩家名称输入框（可选，新账户）
    QPushButton* loginButton_;       // 登录按钮
    QPushButton* guestButton_;       // 游客模式按钮
    QLabel* statusLabel_;            // 状态标签
    
    // 账户信息显示区
    QLabel* playerInfoLabel_;        // 玩家信息标签（登录后显示）
    QPushButton* logoutButton_;      // 登出按钮
    
    // 当前登录信息
    QString currentPlayerId_;
    QString currentPlayerName_;
    int currentPlayerScore_;
    
    /**
     * @brief 执行登录操作
     */
    bool performLogin(const QString& playerId, const QString& playerName = "");
    
    /**
     * @brief 更新玩家信息显示
     */
    void updatePlayerInfoDisplay();
    
    /**
     * @brief 显示登录界面
     */
    void showLoginInterface();
    
    /**
     * @brief 显示账户信息界面
     */
    void showAccountInterface();
};

#endif // LOGINWIDGET_H
