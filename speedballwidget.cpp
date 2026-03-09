#include "SpeedBallWidget.h"
#include "SimpleCameraView.h"
#include "SpeedBallWidget.h"
#include <QScreen>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QTimer>
#include "global.h"
// 构造函数初始化
SpeedBallWidget::SpeedBallWidget(QWidget *parent)
    : QWidget(parent), flashMode(false), flashOn(false), edgeWidth(10), cameraView(nullptr), isDocked(false) {


    setScaledFixedSize(this,100 , 260);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    FSG = new FullScreenGradientWidget();
    flashTimer = new QTimer(this);
    connect(flashTimer, &QTimer::timeout, this, &SpeedBallWidget::toggleFlash);
//    setAttribute(Qt::WA_Hover);
//    QToolTip::showText(cursor().pos(), wStatusIconButton->toolTip());

    // 初始化边缘颜色和呼吸效果定时器
    edgeColor = QColor(0, 255, 0);
    breathingTimer = new QTimer(this);
    connect(breathingTimer, &QTimer::timeout, this, &SpeedBallWidget::updateEdgeColor);
    breathingTimer->start(50);  // 每50毫秒更新一次边缘颜色

    setupLayout();

    // 设置贴边定时器，5秒内无操作则自动贴边
    dockTimer = new QTimer(this);
    dockTimer->setInterval(dockDelay);
    connect(dockTimer, &QTimer::timeout, this, &SpeedBallWidget::checkAutoDock);
    dockTimer->start();

    // 将窗口放置在屏幕右上角，留出10像素的边距
    const int margin = 10;
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move(screenGeometry.width() - width() - margin, margin);
    setupContextMenuForImageLabel();
qDebug() << "当前: "; qDebug() << "8610";


getTheTotalNumberOfNotifications();

}
/**
 * @brief SpeedBallWidget::getTheTotalNumberOfNotifications
 * @brief 获取通知总条数
 */
