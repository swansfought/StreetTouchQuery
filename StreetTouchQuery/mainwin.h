#ifndef MAINWIN_H
#define MAINWIN_H

#include <QWidget>
#include <QApplication>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QTextBlockFormat>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QRegExpValidator>
#include <QMessageBox>
#include <QClipboard>
#include <QScrollBar>
#include <QDateTime>
#include <QTextCursor>
#include <QTextBlock>
#include <QFileDialog>
#include <QScreen>
#include <QClipboard>
#include <QCheckBox>
#include <QMovie>
#include <QColorDialog>
#include <QListWidget>
#include <QTimer>
#include <QBuffer>

#include <QDebug>

#include "db/database.h"
#include "dev/config.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWin; }
QT_END_NAMESPACE


#define BUTTON_TOOL_SIZE 70
#define BUTTON_TOOL_ALIGN_RIGHT 140
#define BUTTOM_TOOL_ALIGN_SPACE 30
#define BUTTOM_TOOL_ALIGN_VERTICAL 100

#define CONTEXT_SHOW_ALIGN_LEFT 215
#define CONTEXT_SHOW_ALIGN_RIGHT 180
#define CONTEXT_SHOW_ALIGN_TOP 150
#define CONTEXT_SHOW_ALIGN_BOTTOM 60
#define CONTEXT_LINE_SPACE 22

#define BUTTON_MANAGE_SIZE 40
#define BUTTON_MANAGE_ALIGN_RIGHT 80
#define BUTTON_MANAGE_ALIGN_BOTTOM 50

#define BUTTON_WORK_WIDTH 250
#define BUTTON_WORK_HEIGHT 40
#define BUTTON_WORK_ALIGN_TOP 265
#define BUTTON_WORK_ALIGN_RIGHT 40
#define BUTTON_WORK_ALIGN_VERTICAL 40

#define PAGE_SIZE 300

#define TIMER_MSEC 100
#define MOUSE_HIDE_FLAG 4000/TIMER_MSEC //固定4s自动隐藏

class MainWin : public QWidget
{
    Q_OBJECT

public:
    MainWin(QWidget *parent = nullptr);
    ~MainWin();

    QString getSysName() const;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent * event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    // 后台文本编辑
    void on_cBox_editMode_currentTextChanged(const QString &mode);//编辑模式
    void on_btn_save_clicked();//保存
    void on_btn_clear_clicked();//清除文本或图片
    void on_btn_restore_clicked(); //恢复
    void on_btn_selectImageShow_clicked(); //选择图片
    void on_btn_selectAll_clicked(); //全选
    void on_btn_cut_clicked(); //剪切
    void on_btn_copy_clicked(); //复制
    void on_btn_paste_clicked(); //粘贴
    void on_btn_undo_clicked(); //撤销
    void on_btn_redo_clicked(); //恢复
    void on_btn_open_clicked();//打开

    void on_spinBox_fontSize_valueChanged(int size);//字体大小
    void on_fontComboBox_currentFontChanged(const QFont &font);//字体选择
    void on_btn_currLine_clicked();//行距-设置当前行
    void on_btn_allLine_clicked();//行距-设置全部行
    void on_btn_fontLeft_clicked();//居左
    void on_btn_fontCenter_clicked();//居中
    void on_btn_fontRight_clicked();//居右
    void on_btn_fontJustify_clicked();//两端对齐
    void on_btn_fontBold_clicked(); //加粗
    void on_btn_fontItalic_clicked();//斜体
    void on_btn_fontUnder_clicked();//下划线
    void on_btn_fontColor_clicked();//字体颜色
    void on_btn_exitEdit_clicked(); //退出编辑
    void on_tree_title_itemClicked(QTreeWidgetItem *item, int column);//当前项改变
    void on_context_edit_selectionChanged();//当前选择的文字发生变化,更新粗体、斜体、下划线

    void on_cBox_lineSpace_currentIndexChanged(int index); //恢复焦点



private:
    enum PageType{
        //页面类型
        HomePage=0, //主页
        RoadPage,//街道
        WorkPage, //工作
        SocietyPage,//社会
        MedicalPage,//医疗
        EducationPage,//教育
        LawPage,//法律
        SubWorkPage,//子工作
        SubSocietyPage,//子社会

        //页面类型-用于按钮显隐
        ImagePage,
        ShowPage,
        SubShowPage
    };
    enum EditState{
        Loading,
        Editing,
        Saving,
        Saved,
    };
    enum RollType{
        Roll,
        Pause,
        Continue,
        Start,
        Stop
    };

    void loadStyleSheet(); //加载样式表
    void loadTreeTitle();//加载标题树
    void loadCfgInfo();//加载配置信息
    void loadLocalTime();
    void loadContextInfo(const EditState &state);//加载内容信息展示
    void loadContextShow(const Record &record);//加载内容
    void loadImageShow(const Record& record);//加载图片
    void toolButtonVisible(const PageType &pageType);//按钮可视操作
    void toolButtonHandle(const int &buttonId);//按钮响应操作
    void updateRoll(const RollType& rollType);//轮播
    void loginInfoClear();//清除登录信息

    Config *config;
    DataBase *db;
    PageType pageType;

    QPushButton *btnPause;
    QPushButton *btnPreviousPage;
    QPushButton *btnNextPage;
    QPushButton *btnSubReturn;
    QPushButton *btnReturnHome;

    QPushButton *btnWorkRule;//21
    QPushButton *btnWordChart;//22

    QPushButton *btnManage;
    QPushButton *btnClose;

    QLabel *labelTime;

    QTreeWidgetItem *currTitleItem;
    EditState editState; //编辑状态
    QClipboard *clipboard;

    QTimer *timer;
    QTimer *updateTimer;
    QScrollBar *scrollbar;//滚动条
    bool allowRoll; //主页下不可启动定时器
    int sliderPos; //文本轮播位置
    bool isPause;//暂停标志

    bool isLogin; //成功登录标志，用于加载配置避免槽函数被触发
    bool isImage;
    int hideFlag;

private:
    Ui::MainWin *ui;
};
#endif // MAINWIN_H
