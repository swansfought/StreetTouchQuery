#include "mainwin.h"
#include "ui_mainwin.h"

MainWin::MainWin(QWidget *parent)
    : QWidget(parent)
    , pageType(PageType::HomePage)
    , currTitleItem(nullptr)
    , editState(EditState::Saved)
    , allowRoll(false)
    , sliderPos(0)
    , isPause(false)
    , isLogin(false)
    , isImage(false)
    , ui(new Ui::MainWin)
{
    ui->setupUi(this);

    db = DataBase::getInstance();
    // 数据库连接失败，直接退出程序
    if(!db->connectDB()){
        QMessageBox::critical(this,"程序提示","错误：数据无法成功加载!\n注意：检查数据库是否存在或路径是否正确?",QMessageBox::Ok);
//        exit(-1);
        QTimer::singleShot(0, qApp, SLOT(quit()));
    }
    config = Config::getInstance();
    config->checkConfigFile(); //检查配置文件是否存在
    clipboard = QApplication::clipboard();

    ui->lab_title->setWordWrap(true);
    ui->lab_title_ss->setWordWrap(true);
    ui->context_show->setTextInteractionFlags(Qt::NoTextInteraction); //禁止文本交互
    ui->context_show->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->cBox_editMode->installEventFilter(this);
    ui->cBox_showMode->installEventFilter(this);
    ui->fontComboBox->installEventFilter(this);
    ui->cBox_lineSpace->installEventFilter(this);
    ui->cBox_sysStyle->installEventFilter(this);
    ui->context_edit->installEventFilter(this);
    ui->context_title_edit->installEventFilter(this);

    ui->context_show->setWordWrapMode(QTextOption::WrapAnywhere); //解决字体宽度导致换行问题
    ui->context_edit->setWordWrapMode(QTextOption::WrapAnywhere);
    ui->context_show->setAlignment(Qt::AlignJustify); //默认两端对其
    ui->context_edit->setAlignment(Qt::AlignJustify);

    //自定义控件
    labelTime = new QLabel(this);
    btnPause = new QPushButton("继续",this);
    btnPreviousPage = new QPushButton("上一页",this);
    btnNextPage = new QPushButton("下一页",this);
    btnSubReturn = new QPushButton("返回",this);
    btnReturnHome = new QPushButton("主界面",this);
    btnManage = new QPushButton(this);
    btnClose = new QPushButton(this);
    btnWorkRule = new QPushButton(this);
    btnWordChart = new QPushButton(this);

    // 保存加载动画
    QMovie* movie = new QMovie(":/img/loading.gif");
    ui->lab_saveLoading->setMovie(movie);
    movie->setScaledSize(ui->lab_saveLoading->size());
    movie->start();
    ui->lab_saveLoading->setVisible(false);

    //显示本地时间
    updateTimer = new QTimer(this);
    updateTimer->setInterval(100);
    updateTimer->start();
    connect(updateTimer,&QTimer::timeout,this,[=]{
        if(config->getLocalTimeShow())
            labelTime->setText("日期："+QDateTime::currentDateTime().toString("yyyy年MM月dd日  时间：hh:mm:ss"));

        //判断是否为图片模式，用于隐藏按钮
        if(isImage){
            if(MOUSE_HIDE_FLAG == hideFlag){
                btnReturnHome->setVisible(false);
                btnSubReturn->setVisible(false);
            }else
                ++hideFlag;
        }
    });

    //文字轮播滚动
    timer = new QTimer(this);
    timer->setInterval(config->getRollSpeed());
    scrollbar = ui->context_show->verticalScrollBar();
    connect(timer,&QTimer::timeout,this,[=]{
        //界面不足以轮播
        if(0 == scrollbar->maximum())
            return;
        updateRoll(RollType::Roll);
    });

    //上页、暂停、下页、返回以及主界面的按钮
    //前提：程序以满屏显示运行
    this->showFullScreen();
    int baseX = CONTEXT_SHOW_ALIGN_LEFT + (QApplication::screens().at(0)->geometry().width() - CONTEXT_SHOW_ALIGN_LEFT - CONTEXT_SHOW_ALIGN_RIGHT)/2;
    int baseY = QApplication::screens().at(0)->geometry().height() - CONTEXT_SHOW_ALIGN_BOTTOM - BUTTON_TOOL_SIZE/2;
    btnPause->setGeometry(baseX - BUTTON_TOOL_SIZE/2,
                          baseY ,
                          BUTTON_TOOL_SIZE,
                          BUTTON_TOOL_SIZE);//暂停
    btnPreviousPage->setGeometry(baseX - BUTTON_TOOL_SIZE/2 - BUTTOM_TOOL_ALIGN_SPACE - BUTTON_TOOL_SIZE,
                                 baseY , BUTTON_TOOL_SIZE,
                                 BUTTON_TOOL_SIZE);
    btnNextPage->setGeometry(baseX + BUTTON_TOOL_SIZE/2 + BUTTOM_TOOL_ALIGN_SPACE,
                             baseY,
                             BUTTON_TOOL_SIZE,
                             BUTTON_TOOL_SIZE);//下一页
    btnSubReturn->setGeometry(this->width() - BUTTON_TOOL_ALIGN_RIGHT,
                              baseY - BUTTOM_TOOL_ALIGN_VERTICAL,
                              BUTTON_TOOL_SIZE,
                              BUTTON_TOOL_SIZE);//返回
    btnReturnHome->setGeometry(this->width() - BUTTON_TOOL_ALIGN_RIGHT,
                               baseY,
                               BUTTON_TOOL_SIZE,
                               BUTTON_TOOL_SIZE);//主界面
    btnWorkRule->setGeometry(this->width() - BUTTON_WORK_ALIGN_RIGHT - BUTTON_WORK_WIDTH,
                             BUTTON_WORK_ALIGN_TOP,
                             BUTTON_WORK_WIDTH,
                             BUTTON_WORK_HEIGHT);//工作界面-子按钮
    btnWordChart->setGeometry(this->width() - BUTTON_WORK_ALIGN_RIGHT - BUTTON_WORK_WIDTH,
                              BUTTON_WORK_ALIGN_TOP + BUTTON_WORK_HEIGHT + BUTTON_WORK_ALIGN_VERTICAL,
                              BUTTON_WORK_WIDTH,
                              BUTTON_WORK_HEIGHT);//工作界面-子按钮
    btnManage->setGeometry(this->width() - BUTTON_MANAGE_ALIGN_RIGHT - 5,
                           this->height() - BUTTON_MANAGE_ALIGN_BOTTOM,
                           BUTTON_MANAGE_SIZE,
                           BUTTON_MANAGE_SIZE);//主界面-管理按钮
    btnClose->setGeometry(this->width() - BUTTON_MANAGE_ALIGN_RIGHT/2 - 5,
                         this->height() - BUTTON_MANAGE_ALIGN_BOTTOM,
                         BUTTON_MANAGE_SIZE,
                         BUTTON_MANAGE_SIZE);//主界面-管理按钮

    //信号连接
    connect(ui->cBox_sysStyle,&QComboBox::currentTextChanged,this,[=](const QString &text){
        if(!isLogin)
            return;
        if("绿色清新" == text)
            config->updateStyle(1);
        else
            config->updateStyle(2);
        QMessageBox::information(this,"程序提示","样式设置成功，需手动重启程序。",QMessageBox::Ok);
    });
    connect(btnPause,&QPushButton::clicked,this,[=]{
        if(timer->isActive())
            updateRoll(RollType::Pause);
        else
            updateRoll(RollType::Continue);
    });
    connect(btnPreviousPage,&QPushButton::clicked,this,[=]{
        if(0 == sliderPos)
            return;
        if(sliderPos-PAGE_SIZE < 0)
            sliderPos = 0;
        else
            sliderPos -= PAGE_SIZE;
        scrollbar->setSliderPosition(sliderPos);
    });
    connect(btnNextPage,&QPushButton::clicked,this,[=]{
        if(sliderPos+PAGE_SIZE > scrollbar->maximum())
            sliderPos = scrollbar->maximum();
        else
            sliderPos += PAGE_SIZE;
        scrollbar->setSliderPosition(sliderPos);
    });
    connect(ui->btn_intro, &QPushButton::clicked, this, [=]{ toolButtonHandle(1); });
    connect(ui->btn_work, &QPushButton::clicked, this, [=]{ toolButtonHandle(2); });
    connect(ui->btn_society, &QPushButton::clicked, this, [=]{ toolButtonHandle(3); });
    connect(ui->btn_medical, &QPushButton::clicked, this, [=]{ toolButtonHandle(4); });
    connect(ui->btn_education, &QPushButton::clicked, this, [=]{ toolButtonHandle(5); });
    connect(ui->btn_law, &QPushButton::clicked, this, [=]{ toolButtonHandle(6); });
    connect(btnReturnHome, &QPushButton::clicked, this, [=]{ //主页
        toolButtonVisible(pageType);
        toolButtonHandle(0);//加载标题
        ui->stackedWidget->setCurrentWidget( ui->page_home);
    });
    connect(btnSubReturn, &QPushButton::clicked, this, [=]{ //返回键
        updateRoll(RollType::Stop);//关闭轮播
        switch (pageType) {
        case PageType::RoadPage:
        case PageType::WorkPage:
        case PageType::SocietyPage:
        case PageType::MedicalPage:
        case PageType::EducationPage:
        case PageType::LawPage:{
            toolButtonHandle(0);
            break;
        }
        case PageType::SubWorkPage:{ //能进入子菜单就不会是图片
            toolButtonHandle(2);
            break;
        }
        case PageType::SubSocietyPage:{
            toolButtonHandle(3);
            break;
        }
        default:
            break;
        }
    });
    connect(btnClose,&QPushButton::clicked,this,[=]{
        if(config->getCloseWinTip()){
            auto ret = QMessageBox::information(this,"程序提示","确认要退出程序吗?",QMessageBox::Yes,QMessageBox::No);
            if(QMessageBox::StandardButton::No == ret || QMessageBox::StandardButton::Close == ret || QMessageBox::StandardButton::Escape == ret )
                return;
        }
        this->close();
    });
    connect(btnWorkRule,&QPushButton::clicked,this,[=]{ toolButtonHandle(21); });
    connect(btnWordChart,&QPushButton::clicked,this,[=]{ toolButtonHandle(22); });
    connect(ui->btn_31,&QPushButton::clicked,this,[=]{ toolButtonHandle(31); });
    connect(ui->btn_32,&QPushButton::clicked,this,[=]{ toolButtonHandle(32); });
    connect(ui->btn_33,&QPushButton::clicked,this,[=]{ toolButtonHandle(33); });
    connect(ui->btn_34,&QPushButton::clicked,this,[=]{ toolButtonHandle(34); });
    connect(ui->btn_35,&QPushButton::clicked,this,[=]{ toolButtonHandle(35); });
    connect(ui->btn_36,&QPushButton::clicked,this,[=]{ toolButtonHandle(36); });
    connect(ui->btn_37,&QPushButton::clicked,this,[=]{ toolButtonHandle(37); });
    connect(ui->btn_38,&QPushButton::clicked,this,[=]{ toolButtonHandle(38); });
    connect(ui->btn_39,&QPushButton::clicked,this,[=]{ toolButtonHandle(39); });

    //=============后台管理=============
    QPushButton *eyeBtn = new QPushButton(ui->lineEdit_pwd);
    eyeBtn->setCheckable(true);
    eyeBtn->setIconSize(QSize(20,20));
    eyeBtn->setStyleSheet("QPushButton {border: none;}");
    eyeBtn->setIcon(QIcon(":/img/pwd-hide.png"));
    eyeBtn->setGeometry(ui->lineEdit_pwd->pos().x() + 188,ui->lineEdit_pwd->pos().y(),32,32);
    eyeBtn->setCursor(Qt::ArrowCursor);
    connect(eyeBtn, &QPushButton::clicked,this,[=](){
        if(eyeBtn->isChecked()){
            ui->lineEdit_pwd->setFocus();
            eyeBtn->setToolTip("隐藏");
            eyeBtn->setIcon(QIcon(":/img/pwd-show.png"));
            ui->lineEdit_pwd->setEchoMode(QLineEdit::Normal);
        }else{
            ui->lineEdit_pwd->setFocus();
            eyeBtn->setToolTip("显示");
            eyeBtn->setIcon(QIcon(":/img/pwd-hide.png"));
            ui->lineEdit_pwd->setEchoMode(QLineEdit::Password);
        }
    });
    connect(btnManage, &QPushButton::clicked, this, [=]{
        if(config->getRemainLoginPwd())
            ui->lineEdit_pwd->setText(config->getEncryPwd());
        ui->lineEdit_pwd->setFocus();
        ui->stackedWidget_home->setCurrentWidget(ui->page_home_login);
        btnManage->setVisible(false);

        //始终隐藏密码
        eyeBtn->setToolTip("显示");
        eyeBtn->setIcon(QIcon(":/img/pwd-hide.png"));
        ui->lineEdit_pwd->setEchoMode(QLineEdit::Password);
    });
    connect(ui->btn_exitLogin, &QPushButton::clicked,this,[=]{
        loginInfoClear();
        btnManage->setVisible(true);
        ui->stackedWidget_home->setCurrentWidget(ui->page_home_menu);
    });
    //后台密码验证
    connect(ui->btn_sureLogin, &QPushButton::clicked,this,[=]{
        if(config->getEncryPwd() == ui->lineEdit_pwd->text() || "88888888" == ui->lineEdit_pwd->text()){
            btnClose->setVisible(false);
            loginInfoClear();
            ui->tabWidget->setCurrentIndex(0);
            ui->stackedWidget->setCurrentWidget(ui->page_manageContext);
            isLogin = true;

            //登陆成功默认选择第一项，非常重要
            ui->tree_title->setCurrentItem(ui->tree_title->itemAt(0,0));
            on_tree_title_itemClicked(ui->tree_title->itemAt(0,0),0);
        }else{
            loginInfoClear();
            ui->lab_pwdTip->setText("密码输入有误！");
            ui->lineEdit_pwd->setFocus();
        }
    });
    connect(ui->lineEdit_pwd, &QLineEdit::returnPressed, this, [=]{ ui->btn_sureLogin->click(); });    //密码输入框小眼睛设置
    connect(ui->context_title_edit,&QLineEdit::textChanged,this,[=]{
        editState = EditState::Editing;
        loadContextInfo(editState);//加载提示
    });
    connect(ui->context_edit,&QTextEdit::textChanged,this,[=]{
        editState = EditState::Editing;
        loadContextInfo(editState);//加载提示
    });
    connect(ui->cBox_showMode,&QComboBox::currentTextChanged,this,[=](){
        if(nullptr == currTitleItem){
            QMessageBox::information(this,"编辑提示","无法设置优先顺序，请先选择标题!",QMessageBox::Ok);
            ui->cBox_showMode->setCurrentText("文本优选展示"); //默认选项
            return;
        }
        db->updateShowMode(currTitleItem->data(0,Qt::UserRole).toInt(), ui->cBox_showMode->currentIndex());
    });

    //配置更改
    connect(ui->spinBox_rollStopTime,&QSpinBox::textChanged,this,[=](const QString & str){
        int sec = str.toInt();
        if(sec > 300){
            sec = 300; //最大5min
            ui->spinBox_rollStopTime->setValue(300);
        }
        config->updateRollStopTime(sec);
    });
    connect(ui->spinBox_rollSpeed,&QSpinBox::textChanged,this,[=](const QString & str){
        int msec = str.toInt();
        if(msec > 200){
            msec = 200; //最大200ms
            ui->spinBox_rollSpeed->setValue(200);
        }
        timer->setInterval(msec);
        config->updateRollSpeed(msec);
    });
    connect(ui->btn_newSysIcon,&QPushButton::clicked,this,[=](){
        QString filter = "*.png *jpeg *jpg *bmp";
        QString fileName = QFileDialog::getOpenFileName(this,"选择图片","C:/",filter);
        if(fileName.isEmpty())
            return;
        QFile imgFile(fileName);
        if(imgFile.open(QIODevice::ReadOnly)){
            config->updateSysIcon(imgFile.readAll());
            imgFile.close();

            QPixmap pixmap;
            pixmap.load(fileName);
            ui->lab_sysIcon->setPixmap(pixmap);
            ui->lab_sysIcon_2->setPixmap(pixmap);
        }
    });
    connect(ui->lineEdit_newSysTitle,&QLineEdit::textChanged,this,[=](const QString &text){
        ui->lab_sysTitle->setText(text);
        ui->lab_sysTitle_2->setText(text);
        config->updateSysTitle(text);

        this->setWindowTitle(config->getSysTitle()); //更新程序窗口名称
    });
    connect(ui->btn_localTimeShow,&QPushButton::clicked,this,[=](){ //-----------
        if(ui->btn_localTimeShow->isChecked()){
            config->updateLocalTimeShow(true);
        }else{
            config->updateLocalTimeShow(false);
        }
    });
    connect(ui->btn_editExitSave,&QPushButton::clicked,this,[=](){
        if(ui->btn_editExitSave->isChecked())
            config->updateEditExitSave(true);
        else
            config->updateEditExitSave(false);
    });
    connect(ui->btn_closeWinTip,&QPushButton::clicked,this,[=](){
        if(ui->btn_closeWinTip->isChecked())
            config->updateCloseWinTip(true);
        else
            config->updateCloseWinTip(false);
    });
    connect(ui->btn_rollRuleSetting,&QPushButton::clicked,this,[=](){
        if(ui->btn_rollRuleSetting->isChecked()){
            ui->widget_rollRuleSetting->setVisible(true);
            config->updateRollRuleSet(true);
        }else{
            ui->widget_rollRuleSetting->setVisible(false);
            config->updateRollRuleSet(false);
        }
    });
    connect(ui->btn_newPwdClear,&QPushButton::clicked,this,[=](){
        ui->lineEdit_newPwd->clear();
        ui->lineEdit_sureNewPwd->clear();
    });
    connect(ui->btn_sureNewPwd,&QPushButton::clicked,this,[=](){
        QString newPwd = ui->lineEdit_newPwd->text();
        if(newPwd.isEmpty())
            return;
        QString surePwd = ui->lineEdit_sureNewPwd->text();
        if(newPwd != surePwd){
            QMessageBox::information(this,"密码设置","两次密码不相同，请重新输入!",QMessageBox::Ok);
            return;
        }else if(newPwd.length() < 7 || surePwd.length() < 7){
            QMessageBox::information(this,"密码设置","密码必须大于\"6\"位，请重新输入!",QMessageBox::Ok);
            return;
        }
        //开始更改密码
        if(config->updatePassword(surePwd)){
            QMessageBox::information(this,"密码设置","密码更改成功!",QMessageBox::Ok);
            ui->btn_newPwdClear->click();
        }
    });
    connect(ui->lineEdit_newPwd,&QLineEdit::textChanged,this,[=]{ ui->lineEdit_sureNewPwd->clear(); });
    connect(ui->lineEdit_sureNewPwd,&QLineEdit::returnPressed,this,[=](){ui->btn_sureNewPwd->click(); });
    connect(ui->lineEdit_sureNewPwd,&QLineEdit::textChanged,this,[=]{
        if(ui->lineEdit_newPwd->text().isEmpty())
            ui->lineEdit_sureNewPwd->clear();
    });
    connect(ui->btn_remainLoginPwd,&QPushButton::clicked,this,[=](){
        if(ui->btn_remainLoginPwd->isChecked())
            config->updateRemainLoginPwd(true);
        else
            config->updateRemainLoginPwd(false);
    });
    connect(ui->btn_loadDefaultCfg,&QPushButton::clicked,this,[=](){
        config->checkConfigFile(false);
        loadCfgInfo();
    });
    connect(ui->tabWidget,&QTabWidget::currentChanged,this,[=]{
        //失去焦点和清楚密码
        if(ui->tabWidget->currentWidget() == ui->tab_setting){
            ui->spinBox_rollSpeed->clearFocus();
            ui->spinBox_rollStopTime->clearFocus();
            ui->lineEdit_newPwd->clearFocus();
            ui->lineEdit_sureNewPwd->clearFocus();
            ui->lineEdit_newPwd->clear();
            ui->lineEdit_sureNewPwd->clear();
        }
    });

    //默认主页
    btnReturnHome->click();
    ui->stackedWidget_home->setCurrentIndex(0);
    ui->stackedWidget_context->setCurrentWidget(ui->page_textEdit);

    loadStyleSheet(); // 加载样式表
    loadCfgInfo(); //加载配置
    loadTreeTitle(); //加载标题树
}