void SpeedBallWidget::getTheTotalNumberOfNotifications() {

    QJsonObject json;
    json.insert("projectId", kProjectId);

    json.insert("userId", kUserID);
    QString jsonString =    utils->startRequestGet(kNotificationPath, json, Token);

    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!doc.isNull()) {
        if (doc.isObject()) {
            QJsonObject jsonObject = doc.object();
            // Parse top level keys
            int code = jsonObject["code"].toInt();
            qDebug() << "code:" << code;
            QString msg = jsonObject["msg"].toString();
            qDebug() << "msg:" << msg;
            QJsonObject data = jsonObject["data"].toObject();
            // Parse 'data' object keys
            QString sign = data["sign"].toString();
            qDebug() << "sign:" << sign;
            qint64 random = data["random"].toInt();
            qDebug() << "random:" << random;
            QJsonArray wcNotifications = data["wcNotifications"].toArray();
            qDebug() << "wcNotifications.size(): "; qDebug() << wcNotifications.size();

            saveLog->save("通知总条数"+QString::number(wcNotifications.size()));
             setSatusIconLabelText(QString::number(wcNotifications.size()));
        }
    }
}
// 更新边缘颜色以实现呼吸效果
void SpeedBallWidget::updateEdgeColor() {
    int alpha = (qAbs(breathingStep - 50) * 5);  // alpha 在 0-255 之间波动
    edgeColor.setAlpha(alpha);
    breathingStep = (breathingStep + 1) % 100;
    update();  // 触发重新绘制
}
void SpeedBallWidget::setScaledFixedSize(QWidget *widget, int originalWidth, int originalHeight) {
    // 获取当前屏幕的分辨率
    QScreen *screen = QApplication::primaryScreen();
    QSize screenSize = screen->size();
    int screenWidth = screenSize.width();
    int screenHeight = screenSize.height();

    // 4K分辨率
    int baseWidth = 1920;
    int baseHeight = 1080;

    // 计算缩放比例
    widthRatio = static_cast<double>(screenWidth) / baseWidth;
    heightRatio = static_cast<double>(screenHeight) / baseHeight;

    // 根据比例调整宽高
    wScaledWidth = static_cast<int>(originalWidth * widthRatio) + 6;  // 增加6像素的边框宽度
    wScaledHeight = static_cast<int>(originalHeight * heightRatio);

    // 计算 ：在 4K 分辨率下 60 像素对应的实际宽度
    edgeMarginWidth = static_cast<int>(30 * widthRatio);
    backgroundWidth = static_cast<int>(75 * widthRatio);

    // 输出计算结果
    qDebug() << "Scaled Width:" << wScaledWidth << ", Scaled Height:" << wScaledHeight << ", Edge Margin Width:" << edgeMarginWidth;

    // 设置固定大小
    widget->setFixedSize(wScaledWidth, wScaledHeight);
}
void SpeedBallWidget::setWTime(const QString &timeText) {
    wTimeButton->setText(timeText);
}
void SpeedBallWidget::setSatusIconLabelText(const QString &text) {
    qDebug() << "当前: "; qDebug() << "3799";
qDebug() << "text3232: "; qDebug() << text;

    wStatusIconButton->setText(text);
}
void SpeedBallWidget::setupLayout() {

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 计算基于分辨率的字体大小
    int baseFontSize = 10; // 设计时的基准字号
     scaledFontSize = static_cast<int>(baseFontSize * widthRatio);


    wStatusIconButton = new QPushButton("0", this);
    wStatusIconButton->setToolTip("查看通知");
    int statusIconWidth = static_cast<int>(25 * widthRatio);
    int statusIconHeight = static_cast<int>(24 * heightRatio);
    wStatusIconButton->setFixedSize(statusIconWidth, statusIconHeight);

    wStatusIconButton->setFlat(true);
    wStatusIconButton->setStyleSheet(QString("QPushButton {"
                                             "border: none;"
                                             "border-image: url(:/image/FloatingWindow/new_information.png);"
                                             "font-size: %1px;" // 动态字体大小
                                             "}").arg(scaledFontSize));
    connect(wStatusIconButton, &QPushButton::clicked, this, &SpeedBallWidget::onWStatusIconButtonClicked);

    mainLayout->addWidget(wStatusIconButton, 0, Qt::AlignHCenter);
    qDebug() << "当前: "; qDebug() << "5379";

    // 中间头像
    centerImageLabel = new QLabel(this);
    centerImageLabel->setToolTip("头像");

    int centerImageWidth = static_cast<int>(75 * widthRatio);
    int centerImageHeight = static_cast<int>(75 * heightRatio);
    centerImageLabel->setFixedSize(centerImageWidth, centerImageHeight);
    QString genderImagePath = kGender
                                  ? ":/image/FloatingWindow/normal_face_female.png"
                                  : ":/image/FloatingWindow/normal_face_male.png";

    QString styleSheet = QString("QLabel {"
                                 "border-image: url(%1);"
                                 "}")
                             .arg(genderImagePath);

    centerImageLabel->setStyleSheet(styleSheet);
    mainLayout->addWidget(centerImageLabel, 0, Qt::AlignHCenter);

#ifdef BETA_VERSION
    cameraView = new SimpleCameraView(this);
    cameraView->hide();
#endif

    // 时间按钮
    wTimeButton = new QPushButton(jss->getTimeLabel(), this);
    wTimeButton ->setToolTip("工作时间");

    cameraButtonWidth = static_cast<int>(70 * widthRatio);
    cameraButtonHeight = static_cast<int>(22 * heightRatio);
    wTimeButton->setFixedSize(cameraButtonWidth, cameraButtonHeight);

    wTimeButton->setFlat(true);
    wTimeButton->setStyleSheet(QString("QPushButton {"
                                       "border: none;"
                                       "color: rgb(255, 255, 255);"
                                       "}").arg(scaledFontSize));
    mainLayout->addWidget(wTimeButton, 0, Qt::AlignHCenter);

    // 开始工作按钮
    wStartButton = new QPushButton(this);
    wStartButton->setText("开始工作");
    wStartButton->setFixedSize(cameraButtonWidth, cameraButtonHeight);

    connect(wStartButton, &QPushButton::clicked, this, &SpeedBallWidget::onWStartButtonClicked);

    wStartButton->setStyleSheet(QString("QPushButton {"
                                        "border-image: url(:/image/FloatingWindow/button_background2.png);"
                                        "border: none;"
                                        "font-size: %1px;" // 动态字体大小
                                        "}").arg(scaledFontSize));

    QIcon icon(":/image/FloatingWindow/wsuspension_of_work.png");
    wStartButton->setIcon(icon);
    wStartButton->setIconSize(QSize(15, 15));
    mainLayout->addWidget(wStartButton, 0, Qt::AlignHCenter);

    // 停止工作按钮
    wEndButton = new QPushButton(this);
    wEndButton->setText(STOP_WORK);

    wEndButton->setFixedSize(cameraButtonWidth, cameraButtonHeight);

    wEndButton->setStyleSheet(QString("QPushButton {"
                                      "border-image: url(:/image/FloatingWindow/button_background2.png);"
                                      "border: none;"
                                      "font-size: %1px;" // 动态字体大小
                                      "}").arg(scaledFontSize));

    QIcon icon2(":/image/FloatingWindow/wstopbtn.png");
    wEndButton->setIcon(icon2);
    wEndButton->setIconSize(QSize(15, 15));
     connect(wEndButton, &QPushButton::clicked, this, &SpeedBallWidget::onWEndButtonClicked);
    mainLayout->addWidget(wEndButton, 0, Qt::AlignHCenter);

    // 底部相机按钮
    cameraButton = new QPushButton(this);
    wStartButton->setText("开始工作");

    int cameraButtonWidth = static_cast<int>(24 * widthRatio);
    int cameraButtonHeight = static_cast<int>(19 * heightRatio);
    cameraButton->setFixedSize(cameraButtonWidth, cameraButtonHeight);
    QPixmap cameraButtonPixmap(":/image/FloatingWindow/face_camera.png");
    cameraButton->setIcon(QIcon(cameraButtonPixmap));
    cameraButton->setIconSize(QSize(cameraButtonWidth, cameraButtonHeight));
    cameraButton->setFlat(true);
    connect(cameraButton, &QPushButton::clicked, this, &SpeedBallWidget::onCameraButtonClicked);

    cameraButton->setStyleSheet(QString("QPushButton {"
                                        "font-size: %1px;" // 动态字体大小
                                        "}").arg(scaledFontSize));
    mainLayout->addWidget(cameraButton, 0, Qt::AlignHCenter);

#ifdef BETA_VERSION

#else
    cameraButton->hide();
#endif
    setEdgeColor(Qt::green);
    // 启用工具提示显示

}

