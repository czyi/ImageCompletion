#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include <string>

using namespace std;

namespace Ui {
class MainWindow;
}

struct Node
{
    int x;
    int y;

    int neighbor;
    int ng[4];
    int count;

    int sam;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    int node;
    int x[100], y[100];
    int circle;
    int cx[100], cy[100];
    int line_num;
    int line_head[10];
    int cir_left, cir_right, cir_up, cir_down;
    int line_left[10], line_right[10], line_up[10], line_down[10];
    int lineorcircle;
    int state;
     QPixmap img;
     IplImage* src;
     IplImage* dst;
     IplImage* dst_line;
     IplImage* fill_line;
     string file;
     int w,h;
     int px[500], py[500];
     Node pn[500];
     int jnum, jsam, jx[20],jy[20] ,jsamx[100], jsamy[100];
     int pnode, pnode_head[10];
     int jointx[20][2], jointy[20][2];
     int sx[2000], sy[2000];
     int sam_head[10];
     int joint[10], joi_num;
     int samnum, sammax, patch, radius;
     double laprate;

     IplImage* fill_cir;
     IplImage* all_line;
     IplImage* line_heng;
     IplImage* line_shu;
     int later_circle;
     int patchlap, samlap;
     int nor;
     int flag;
     int parallel;

protected:
    void mousePressEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *event);

private slots:
    void on_openButton_clicked();

    void on_closeButton_clicked();

    void on_clearButton_clicked();

    void on_clButton_clicked();

    void on_clearcircleButton_clicked();

    void on_clearselectButton_clicked();

    void on_resetButton_clicked();

    void on_filllineButton_clicked();

    void on_filllinenextButton_clicked();

    void on_nextlineButton_clicked();

    void on_circleButton_clicked();

    void on_fillcircleButton_clicked();

    void on_parallelButton_clicked();

private:
    Ui::MainWindow *ui;

    double energe_one(int pnum, int samth, int edge, int k);
    double energe_one_j(int pnum, int samth);
    double energe_two(int pnum, int samq, int samh);
    double energe_three(int t, int choose);
    int cross_line(int ln, int cr);

};

#endif // MAINWINDOW_H