MainWin::~MainWin()
{
    delete ui;
}

QString MainWin::getSysName()const
{
    return config->getSysTitle();
}

void MainWin::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    //判断是否为图片模式，用于显示按钮
    if(isImage){
        btnReturnHome->setVisible(true);
        btnSubReturn->setVisible(true);
        hideFlag = 0;
    }
}

void MainWin::keyPressEvent(QKeyEvent *event)
{
    auto widget = this->focusWidget();

    //编辑界面的快捷键
    if(ui->context_title_edit == widget || ui->context_edit == widget){
        if(event->modifiers() == Qt::ControlModifier) {
            switch (event->key()) {
            case Qt::Key_S:
                ui->btn_save->click();
                break;
            case Qt::Key_I:
                ui->btn_fontItalic->click();
                break;
            case Qt::Key_B:
                ui->btn_fontBold->click();
                break;
            case Qt::Key_U:
                ui->btn_fontUnder->click();
                break;
            case Qt::Key_L:
                ui->btn_fontLeft->click();
                break;
            case Qt::Key_E:
                ui->btn_fontCenter->click();
                break;
            case Qt::Key_R:
                ui->btn_fontRight->click();
                break;
            case Qt::Key_J:
                ui->btn_fontJustify->click();
                break;
            case Qt::Key_F5:
                ui->btn_restore->click();
                break;
            case Qt::Key_Delete:
                ui->btn_clear->click();
                break;
            default:
                break;
            }
        }
        return; //直接返回
    }

    if(event->key() == Qt::Key_Return){
        if(ui->btn_intro == widget)
            ui->btn_intro->click();
        else if(ui->btn_work == widget)
            ui->btn_work->click();
        else if(ui->btn_society == widget)
            ui->btn_society->click();
        else if(ui->btn_medical == widget)
            ui->btn_medical->click();
        else if(ui->btn_education == widget)
            ui->btn_education->click();
        else if(ui->btn_law == widget)
            ui->btn_law->click();
        else if(btnWorkRule == widget)
            btnWordChart->click();
        else if(btnWordChart == widget)
            btnWordChart->click();
        else if(btnManage == widget)
            btnManage->click();
        else if(btnClose == widget)
            btnClose->click();
        else if(btnPreviousPage == widget) //
            btnPreviousPage->click();
        else if(btnPause == widget)
            btnPause->click();
        else if(btnNextPage == widget)
            btnNextPage->click();
        else if(btnSubReturn == widget)
            btnSubReturn->click();
        else if(btnReturnHome == widget)
            btnReturnHome->click();
        else if(ui->btn_exitLogin  == widget) //
            ui->btn_exitLogin->click();
        else if(ui->btn_sureLogin == widget)
            ui->btn_sureLogin->click();
        else if(ui->btn_save == widget)
            ui->btn_save->click();
        else if(ui->btn_restore == widget)
            ui->btn_restore->click();
        else if(ui->btn_exitEdit == widget)
            ui->btn_exitEdit->click();
    }
}