//void SpeedBallWidget::setupLayout() {

//    QVBoxLayout *mainLayout = new QVBoxLayout(this);
//    mainLayout->setContentsMargins(0, 0, 0, 0);


//    //    // 状态图标
//    //    statusIconLabel = new QLabel(this);
//    //    int statusIconWidth = static_cast<int>(25 * widthRatio);
//    //    int statusIconHeight = static_cast<int>(24 * heightRatio);
//    //    statusIconLabel->setFixedSize(statusIconWidth, statusIconHeight);
//    //    QPixmap statusIconPixmap(":/image/FloatingWindow/new_information.png");
//    //    statusIconLabel->setPixmap(statusIconPixmap.scaled(statusIconWidth, statusIconHeight, Qt::IgnoreAspectRatio));
//    //    mainLayout->addWidget(statusIconLabel, 0, Qt::AlignHCenter);


//    // 底部相机按钮
//    wStatusIconButton = new QPushButton("0",this);

//    int statusIconWidth = static_cast<int>(25 * widthRatio);
//    int statusIconHeight = static_cast<int>(24 * heightRatio);
//    wStatusIconButton->setFixedSize(statusIconWidth, statusIconHeight);

//    wStatusIconButton->setFlat(true);
//    wStatusIconButton->setStyleSheet("QPushButton {"
//                                     "border: none;"
//                                     "border-image: url(:/image/FloatingWindow/new_information.png);"
//                                     "}");
//    connect(wStatusIconButton, &QPushButton::clicked, this, &SpeedBallWidget::onWStatusIconButtonClicked);

//    mainLayout->addWidget(wStatusIconButton, 0, Qt::AlignHCenter);
//    qDebug() << "当前: "; qDebug() << "5379";

//    // 中间头像
//    centerImageLabel = new QLabel(this);
//    int centerImageWidth = static_cast<int>(75 * widthRatio);
//    int centerImageHeight = static_cast<int>(75 * heightRatio);
//    centerImageLabel->setFixedSize(centerImageWidth, centerImageHeight);
//    // 根据性别设置图片路径
//    QString genderImagePath = kGender
//            ? ":/image/FloatingWindow/normal_face_female.png"
//            : ":/image/FloatingWindow/normal_face_male.png";

//    // 设置样式表
//    QString styleSheet = QString("QLabel {"
//                                 "border-image: url(%1);"
//                                 "}")
//            .arg(genderImagePath);


//    // 应用样式表到 QLabel
//    centerImageLabel->setStyleSheet(styleSheet);

//    mainLayout->addWidget(centerImageLabel, 0, Qt::AlignHCenter);

//#ifdef BETA_VERSION
//    // 内测功能代码
//    cameraView = new SimpleCameraView(this);
//    cameraView->hide();  // 初始隐藏摄像头画面
//#else
//    // 正式功能代码
//#endif

//    // 时间按钮
//    wTimeButton = new QPushButton(jss->getTimeLabel(),this);


//    cameraButtonWidth = static_cast<int>(70 * widthRatio);
//    cameraButtonHeight = static_cast<int>(17 * heightRatio);
//    wTimeButton->setFixedSize(cameraButtonWidth, cameraButtonHeight);

//    wTimeButton->setFlat(true);
//    wTimeButton->setStyleSheet("QPushButton {"
//                               "border: none;"
//                               "color: rgb(255, 255, 255);;"
//                               "}");
//    mainLayout->addWidget(wTimeButton, 0, Qt::AlignHCenter);
//    // 开始工作按钮
//    wStartButton = new QPushButton(this);
//    // 设置文本
//    wStartButton->setText(START_WORK);
//    wStartButton->setFixedSize(cameraButtonWidth, cameraButtonHeight);

