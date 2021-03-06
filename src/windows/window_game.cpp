﻿#include "src/windows/window_game.h"
#include "src/utils/util_generic_define.h"
#include "src/windows/window_stacked.h"
#include "src/windows/window_info.h"
#include "src/utils/util_pixel_font.h"
#include "src/controllers/controller_game.h"

#include <QIcon>
#include <QScreen>
#include <QGuiApplication>
#include <cstdio>

/**
 * \brief 游戏窗口构造函数
 * \param parent 父部件指针
 */
GameWindow::GameWindow(QWidget* parent) : QWidget(parent) {
    // 判断能否回到主界面
    can_return = false;
    // 工具类型（默认画笔）
    tool_type = GameWindow::TOOL_PAINT;

    // 设置窗口标题
    setWindowTitle(tr("Nonogram[*]"));

    // 设置窗口颜色
    GameController::setBackgroundColor(this, DARK_COLOR);

    // 创建游戏控件
    game_widget = new Nonogram(this);

    // 图片素材
    icon_tools = QPixmap(":/icon/icon-buttons");
    icon_paint = icon_tools.copy(0, 0, 96, 96);
    icon_erase = icon_tools.copy(96, 0, 96, 96);
    icon_cross = icon_tools.copy(192, 0, 96, 96);
    icon_paint_checked = icon_tools.copy(0, 96, 96, 96);
    icon_erase_checked = icon_tools.copy(96, 96, 96, 96);
    icon_cross_checked = icon_tools.copy(192, 96, 96, 96);

	// 光标素材
    cursor_tools = QPixmap(":/cursor/cursor-tools");
    cursor_paint = cursor_tools.copy(3,3,23,23);
    cursor_erase = cursor_tools.copy(33,3,23,23);
    cursor_cross = cursor_tools.copy(63,3,23,23);

    // 工具按钮
    btn_paint = new ToolButton(QIcon(icon_paint_checked), tr("填充"), this, true, false);
    connect(btn_paint, &QPushButton::clicked, this, &GameWindow::checkedPaint);
    game_widget->viewport()->setCursor(QCursor(cursor_paint, 1, 22));
    btn_paint->setChecked(true);

    btn_erase = new ToolButton(QIcon(icon_erase), tr("清除"), this, true, false);
    connect(btn_erase, &QPushButton::clicked, this, &GameWindow::checkedErase);

    btn_cross = new ToolButton(QIcon(icon_cross), tr("标记空白"), this, true, false);
    connect(btn_cross, &QPushButton::clicked, this, &GameWindow::checkedCross);

    btn_undo = new ToolButton(QIcon(icon_tools.copy(288, 0, 96, 96)), tr("撤销"), this);
    connect(btn_undo, &QPushButton::clicked, game_widget, &Nonogram::undo);
    btn_undo->setDisabled(true);

    btn_redo = new ToolButton(QIcon(icon_tools.copy(288, 96, 96, 96)), tr("重做"), this);
    connect(btn_redo, &QPushButton::clicked, game_widget, &Nonogram::redo);
    btn_redo->setDisabled(true);

    btn_reset = new ToolButton(QIcon(icon_tools.copy(384, 0, 96, 96)), tr("重置"), this);
    connect(btn_reset, &QPushButton::clicked, game_widget, &Nonogram::resetGame);
    btn_reset->setDisabled(true);

    // 游戏标题
    label_game_title = new QLabel(tr(R"(<font color="#cccccc">Nono</font><font color="#FF7800">gram</font>)"), this);
    label_game_title->setFont(PixelFont("Berlin Sans FB", 30, 50));

    // 菜单按钮
    btn_help = new MenuButton(tr("帮助"), this, false);
    btn_help->setFixedWidth(NAV_BUTTON_WIDTH);
    connect(btn_help, &QPushButton::clicked, this, &GameWindow::showHelp);

    btn_back = new MenuButton(tr("返回"), this, false);
    btn_back->setFixedWidth(NAV_BUTTON_WIDTH);
    connect(btn_back, &QPushButton::clicked, this, &GameWindow::showMain);

    // 工具按钮布局
    layout_btn_tools = new QHBoxLayout;
    layout_btn_tools->addStretch();
    layout_btn_tools->addWidget(btn_paint);
    layout_btn_tools->addWidget(btn_erase);
    layout_btn_tools->addWidget(btn_cross);
    layout_btn_tools->addWidget(btn_undo);
    layout_btn_tools->addWidget(btn_redo);
    layout_btn_tools->addWidget(btn_reset);
    layout_btn_tools->addStretch();
    layout_btn_tools->setSpacing(SPACING_SMALL / 2);

    // 菜单按钮布局
    layout_btn_menu = new QHBoxLayout;
    layout_btn_menu->addStretch();
    layout_btn_menu->addWidget(label_game_title, 0, Qt::AlignCenter);
    label_place_holder = new QLabel(this);
    label_place_holder->setFixedWidth(10);
    layout_btn_menu->addWidget(label_place_holder, 0, Qt::AlignCenter);
    layout_btn_menu->addWidget(btn_help, 0, Qt::AlignCenter);
    layout_btn_menu->addWidget(btn_back, 0, Qt::AlignCenter);
    layout_btn_menu->addStretch();
    layout_btn_menu->setSpacing(SPACING_SMALL);

    // 创建窗口布局
    layout_this = new QVBoxLayout;
    setLayout(layout_this);
    layout_this->addWidget(game_widget, 0, Qt::AlignCenter);
    layout_this->addLayout(layout_btn_tools);
    layout_this->addStretch();
    layout_this->addLayout(layout_btn_menu);
    layout_this->setContentsMargins(30, 30, 30, 30);
    layout_this->setSpacing(SPACING_LARGE);
}

/**
 * \brief 回到主界面
 */
void GameWindow::showMain() {
    close();
    if (can_return) {
        GameController::stacked_window->setIndex(StackedWindow::MAIN_WINDOW_INDEX);
        GameController::stacked_window->checkBtnLoad();
        GameController::stacked_window->show();
        GameController::stacked_window->activateWindow();
    }
}

/**
 * \brief 显示帮助界面
 */
void GameWindow::showHelp() {
    InfoWindow info(tr("<font color=\"#FF7800\">Nonogram</font> 是一种逻辑游戏，玩家在游戏中以猜谜的方式画图。<br><br>"
                       "在一个网格中，每行每列的开头都有一组数。玩家需要根据它们来填充格子，<br>"
                       "最后可以得出一幅图画，谓之网格画。<br><br>"
                       "例如“4 8 1 1”是指该行（或该列）上，按照数字顺序，有四条<font color=\"#FF7800\">连续的</font>由格子连成的线，<br>"
                       "四条线分别占了4格、8格、1格和1格。每条线之间最少要有一个空格。<br>"
                       "即“████ ████████ █ █”。<font color=\"#FF7800\">线条间可以间隔多个空格</font>，但<font color=\"#FF7800\">线条的顺序不能变</font>。<br><br>"
                       "玩家应尽量用“╳”号标记一定不需要填充的格子，以辅助判断。"),
                       1,
                       this);
    info.exec();
}

/**
 * \brief 点击画笔按钮
 */
void GameWindow::checkedPaint() {
    game_widget->viewport()->setCursor(QCursor(cursor_paint, 1, 22));
    btn_paint->setIcon(QIcon(icon_paint_checked));
    tool_type = GameWindow::TOOL_PAINT;
    btn_erase->setChecked(false);
    btn_erase->setIcon(QIcon(icon_erase));
    btn_cross->setChecked(false);
    btn_cross->setIcon(QIcon(icon_cross));
}

/**
 * \brief 点击橡皮擦按钮
 */
void GameWindow::checkedErase() {
    game_widget->viewport()->setCursor(QCursor(cursor_erase));
    btn_erase->setIcon(QIcon(icon_erase_checked));
    tool_type = GameWindow::TOOL_ERASE;
    btn_paint->setChecked(false);
    btn_paint->setIcon(QIcon(icon_paint));
    btn_cross->setChecked(false);
    btn_cross->setIcon(QIcon(icon_cross));
}

/**
 * \brief 点击叉号按钮
 */
void GameWindow::checkedCross() {
    game_widget->viewport()->setCursor(QCursor(cursor_cross));
    btn_cross->setIcon(QIcon(icon_cross_checked));
    tool_type = GameWindow::TOOL_CROSS;
    btn_paint->setChecked(false);
    btn_paint->setIcon(QIcon(icon_paint));
    btn_erase->setChecked(false);
    btn_erase->setIcon(QIcon(icon_erase));
}

/**
 * \brief 游戏结束后屏蔽鼠标输入
 * \param obj
 * \param event
 * \return
 */
bool GameWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == game_widget->viewport()) {
        return event->type() == QEvent::MouseButtonPress 
            || event->type() == QEvent::MouseButtonRelease 
            || event->type() == QEvent::MouseMove 
            || event->type() == QEvent::MouseButtonDblClick;
    } 
    return QWidget::eventFilter(obj, event);
}

