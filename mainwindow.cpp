#include "mainwindow.h"
#include "ui_mainwindow.h"
//工具类头文件
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
//主窗口构造器
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    count(0),
    flag(1)
{
    ui->setupUi(this);
    cout << "in MainWindow " << endl;
    initVideoCapture();
    initCascadeClassifier();
    initFaceRecognizer();
    initNameLabels();
    //启动人脸识别刷新定时器，获得定时器标识符start_timerID
    start_timerID = startTimer(50);
    //启动预测人脸定时器，获得定时器标识符check_timerID
    check_timerID = startTimer(2000);

}
//(6)实现找脸模块
bool MainWindow::findFace()
{
    qDebug() << "find Face ing!" << endl;
    //创建Mat对象，存储灰度8bit图片内容
    Mat gray;
    //vc：多媒体对象 读取 摄像头
    //frame：通过多媒体对象读取的Mat图片数据
    vc >> frame;
    //沿着y轴反转frame
    flip(frame,frame,1);
    //参数1：输入图片
    //参数2：输出图片
    //参数3：y轴反转
    //更改色彩空间，更改图片的通道3->1，bgr -> gray CV_BGR2GRAY
    cvtColor(frame,gray,CV_BGR2GRAY);
    //直方图均衡化，数据明显标记
    equalizeHist(gray,gray);
    //通过级联分类器对象classifier通过detectMultiScale人脸识别
    classifier.detectMultiScale(gray,faces,1.1,5,0,Size(90,90),Size(300,300));
    //参数1：输入图片
    //参数2：存储人脸的矩形的区域
    //参数3：缩放的比例
    //参数4：像素集合
    //参数5：不用Canny算法检测
    //参数6：最小尺寸
    //参数7：最大尺寸
    qDebug() << "face : size : " << faces.size();
    //获得人脸矩形区域的大小size()
    return faces.size() > 0;
}

//(10)人脸预测函数(考勤模块)
void MainWindow::checkFace()
{
    //QFile封装文件E:/FaceSignInProjecttest/face.xml
    QFile file("D:/FaceSignInProject/face.xml");
    //通过exists()方法判断文件是否存储，存在返回true, 否则是false
    if(file.exists() == true){
        //faces.empty() 判断是否识别到人脸，是返回true ,否返回false
        //recognizer->empty() 判断人脸识别器是否存在，是返回true，否返回false
        if(faces.empty() || recognizer->empty())
        {
            return;
        }
        //根据人脸位置faces[0]通过frame Mat图片对象获得矩形区域人脸 给 face 存储

        Mat face = frame(faces[0]);
        //更改色彩空间是灰度gray
        cvtColor(face,face,CV_BGR2GRAY);
        //设置人脸的大小
        cv::resize(face,face,Size(100,100));
        //人脸对应的标签，唯一的标示
        int label = -1;
        //图片的信任度
        double confidence = 0;
        //通过predict根据face人脸图片预测人脸，参数1 label是人脸对应的标签，confidence是识别的信任度
        recognizer->predict(face,label,confidence);
        qDebug()<< "label : " << label;
        qDebug()<<"confidence : " << confidence;
        if(label != -1)
        {//识别成功
            //获得当前的时间
            QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            //QString字符串工具类
            //time字符串工具类的对象
            //currentDateTime()获得当前的日期时间
            //toString获得时间的字符串
            //"yyyy-MM-dd HH:mm:ss"输出的时间的格式2020-5-28 15:37:11

            //通过消息盒子显示签到的人员姓名和时间
            QMessageBox::warning(this,namelabels[label]+" 签到 ",time);
            //this：主窗口对象
            //namelabels[label] - 学生的名字
            //label - 学生名字
            //1 - "学生1"
            //time:内容字符串
            saveSignInTime(label,time);
        }

    }
}
//获得记录文件namelabels.txt内容，进行关联容器map<int,QString> namelabels初始化
//(4)已经录入打卡机的学生的信息
void MainWindow::initNameLabels()
{   //封装namelabels.txt文件，获得文件的QFILe对象
    QFile file("D:/FaceSignInProject/namelabels.txt");
    //通过open方法通过ReadOnly方式打开文件namelabels.txt，成功返回true，否则false
    if(file.open(QIODevice::ReadOnly) == false)
    {
        //输出调试信息
        qDebug()<<"open namelabels.txt failed!!";
        return;
    }
    qDebug()<<"打开namelabels.txt成功！！！";
    //获得读取文件的文本流对象stream
    QTextStream stream(&file);
    //参数1：文件对象的指针
    //循环读取数据，判断是否结束的标志方法atEnd，结束返回true，否则返回false
    while(stream.atEnd() == false)
    {   //通过readLine方法读取文件的一行
        //获得文件的一行后给line赋值
        QString line = stream.readLine();
        //通过qDebug()输出调试信息line
        qDebug()<<line;
        //line ---》 "1,任" ---》 "1" "任"
        //通过","拆分line获得String对象赋值给QStringList对象存储
        QStringList temp = line.split(",");
        //temp[0] = "1", temp[1] = "任"
        //temp[0].toInt() 将"1"转换为整数1
        //初始化关联容器namelabels,容器存储录入打卡机的人员的名单
        namelabels[temp[0].toInt()] = temp[1];//1-“任”，temp[0].toInt() - temp[1]
    }
    //关闭流文件，释放资源
    file.close();
}