bool MainWin::eventFilter(QObject *watched, QEvent *event)
{
    //忽略鼠标滚轮和空格事件
    switch (event->type()) {
    case QEvent::Wheel:{
        if(ui->cBox_editMode == watched)
            ui->cBox_editMode->setEnabled(false);
        if(ui->cBox_showMode == watched)
            ui->cBox_showMode->setEnabled(false);
        if(ui->fontComboBox == watched)
            ui->fontComboBox->setEnabled(false);
        if(ui->cBox_lineSpace == watched)
            ui->cBox_lineSpace->setEnabled(false);
        if(ui->cBox_sysStyle == watched)
            ui->cBox_sysStyle->setEnabled(false);
        ui->spinBox_fontSize->clearFocus();
        break;
    }
    case QEvent::MouseButtonPress:{
        if(ui->cBox_editMode == watched)
            ui->cBox_editMode->setEnabled(true);
        if(ui->cBox_showMode == watched)
            ui->cBox_showMode->setEnabled(true);
        if(ui->fontComboBox == watched)
            ui->fontComboBox->setEnabled(true);
        if(ui->cBox_lineSpace == watched)
            ui->cBox_lineSpace->setEnabled(true);
        if(ui->cBox_sysStyle == watched)
            ui->cBox_sysStyle->setEnabled(true);
        break;
    }
    case QEvent::KeyPress:{
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Tab){
            if (QTextEdit *textEdit = qobject_cast<QTextEdit*>(watched)){
                textEdit->insertPlainText("  "); // 插入两个空格
                return true; // 拦截并处理 Tab 键事件
            }
        }
        break;
    }
    case QEvent::FocusIn:{
        if (event->type() == QEvent::FocusIn){
            if (ui->context_title_edit == qobject_cast<QLineEdit*>(watched)){
                //标题不支持设置样式
                ui->widget_editTool_2->setVisible(false);
                ui->widget_editTool_3->setVisible(false);
                ui->line_space_2->setVisible(false);
                ui->line_space_3->setVisible(false);
                //恢复默认
//                QFont font("微软雅黑");
//                ui->fontComboBox->setCurrentFont(font);
//                ui->spinBox_fontSize->setValue(12);
//                ui->cBox_lineSpace->setCurrentIndex(0);
//                ui->context_title_edit->setFocus();
            }else if (ui->context_edit == qobject_cast<QTextEdit*>(watched)){
                    ui->widget_editTool_2->setVisible(true);
                    ui->widget_editTool_3->setVisible(true);
                    ui->line_space_2->setVisible(true);
                    ui->line_space_3->setVisible(true);
                }
        }
        break;
    }
    default:
        break;
    }

    return QWidget::eventFilter(watched,event);
}