//    connect(wStartButton, &QPushButton::clicked, this, &SpeedBallWidget::onWStartButtonClicked);

//    wStartButton->setStyleSheet("QPushButton {"
//                                "border-image: url(:/image/FloatingWindow/button_background2.png);"
//                                "border: none;"
//                                "}");
//    // 设置图标
//    QIcon icon("./image/FloatingWindow/wsuspension_of_work.png"); // 这里使用了资源文件路径
//    wStartButton->setIcon(icon);
//    if (icon.isNull()) {
//        qDebug() << "Icon loading failed!";
//    }else{
//        qDebug() << "Icon loading !";
//    }
//    // 调整图标大小
//    wStartButton->setIconSize(QSize(15, 15)); // 设置图标的大小为32x32像素
//    mainLayout->addWidget(wStartButton, 0, Qt::AlignHCenter);


//    // 底部相机按钮
//    wEndButton = new QPushButton(this);
//    wEndButton->setText(STOP_WORK);

//    wEndButton->setFixedSize(cameraButtonWidth, cameraButtonHeight);


//    // 调整图标大小
//    wEndButton->setIconSize(QSize(15, 15)); // 设置图标的大小为32x32像素
//    wEndButton->setFixedSize(cameraButtonWidth, cameraButtonHeight);
//    wEndButton->setStyleSheet("QPushButton {"
//                              "border-image: url(:/image/FloatingWindow/button_background2.png);"
//                              "border: none;"
//                              "}");

//    connect(wEndButton, &QPushButton::clicked, this, &SpeedBallWidget::onWEndButtonClicked);
//    // 设置图标
//    QIcon icon2("./image/complete.png"); // 这里使用了资源文件路径
//    if (icon2.isNull()) {
//        qDebug() << "Icon2 loading failed!";
//    }
//    wEndButton->setIcon(icon2);
//    mainLayout->addWidget(wEndButton, 0, Qt::AlignHCenter);
//    // 底部相机按钮
//    cameraButton = new QPushButton(this);
//    int cameraButtonWidth = static_cast<int>(24 * widthRatio);
//    int cameraButtonHeight = static_cast<int>(19 * heightRatio);
//    cameraButton->setFixedSize(cameraButtonWidth, cameraButtonHeight);
//    QPixmap cameraButtonPixmap(":/image/FloatingWindow/face_camera.png");
//    cameraButton->setIcon(QIcon(cameraButtonPixmap));
//    cameraButton->setIconSize(QSize(cameraButtonWidth, cameraButtonHeight));
//    cameraButton->setFlat(true);
//    connect(cameraButton, &QPushButton::clicked, this, &SpeedBallWidget::onCameraButtonClicked);
//    mainLayout->addWidget(cameraButton, 0, Qt::AlignHCenter);
//#ifdef BETA_VERSION
//    // 内测功能代码

//#else
//    // 正式功能代码
//    cameraButton->hide();
//#endif
//    setEdgeColor(Qt::green);



//}