//(3)初始化人脸识别器对象
void MainWindow::initFaceRecognizer()
{
    //封装人脸的模型配置文件face.xml
    QFile file("D:/FaceSignInProject/face.xml");
    //判断，验证模型配置文件是否存在，存在true，否则false
    if(file.exists() == true){
        //通过load方法加载人脸模型配置文件face.xml，创建识别器对象（可以识别特定人脸）
        recognizer = FaceRecognizer::load<LBPHFaceRecognizer>("D:/FaceSignInProject/face.xml");
        qDebug()<< "get face.xml ok";
    }else{
        //配置文件不存在，通过create方法创建新的识别器对象
        recognizer = LBPHFaceRecognizer::create();
        qDebug()<< "face.xml not existing";
    }
    recognizer->setThreshold(100);
    qDebug()<<"创建人脸识别器成功！！";
}

//(2)初始化级联分类器对象，后续识别使用
void MainWindow::initCascadeClassifier(){
    //通过构造器CascadeClassifier()创建(初始化)级联分类器对象
    //级联分类器加载的配置文件haarcascade_frontalface_alt2.xml
    classifier = CascadeClassifier("D:/opencv/opencv3.4-qt-install/install/etc/haarcascades/haarcascade_frontalface_alt2.xml");
    //判断，验证， empty() == true空，empty() == false非空
    if(classifier.empty() == true){
        cout << "classifier error" << endl;
        return;
    }else{
        cout << "classifier ok" << endl;
    }
}