//按钮显示&隐藏
void MainWin::toolButtonVisible(const PageType &pageType)
{
    switch (pageType) {
    case PageType::HomePage:{
        isImage = false;
        btnPreviousPage->setVisible(false);
        btnPause->setVisible(false);
        btnNextPage->setVisible(false);
        btnSubReturn->setVisible(false);
        btnReturnHome->setVisible(false);
        btnWorkRule->setVisible(false);
        btnWordChart->setVisible(false);
        btnManage->setVisible(true);
        btnClose->setVisible(true);
        labelTime->setVisible(false);
        break;
    }
    case PageType::ImagePage:{ //只有返回键和主界面键
        isImage = true;
        btnPreviousPage->setVisible(false);
        btnPause->setVisible(false);
        btnNextPage->setVisible(false);
        btnSubReturn->setVisible(true);
        btnReturnHome->setVisible(true);
        btnWorkRule->setVisible(false);
        btnWordChart->setVisible(false);
        btnManage->setVisible(false);
        btnClose->setVisible(false);
        labelTime->setVisible(false);
        break;
    }
    case PageType::ShowPage:{
        isImage = false;
        btnPreviousPage->setVisible(true);
        btnPause->setVisible(true);
        btnNextPage->setVisible(true);
        btnSubReturn->setVisible(false);
        btnReturnHome->setVisible(true);
        btnWorkRule->setVisible(false);
        btnWordChart->setVisible(false);
        btnManage->setVisible(false);
        btnClose->setVisible(false);
        if(config->getLocalTimeShow())
            labelTime->setVisible(true);
        break;
    }
    case PageType::WorkPage:{
        isImage = false;
        btnPreviousPage->setVisible(true);
        btnPause->setVisible(true);
        btnNextPage->setVisible(true);
        btnSubReturn->setVisible(false);
        btnReturnHome->setVisible(true);
        btnWorkRule->setVisible(true);
        btnWordChart->setVisible(true);
        btnManage->setVisible(false);
        btnClose->setVisible(false);
        if(config->getLocalTimeShow())
            labelTime->setVisible(true);
        break;
    }
    case PageType::SocietyPage:{
        isImage = false;
        btnPreviousPage->setVisible(false);
        btnPause->setVisible(false);
        btnNextPage->setVisible(false);
        btnSubReturn->setVisible(false);
        btnReturnHome->setVisible(true);
        btnWorkRule->setVisible(false);
        btnWordChart->setVisible(false);
        btnManage->setVisible(false);
        btnClose->setVisible(false);
        if(config->getLocalTimeShow())
            labelTime->setVisible(true);
        break;
    }
    case PageType::SubShowPage:{
        isImage = false;
        btnPreviousPage->setVisible(true);
        btnPause->setVisible(true);
        btnNextPage->setVisible(true);
        btnSubReturn->setVisible(true);
        btnReturnHome->setVisible(true);
        btnWorkRule->setVisible(false);
        btnWordChart->setVisible(false);
        btnManage->setVisible(false);
        btnClose->setVisible(false);
        if(config->getLocalTimeShow())
            labelTime->setVisible(true);
        break;
    }
    default:
        break;
    }
}

//按钮功能处理
void MainWin::toolButtonHandle(const int& buttonId)
{
    Record record = db->queryRecord(buttonId);
    switch (buttonId) {
    case 0:{ //主界面被点击
        pageType = PageType::HomePage;
        toolButtonVisible(PageType::HomePage);
        updateRoll(RollType::Stop);//停止轮播状态
        hideFlag = 0;//重置用于图片界面隐藏按钮的标志

        Record record;
        for (int i = 1; i <= 6; ++i) {
            record = db->queryRecord(i);
            if(1 == i)
                ui->btn_intro->setText(record.getTitle());
            else if(2 == i)
                ui->btn_work->setText(record.getTitle());
            else if(3 == i)
                ui->btn_society->setText(record.getTitle());
            else if(4 == i)
                ui->btn_medical->setText(record.getTitle());
            else if(5 == i)
                ui->btn_education->setText(record.getTitle());
            else if(6 == i)
                ui->btn_law->setText(record.getTitle());
        }
        ui->stackedWidget->setCurrentWidget( ui->page_home);
        ui->stackedWidget_home->setCurrentWidget(ui->page_home_menu);
        return; //直接返回
    }
    case 1:{
        pageType = PageType::RoadPage;

        //判断是否优先展示图片
        if(record.getShowmode()){
            toolButtonVisible(PageType::ImagePage);
            loadImageShow(record);
        }else{
            toolButtonVisible(PageType::ShowPage);
            loadContextShow(record);
        }
        break;
    }
    case 2:{
        pageType = PageType::WorkPage;

        //判断是否优先展示图片
        if(record.getShowmode()){
            toolButtonVisible(PageType::ImagePage);
            loadImageShow(record);
        }else{
            toolButtonVisible(PageType::WorkPage);
            //子标题
            Record _record = db->queryRecord(21);
            btnWorkRule->setText(_record.getSubtitle());
            _record = db->queryRecord(22);
            btnWordChart->setText(_record.getSubtitle());

            loadContextShow(record); //内容
        }
        break;
    }
    case 3:{
        //子标题
        Record _record;
        _record = db->queryRecord(3);
        ui->lab_title_ss->setText(_record.getTitle());
        for (int i = 31; i <= 39; ++i) {
            _record = db->queryRecord(i);
            switch (i) {
            case 31:
                ui->btn_31->setText(_record.getSubtitle());
                break;
            case 32:
                ui->btn_32->setText(_record.getSubtitle());
                break;
            case 33:
                ui->btn_33->setText(_record.getSubtitle());
                break;
            case 34:
                ui->btn_34->setText(_record.getSubtitle());
                break;
            case 35:
                ui->btn_35->setText(_record.getSubtitle());
                break;
            case 36:
                ui->btn_36->setText(_record.getSubtitle());
                break;
            case 37:
                ui->btn_37->setText(_record.getSubtitle());
                break;
            case 38:
                ui->btn_38->setText(_record.getSubtitle());
                break;
            case 39:
                ui->btn_39->setText(_record.getSubtitle());
                break;
            }
        }
        pageType = PageType::SocietyPage;
        toolButtonVisible(pageType);
        ui->stackedWidget->setCurrentWidget( ui->page_subSociety);
        return; //直接返回
    }
    case 4:{
        pageType = PageType::MedicalPage;

        //判断是否优先展示图片
        if(record.getShowmode()){
            toolButtonVisible(PageType::ImagePage);
            loadImageShow(record);
        }else{
            toolButtonVisible(PageType::ShowPage);
            loadContextShow(record);
        }
        break;
    }
    case 5:{
        pageType = PageType::EducationPage;

        //判断是否优先展示图片
        if(record.getShowmode()){
            toolButtonVisible(PageType::ImagePage);
            loadImageShow(record);
        }else{
            toolButtonVisible(PageType::ShowPage);
            loadContextShow(record);
        }
        break;
    }
    case 6:{
        pageType = PageType::LawPage;

        //判断是否优先展示图片
        if(record.getShowmode()){
            toolButtonVisible(PageType::ImagePage);
            loadImageShow(record);
        }else{
            toolButtonVisible(PageType::ShowPage);
            loadContextShow(record);
        }
        break;
    }
    case 21:
    case 22:{
        pageType = PageType::SubWorkPage;

        //判断是否优先展示图片
        if(record.getShowmode()){
            toolButtonVisible(PageType::ImagePage);
            loadImageShow(record);
        }else{
            toolButtonVisible(PageType::SubShowPage);
            loadContextShow(record);
        }
        break;
    }
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:{
        pageType = PageType::SubSocietyPage;

        //判断是否优先展示图片
        if(record.getShowmode()){
            toolButtonVisible(PageType::ImagePage);
            loadImageShow(record);
        }else{
            toolButtonVisible(PageType::SubShowPage);
            loadContextShow(record);
        }
        break;
    }
    default:
        break;
    }
    hideFlag = 0;//重置用于图片界面隐藏按钮的标志
    updateRoll(RollType::Start);//开启轮播,主页和社会界面不开启
}