// 日志输出函数
void SpeedBallWidget::logMessage(const QString &message) {
    saveLog->save(message);
}
void SpeedBallWidget::startButtonText(const QString &btnText) {
    if(btnText=="开始工作"){
        QIcon icon(":/image/FloatingWindow/wsuspension_of_work.png");
        wStartButton->setIcon(icon);
        wStartButton->setIconSize(QSize(15, 15));
    }else if(btnText=="暂停工作"){
        QIcon icon(":/image/FloatingWindow/wstart_working.png");
        wStartButton->setIcon(icon);
        wStartButton->setIconSize(QSize(15, 15));
    }
    wStartButton->setText(btnText);
}
void SpeedBallWidget::checkAutoDock() {
    if (!isDocked) {

        int mheight = static_cast<int>(150 * heightRatio);
        this->setFixedSize(wScaledWidth,mheight);
        logMessage("未检测到鼠标操作，窗口将自动贴边");
        wTimeButton->hide();

        // 获取当前屏幕的几何信息
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();

        // 计算窗口到左右两边的距离
        int leftDistance = frameGeometry().left();   // 距离左边的距离
        int rightDistance = screenGeometry.width() - frameGeometry().right(); // 距离右边的距离

        // 选择距离最近的边进行贴边
        const int margin = 10;  // 边距设置为10像素

        // 获取当前的窗口高度 (可以根据需要调整，通常会保持不变)
        int currentY = frameGeometry().top(); // 获取当前窗口的 Y 坐标（即距离屏幕上边的距离）

        if (leftDistance < rightDistance) {
            // 如果窗口距离左边近，则贴到左边
            move(margin, currentY); // 只修改 X 坐标，Y 坐标保持不变
        } else {
            // 如果窗口距离右边近，则贴到右边
            move(screenGeometry.width() - edgeMarginWidth - margin, currentY); // 只修改 X 坐标，Y 坐标保持不变
        }
        qDebug() << "wStatusIconButton->width(): "; qDebug() << wStatusIconButton->width();
        qDebug() << "wStartButton->width(): "; qDebug() << wStartButton->width();
        wStatusIconButton->setStyleSheet("QPushButton {"
                                         "border: none;"
                                         "border-image: url(:/image/FloatingWindow/new_information.png);"
                                         "}");

        // 将 QColor 转换为字符串格式
        QString colorStyle = QString("QLabel {"
                                     "border: none;"
                                     "}");

        // 应用样式
        centerImageLabel->setFixedSize(wStatusIconButton->width(),wStartButton->height());
        centerImageLabel->setStyleSheet(colorStyle);
        wStartButton->setText("");
        wStartButton->setFixedSize(wStatusIconButton->width(),wStartButton->height());
        wEndButton->setText("");
        wEndButton->setFixedSize(wStatusIconButton->width(),wStartButton->height());

        // 为了加边缘条，修改窗口的外部边框宽度
        //        setFixedSize(edgeMarginWidth + 6, height());  // 为了边缘条，宽度增加6像素
        setFixedSize( edgeMarginWidth+ 10, height());  // 增加额外的宽度以显示圆角
        isDocked = true;
        update();  // 更新界面
    }
}


// 鼠标进入和离开事件，用于控制贴边功能
void SpeedBallWidget::enterEvent(QEvent *event) {
    Q_UNUSED(event);
    if (isDocked) {

        // 恢复窗口到原始大小
       setScaledFixedSize(this,100 , 260);
        wStartButton->setFixedSize(cameraButtonWidth,cameraButtonHeight);
        int centerImageWidth = static_cast<int>(75 * widthRatio);
        int centerImageHeight = static_cast<int>(75 * heightRatio);

        centerImageLabel->setFixedSize(centerImageWidth, centerImageHeight);
        if (edgeColor.isValid()) {
            qDebug() << "Current Color:" << edgeColor.name();
        } else {
            qDebug() << "Color is not set!";
        }

        if (edgeColor.name() == "#ff0000") {

            QString genderImagePath = kGender ? ":/image/FloatingWindow/out-of-picture_face_female.png"
                                              : ":/image/FloatingWindow/out-of-picture_face_male.png";

            // 应用样式表到 QLabel
            centerImageLabel->setStyleSheet(QString("QLabel {"
                                                    "border-image: url(%1);"
                                                    "}")
                               .arg(genderImagePath));


           } else if ( edgeColor.name() == "#00ff00") {
            QString genderImagePath = kGender ? ":/image/FloatingWindow/normal_face_female.png"
                                              : ":/image/FloatingWindow/normal_face_male.png";
            // 设置样式表
            QString styleSheet = QString("QLabel {"
                                         "border-image: url(%1);"
                                         "}")
                    .arg(genderImagePath);

            // 应用样式表到 QLabel
            centerImageLabel->setStyleSheet(styleSheet);
           }
//        else {
//              qDebug() << "当前: "; qDebug() << "5264";
//                QString genderImagePath = kGender ? ":/image/FloatingWindow/blurred_face_female.png"
//                                            : ":/image/FloatingWindow/blurred_face_male.png";
//                      // 应用样式表到 QLabel
//                      centerImageLabel->setStyleSheet(QString("QLabel {"
//                                                              "border-image: url(%1);"
//                                                              "}")
//                                         .arg(genderImagePath));
//           }



        wEndButton->setFixedSize(cameraButtonWidth,cameraButtonHeight);

        wStartButton->setText(jss->getBtnTest());
        wEndButton->setText(STOP_WORK);



        // 获取屏幕的可用区域，避免任务栏遮挡
        QScreen *screen = QApplication::primaryScreen();
        QRect screenGeometry = screen->availableGeometry(); // 获取屏幕的可用区域

        // 获取当前窗口的位置，确保不会超出屏幕
        int xPos = qMin(screenGeometry.width() - width(), frameGeometry().left());  // 如果窗口超出右边界，调整位置
        int yPos = qMin(screenGeometry.height() - height(), frameGeometry().top()); // 如果窗口超出下边界，调整位置

        move(xPos, yPos);  // 设置恢复后的窗口位置
        wTimeButton->show();
        wStartButton->setText(jss->getBtnTest());
        wEndButton->setText(STOP_WORK);
        isDocked = false;  // 设置为未贴边状态
        logMessage("检测到鼠标进入，窗口恢复正常大小");
    }
    dockTimer->start(dockDelay);  // 重置贴边定时器
}