//(1)摄像头初始化，打开多媒体设备
//MainWindow主窗口
//::属于
void MainWindow::initVideoCapture(){
   //通过多媒体对象vc通过open方法打开摄像头0，成功true，失败false
   //判断验证是否正常打开
   if(vc.open(0) == false){
       //输出信息
       cout << "vc error" << endl;
       return ;
   }else{
       cout << "vc ok" << endl;
   }
}
//(7)绘制人脸Rect矩形区域
void MainWindow::drawFace()
{
    //根据faces集合内容循环绘制人脸
    for(int i = 0; i < faces.size(); i++)
    {
        //自定义视图，rectangle绘制人脸矩形
        rectangle(frame,faces[i],Scalar(0,0,255), 3, LINE_8, 0);
        //frame：输入图片
        //faces[i]：矩形区域的尺寸
        //Scalar：颜色
        //3:粗细
        //LINE_8：直线类型
        //0：坐标没有小数
        qDebug() << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~rectangle i : " << i << endl;

    }
    //通过count识别是否进行机器学习，如果count是非零就是在学习呢，否则空闲
    if(count)//非0即为真，非0说明正采集人脸
    {   //左上角显示文字
        //自定义视图putText
        putText(frame,QString("%1").arg(count).toStdString(),faces[0].tl(),FONT_ITALIC,2,Scalar(0,0,255));
        //参数1：输入图片
        //参数2：显示的QString字符串对象，QString内容"%1"，
        //%1占位符，arg是代表参数1，
        //count参数1的内容，count是样本的个数
        //toStdString()方法是转化为字符串对象
        //faces[0]:识别好的人脸的矩形区域Rect对象
        //tl():显示文字到Rect矩形区域的左上角
        //FONT_ITALIC:字体
        //2:文字大小
        //Scalar:颜色
    }
}
//(8)人脸绘制到qt界面模块
void MainWindow::showFrame()
{
    //创建Mat对象存储rgb Mat对象
    Mat showframe;
    //设置Label标签的尺寸400*400
    //QSize封装尺寸的工具类
    ui->label_show->resize(QSize(400,400));
    //设置图片的大小尺寸
    //Size尺寸工具类
    cv::resize(frame,showframe,Size(400,400));
    //frame：输入图片
    //showframe：输出图片
    //Size：尺寸对象 400*400
    //更改色彩空间bgr - rgb
    cvtColor(showframe,showframe,CV_BGR2RGB);
    //QImage::Format_RGB888，存入格式为R, G, B
    //opencv Mat showframe - >QT QImage img
    QImage img(showframe.data,showframe.cols,showframe.rows,showframe.cols*showframe.channels(),QImage::Format_RGB888);
    //参数1：rgb Mat对象data数据
    //参数2：rgb Mat对象的列数
    //参数3：rgb Mat对象的行数
    //参数4：总列数(*通道)
    //参数5：图片编码格式rgb
    //setPixmap方法设置QImage到Label里面
    ui->label_show->setPixmap(QPixmap::fromImage(img));
    //QPixmap：绘制图片的工具类
    //fromImage：指定QPixmap的图片源头
    //img：QImage对象
    //setPixmap：设置QPixmap图片对象方法
    //label_show：UI Label标签
    //ui：主窗口MainWindow指针变量
    qDebug() << "showFrame ))))))))))))))ing" ;

}
//(5)定时器超时处理函数
//参数1：标示超时事件指针变量
void MainWindow::timerEvent(QTimerEvent *event){
    //timerId（）获得当前超时事件的定时器的标示
    //判断是否是start_timerID标示的定时器超时发出事件
    if(start_timerID == event->timerId()){
        qDebug() << "############start_timerID  绘制摄像头数据 定时器触发";
        //找人脸位置
        if(findFace()){
            qDebug() << "############start_timerID findFace " << endl;
            //绘制人脸（参考人脸的矩形区域）
            drawFace();
        }
        //显示人脸到qt界面里面观看
        showFrame();
    }
    //学习定时器超时事件触发
    //study_timerID学习定时器id标识符
    if(study_timerID == event->timerId()){
        qDebug() << "**********************************************************study_timerID 机器学习 定时器触发";
        //结合样本进行机器学习
        studyFace();
    }
    //预测定时器超时事件触发
    //check_timerID预测定时器id标识符
    if(check_timerID == event->timerId()){
        qDebug() << "$$$$$$$$$$$$$$$$$$$$$$$$check_timerID  人脸预测 定时器触发";
        //当flag是1的时候进行
        if(flag == 1){
            qDebug() << "-----------------------------------------------------check_timerID  人脸预测";
            //预测人脸
            checkFace();
        }
    }
}
//(9)人脸学习模块
void MainWindow::studyFace()
{
    //通过级联分类器识别到人脸，人脸矩形区域存储在faces的线性集合里面
    //定义vector<Rect> faces
    //定义存储人脸区域的Mat对象face
    Mat face;
    //检测，查看人脸的矩形区域的是否是空，空返回true, 否则是false
    if(faces.empty() == true)
        return;
    //通过人脸的矩形区域获得人脸，给face赋值
    face = frame(faces[0]);
    //faces[0]当前的人脸的矩形的区域
    //设置图片对象的对象100*100
    cv::resize(face,face,Size(100,100));
    //cv:opencv的命名空间,代表opencv的库
    //参数1：输入图片
    //参数2：输出图片
    //参数3：图片的尺寸
    //更改色彩空间获得灰度图片(8位)
    cvtColor(face,face,CV_BGR2GRAY);
    //参数1：输入图片
    //参数2：输出图片
    //参数3：标示
    //通过push_back方法填充人脸灰度图片到线性集合数组里面存储
    study_faces.push_back(face);
    //参数1:Mat对象
    //通过push_back方法填充标签到线性结合数组里面存储
    study_labels.push_back(namelabels.size()+1);
    //namelabels.size():关联集合map数组的大小
    //namelabels.size()+1:新加入打卡机的学生的标签（新加入的学生！）
    //输出调试信息
    qDebug()<<count;
    //记录采集的样本的数目
    count++;
    //判断是否采集了10个图片
    if(count == 10)
    {
        //清理计数，后续还要使用
        count = 0;
        //设置按键是激活状态
        ui->pushButton_study->setEnabled(true);
        //ui：主窗口指针
        //pushButton_study：主窗口里面的按键名称
        //setEnabled：设置为激活，true使能
        //学习(训练)模型并且更新模型
        recognizer->update(study_faces,study_labels);
        //参数1：要训练的Mat采样线性集合
        //参数2：要训练的标签线性集合
        //存储训练好的模型face.xml
        recognizer->save("D:/FaceSignInProject/face.xml");
        //通过killTimer()方法结合study_timerID关闭学习定时器
        killTimer(study_timerID);
        //显示消息窗口
        QMessageBox::warning(this,"note","insert ok");
        //参数1：主窗口MainWindow对象（this）
        //参数2：标题
        //参数3：内容
        //获得编辑框的内容
        QString name = ui->lineEdit_name->text();
        //ui:主窗口指针
        //lineEdit_name:编辑框名字
        //text():编辑框的内容
        //判断是否写入姓名，size（）==0没有输入人名
        if(name.size() == 0)
        {   //显示警告框
            QMessageBox::warning(this,"note","input your name");
            return;
        }
        //将人名存储到关联集合map里面存储
        namelabels[namelabels.size()+1] = name;
        //输出调试信息
        qDebug() << "name : " << name;
        //存储关联集合map内容到文件namelabels.txt
        saveNameLabels();
        //更改标志位，让机器识别或预测人脸
        flag = 1;

    }

}
//(4)初始化已录考勤姓名标签模块
void MainWindow::saveNameLabels()
{   //E:/FaceSignInProjecttest/namelabels.txt存储录入人脸的学生的名字和标签的对应的关系
    //QFile封装"E:/FaceSignInProjecttest/namelabels.txt"，获得file代表E:/FaceSignInProjecttest/namelabels.txt
    QFile file("D:/FaceSignInProject/namelabels.txt");
    //打开文件E:/FaceSignInProjecttest/namelabels.txt
    if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        //QIODevice::WriteOnly：写入E:/FaceSignInProjecttest/namelabels.txt文件
        //QIODevice::Truncate：清理原先的内容写入新的数据namelabels集合, namelabels 关联集合map数组,
        //namelabels集合的内容是学生的名字和标签的对应关系
        qDebug()<<"write: open namelabels.txt failed!!";
        return;
    }
    qDebug()<<"打开文件namelabels.txt成功！！";
    //获得访问E:/FaceSignInProjecttest/namelabels.txt文件的文本流对象stream（IO通讯）
    //通过file获得stream
    QTextStream stream(&file);
    //通过map的迭代器指针访问map里面的数据
    //map类型：map<   int,          QString>， int人脸对应的标签，QString标签对应的人脸的姓名
    //               iter->first   iter->second
    for(map<int,QString>::iterator iter = namelabels.begin(); iter != namelabels.end(); iter++)
    {
        QString line = QString("%1,%2\n").arg(iter->first).arg(iter->second);
        //%1 <- arg(iter->first)获得iter->first 对应的内容给%1
        //%2 <- arg(iter->second)获得iter->second 对应的内容给%2
        //"%1,%2\n" - > line拼接后的字符串“1(int)(iter->first),学生1(QString)(iter->second)”
        //将拼接好的QString字符串对象给stream关联的文件E:/FaceSignInProjecttest/namelabels.txt
        stream<<line;
    }
    //关闭流，有限资源
    file.close();
}