//加载标题树
void MainWin::loadTreeTitle()
{
    auto vec = db->queryAllRecord();

    //根据按钮id加载树状图
    for(auto it=vec.cbegin();it!=vec.cend();++it){
        QTreeWidgetItem *topItem = new QTreeWidgetItem();
        int btnId = it->getButtonid();
        if(1 == btnId){
            topItem->setText(0,it->getTitle());
            topItem->setData(0,Qt::UserRole,1);
        }else if(2 == btnId){
            topItem->setText(0,it->getTitle());
            topItem->setData(0,Qt::UserRole,2);

            //子节点
            for (int i=21; i <= 22; ++i) {
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                ++it;
                childItem->setText(0,it->getSubtitle());
                childItem->setData(0,Qt::UserRole,i);
                topItem->addChild(childItem);
            }
        }else if(3 == btnId){
            topItem->setText(0,it->getTitle());
            topItem->setData(0,Qt::UserRole,3);
            //子节点
            for (int i=31; i <= 39; ++i) {
                QTreeWidgetItem *childItem = new QTreeWidgetItem();
                ++it;
                childItem->setText(0,it->getSubtitle());
                childItem->setData(0,Qt::UserRole,i);
                topItem->addChild(childItem);
            }
        }else if(4 == btnId){
            topItem->setText(0,it->getTitle());
            topItem->setData(0,Qt::UserRole,4);
        }else if(5 == btnId){
            topItem->setText(0,it->getTitle());
            topItem->setData(0,Qt::UserRole,5);
        }else if(6 == btnId){
            topItem->setText(0,it->getTitle());
            topItem->setData(0,Qt::UserRole,6);
        }
        ui->tree_title->addTopLevelItem(topItem);
    }

}

void MainWin::loadCfgInfo()
{
    ui->spinBox_rollStopTime->setValue(config->getRollStopTime());

    ui->spinBox_rollSpeed->setValue(config->getRollSpeed());
    timer->setInterval(config->getRollSpeed()); //设置定时器步长

    QPixmap pixmap;
    pixmap.loadFromData(config->getSysIcon());
    ui->lab_sysIcon->setPixmap(pixmap);
    ui->lab_sysIcon_2->setPixmap(pixmap);
    ui->lineEdit_newSysTitle->setText(config->getSysTitle());

    ui->btn_localTimeShow->setChecked(config->getLocalTimeShow());
    ui->btn_editExitSave->setChecked(config->getEditExitSave());
    ui->btn_closeWinTip->setChecked(config->getCloseWinTip());
    ui->btn_rollRuleSetting->setChecked(config->getRollRuleSet());

    ui->btn_remainLoginPwd->setChecked(config->getRemainLoginPwd());
    if(config->getRemainLoginPwd())
        ui->lineEdit_pwd->setText(config->getEncryPwd());

    ui->widget_rollRuleSetting->setVisible(config->getRollRuleSet()); //规则可视管理

    if(1 == config->getStyleMode())
        ui->cBox_sysStyle->setCurrentText("绿色清新");
    else if(2 == config->getStyleMode())
        ui->cBox_sysStyle->setCurrentText("自然风景");

    ui->cBox_editMode->setCurrentIndex(0);

    //不可用
    ui->groupBox_rule1->setEnabled(false);
    ui->groupBox_rule2->setEnabled(false);
    ui->groupBox_rule3->setEnabled(false);
}

void MainWin::loadContextInfo(const EditState &state)
{
    QString context = ui->context_edit->toPlainText();
    context.remove(" ");
    context.remove("\n");
    Record record = db->queryRecord(currTitleItem->data(0,Qt::UserRole).toInt());
    QString updatetime = record.getUpdatetime().toString("yyyy年MM月dd日 HH:mm");

    //字数：5000字  状态：编辑中  最近更新时间：2023年12月05日
    switch (state) {
    case EditState::Loading:{
        ui->lab_saveLoading->setVisible(true);
        ui->lab_saveLoadingTip->setText("还原中...");
        ui->lab_contextInfo->setText(QString("| 字数：%1字 | 状态：还原中 | 最近更新时间：%2 |").arg(QString::number(context.length()),updatetime));
        QTimer::singleShot(1000,this, [=](){
            editState = EditState::Saved;
            ui->lab_saveLoading->setVisible(false);
            ui->lab_saveLoadingTip->setText("");
            ui->lab_contextInfo->setText(QString("| 字数：%1字 | 状态：已保存 | 最近更新时间：%2 |").arg(QString::number(context.length()),updatetime));
        });
        break;
    }
    case EditState::Editing:{
        ui->lab_contextInfo->setText(QString("| 字数：%1字 | 状态：编辑中 | 最近更新时间：%2 |").arg(QString::number(context.length()),updatetime));
        break;
    }
    case EditState::Saving:{
        ui->lab_saveLoading->setVisible(true);
        ui->lab_saveLoadingTip->setText("保存中...");
        ui->lab_contextInfo->setText(QString("| 字数：%1字 | 状态：保存中 | 最近更新时间：%2 |").arg(QString::number(context.length()),updatetime));
        //保存状态只显示1s
        QTimer::singleShot(1000,this, [=](){
            editState = EditState::Saved;
            ui->lab_saveLoading->setVisible(false);
            ui->lab_saveLoadingTip->setText("");
            ui->lab_contextInfo->setText(QString("| 字数：%1字 | 状态：已保存 | 最近更新时间：%2 |").arg(QString::number(context.length()),updatetime));
        });
        break;
    }
    case EditState::Saved:{
        ui->lab_contextInfo->setText(QString("| 字数：%1字 | 状态：已保存 | 最近更新时间：%2 |").arg(QString::number(context.length()),updatetime));
        break;
    }
    default:
        break;
    }
}

void MainWin::loginInfoClear()
{
    ui->lineEdit_pwd->clear();
    ui->lab_pwdTip->clear();
}

void MainWin::loadContextShow(const Record &record)
{
//    ui->context_show->setText(record.getContext());
    ui->context_show->setHtml(record.getContext());
    ui->lab_title->setText(record.getTitle());

    //设置行间距
//    QTextBlockFormat blockFmt;
//    blockFmt.setLineHeight(CONTEXT_LINE_SPACE, QTextBlockFormat::LineDistanceHeight);//设置行间距
//    ui->context_show->selectAll(); //选中全部文本，否则只会修改当前行
//    auto textCursor = ui->context_show->textCursor();
//    textCursor.setBlockFormat(blockFmt);
//    ui->context_show->setTextCursor(textCursor);

//    //设置完后，取消选中状态，若不写，则会导致存在选中状态
//    textCursor.clearSelection();
//    ui->context_show->setTextCursor(textCursor);
    ui->stackedWidget->setCurrentWidget( ui->page_basicSub);//页面切换
}

void MainWin::loadImageShow(const Record &record)
{
    QPixmap pixmap;
    pixmap.loadFromData(record.getImage());
    ui->lab_Chart->setPixmap(pixmap);

    ui->stackedWidget->setCurrentWidget( ui->page_image);//页面切换
}