void SpeedBallWidget::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    dockTimer->start(dockDelay);  // 鼠标离开后重启贴边定时器
}

// 闪烁控制
void SpeedBallWidget::startFlashing(int interval) {
    flashMode = true;
    flashTimer->start(interval);
}

void SpeedBallWidget::stopFlashing() {
    flashMode = false;
    flashTimer->stop();
    flashOn = true;
    update();
}

QPixmap SpeedBallWidget::getPixmap() {
    return cameraView->getPixmap();
}

// 设置中心图像
void SpeedBallWidget::setImage(const QString &imagePath) {
    centerImage.load(imagePath);
    centerImageLabel->setPixmap(QPixmap(imagePath).scaled(200, 200, Qt::KeepAspectRatio));
}

// 设置悬浮球状态
void SpeedBallWidget::setBallState(int ballState, double faceFatching) {
    Q_UNUSED(faceFatching);
    QString genderImage;
    QPixmap statusIconPixmap;
qDebug() << "isDocked: "; qDebug() << isDocked;

    switch (ballState) {
    case 1:  // 正常
        if (isDocked) {
            centerImageLabel->setStyleSheet("QLabel {"
                                                "border: none;"

                                                "}");
        }else{


        genderImage = kGender ? ":/image/FloatingWindow/normal_face_female.png"
                              : ":/image/FloatingWindow/normal_face_male.png";


        // 应用样式表到 QLabel
        centerImageLabel->setStyleSheet(QString("QLabel {"
                                                "border-image: url(%1);"
                                                "}")
                           .arg(genderImage));
       }
        kinCamera = 1;
        kisSelf = 1;
        setEdgeColor(Qt::green);
        if (FSG->isVisible()) FSG->hide();

        break;

    case 2:  // 模糊
        genderImage = kGender ? ":/image/FloatingWindow/blurred_face_female.png"
                              : ":/image/FloatingWindow/blurred_face_male.png";
        // 应用样式表到 QLabel
        centerImageLabel->setStyleSheet(QString("QLabel {"
                                                "border-image: url(%1);"
                                                "}")
                           .arg(genderImage));
        kinCamera = 1;
        kisSelf = 0;
        setEdgeColor(Qt::yellow);
        if (FSG->isVisible()) FSG->hide();
        break;

    case 3:  // 没有人脸
        if (isDocked) {
            centerImageLabel->setStyleSheet("QLabel {"
                                                "border: none;"

                                                "}");
        }else{
        genderImage = kGender ? ":/image/FloatingWindow/out-of-picture_face_female.png"
                              : ":/image/FloatingWindow/out-of-picture_face_male.png";
        // 应用样式表到 QLabel
        centerImageLabel->setStyleSheet(QString("QLabel {"
                                                "border-image: url(%1);"
                                                "}")
                           .arg(genderImage));
        }
        kinCamera = 2;
        kisSelf = 2;
        setEdgeColor(Qt::red);
        utils->showToast("检测你己经一段时间未操作，正处于离岗状态，请及时操作键盘或鼠标");
        logMessage("检测你己经一段时间未操作，正处于离岗状态，请及时对电脑做出操作");
        FSG->showForDuration(3000);

        break;

    default:  // 默认情况
        genderImage = kGender ? ":/image/FloatingWindow/blurred_face_female.png"
                              : ":/image/FloatingWindow/blurred_face_male.png";
        centerImageLabel->setStyleSheet(QString("QLabel {"
                                                "border-image: url(%1);"
                                                "}")
                           .arg(genderImage));
        kinCamera = 1;
        kisSelf = 0;
        setEdgeColor(Qt::yellow);
        if (FSG->isVisible()) FSG->hide();
//        logMessage("检测到人脸识别异常，5秒后将重新识别");
        break;
    }

//    // 设置图像
//    centerImage.load(genderImage);
//    int centerImageWidth = static_cast<int>(75 * widthRatio);
//    int centerImageHeight = static_cast<int>(75 * heightRatio);
//    centerImageLabel->setPixmap(centerImage.scaled(centerImageWidth, centerImageHeight, Qt::KeepAspectRatio));
//    int statusIconWidth = static_cast<int>(25 * widthRatio);
//    int statusIconHeight = static_cast<int>(24 * heightRatio);
//    wStatusIconButton->setFixedSize(statusIconWidth, statusIconHeight);
}

