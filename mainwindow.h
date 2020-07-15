#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//opencv所需头文件
#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
//定时器事件工具类
#include <QTimerEvent>
//顺序容器vector工具类
#include <vector>
//关联容器map工具类
#include <map>
//命名空间，标示使用的库
using namespace cv;
using namespace cv::face;
using namespace std;

#define STUDY_FACE_COUNT 10

namespace Ui {
class MainWindow;
}
//定义主窗口类
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    //创建主窗口对象的构造器函数
    explicit MainWindow(QWidget *parent = 0);
    //主窗口对象回收时使用的析构函数
    ~MainWindow();

private slots:
    //槽：其实就是一个处理函数，是在对象中声明为slots：之下的函数及其实现
    //当一个对象的状态发生变化时，通过信号的方式通知其他对象，其他对象通过执行相应的槽函数来响应该信号。
    //槽函数可以跟信号建立起关联，而普通的成员函数不可以
    //录入签到学生的槽函数
    //机器学习，产生模型，是当前学生的人脸特征模型
    void on_pushButton_study_clicked();

private:
    void initVideoCapture();//实现打开摄像头
    void timerEvent(QTimerEvent *event);//定时事件处理函数，定时器超时就会访问此函数
    void initCascadeClassifier();//创建级联分类器
    bool findFace();//人脸查找，如果找到脸返回值为true，没找到返回false
    void drawFace();//人脸绘制
    void showFrame();//人脸显示，将摄像头图像显示在label_show上
    void studyFace();//机器学习，学习人脸，生成模型
    void initFaceRecognizer();//创建人脸识别器对象
    void initNameLabels();//从namelabels.txt文档中，一次读一行，把标签label和对应的名字读取出来，装到map容器中
    void saveNameLabels();//将map容器中的所有键值对，写入到namelabels.txt中
    void checkFace();//人脸预测，签到，检查脸
    void saveSignInTime(int label, QString time);//用来存储签到人的时间

private:
    Ui::MainWindow *ui;//主窗口指针变量，访问主窗口图形图像使用的指针变量
    VideoCapture vc;//多媒体对象，用来打开摄像头
    int start_timerID;//摄像头显示所需的定时器的标示id
    CascadeClassifier classifier;//级联分类器对象
    Mat frame;//用来保存从摄像头中读取出的一帧图像
    vector<Rect> faces;//用来保存从摄像头中识别到的人脸的矩形区域集合
    int study_timerID;//点击录入按钮后，启动学习定时器,学习定时器的标示id
    vector<Mat> study_faces;//用来保存从图像中截取的n张人脸，是同一个人的Mat人脸区域图片集合
    vector<int> study_labels;//用来保存人脸对应的标签的集合
    Ptr<FaceRecognizer> recognizer;//人脸识别器对象指针
    int count;//用来统计捕捉人脸图像的数目，初始化为0
    map<int,QString> namelabels;//key--value，int->label  QString->名字字符串
    //namelabels用来存储namelabels.txt文件中出录入的label和对应的名字的集合
    //例如：1,“张三”
    //map<int,QString> SignInTimer;
    int check_timerID;//每隔5s，预测一次人脸，预测人脸定时器标示id
    int flag;//用来控制在人脸学习的时候不要进行检测人脸，1标示检测人脸，0标示不检测人脸，初始化为1
};

#endif // MAINWINDOW_H