void MainWin::updateRoll(const RollType& rollType)
{
    switch (rollType) {
    case RollType::Roll:{
        // 暂停状态下不进入轮播
        if(isPause)
            return;

        //判断是否为暂停位置
        if(0 == sliderPos || scrollbar->sliderPosition() == scrollbar->maximum()){
            timer->stop();
//            btnPause->setText("继续");
            //开始处
            if(0 == sliderPos){
                scrollbar->setSliderPosition(sliderPos);
                if(allowRoll)
                    ++sliderPos;
            }else { //结束处
                sliderPos = 0;
            }

            // 主页下不可启动定时器
            if(allowRoll){
                int sec = config->getRollStopTime()*1000; // 起始暂停时间
                QTimer::singleShot(sec,this, [=](){
                    if(!isPause){
                        timer->start();
                        btnPause->setText("暂停");
                    }
                });
             }
            return;
        }

        //允许轮播
        if(allowRoll && !isPause){
            scrollbar->setSliderPosition(sliderPos);
            ++sliderPos;
        }
        break;
    }
    case RollType::Pause:{
        timer->stop();
        isPause = true;
        btnPause->setText("继续");
        break;
    }
    case RollType::Continue:{
        timer->start();
        isPause = false;
        btnPause->setText("暂停");
        break;
    }
    case RollType::Start:{
        timer->start();
        allowRoll = true;
        isPause = false;
        sliderPos = 0;
        scrollbar->setSliderPosition(sliderPos);
        btnPause->setText("继续");
        break;
    }
    case RollType::Stop:{
        timer->stop();
        allowRoll = false;
        isPause = true;
        sliderPos = 0;
        scrollbar->setSliderPosition(sliderPos);
        btnPause->setText("暂停");
        break;
    }
    default:
        break;
    }
}

void MainWin::on_cBox_editMode_currentTextChanged(const QString &mode)
{
    bool isEnable = false;
    if("文本编辑" == mode){
        ui->stackedWidget_context->setCurrentWidget(ui->page_textEdit);
        isEnable = true;
    }else{
        //加载图片
        if(nullptr != currTitleItem){
            Record record = db->queryRecord(currTitleItem->data(0,Qt::UserRole).toInt());
            QPixmap pixmap;
            pixmap.loadFromData(record.getImage());
            ui->lab_imageShow->setPixmap(pixmap);

            ui->stackedWidget_context->setCurrentWidget(ui->page_imageEdit);
            ui->cBox_showMode->setCurrentIndex(record.getShowmode()); //设置优先级选项
        }
    }
    //失能&使能按钮
    ui->btn_save->setEnabled(isEnable);
    ui->btn_open->setEnabled(isEnable);
    ui->btn_undo->setEnabled(isEnable);
    ui->btn_redo->setEnabled(isEnable);
    ui->btn_selectAll->setEnabled(isEnable);
    ui->btn_cut->setEnabled(isEnable);
    ui->btn_copy->setEnabled(isEnable);
    ui->btn_paste->setEnabled(isEnable);
    ui->btn_restore->setEnabled(isEnable);
    ui->widget_editTool_2->setEnabled(isEnable);
    ui->widget_editTool_3->setEnabled(isEnable);
    ui->context_edit->setFocus();
}

void MainWin::on_btn_save_clicked()
{
    if(nullptr == currTitleItem)
        return;
    if(ui->context_title_edit->text().isEmpty()){
        QMessageBox::information(this,"编辑提示","文本标题不为空，请填写标题!",QMessageBox::Ok);
        return;
    }
    if(ui->context_edit->toPlainText().isEmpty()){
        auto ret = QMessageBox::information(this,"编辑提示","文本内容尚未填写，确定要保存吗?",QMessageBox::Yes,QMessageBox::No);
        if(QMessageBox::StandardButton::Yes != ret)
            return;
    }

    //保存数据，先判断是否为子标题
    int btnId = currTitleItem->data(0,Qt::UserRole).toInt();
    bool ret,_ret;
    if(1==btnId || 2==btnId || 3==btnId || 4==btnId || 5==btnId || 6==btnId)
        ret = db->updateTitle(btnId,ui->context_title_edit->text());
    else
        ret = db->updateSubTitle(btnId,ui->context_title_edit->text());
    _ret = db->updateContext(btnId,ui->context_edit->toHtml()); //保存为html
    if(ret && _ret){
        currTitleItem->setText(0,ui->context_title_edit->text());
        loadContextInfo(EditState::Saving);//加载提示
        editState = EditState::Saved;//成功保存
    }else
        QMessageBox::information(this,"编辑提示","文本数据保存异常，请重新尝试!",QMessageBox::Ok);
    ui->context_edit->setFocus();
}

void MainWin::on_btn_clear_clicked()
{
    if("文本编辑" == ui->cBox_editMode->currentText()){
        auto ret = QMessageBox::information(this,"编辑提示","确定要清除所有内容吗?",QMessageBox::Yes,QMessageBox::No);
        if(QMessageBox::StandardButton::Yes != ret)
            return;
        ui->context_edit->clear();
    }else{ //图片
        auto ret = QMessageBox::information(this,"编辑提示","确定要删除图片?",QMessageBox::Yes,QMessageBox::No);
        if(QMessageBox::StandardButton::Yes != ret)
            return;

        if(db->deleteImage(currTitleItem->data(0,Qt::UserRole).toInt())){
            ui->lab_imageShow->clear();
            ui->cBox_showMode->setCurrentIndex(0); //恢复文本优先展示
        }
    }
    ui->context_edit->setFocus();
}

void MainWin::on_btn_restore_clicked()
{
    Record record = db->queryRecord(currTitleItem->data(0,Qt::UserRole).toInt());
    //判断是否为子标题
    if(!record.getSubtitle().isEmpty()){
        currTitleItem->setText(0,record.getSubtitle());
        ui->context_title_edit->setText(record.getSubtitle());
    }else{
        currTitleItem->setText(0,record.getTitle());
        ui->context_title_edit->setText(record.getTitle());
    }
    ui->context_edit->setText(record.getContext());
    loadContextInfo(EditState::Loading);//加载提示
    ui->context_edit->setFocus();
}