// 设置离开状态
void SpeedBallWidget::setBallLeaveState(int ballState) {
    if (ballState == 1) {
        QString genderImage = kGender ? ":/image/FloatingWindow/out-of-picture_face_female.png"
                                      : ":/image/FloatingWindow/out-of-picture_face_male.png";
        centerImage.load(genderImage);
        int centerImageWidth = static_cast<int>(75 * widthRatio);
        int centerImageHeight = static_cast<int>(75 * heightRatio);
        centerImageLabel->setPixmap(centerImage.scaled(centerImageWidth, centerImageHeight, Qt::KeepAspectRatio));
        //        statusIconLabel->setPixmap(QPixmap(":/image/FloatingWindow/out-of-picture_face_logo.png").scaled(60, 60, Qt::KeepAspectRatio));

        kinCamera = 2;
        kisSelf = 2;
        setEdgeColor(Qt::red);

        FSG->showForDuration(1000);
        logMessage("人脸识别异常，未检测到人脸，将在1秒后重新识别");
    }
}

// 设置边缘颜色
void SpeedBallWidget::setEdgeColor(const QColor &color) {
    if (edgeColor != color) {
        edgeColor = color;
        update();  // 更新界面
    }
}

// 绘制事件
void SpeedBallWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    // 创建 QPainter 对象，用于绘制窗口内容
    QPainter painter(this);

    // 开启抗锯齿，确保绘制出来的图形平滑
    painter.setRenderHint(QPainter::Antialiasing);

    // 创建 QPainterPath 对象，用来绘制背景矩形
    QPainterPath path;

    // 设置矩形的宽度为 150 像素，宽度可以根据实际需要调整
    int rectWidth = backgroundWidth;

    // 计算矩形的 X 坐标，使其居中对齐
    int rectX = (width() - rectWidth) / 2;

    // 绘制一个圆角矩形，矩形的宽度为 rectWidth，高度为窗口的高度
    // 圆角半径为 30 像素
    path.addRoundedRect(rectX, 0, rectWidth, height(), 30, 30);

    // 使用半透明的黑色填充矩形背景，颜色的透明度设置为 200（取值范围 0-255）
    painter.fillPath(path, QColor(0, 0, 0, 200));

    // 如果窗口已经贴边，绘制外部的边缘条
    if (isDocked) {
        // 设置外部边缘的颜色和宽度
        QPen edgePen(edgeColor, 6);  // 设置边缘条宽度为 10 像素
        painter.setPen(edgePen);

        // 绘制窗口外部的圆角边缘条
        // 这里的圆角半径应与背景矩形一致，确保圆角效果一致
        painter.drawRoundedRect(rect(), 30, 30);  // 使用圆角矩形绘制外边缘，圆角半径与背景一致
    }
    // 设置边框颜色和宽度
    QPen pen(edgeColor, 6);  // 边框宽度为 6 像素
    painter.setPen(pen);

    // 设置填充颜色（例如和边框颜色一致，也可以使用其他颜色）
    QBrush brush(edgeColor); // 实心部分颜色为 edgeColor
    painter.setBrush(brush);

    // 获取圆形半径
    int circleRadius = centerImageLabel->width() / 2 + pen.width() / 2;

    // 圆形的中心点
    QPoint circleCenter(width() / 2, centerImageLabel->y() + centerImageLabel->height() / 2);

    // 创建 QPainterPath 对象
    QPainterPath circlePath;
    circlePath.addEllipse(circleCenter, circleRadius, circleRadius);

    // 填充圆形路径
    painter.fillPath(circlePath, brush);

    // 绘制圆形边框
    painter.drawPath(circlePath);

    //空心圆
    //    // 设置内部圆形的颜色和宽度
    //    QPen pen(edgeColor, 6);  // 设置圆形的边框宽度为 6 像素
    //    painter.setPen(pen);

    //    // 获取圆形半径，半径为 centerImageLabel 的宽度的一半，加上画笔宽度的一半
    //    int circleRadius = centerImageLabel->width() / 2 + pen.width() / 2;

    //    // 圆形的中心点是 centerImageLabel 的中心位置，这里通过 centerImageLabel 的位置和大小来计算
    //    QPoint circleCenter(width() / 2, centerImageLabel->y() + centerImageLabel->height() / 2);

    //    // 创建一个 QPainterPath 对象，用来绘制圆形
    //    QPainterPath circlePath;

    //    // 在计算得到的圆心位置绘制圆形，半径为 circleRadius
    //    circlePath.addEllipse(circleCenter, circleRadius, circleRadius);

    //    // 使用画笔绘制圆形路径
    //    painter.drawPath(circlePath);
}
#include <QMenu>
#include <QFileDialog>



void SpeedBallWidget::setupContextMenuForImageLabel() {
    // 假设 centerImageLabel 是 SpeedBallWidget 中的 QLabel 对象
    centerImageLabel->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(centerImageLabel, &QLabel::customContextMenuRequested,
            this, &SpeedBallWidget::onCenterImageLabelRightClick);
}