//(9)用来保存签到时间,参数label是签到人脸的标签label，time是签到时间
void MainWindow::saveSignInTime(int label,QString time)
{
    QFile file("D:/FaceSignInProject/SignInTime.txt");
    //QIODevice::WriteOnly 只写的方式打开
    //QIODevice::Append 追加到文件尾巴写入
    //1.打开文件，如果没有文件，会创建文件，写入追加
    if(!file.open(QIODevice::WriteOnly|QIODevice::Append))
    {
        qDebug()<<"write: open SignInTime.txt failed!!";
        return;
    }
    qDebug()<<"打开文件SignInTimer.txt成功！！";
    QTextStream stream(&file);
    //for(map<int,QString>::iterator iter=SignInTimer.begin();iter!=SignInTimer.end();iter++)
   // {
    //将键值对格式化出一行字符串 "任,2020-05-09 23:00:01"
    QString line = QString("%1,%2\n").arg(namelabels[label]).arg(time);
    //2.写入文件
    stream<<line;
   // }
    //3.关闭文件
    file.close();
}


//机器学习槽函数
void MainWindow::on_pushButton_study_clicked(){
    qDebug() <<  "+++++++++++++++++++++++++++++++++++++++++++++++++++on_pushButton_study_clicked" ;
    //更改标识符flag，标示是机器学习，不做人脸预测
    flag = 0;
    //启动机器学习定时器，产生定时器标识符study_timerID
    study_timerID = startTimer(1000);
    //通过setEnabled方法 让按键失活
    ui->pushButton_study->setEnabled(false);

}
MainWindow::~MainWindow()
{
    delete ui;
}