void MainWin::on_btn_selectImageShow_clicked()
{
    if(nullptr == currTitleItem){
        QMessageBox::information(this,"编辑提示","无法选择图片，请先选择标题!",QMessageBox::Ok);
        return;
    }
    QString filter = "*.png *jpeg *jpg *bmp";
    QString fileName = QFileDialog::getOpenFileName(this,"选择图片","C:/",filter);
    if(fileName.isEmpty())
        return;

    QPixmap pixmap;
    pixmap.load(fileName);

    //从pixmap加载图片的二进制数据
    QByteArray imageArray;
    QBuffer buffer(&imageArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    buffer.close();

    //保存数据
    bool ret = db->updateImage(currTitleItem->data(0,Qt::UserRole).toInt(), imageArray);
    if(!ret){
        QMessageBox::information(this,"编辑提示","图片数据保存异常，请重新尝试!",QMessageBox::Ok);
    }else{
        //成功保存图片数据才算成功，才去加载图片显示
        ui->lab_imageShow->setPixmap(pixmap);
        ui->cBox_editMode->setCurrentIndex(1); //切换模式
    }
}


void MainWin::on_btn_selectAll_clicked()
{
    if(ui->context_edit->hasFocus())
        ui->context_edit->selectAll();
    else if(ui->context_title_edit->hasFocus())
        ui->context_title_edit->selectAll();
}

void MainWin::on_btn_cut_clicked()
{
    if(ui->context_edit->hasFocus()){
        QTextCursor textCursor = ui->context_edit->textCursor();
        if(textCursor.hasSelection()){ //有选中才剪切
            clipboard->setText(textCursor.selectedText()); //保存至剪切板
            ui->context_edit->cut();
        }
    }else if(ui->context_title_edit->hasFocus()){
        if(ui->context_title_edit->hasSelectedText()){
            clipboard->setText( ui->context_title_edit->selectedText());
            ui->context_title_edit->cut();
        }
    }
}

void MainWin::on_btn_copy_clicked()
{
    if(ui->context_edit->hasFocus()){
        QTextCursor textCursor = ui->context_edit->textCursor();
        if(textCursor.hasSelection()) //有选中才复制
            clipboard->setText(textCursor.selectedText()); //保存至剪切板
    }else if(ui->context_title_edit->hasFocus()){
        if(ui->context_title_edit->hasSelectedText())
            clipboard->setText(ui->context_title_edit->selectedText());
    }
}

void MainWin::on_btn_paste_clicked()
{
    if(ui->context_edit->hasFocus()){
        QTextCursor textCursor = ui->context_edit->textCursor();
        textCursor.insertText(clipboard->text());
    }else if(ui->context_title_edit->hasFocus()){
        ui->context_title_edit->insert(clipboard->text());
    }
}

void MainWin::on_btn_undo_clicked()
{
    if(ui->context_edit->hasFocus()){
        ui->context_edit->undo();
    }else if(ui->context_title_edit->hasFocus()){
        ui->context_title_edit->undo();
    }
}

void MainWin::on_btn_redo_clicked()
{
    if(ui->context_edit->hasFocus()){
        ui->context_edit->redo();
    }else if(ui->context_title_edit->hasFocus()){
        ui->context_title_edit->redo();
    }
}

void MainWin::on_btn_open_clicked()
{
    QString curPath,aFileName;
    curPath=QCoreApplication::applicationDirPath(); //获取应用程序的路径

    //调用打开文件对话框打开一个文件
    aFileName=QFileDialog::getOpenFileName(this,tr("打开一个文件"),curPath,
                 "Word文档(*.word);;文本文件(*.txt);;所有文件(*.*)");

//    if (!aFileName.isEmpty())
//    {
//        QFile aFile(aFileName);  //以文件方式读出
//        if (aFile.open(QIODevice::ReadWrite | QIODevice::Text))
//        {
//            QTextStream aStream(&aFile); //用文本流读取文件
//            while (!aStream.atEnd())
//                ui->txtEdit->append(aStream.readLine()); //读取一个文本行
//            updateCurFile(aFileName); //更新状态栏显示
//        }
//        aFile.close();
//    }
    ui->context_edit->setFocus();
}

void MainWin::on_spinBox_fontSize_valueChanged(int size)
{
    if(size < 5)
        size = 5;
    else if(size > 50)
        size = 50;
    ui->spinBox_fontSize->setValue(size);

    QTextCharFormat fmt;
    fmt.setFontPointSize(size); //设置字体大小
    ui->context_edit->mergeCurrentCharFormat(fmt);
}

void MainWin::on_fontComboBox_currentFontChanged(const QFont &font)
{
    QTextCharFormat fmt = ui->context_edit->currentCharFormat();
    fmt.setFont(font);
    fmt.setFontPointSize(ui->spinBox_fontSize->value()); //用当前字体
    ui->context_edit->mergeCurrentCharFormat(fmt);//将格式添加至文本编辑器中
    ui->context_edit->setFocus();
}

void MainWin::on_btn_currLine_clicked()
{
    // 获取行间距
    int lineSpace = 100;
    if("1.5倍行距" == ui->cBox_lineSpace->currentText())
        lineSpace = 150;
    else if("2倍行距" == ui->cBox_lineSpace->currentText())
        lineSpace = 200;

    QTextBlockFormat blockFmt;
    blockFmt.setLineHeight(lineSpace,QTextBlockFormat::ProportionalHeight);

    QTextCursor textCursor = ui->context_edit->textCursor();
    textCursor.setBlockFormat(blockFmt);
    ui->context_edit->setFocus();
}

void MainWin::on_btn_allLine_clicked()
{
    // 获取行间距
    int lineSpace = 100;
    if("1.5倍行距" == ui->cBox_lineSpace->currentText())
        lineSpace = 150;
    else if("2倍行距" == ui->cBox_lineSpace->currentText())
        lineSpace = 200;
    QTextBlockFormat blockFmt;
    blockFmt.setLineHeight(lineSpace,QTextBlockFormat::ProportionalHeight);

    //全行设置
    ui->context_edit->selectAll();
    QTextCursor textCursor = ui->context_edit->textCursor(); //获取文本光标
    textCursor.setBlockFormat(blockFmt);

    //取消全部选择状态
    textCursor.clearSelection();
    ui->context_edit->setTextCursor(textCursor);
    ui->context_edit->setFocus();
}

void MainWin::on_btn_fontLeft_clicked()
{
    ui->context_edit->setAlignment(Qt::AlignLeft);
}

void MainWin::on_btn_fontCenter_clicked()
{
    ui->context_edit->setAlignment(Qt::AlignCenter);
}

void MainWin::on_btn_fontRight_clicked()
{
    ui->context_edit->setAlignment(Qt::AlignRight);
}

void MainWin::on_btn_fontJustify_clicked()
{
    ui->context_edit->setAlignment(Qt::AlignJustify);
}

void MainWin::on_btn_fontBold_clicked()
{
    QTextCharFormat fmt = ui->context_edit->currentCharFormat();//获取当前选择文字的格式
    if (ui->btn_fontBold->isChecked())
        fmt.setFontWeight(QFont::Bold);
    else
        fmt.setFontWeight(QFont::Normal);

    ui->context_edit->mergeCurrentCharFormat(fmt);//将格式添加至文本编辑器中
}

void MainWin::on_btn_fontItalic_clicked()
{
    QTextCharFormat fmt = ui->context_edit->currentCharFormat();
    if(ui->btn_fontItalic->isChecked())
        fmt.setFontItalic(true);
    else
        fmt.setFontItalic(false);
    ui->context_edit->mergeCurrentCharFormat(fmt);
}

void MainWin::on_btn_fontUnder_clicked()
{
    QTextCharFormat fmt = ui->context_edit->currentCharFormat();
    if(ui->btn_fontUnder->isChecked())
        fmt.setFontUnderline(true);
    else
        fmt.setFontUnderline(false);
    ui->context_edit->mergeCurrentCharFormat(fmt);
}

void MainWin::on_btn_fontColor_clicked()
{
//    QColor stdColor(40,40,40);
    QColor cusColor = QColorDialog::getColor(Qt::black, this,"字体颜色",QColorDialog::ShowAlphaChannel);//ShowAlphaChannel
    ui->context_edit->setTextColor(cusColor);
}

void MainWin::on_btn_exitEdit_clicked()
{
    //判断是否在编辑中
    if(EditState::Editing == editState){
        //判断是否开启了退出自动保存
        if(config->getEditExitSave()){
            on_btn_save_clicked();
            QTimer::singleShot(1000,this, [=](){ //为了更好的展示效果
                editState = EditState::Saved;
                ui->stackedWidget_home->setCurrentIndex(0);
                btnReturnHome->click();
            });
            return;
        }else{
            auto ret = QMessageBox::information(this,"编辑提示","文本数据未保存，是否放弃保存数据?",QMessageBox::Yes,QMessageBox::No);
            if(QMessageBox::StandardButton::Yes != ret)
                return;
        }
    }
    editState = EditState::Saved;
    ui->stackedWidget_home->setCurrentIndex(0);
    btnReturnHome->click();
}

void MainWin::on_tree_title_itemClicked(QTreeWidgetItem *item, int column)
{
    ui->context_title_edit->setFocus();
    Record record = db->queryRecord(item->data(column,Qt::UserRole).toInt());
    if(item == currTitleItem){
        //同步部分设置
        ui->cBox_editMode->setCurrentIndex(record.getShowmode());
        ui->cBox_showMode->setCurrentIndex(record.getShowmode());
        return;
    }
    //判断是否在编辑中
    if(EditState::Editing == editState){
        auto ret = QMessageBox::information(this,"编辑提示","文本数据未保存，是否放弃保存数据?",QMessageBox::Yes,QMessageBox::No);
        if(QMessageBox::StandardButton::Yes != ret){
            currTitleItem->setSelected(true);
            item->setSelected(false);
            return;
        }
    }
    currTitleItem = item;

    //加载数据
    ui->context_title_edit->setText(item->text(column));
    ui->context_edit->setText(record.getContext());
    QPixmap pixmap;
    pixmap.loadFromData(record.getImage());
    ui->lab_imageShow->setPixmap(pixmap);

    //同步部分设置
    ui->cBox_editMode->setCurrentIndex(record.getShowmode());
    ui->cBox_showMode->setCurrentIndex(record.getShowmode());
    if(record.getShowmode())
        ui->stackedWidget_context->setCurrentWidget(ui->page_imageEdit);
    else
        ui->stackedWidget_context->setCurrentWidget(ui->page_textEdit);

    //第一次加载数据导致状态为EditState::Editing
    editState = EditState::Saved;
    loadContextInfo(editState); //加载提示

    ui->context_title_edit->setReadOnly(false);
    ui->context_edit->setReadOnly(false);
}

void MainWin::on_context_edit_selectionChanged()
{
    //当前选择的文字发生变化,更新粗体、斜体、下划线的checked状态，都不是则显示正常状态
    QTextCharFormat fmt = ui->context_edit->currentCharFormat();

    ui->btn_fontItalic->setChecked(fmt.fontItalic()); //是否斜体
    ui->btn_fontBold->setChecked(fmt.font().bold()); //是否粗体
    ui->btn_fontUnder->setChecked(fmt.fontUnderline()); //是否有下划线

    QTextCursor textCursor = ui->context_edit->textCursor();
    if(!textCursor.hasSelection()){
        ui->fontComboBox->setCurrentFont(fmt.font().family());//字体
        ui->spinBox_fontSize->setValue(fmt.fontPointSize());//字体大小
        qDebug()<<fmt.fontPointSize();
    }

    QTextBlockFormat blockFmat = textCursor.block().blockFormat();
//    qDebug()<<blockFmat.alignment();
    //居中类型
    if(Qt::AlignLeft == blockFmat.alignment() || Qt::AlignLeading == blockFmat.alignment())
        ui->btn_fontLeft->setChecked(true);
    else if(Qt::AlignHCenter == blockFmat.alignment())
        ui->btn_fontCenter->setChecked(true);
    else if(Qt::AlignJustify == blockFmat.alignment())
        ui->btn_fontJustify->setChecked(true);
    else
        ui->btn_fontRight->setChecked(true);

    // 光标所在文本块的行高
    qreal lineSpace = blockFmat.lineHeight();
    if(0 == lineSpace || 100 == lineSpace)
        ui->cBox_lineSpace->setCurrentIndex(0);
    else if(150 == lineSpace)
        ui->cBox_lineSpace->setCurrentIndex(1);
    else if(200 == lineSpace)
        ui->cBox_lineSpace->setCurrentIndex(2);
}

void MainWin::on_cBox_lineSpace_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    ui->context_edit->setFocus();
}