/**
 * \brief 询问是否要保存游戏
 * \param event 窗口关闭事件
 */
void GameWindow::closeEvent(QCloseEvent *event) {
    if (isWindowModified() && !game_widget->isComplete()) {
        InfoWindow info(tr("是否保存游戏进度？"), 3, this);
        const auto response = info.exec();
        if (response == QDialog::Accepted) {
            game_widget->save();
            event->accept();
            can_return = true;
        } else if (response == 3) {
            event->ignore();
            can_return = false;
        } else {
            event->accept();
            can_return = true;
        }
    } else {
        if (game_widget->isComplete()) {
            remove(SAVE_NAME);
        }
        event->accept();
        can_return = true;
    }
}

/**
 * \brief 游戏开始前智能调整窗口大小和位置
 * \param event 窗口显示事件
 */
void GameWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    resize(game_widget->getTableSize().width() + 60, game_widget->getTableSize().height() + SPACING_LARGE * 2 + 60 + BUTTON_HEIGHT + 10 + BUTTON_HEIGHT);
    auto desktop_geometry = QGuiApplication::screens()[0]->availableGeometry();
    PVector pos(
        GameController::stacked_window->pos().x() + (GameController::stacked_window->frameGeometry().width() - frameGeometry().width()) / 2,
        GameController::stacked_window->pos().y() + (GameController::stacked_window->frameGeometry().height() - frameGeometry().height()) / 2
    );
    if (pos.x() < 8) {
        pos.x() = 8;
    } else if (pos.x() + frameGeometry().width() > desktop_geometry.width()) {
        pos.x() = desktop_geometry.width() - frameGeometry().width() - 8;
    }
    if (pos.y() < 16) {
        pos.y() = 16;
    } else if (pos.y() + frameGeometry().height() > desktop_geometry.height()) {
        pos.y() = desktop_geometry.height() - frameGeometry().height() - 8;
    }
    move(pos.x(), pos.y());
}

/**
 * \brief 游戏结束后调整界面显示
 * \param title 网格画标题
 */
void GameWindow::showComplete(const QString& title) {
    btn_help->setVisible(false);
    btn_paint->setVisible(false);
    btn_erase->setVisible(false);
    btn_cross->setVisible(false);
    btn_undo->setVisible(false);
    btn_redo->setVisible(false);
    btn_reset->setVisible(false);
    label_place_holder->setVisible(false);
    layout_btn_menu->removeWidget(label_game_title);
    layout_btn_menu->removeWidget(btn_back);
    layout_this->addWidget(label_game_title, 0, Qt::AlignCenter);
    layout_this->addWidget(btn_back, 0, Qt::AlignCenter);

    InfoWindow info(tr(R"(恭喜您，游戏<font color="#ff7800"> “%1” </font>已完成！)").arg(title), 1, this);
    info.exec();
}

int GameWindow::getToolType() const {
    return tool_type;
}

/**
 * \brief 析构函数
 */
GameWindow::~GameWindow() {
    delete layout_btn_tools;
    layout_btn_tools = nullptr;
    delete layout_btn_menu;
    layout_btn_menu = nullptr;
    delete layout_this;
    layout_this = nullptr;
}