void SpeedBallWidget::onCenterImageLabelRightClick(const QPoint &pos) {
    // 创建右键菜单
    //    QMenu contextMenu(this);

    // 添加 "选择头像" 菜单项
    //    QAction *selectAvatarAction = contextMenu.addAction("选择头像");

    // 连接点击事件
    //    connect(selectAvatarAction, &QAction::triggered, this, &SpeedBallWidget::onSelectAvatarClicked);

    // 弹出菜单
    //    contextMenu.exec(centerImageLabel->mapToGlobal(pos));
}

//void SpeedBallWidget::paintEvent(QPaintEvent *event) {
//    Q_UNUSED(event);

//    // 创建 QPainter 对象，用于绘制窗口内容
//    QPainter painter(this);

//    // 开启抗锯齿，确保绘制出来的图形平滑
//    painter.setRenderHint(QPainter::Antialiasing);

//    // 创建 QPainterPath 对象，用来绘制矩形
//    QPainterPath path;

//    // 设置矩形的宽度为 150 像素，宽度可以根据实际需要调整
//    int rectWidth = 150;
//    // 计算矩形的 X 坐标，使其居中对齐
//    int rectX = (width() - rectWidth) / 2;

//    // 绘制一个圆角矩形，矩形的宽度为 rectWidth，高度为窗口的高度
//    // 圆角半径为 30 像素
//    path.addRoundedRect(rectX, 0, rectWidth, height(), 30, 30);

//    // 使用半透明的黑色填充矩形背景，颜色的透明度设置为 200（取值范围 0-255）
//    painter.fillPath(path, QColor(0, 0, 0, 200));

//    // 如果窗口已经贴边，绘制外部的边缘条
//    if (isDocked) {
//        // 设置外部边缘的颜色和宽度
//        QPen edgePen(edgeColor, 10);  // 设置边缘条宽度为 6 像素
//        painter.setPen(edgePen);

//        // 绘制窗口外部的边缘条
//        painter.drawRoundedRect(rect(), 10, 10);  // 使用圆角矩形绘制外边缘

//    }

//    // 设置内部圆形的颜色和宽度
//    QPen pen(edgeColor, 6);  // 设置圆形的边框宽度为 6 像素
//    painter.setPen(pen);

//    // 获取圆形半径，半径为 centerImageLabel 的宽度的一半，加上画笔宽度的一半
//    int circleRadius = centerImageLabel->width() / 2 + pen.width() / 2;

//    // 圆形的中心点是 centerImageLabel 的中心位置，这里通过 centerImageLabel 的位置和大小来计算
//    QPoint circleCenter(width() / 2, centerImageLabel->y() + centerImageLabel->height() / 2);

//    // 创建一个 QPainterPath 对象，用来绘制圆形
//    QPainterPath circlePath;
//    // 在计算得到的圆心位置绘制圆形，半径为 circleRadius
//    circlePath.addEllipse(circleCenter, circleRadius, circleRadius);

//    // 使用画笔绘制圆形路径
//    painter.drawPath(circlePath);
//}


// 切换闪烁状态
void SpeedBallWidget::toggleFlash() {
    flashOn = !flashOn;
    update();
}

// 相机按钮点击事件
void SpeedBallWidget::onCameraButtonClicked() {
#ifdef BETA_VERSION
    // 内测功能代码
    if (cameraView->isVisible()) {
        cameraView->hide();
        centerImageLabel->setPixmap(centerImage.scaled(200, 200, Qt::KeepAspectRatio));
    } else {
        cameraView->show();
        cameraView->setGeometry(centerImageLabel->geometry());
        cameraView->raise();
        centerImageLabel->clear();
    }
#else
    // 正式功能代码

#endif


}

int SpeedBallWidget::getnumberOfNotifications()
{
   return    wStatusIconButton->text().toInt();
}
void SpeedBallWidget::onWStatusIconButtonClicked()
{

    int wStartButtonInt = wStatusIconButton->text().toInt();
    qDebug() << "wStartButtonInt: "; qDebug() << wStartButtonInt;

    if(wStartButtonInt>0){

        iu->getNotifications();
    }else{

        utils->showToast("暂无通知");

    }
}

void SpeedBallWidget::onWStartButtonClicked()
{
    jss->on_commence_btn_clicked();

}
void SpeedBallWidget::onWEndButtonClicked()
{
    if (iu->isVisible()) {
        utils->adjustWindowFlags(iu);

        qDebug() << "当前: "; qDebug() << "3746";

    }
    jss->on_stop_work_btn_clicked();

}
// 鼠标事件处理
void SpeedBallWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }

}

void SpeedBallWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - dragPosition);
        dockTimer->start(dockDelay);  // 重置贴边定时器
        event->accept();
    }
}