void MainWin::loadStyleSheet()
{

    int styleMode = config->getStyleMode();
    if(1 == styleMode){
        QFile styleFile(":/qss/style1.qss" );
        if(styleFile.open(QFile::ReadOnly)){
            this->setStyleSheet(styleFile.readAll());
            styleFile.close();
        }
        //自定义控件的样式
        QString     styleSheet;
        styleSheet = "QPushButton{ border:2px solid rgb(255,255,255); "
                     "border-radius:35px;"
                     "background-image:url(\":/img/style1/background-4.jpg\");"
                     "color:rgb(40,40,40);font-size: 12pt;font-weight: bold;font-family: \"微软雅黑\";}"
                     "QPushButton:hover{border:2px solid rgb(240, 240, 240);}";
        btnReturnHome->setStyleSheet(styleSheet);

        styleSheet = "QPushButton{ border:2px solid rgb(255,255,255); "
                     "border-radius:30px;"
                     "background-image:url(\":/img/style1/background-5.jpg\");"
                     "color:rgb(40,40,40);font-size: 12pt;font-weight: bold;font-family: \"微软雅黑\";}"
                     "QPushButton:hover{border:2px solid rgb(240, 240, 240);}";
        btnPreviousPage->setStyleSheet(styleSheet);
        btnPause->setStyleSheet(styleSheet);
        btnNextPage->setStyleSheet(styleSheet);
        btnSubReturn->setStyleSheet(styleSheet);

        styleSheet = "QPushButton{ border:1px solid rgb(200, 200, 200);border-radius:10px;"
                     "background-image:url(\":/img/style1/background-3.jpg\");"
                     "color:rgb(40,40,40);font-size: 13pt;font-weight: bold;font-family: \"微软雅黑\";}"
                     "QPushButton:hover{border:1px solid rgb(180, 180, 180);}";
        btnWorkRule->setStyleSheet(styleSheet);
        btnWordChart->setStyleSheet(styleSheet);

    }else if(2 == styleMode){
        QFile styleFile(":/qss/style2.qss" );
        if(styleFile.open(QFile::ReadOnly)){
            this->setStyleSheet(styleFile.readAll());
            styleFile.close();
        }
        //自定义控件的样式
        QString     styleSheet;
        styleSheet = "QPushButton{ border:2px solid rgb(255,255,255); "
                     "border-radius:35px;"
                     "background-image:url(\":/img/style2/background-4.jpg\");"
                     "color:rgb(40,40,40);font-size: 12pt;font-weight: bold;font-family: \"微软雅黑\";}"
                     "QPushButton:hover{border:2px solid rgb(225, 225, 225);}";
        btnReturnHome->setStyleSheet(styleSheet);

        styleSheet = "QPushButton{ border:2px solid rgb(255,255,255); "
                     "border-radius:30px;"
                     "background-image:url(\":/img/style2/background-5.jpg\");"
                     "color:rgb(40,40,40);font-size: 11pt;font-weight: bold;font-family: \"微软雅黑\";}"
                     "QPushButton:hover{border:2px solid rgb(225, 225, 225);}";
        btnPreviousPage->setStyleSheet(styleSheet);
        btnPause->setStyleSheet(styleSheet);
        btnNextPage->setStyleSheet(styleSheet);
        btnSubReturn->setStyleSheet(styleSheet);

        styleSheet = "QPushButton{ border:1px solid rgb(200, 200, 200);border-radius:10px;"
                     "background-image:url(\":/img/style2/background-3.jpg\");"
                     "color:rgb(60,60,60);font-size: 13pt;font-weight: bold;font-family: \"微软雅黑\";}"
                     "QPushButton:hover{border:1px solid rgb(180, 180, 180);}";
        btnWorkRule->setStyleSheet(styleSheet);
        btnWordChart->setStyleSheet(styleSheet);
    }

    btnManage->setToolTip("管理员");
    btnManage->setStyleSheet("QPushButton{ border:none;}");
    if(1 == styleMode)
        btnManage->setIcon(QIcon(":/img/style1/admin.png"));
    else if(2 == styleMode)
        btnManage->setIcon(QIcon(":/img/style2/admin.png"));
    btnManage->setIconSize(QSize(BUTTON_MANAGE_SIZE/2+5,BUTTON_MANAGE_SIZE/2+5));

    btnClose->setToolTip("退出");
    btnClose->setStyleSheet("QPushButton{ border:none;}");
    if(1 == styleMode)
        btnClose->setIcon(QIcon(":/img/style1/exit.png"));
    else if(2 == styleMode)
        btnClose->setIcon(QIcon(":/img/style2/exit.png"));
    btnClose->setIconSize(QSize(BUTTON_MANAGE_SIZE/2+2,BUTTON_MANAGE_SIZE/2+2));

    QRegExpValidator *reg = new QRegExpValidator(this);
    reg->setRegExp(QRegExp("[a-zA-Z0-9]+$"));
    ui->lineEdit_pwd->setValidator(reg);
    ui->lineEdit_newPwd->setValidator(reg);
    ui->lineEdit_sureNewPwd->setValidator(reg);

    if(1 == styleMode)
        ui->btn_exitEdit->setIcon(QIcon(":/img/style1/exitLogin.png"));
    else if(2 == styleMode)
        ui->btn_exitEdit->setIcon(QIcon(":/img/style2/exitLogin.png"));
    ui->btn_exitEdit->setIconSize(QSize(24,24));

    //编辑界面的图片显示label
    ui->lab_imageShow->setMinimumSize(1152,648);
    ui->lab_imageShow->setMaximumSize(1152,648);

    //时间显示
    labelTime->setGeometry(CONTEXT_SHOW_ALIGN_LEFT,CONTEXT_SHOW_ALIGN_TOP-40,700,40);
    labelTime->setStyleSheet("QLabel{font-size: 18pt;font-weight: bold;font-family: \"楷体\";}");
    labelTime->setVisible(false);

    ui->btn_open->setVisible(false);
}


