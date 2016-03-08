#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>

//#include <QtGui/QMainWindow>
#include <QFileDialog>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
#include <opencv/cxcore.hpp>

#include <cmath>
#include <iostream>
#include <string>
#include <QString>

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    node=0;
    circle=0;
    lineorcircle=0;//node=0,circle=1;
    state=-1;
    pnode=0;
    line_num=1;
    line_head[0]=0;
    patch=16;
    radius=patch/2;
    samnum=0;
    sammax=1000;
    laprate=0.5;
    later_circle=0;
    //nor=40000;//20
    nor=1000;//16
    flag=0;
    parallel=1;
    jnum=0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openButton_clicked()
{
    //open image
    QString name=ui->lineEdit->text();
    file="/home/chen/img/"+name.toStdString();

    src=cvLoadImage(file.c_str(), 1);//src
    fill_line=cvLoadImage(file.c_str(), 1);//src_kong
    w=src->width;
    h=src->height;

    dst=cvCreateImage(cvSize(w,h),8,1);//heibai
    dst_line=cvCreateImage(cvSize(w,h),8,1);//heibai+line
    fill_cir=cvCreateImage(cvSize(w,h),8,1);
    all_line=cvCreateImage(cvSize(w,h),8,1);//all line
    line_heng=cvCreateImage(cvSize(w,h),8,1);//heng line
    line_shu=cvCreateImage(cvSize(w,h),8,1);//shu line

    for(int j=0;j<w;++j)//初始化为黑色
        for(int i=0;i<h;++i)
        {
            CV_IMAGE_ELEM(dst,uchar,i,j)=0;
            CV_IMAGE_ELEM(dst_line,uchar,i,j)=0;
            CV_IMAGE_ELEM(all_line,uchar,i,j)=0;
            CV_IMAGE_ELEM(line_heng,uchar,i,j)=0;
            CV_IMAGE_ELEM(line_shu,uchar,i,j)=0;
        }

    state=0;
}

void MainWindow::on_closeButton_clicked()
{
    close();
}


void MainWindow::mousePressEvent(QMouseEvent *e)
{//鼠标事件的响应
    //在右下角label显示鼠标点击的坐标
    //ui->label1->setText(tr("(%1,%2)").arg(e->x()).arg(e->y()));

    if(later_circle==0 && lineorcircle==0) {
        x[node]=e->x()-30;
        y[node]=e->y()-30;
        node++;//共有n个点
    }
    else{
        cx[circle]=e->x()-30;
        cy[circle]=e->y()-30;
        circle++;
    }
}

void MainWindow::paintEvent(QPaintEvent *){
    if(state==0)
        img.load(QString::fromStdString(file));
    else if(state==1||state==2)
        img.load("/home/chen/img/src_kong.jpg");
   else if(state==3&&flag==0)
        img.load("/home/chen/img/fill_line.jpg");
    else if(state==3&&flag==1)
        img.load("/home/chen/img/fill_line_circle.jpg");

    QPainter painter(this);
    painter.drawPixmap(30,30,img);

    painter.setPen(Qt::red);//red
    painter.setBrush(QBrush(Qt::red));//red

    for(int j=0;j<node;j++){//绘制控制点
        painter.drawEllipse(x[j]-1+30, y[j]-1+30,2,2);
    }

    line_head[line_num]=node;
    for(int k=0; k<line_num;k++)//drawline
        for(int j=line_head[k]+1;j<line_head[k+1];j++)
        {
                painter.drawLine(x[j-1]+30, y[j-1]+30, x[j]+30, y[j]+30);
        }

    painter.setPen(Qt::blue);//blue
    painter.setBrush(QBrush(Qt::blue));//blue

    for(int i=0;i<circle;i++)//绘制控制点
        painter.drawEllipse(cx[i]-1+30, cy[i]-1+30,2,2);

    for(int i=1;i<circle;i++)//drawcircle
        painter.drawLine(cx[i-1]+30, cy[i-1]+30, cx[i]+30, cy[i]+30);

    //if(state==2||(state==3&&later_circle==1))
    if(state==2)
        for(int i=0; i<pnode; i++)
            painter.drawEllipse(px[i]-2+30, py[i]-2+30,4,4);

    update();
}

void MainWindow::on_parallelButton_clicked()
{
    parallel=1-parallel;
    if(parallel==0) ui->parallelButton->setText("joint");
    else ui->parallelButton->setText("parallel");
}

void MainWindow::on_clearButton_clicked()
{
    //clear last line
    if(line_num==1) node=0;
    else
    {
        line_num--;
        node=line_head[line_num];
    }
}

void MainWindow::on_clButton_clicked()
{
    lineorcircle=1-lineorcircle;
    if(lineorcircle==0)
        ui->clButton->setText("line");
    else
        ui->clButton->setText("circle");
}

void MainWindow::on_nextlineButton_clicked()
{
    if(node-1>line_head[line_num-1])
        line_num++;
}


void MainWindow::on_clearcircleButton_clicked()
{
    circle=0;
}

void MainWindow::on_clearselectButton_clicked()
{
    if(state==0)
    {
        cx[circle]=cx[0];
        cy[circle]=cy[0];

        cir_left=cx[0];cir_right=cx[0];
        cir_up=cy[0];cir_down=cy[0];

        for(int i=0;i<circle;i++)
        {
           cvLine(dst, cvPoint(cx[i], cy[i]), cvPoint(cx[i+1], cy[i+1]), cvScalar(255,255,255));

           if(cx[i]<cir_left) cir_left=cx[i];
           if(cx[i]>cir_right) cir_right=cx[i];
           if(cy[i]<cir_up) cir_up=cy[i];
           if(cy[i]>cir_down) cir_down=cy[i];
        }
        cvSaveImage("/home/chen/img/circle.jpg",dst);

        cvFloodFill(dst, cvPoint((cir_left+cir_right)/2,(cir_up+cir_down)/2), CV_RGB( 255, 255, 255));

        for(int j=0;j<w;++j)//delete select
            for(int i=0;i<h;++i)
            {
                if(CV_IMAGE_ELEM(dst,uchar,i,j)!=0)
                {
                    CV_IMAGE_ELEM(dst_line,uchar,i,j)=255;
                    CV_IMAGE_ELEM(fill_line,uchar,i,j*3+0)=255;
                    CV_IMAGE_ELEM(fill_line,uchar,i,j*3+1)=255;
                    CV_IMAGE_ELEM(fill_line,uchar,i,j*3+2)=255;
                }
            }

        cvSaveImage("/home/chen/img/src_kong.jpg",fill_line);
        cvSaveImage("/home/chen/img/dst.jpg",dst);

        for(int k=0;k<line_num;k++)
        {
            line_left[k]=x[line_head[k]];
            line_right[k]=x[line_head[k]];
            line_up[k]=y[line_head[k]];
            line_down[k]=y[line_head[k]];

            for(int i=line_head[k];i<line_head[k+1];i++)
            {
                if(x[i]<line_left[k]) line_left[k]=x[i];
                if(x[i]>line_right[k]) line_right[k]=x[i];
                if(y[i]<line_up[k]) line_up[k]=y[i];
                if(y[i]>line_down[k]) line_down[k]=y[i];
            }

            for(int i=line_head[k];i<line_head[k+1]-1;i++)
            {
                cvLine(dst_line, cvPoint(x[i], y[i]), cvPoint(x[i+1], y[i+1]), cvScalar(100+10*k,100+10*k,100+10*k), 1);
                cvLine(all_line, cvPoint(x[i], y[i]), cvPoint(x[i+1], y[i+1]), cvScalar(255,255,255), radius);

                // draw heng/shu
                if(parallel==0)
                {
                    if(abs((y[i+1]-y[i])*1.0/(x[i+1]-x[i]))<=1)//hengxian
                        cvLine(line_heng, cvPoint(x[i], y[i]), cvPoint(x[i+1], y[i+1]), cvScalar(100,100,100));
                    else //shuxian
                        cvLine(line_shu, cvPoint(x[i], y[i]), cvPoint(x[i+1], y[i+1]), cvScalar(100,100,100));
                }
            }

            for(int i=line_head[k];i<line_head[k+1]-1;i++)
            {
                cvLine(all_line, cvPoint(x[i], y[i]), cvPoint(x[i+1], y[i+1]), cvScalar(200,200,200), 4);
                cvCircle(all_line, cvPoint(x[i], y[i]), 1, cvScalar(200,200,200), 2);
            }
            cvCircle(all_line, cvPoint(x[line_head[k+1]-1], y[line_head[k+1]-1]), 1, cvScalar(200,200,200), 2);

            cvSaveImage("/home/chen/img/dst_line.jpg",dst_line);
            cvSaveImage("/home/chen/img/all_line.jpg",all_line);
            cvSaveImage("/home/chen/img/line_heng.jpg",line_heng);
            cvSaveImage("/home/chen/img/line_shu.jpg",line_shu);
        }

        state=1;
    }
}

void MainWindow::on_resetButton_clicked()
{
    node=0;
    circle=0;
    state=0;
    pnode=0;
    line_num=1;
    line_head[0]=0;
    samnum=0;
    later_circle=0;
    patch=16;
    radius=patch/2;
    flag=0;
    jnum=0;

    src=cvLoadImage(file.c_str(), 1);//src
    fill_line=cvLoadImage(file.c_str(), 1);//src_kong

    for(int j=0;j<w;++j)//初始化为黑色
        for(int i=0;i<h;++i)
        {
            CV_IMAGE_ELEM(dst,uchar,i,j)=0;
            CV_IMAGE_ELEM(dst_line,uchar,i,j)=0;
        }
}

void MainWindow::on_filllineButton_clicked()
{
    //if(state==1&&parall/el==1)
    if(state==1)
    {
        for(int k=0;k<line_num;k++)
        {
            pnode_head[k]=pnode;
            cout << k << "th line, head is " << pnode_head[k] << endl;

            jointx[k][0]=w; jointy[k][0]=0;//left
            jointx[k][1]=0; jointy[k][1]=0;//right

            //if shixian / shu curve, cannot work
            for(int j=cir_left;j<cir_right;++j)//find edge point
                for(int i=cir_up;i<cir_down;++i)
                {
                    if(CV_IMAGE_ELEM(dst,uchar,i,j)==255&&CV_IMAGE_ELEM(dst_line,uchar,i,j)==100+10*k)
                    {
                        if(j<jointx[k][0])
                        {
                            jointx[k][0]=j;
                            jointy[k][0]=i;
                        }
                        if(j>jointx[k][1])
                        {
                            jointx[k][1]=j;
                            jointy[k][1]=i;
                        }
                    }
                }

            //heng line
            if(jointx[k][0]!=jointx[k][1]&&abs(jointx[k][0]-jointx[k][1])>=abs(jointy[k][0]-jointy[k][1]))
            {
                //fill in the line per line
                px[pnode]=jointx[k][0];py[pnode]=jointy[k][0];//left point
                pnode++;
                sam_head[k]=samnum;
                cout << k << "th line, left node is (" << jointx[k][0] << ", " << jointy[k][0] << "), right node is (" << jointx[k][1] << ", " << jointy[k][1] << ")." << endl;

                for(int j=px[pnode_head[k]]+1;j<=cir_right;++j)
                    for(int i=cir_up;i<cir_down;++i)
                    {
                         if((CV_IMAGE_ELEM(dst,uchar,i,j)==255 || CV_IMAGE_ELEM(dst,uchar,i,j-radius)==255)&&CV_IMAGE_ELEM(dst_line,uchar,i,j)==100+10*k)
                        {
                             double diff;
                             if(j<line_right[k]-radius) diff=(patch*patch)*laprate;
                             else diff=radius*radius;

                            if(((j-px[pnode-1]) * (j-px[pnode-1]) + (i-py[pnode-1]) * (i-py[pnode-1]))>=diff)
                            {
//                                cout << "i/y is " << i << ", ";
//                                cout << "j/x is " << j << ", ";
//                                cout << "last px is " << px[pnode-1] << ", ";
//                                cout << "last py is " << py[pnode-1] <<endl;
                                px[pnode]=j;py[pnode]=i;pnode++;
                            }
                        }
                    }

                //sam is around line, q up or below
               if(samnum<sammax)
               {
                   for(int j=line_left[k]+radius;j<=line_right[k]-radius;j++)
                       for(int i=line_up[k];i<=line_down[k];i++)
                       {
                            if(CV_IMAGE_ELEM(dst,uchar,i,j)==0 && CV_IMAGE_ELEM(dst_line,uchar,i,j)==100+10*k)
                            {
                                int count=0;
                                for(int n=-radius;n<radius;n++)
                                    for(int m=-radius;m<radius;m++)
                                    {
                                        if(CV_IMAGE_ELEM(dst,uchar,i+m,j+n)==255) count++;
                                    }

                                if(count==0)
                                {
                                    //if(i%2==0) {sx[samnum]=j;sy[samnum]=i;samnum++;}
                                    //else {sx[samnum]=j-1;sy[samnum]=i;sx[samnum+1]=j+1;sy[samnum+1]=i;samnum+=2;}
                                    sx[samnum]=j-1;
                                    sy[samnum]=i;
                                    sx[samnum+1]=j;
                                    sy[samnum+1]=i;
                                    sx[samnum+2]=j+1;
                                    sy[samnum+2]=i;
                                    samnum+=3;
                                }
                            }
                       }
               }
            }
            else//shu line
            {
                jointx[k][0]=0;
                jointy[k][0]=h;//up
                jointx[k][1]=0;
                jointy[k][1]=0;//down

                //if shixian / shu curve, cannot work
                for(int i=cir_up;i<cir_down;++i)
                    for(int j=cir_left;j<cir_right;++j)//find edge point
                    {
                        if(CV_IMAGE_ELEM(dst,uchar,i,j)==255&&CV_IMAGE_ELEM(dst_line,uchar,i,j)==100+10*k)
                        {
                            if(i<jointy[k][0])
                            {
                                jointx[k][0]=j;
                                jointy[k][0]=i;
                            }
                            if(i>jointy[k][1])
                            {
                                jointx[k][1]=j;
                                jointy[k][1]=i;
                            }
                        }
                    }

                //fill in the line per line
                px[pnode]=jointx[k][0];
                py[pnode]=jointy[k][0];//up point
                pnode++;
                cout << k << "th line, up node is (" << jointx[k][0] << ", " << jointy[k][0] << "), down node is (" << jointx[k][1] << ", " << jointy[k][1] << ")." << endl;
                sam_head[k]=samnum;

                for(int i=py[pnode_head[k]]+1;i<cir_down;++i)
                     for(int j=cir_left;j<=cir_right;++j)
                    {
                        if((CV_IMAGE_ELEM(dst,uchar,i,j)==255|| CV_IMAGE_ELEM(dst,uchar,i-radius,j)==255)&& (CV_IMAGE_ELEM(dst_line,uchar,i,j)==100+10*k))
                        {
                            double diff;
//                            if(i<line_down[k]-radius) diff=(patch*patch)*laprate;
//                            else diff=radius*radius;
                            diff=radius*radius;

//                         if(((j-px[node-1]) * (j-px[node-1]) + (i-py[pnode-1]) * (i-py[pnode-1]))>=diff)
//                            {
//                                cout << "j is " << j << ", "; cout << "px[pnode-1] is " << px[pnode-1] << ", ";
//                                cout << "- is " << (j - px[node-1]) << ", "; cout << "*  is " << (j-px[node-1]) * (j-px[node-1]) << ", ";
//                                cout << "i is " << i << ", ";  cout << "py[pnode-1] is " << py[pnode-1] << ", ";
//                                cout << "- is " << (i - py[pnode-1]) << ", ";cout << "* is " << (i-py[pnode-1]) * (i-py[pnode-1]) << ", ";
//                                cout << "xxx is " << ((j-px[node-1]) * (j-px[node-1]) + (i-py[pnode-1]) * (i-py[pnode-1])) << ", ";
//                                cout << "diff is " << diff << endl;
//                                px[pnode]=j;py[pnode]=i;pnode++;
//                            }

                            if((i-py[pnode-1]) * (i-py[pnode-1])>=diff)
                            {
//                                cout << "j is " << j << ", "; cout << "px[pnode-1] is " << px[pnode-1] << ", ";
//                                cout << "- is " << (j - px[node-1]) << ", "; cout << "*  is " << (j-px[node-1]) * (j-px[node-1]) << ", ";
//                                cout << "i is " << i << ", ";  cout << "py[pnode-1] is " << py[pnode-1] << ", ";
//                                cout << "- is " << (i - py[pnode-1]) << ", ";cout << "* is " << (i-py[pnode-1]) * (i-py[pnode-1]) << ", ";
//                                cout << "xxx is " << ((j-px[node-1]) * (j-px[node-1]) + (i-py[pnode-1]) * (i-py[pnode-1])) << ", ";
//                                cout << "diff is " << diff << endl;
                                px[pnode]=j;py[pnode]=i;pnode++;
                            }
                        }
                    }

                if(samnum<sammax)
                {
                    for(int i=line_up[k];i<line_down[k]+1;i++)
                        for(int j=line_left[k];j<=line_right[k];j++)
                        {
                             if(CV_IMAGE_ELEM(dst,uchar,i,j)!=255&&CV_IMAGE_ELEM(dst_line,uchar,i,j)==100+10*k)
                             {
                                 int count=0;
                                 for(int m=-radius;m<radius;m++)
                                     for(int n=-radius;n<radius;n++)
                                     {
                                         if(CV_IMAGE_ELEM(dst,uchar,i+m,j+n)==255) count++;
                                     }

                                 if(count==0)
                                 {
                                     //if(j%2==0) {sx[samnum]=j;sy[samnum]=i;samnum++;}
                                     //else {sx[samnum]=j;sy[samnum]=i-1;sx[samnum+1]=j;sy[samnum+1]=i+1;samnum+=2;}
                                     sx[samnum]=j;
                                     sy[samnum]=i-1;
                                     sx[samnum+1]=j;
                                     sy[samnum+1]=i;
                                     sx[samnum+2]=j;
                                     sy[samnum+2]=i+1;
                                     samnum+=3;
                                 }
                             }
                        }
                }
            }

        }
         //sam is around line, q up or below
        pnode_head[line_num]=pnode;
        sam_head[line_num]=samnum;

        cout << "sannum is " << samnum << endl;

        if(parallel==0)
        {
            for(int i=cir_up;i<cir_down;++i)
                for(int j=cir_left;j<cir_right;++j)
                {
                    if(CV_IMAGE_ELEM(line_shu,uchar,i,j)==100 && CV_IMAGE_ELEM(line_heng,uchar,i,j)==100)
                    {
                        jx[jnum]=j;jy[jnum]=i;jnum++;
                    }
                }
            cout << "jnum is " << jnum << endl;

            //select sample
            for(int i=0;i<h;++i)
                for(int j=0;j<w;++j)
                {
                    int count=0;
                    for(int m=-1;m<1;m++)
                        for(int n=-1;n<1;n++)
                        {
                             if(CV_IMAGE_ELEM(all_line,uchar,i,j)==200 && CV_IMAGE_ELEM(line_shu,uchar,i+m,j+n)==100
                                     && CV_IMAGE_ELEM(line_heng,uchar,i+m,j+n)==100 && CV_IMAGE_ELEM(dst,uchar,i,j)==0)
                                 count++;
                        }

                    if(jsam<100 && count>0)
                    {
                        jsamx[jsam]=j;jsamy[jsam]=i;jsam++;
                        CV_IMAGE_ELEM(all_line,uchar,i,j)=50;
                    }
                }
            cout << "jsam is " << jsam << endl;
        }
        for(int i=0;i<jsam;i++)
        {
            cout << "jsamx[" << i << "] is " << jsamx[i] << ", jsamy[" << i << "] is " << jsamy[i] << endl;
        }

        state=2;//find all patch core
        cout << "state is 2" << endl;
    }
    //parallel=2   graph  belief  progation
    else if(state==1&&parallel==2)
    {
        //find all joint point
        for(int i=cir_up;i<cir_down;++i)
            for(int j=cir_left;j<cir_right;++j)
            {
                if(CV_IMAGE_ELEM(dst,uchar,i,j)==255 && CV_IMAGE_ELEM(line_shu,uchar,i,j)==100
                        && CV_IMAGE_ELEM(line_heng,uchar,i,j)==100)
                {
                    //px[pnode]=j;py[pnode]=i;
                    pn[pnode].x=j;
                    pn[pnode].y=i;
                    pn[pnode].neighbor=0;
                    pn[pnode].count=0;
                    pnode++;
                }
            }
        jnum=pnode;
        cout << "jnum is " << jnum << endl;

        int diff=radius*radius;
        //int dis=radius*radius*2;
        //select pnode
        for(int i=cir_up;i<cir_down;++i)
            for(int j=cir_left;j<cir_right;++j)
            {
                if((CV_IMAGE_ELEM(line_heng,uchar,i,j)==100||CV_IMAGE_ELEM(line_shu,uchar,i,j)==100)
                        && CV_IMAGE_ELEM(dst,uchar,i,j)==255)
                {
                    int count=0;
                    for(int m=0;m<pnode;m++)
                        if((i-py[m]) * (i-py[m]) + (j-px[m]) * (j-px[m]) <= diff)
                            count++;

                    //add anchor
                    if(count==0)
                    {
                        pn[pnode].x=j;
                        pn[pnode].y=i;
                        pn[pnode].neighbor=0;
                        pn[pnode].count=0;
                    }

                    //find neibhorhood
                    for(int k=0;k<line_num;k++)
                    {
                        int cc=0;

                        //if joint point is in the line
                        for(int m=-1;m<1;m++)
                            for(int n=-1;n<1;n++)
                            {
                                if(CV_IMAGE_ELEM(dst_line,uchar,pn[pnode].y+m,pn[pnode].x+n)==100+k*10)
                                    cc=1;
                            }

                        if(cc==1)
                        {//find neighbor
                            for(int m=pn[pnode].x;m<pn[pnode].x+patch;++m)
                                for(int n=pn[pnode].y;n<pn[pnode].y+patch;++n)
                                {
                                    for(int pp=0;pp<pnode;pp++)
                                    {
                                        if(CV_IMAGE_ELEM(dst_line,uchar,pn[pp].y,pn[pp].x)==100+k*10)
                                        {
                                            pn[pnode].ng[pn[pnode].neighbor]=pp;
                                            pn[pnode].neighbor++;
                                            pn[pnode].count++;

                                            pn[pp].ng[pn[pp].neighbor]=pnode;
                                            pn[pp].neighbor++;
                                            pn[pp].count++;

                                            break;
                                           }
                                    }
                                }

                            for(int i=pn[pnode].x;i<pn[pnode].x-patch;i--)
                                for(int j=pn[pnode].y;j<pn[pnode].y-patch;j--)
                                {
                                    for(int pp=0;pp<pnode;pp++)
                                    {
                                        if(CV_IMAGE_ELEM(dst_line,uchar,i,j)==100+k*10)
                                        {
                                            pn[pnode].ng[pn[pnode].neighbor]=pp;
                                            pn[pnode].neighbor++;
                                            pn[pnode].count++;

                                           pn[pp].ng[pn[pp].neighbor]=pnode;
                                            pn[pp].neighbor++;
                                            pn[pp].count++;

                                            break;
                                        }
                                    }
                                }
                        }

                        pnode++;
                    }
                }
            }
        cout << "not parallel, pnode is " << pnode << endl;

        //select sample
        for(int i=0;i<h;++i)
            for(int j=0;j<w;++j)
            {
                int count=0;
                for(int m=-2;m<2;m++)
                    for(int n=-2;n<2;n++)
                    {
                         if(CV_IMAGE_ELEM(all_line,uchar,i,j)==200 && CV_IMAGE_ELEM(line_shu,uchar,i+m,j+n)==100
                                 && CV_IMAGE_ELEM(line_heng,uchar,i+m,j+n)==100)
                             count++;
                    }

                if(samnum<sammax && count>0)
                {
                    sx[samnum]=j;sy[samnum]=i;
                    samnum++;
                    CV_IMAGE_ELEM(all_line,uchar,i,j)=50;
                }
            }

        jsam=samnum;
        cout << "jsam is " << jsam << endl;
        cvSaveImage("/home/chen/img/all_line1.jpg",all_line);

        //select sample
        for(int k=0;k<line_num;k++)
        {
            jointx[k][0]=w;
            jointy[k][0]=0;//left
            jointx[k][1]=0;
            jointy[k][1]=0;//right

            //if shixian / shu curve, cannot work
            for(int j=cir_left;j<cir_right;++j)//find edge point
                for(int i=cir_up;i<cir_down;++i)
                {
                    if(CV_IMAGE_ELEM(dst,uchar,i,j)==255&&CV_IMAGE_ELEM(dst_line,uchar,i,j)==100+10*k)
                    {
                        if(j<jointx[k][0])
                        {
                            jointx[k][0]=j;
                            jointy[k][0]=i;
                        }
                        if(j>jointx[k][1])
                        {
                            jointx[k][1]=j;
                            jointy[k][1]=i;
                        }
                    }
                }

            //heng line
            if(jointx[k][0]!=jointx[k][1]&&abs(jointx[k][0]-jointx[k][1])>=abs(jointy[k][0]-jointy[k][1]))
            {
                //sam is around line, q up or below
               if(samnum<sammax)
               {
                   for(int j=line_left[k]+radius;j<=line_right[k]-radius;j++)
                       for(int i=line_up[k];i<=line_down[k];i++)
                       {
                            if(CV_IMAGE_ELEM(dst,uchar,i,j)==0 && CV_IMAGE_ELEM(dst_line,uchar,i,j)==100+10*k)
                            {
                                int count=0;
                                for(int n=-radius;n<radius;n++)
                                    for(int m=-radius;m<radius;m++)
                                    {
                                        if(CV_IMAGE_ELEM(dst,uchar,i+m,j+n)==255) count++;
                                    }

                                if(count==0)
                                {
                                    //if(i%2==0) {sx[samnum]=j;sy[samnum]=i;samnum++;}
                                    //else {sx[samnum]=j-1;sy[samnum]=i;sx[samnum+1]=j+1;sy[samnum+1]=i;samnum+=2;}
                                    sx[samnum]=j-1;
                                    sy[samnum]=i;
                                    sx[samnum+1]=j;
                                    sy[samnum+1]=i;
                                    sx[samnum+2]=j+1;
                                    sy[samnum+2]=i;
                                    samnum+=3;
                                }
                            }
                       }
               }
            }
            else//shu line
            {
                jointx[k][0]=0;
                jointy[k][0]=h;//up
                jointx[k][1]=0;
                jointy[k][1]=0;//down

                //if shixian / shu curve, cannot work
                for(int i=cir_up;i<cir_down;++i)
                    for(int j=cir_left;j<cir_right;++j)//find edge point
                    {
                        if(CV_IMAGE_ELEM(dst,uchar,i,j)==255&&CV_IMAGE_ELEM(dst_line,uchar,i,j)==100+10*k)
                        {
                            if(i<jointy[k][0])
                            {
                                jointx[k][0]=j;
                                jointy[k][0]=i;
                            }
                            if(i>jointy[k][1])
                            {
                                jointx[k][1]=j;
                                jointy[k][1]=i;
                            }
                        }
                    }

                if(samnum<sammax)
                {
                    for(int i=line_up[k];i<line_down[k]+1;i++)
                        for(int j=line_left[k];j<=line_right[k];j++)
                        {
                             if(CV_IMAGE_ELEM(dst,uchar,i,j)!=255&&CV_IMAGE_ELEM(dst_line,uchar,i,j)==100+10*k)
                             {
                                 int count=0;
                                 for(int m=-radius;m<radius;m++)
                                     for(int n=-radius;n<radius;n++)
                                     {
                                         if(CV_IMAGE_ELEM(dst,uchar,i+m,j+n)==255) count++;
                                     }

                                 if(count==0)
                                 {
                                     //if(j%2==0) {sx[samnum]=j;sy[samnum]=i;samnum++;}
                                     //else {sx[samnum]=j;sy[samnum]=i-1;sx[samnum+1]=j;sy[samnum+1]=i+1;samnum+=2;}
                                     sx[samnum]=j;
                                     sy[samnum]=i-1;
                                     sx[samnum+1]=j;
                                     sy[samnum+1]=i;
                                     sx[samnum+2]=j;
                                     sy[samnum+2]=i+1;
                                     samnum+=3;
                                 }
                             }
                        }
                }
            }

        }
         //sam is around line, q up or below

        cout << "not parallel, samnum is " << samnum;

        state=2;
    }
}

void MainWindow::on_filllinenextButton_clicked()
{
    //if(state==2&&parallel==1)//fill patch
    if(state==2)
    {
        //double e1x[500][1000];//[i][j], i=pnode, j=samnum
        double e_one, e_two;
        int chain[500][2000];//[i][j], i=pnode, j=samnum
        int m_bef[2000], m_aft[2000]; //j=samnum

        for(int k=0;k<line_num;k++)
        {
            cout << "line_num is " << line_num << endl;
            cout << "pnode is "<< pnode_head[k] <<" : " << endl;
            cout << "sam_head[" << k << "] and [" << k+1 << "] is " << sam_head[k] << ", and " << sam_head[k+1] << endl;

            for(int j=sam_head[k]; j<sam_head[k+1]; j++)
            {
                e_one=energe_one(pnode_head[k], j, 1, k);
                m_bef[j]=e_one;
            }
            //cout << endl;

            for(int i=pnode_head[k]+1;i<pnode_head[k+1];i++)
            {
                for(int r=sam_head[k];r<sam_head[k+1];r++)//hou r
                {
                    int min_m=m_bef[r]+energe_two(i-1, sam_head[k], r);//i-1,ith pnode, sam_qian, sam_hou
                    int min_num=sam_head[k];

                    for(int s=sam_head[k]+1;s<sam_head[k+1];s++)//qian s
                    {
                        e_two=m_bef[s]+energe_two(i-1, s, r);
                        if(e_two<min_m)
                        {
                            min_m=e_two;
                            min_num=s;
                        }
                    }
                    //cout << "chain[" << (i-1) << "][" << r << "] is " << min_num << endl;
                    chain[i-1][r]=min_num;

                    if(i!=pnode_head[k+1]-1)
                        e_one=energe_one(i, r, 0, k);
                    else
                        e_one=energe_one(i, r, 1, k);
                    m_aft[r]=e_one+min_m;
                }

                for(int j=sam_head[k];j<sam_head[k+1];j++)
                    m_bef[j]=m_aft[j];
            }

            int sum_min;
            int choose=sam_head[k];
            sum_min=m_bef[sam_head[k]];
            for(int j=sam_head[k];j<sam_head[k+1];j++)//find min chain
            {
               if(sum_min>m_bef[j])
               {
                  // cout << "m_bef[" << j << "] is " << m_bef[j] << endl;
                   sum_min=m_bef[j];
                   choose=j;
               }
            }
            cout << "min chain is " << choose <<", ";
            cout << "sum_min is " << sum_min << endl;

            //without juchi
            int dst1, dst2;
            double rate1, rate2;

            //min last sam is sum_num
            //px[pnode_head[k+1]]=px[pnode_head[k+1]-1]+1;
            //py[pnode_head[k+1]]=py[pnode_head[k+1]-1]+1;
            cout << "pnode_head["<< k << "] is " << pnode_head[k] << ", ";
            cout << "pnode_head["<< k+1 << "]-1 is " << pnode_head[k+1]-1 << endl;
            cout << "choose is " << choose << endl;

            for(int t=pnode_head[k+1]-1;t>=pnode_head[k];t--)
            {
                for(int i=-radius;i<radius;i++)
                    for(int j=-radius;j<radius;j++)
                    {
                       /* CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3);
                        CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+1)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+1);
                        CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+2)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+2);*/

                     if (CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3)==255&&CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+1)==255
                             &&CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+2)==255)
                        {
                            CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3);
                            CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+1)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+1);
                            CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+2)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+2);
                        }
                        else//overlap
                        {
                            dst1=i*i+j*j;

                            if(t<pnode_head[k+1]-1&& t>pnode_head[k])
                                dst2=(py[t+1]-py[t]-i)*(py[t+1]-py[t]-i)+(px[t+1]-px[t]-j)*(px[t+1]-px[t]-j);
                            else
                                dst2=dst1;

                            rate1=dst1*1.0/(dst1+dst2);
                            rate2=1-rate1;

                            CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3)=rate1*CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3)
                                    + rate2*CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3);
                            CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+1)=rate1*CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+1)
                                    + rate2*CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+1);
                            CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+2)=rate1*CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+2)
                                    + rate2*CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+2);
                        }

                    }
                //cout << endl;

                if(t>pnode_head[k])
                {
                    //cout << "choose is " << choose << ", ";
                    //cout << "chain["<<t-1<<"]["<<choose<<"] is " << chain[t-1][choose] << endl;
                     choose=chain[t-1][choose];
                }
            }

        }

        cvSaveImage("/home/chen/img/fill_line.jpg",fill_line);

        cout << endl;
        cout << endl;
        cout << "parallel" << endl;

        if(parallel==0)
        {
           int choose;
           for(int r=0;r<jnum;r++)
           {
               cout << "r is " << r << endl;
               double min_x=energe_one_j(r, 0);
               choose=2;
               for(int s=0;s<jsam;s++)
               {
                   e_one=energe_one_j(r, s);
                   if(e_one<min_x)
                   {
                       e_one=min_x;
                       choose=s;
                   }
               }
               cout << "choose is " <<choose << endl;

               cout << "jy[r] is " << jy[r] << "jx[r] is " << jx[r] << ", ";
               cout << "jsamx[choose] is " << jsamx[choose] << ", jsamy[choose] is " << jsamy[choose] << endl;
               for(int i=-radius;i<radius;i++)
                   for(int j=-radius;j<radius;j++)
                   {
                           int dst1=i*i+j*j;
                           int dst2=radius*radius;

                           double rate1=dst1*1.0/(dst1+dst2);
                           double rate2=1-rate1;

                           if(i>2)
                           {
                               CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3)
                                       =CV_IMAGE_ELEM(src,uchar,jsamy[choose]+i,(jsamx[choose]+j)*3);
                               CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3+1)
                                       =CV_IMAGE_ELEM(src,uchar,jsamy[choose]+i,(jsamx[choose]+j)*3+1);
                               CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3+2)
                                       =CV_IMAGE_ELEM(src,uchar,jsamy[choose]+i,(jsamx[choose]+j)*3+2);
                           }
                           else
                           {
                               CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3)=rate1*CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3)
                                       + rate2*CV_IMAGE_ELEM(src,uchar,jsamy[choose]+i,(jsamx[choose]+j)*3);
                               CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3+1)=rate1*CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3+1)
                                       + rate2*CV_IMAGE_ELEM(src,uchar,jsamy[choose]+i,(jsamx[choose]+j)*3+1);
                               CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3+2)=rate1*CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3+2)
                                       + rate2*CV_IMAGE_ELEM(src,uchar,jsamy[choose]+i,(jsamx[choose]+j)*3+2);
                           }


//                           cout << CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3) << ", " << CV_IMAGE_ELEM(src,uchar,jsamy[choose]+i,(jsamx[choose]+j)*3) << endl;
//                           cout << CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3+1) << ", " << CV_IMAGE_ELEM(src,uchar,jsamy[choose]+i,(jsamx[choose]+j)*3+1) << endl;
//                           cout << CV_IMAGE_ELEM(fill_line,uchar,jy[r]+i,(jx[r]+j)*3+2) << ", " << CV_IMAGE_ELEM(src,uchar,jsamy[choose]+i,(jsamx[choose]+j)*3+2) << endl;
                   }
           }

           cvSaveImage("/home/chen/img/fill_line.jpg",fill_line);
           cout << "done" << endl;
        }

        state=3;
   }
   else if(state==2&&parallel==2)//fill patch
   {
       //double e_one;
       int choose;
       double mess[500][500]={-1};//[i][j], i=pnode, j=samnum
       //int m_bef[2000], m_aft[2000]; //j=samnum
       //int choose[500];//pnode

       for(int nn=0;nn<pnode;nn++)
       {
           for(int mm=0;mm<pn[pnode].neighbor;nn++)
               mess[nn][pn[nn].ng[mm]]=0;
       }

       //initial
       for(int pp=0;pp<pnode;pp++)
       {
           int minpp=0;
           double minee=energe_one(pp,0,1,-1);
           double e1;

           for(int ss=1;ss<samnum;ss++)
           {
               e1=energe_one(pp,ss,1,-1);
               if(e1<minee)
               {
                   minee=e1;
                   minpp=ss;
               }
           }

           pn[pp].sam=minpp;
       }

       for(int t=1;t<100;t++)
       {
           for(int r=0;r<500;r++)
               for(int s=0;s<500;s++)
               {
                   if(mess[r][s]!=-1)
                   {
                       if(pn[r].count==1)
                       {
                           int minpp=0;
                           double minee=energe_one(r,0,1,-1)+energe_two(-1, r, s);
                           double e12;

                           for(int ss=1;ss<samnum;ss++)
                           {
                               e12=energe_one(r,ss,1,-1)+energe_two(-1, ss, s);
                               if(e12<minee)
                               {
                                   minee=e12;
                                   minpp=s;
                               }
                           }

                           pn[r].sam=minpp;
                           pn[r].count--;
                           pn[s].count--;
                       }
                   }
               }
       }

       for(int r=0;r<pnode;r++)
       {
           choose=pn[pnode].sam;

           for(int i=-radius;i<radius;i++)
               for(int j=-radius;j<radius;j++)
               {
                if (CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3)==255&&CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3+1)==255
                        &&CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3+2)==255)
                   {
                       CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3);
                       CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3+1)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+1);
                       CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3+2)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+2);
                   }
                   else//overlap
                   {
                       int dst1=i*i+j*j;
                       int dst2=radius*radius;

                       double rate1=dst1*1.0/(dst1+dst2);
                       double rate2=1-rate1;

                       CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3)=rate1*CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3)
                               + rate2*CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3);
                       CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3+1)=rate1*CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3+1)
                               + rate2*CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+1);
                       CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3+2)=rate1*CV_IMAGE_ELEM(fill_line,uchar,py[r]+i,(px[r]+j)*3+2)
                               + rate2*CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+2);
                   }
               }
       }

   }
}

double MainWindow::energe_one(int pnum, int samth, int edge, int k)//ith pnode, jth sample, if edge
{
    int ks=50, ki=2;
    int dis2pnode=0, dis2sam=0;
    int dis_min, dis,pnsum=0;

    if(k==-1)
    {
        samth=pn[pnum].sam;
        px[pnum]=pn[pnum].x;
        py[pnum]=pn[pnum].y;
    }

    //cout << "pnode is " << pnum << ", sam is " << samth << ", ";
    //Es(x) dis2pnode
    for(int i=-radius; i<radius;i++)//sam
        for(int j=-radius; j<radius;j++)
        {
            if(CV_IMAGE_ELEM(dst_line,uchar,sy[samth]+i, sx[samth]+j)==100+10*k)
            {
                dis_min=2*patch*patch;
                for(int m=-radius;m<radius;m++)
                    for(int n=-radius;n<radius;n++)
                        if(CV_IMAGE_ELEM(dst_line,uchar,py[pnum]+m, px[pnum]+n)==100+10*k)
                        {
                            dis=(i-m)*(i-m)+(j-n)*(j-n);
                            if(dis_min>dis) dis_min=dis;
                        }
               //cout << " dis_min is " << dis_min <<", ";

                dis2pnode+=dis_min;
            }
        }
   // cout << "dis2pnode is " << dis2pnode << ", ";

    //dis2sam
    for(int m=-radius;m<radius;m++)
        for(int n=-radius;n<radius;n++)
        {
            if(CV_IMAGE_ELEM(dst_line,uchar,py[pnum]+m,px[pnum]+n)==100+10*k)
            {
                dis_min=2*patch*patch;

                for(int i=-radius; i<radius;i++)
                    for(int j=-radius; j<radius;j++)
                    {
                        //cout << "(i,j) is (" <<  i << "," << j <<"), ";
                        if(CV_IMAGE_ELEM(dst_line,uchar,sy[samth]+i, sx[samth]+j)==100+10*k)
                        {
                            dis=(i-m)*(i-m)+(j-n)*(j-n);
                           // cout << "(i,j) is (" <<  i << "," << j <<"), dis is " << dis << ", ";
                            //cout << " dis is " << dis << ", ";
                            if(dis_min>dis) dis_min=dis;
                        }
                    }

                //cout << endl;

                //cout << " dis_min is " << dis_min <<endl;
               // if (dis_min<10)
                {
                    dis2sam+=dis_min;
                    pnsum++;
                }
            }
        }
    //cout << "dis2sam is " << dis2sam << ", ";
    //cout << "pnsum is " << pnsum << ", ";

    double es_x=0;
    es_x=ks*(dis2pnode+dis2sam)*1.0/pnsum;
    int ssd, sum_ssd=0;
    //cout << "Es(x) is " << es_x << " , ";

    if(edge)//is edge
    {//cai se!!!!!!!!!
        pnsum=0;

        for(int m=-radius;m<radius;m++)
            for(int n=-radius;n<radius;n++)
            {
                if(CV_IMAGE_ELEM(dst,uchar,m,n)!=255)
                {
                    pnsum++;
                    ssd=CV_IMAGE_ELEM(fill_line,uchar,py[pnum]+m,(px[pnum]+n)*3)
                            -CV_IMAGE_ELEM(fill_line,uchar,sy[samth]+m,(sx[samth]+n)*3);
                    sum_ssd+=ssd*ssd;
                    ssd=CV_IMAGE_ELEM(fill_line,uchar,py[pnum]+m,(px[pnum]+n)*3+1)
                            -CV_IMAGE_ELEM(fill_line,uchar,sy[samth]+m,(sx[samth]+n)*3+1);
                    sum_ssd+=ssd*ssd;
                    ssd=CV_IMAGE_ELEM(fill_line,uchar,py[pnum]+m,(px[pnum]+n)*3+2)
                            -CV_IMAGE_ELEM(fill_line,uchar,sy[samth]+m,(sx[samth]+n)*3+2);
                    sum_ssd+=ssd*ssd;
                }
            }

        double ei_x;
        ei_x=sum_ssd*1.0*ki/(pnsum*nor);
        es_x+=ei_x;
       // cout << "edge, Ei(x) " << ei_x << " , " ;
        //cout << "plus Es(x) is " << es_x;
    }
    //cout << endl;

    return es_x;
}

double MainWindow::energe_one_j(int pnum, int samth)//ith pnode, jth sample, if edge
{
    //int ks=50, ki=2;
    int ks=50, ki=2;
    int dis2pnode=0, dis2sam=0;
    int dis_min, dis,pnsum=0;
    int norl=500, edge=5;

    cout << "pnode is " << pnum << ", sam is " << samth << ", ";
    //Es(x) dis2pnode
    for(int i=-radius; i<radius;i++)//sam
        for(int j=-radius; j<radius;j++)
        {
            if(CV_IMAGE_ELEM(dst_line,uchar,sy[samth]+i, sx[samth]+j)==100)
            {
                dis_min=2*patch*patch;
                for(int m=-radius;m<radius;m++)
                    for(int n=-radius;n<radius;n++)
                        if(CV_IMAGE_ELEM(dst_line,uchar,jy[pnum]+m, jx[pnum]+n)==100)
                        {
                            dis=(i-m)*(i-m)+(j-n)*(j-n);
                            if(dis_min>dis) dis_min=dis;
                        }
               //cout << " dis_min is " << dis_min <<", ";

                dis2pnode+=dis_min;
            }
        }
   // cout << "dis2pnode is " << dis2pnode << ", ";

    //dis2sam
    for(int m=-radius;m<radius;m++)
        for(int n=-radius;n<radius;n++)
        {
            if(CV_IMAGE_ELEM(dst_line,uchar,jy[pnum]+m,jx[pnum]+n)==100)
            {
                dis_min=2*patch*patch;

                for(int i=-radius; i<radius;i++)
                    for(int j=-radius; j<radius;j++)
                    {
                        //cout << "(i,j) is (" <<  i << "," << j <<"), ";
                        if(CV_IMAGE_ELEM(dst_line,uchar,sy[samth]+i, sx[samth]+j)==100)
                        {
                            dis=(i-m)*(i-m)+(j-n)*(j-n);
                           // cout << "(i,j) is (" <<  i << "," << j <<"), dis is " << dis << ", ";
                            //cout << " dis is " << dis << ", ";
                            if(dis_min>dis) dis_min=dis;
                        }
                    }

                //cout << endl;

                //cout << " dis_min is " << dis_min <<endl;
               // if (dis_min<10)
                {
                    dis2sam+=dis_min;
                    pnsum++;
                }
            }
        }
    //cout << "dis2sam is " << dis2sam << ", ";
    //cout << "pnsum is " << pnsum << ", ";

    double es_x=0;
    es_x=ks*(dis2pnode+dis2sam)*1.0/pnsum;
    int ssd, sum_ssd=0;
    cout << "Es(x) is " << es_x << " , ";

    //similarity
    pnsum=0;
    for(int m=-radius-edge;m<radius+edge;m++)
        for(int n=-radius-edge;n<radius+edge;n++)
        {
            if(CV_IMAGE_ELEM(dst,uchar,m,n)!=255)
            {
                pnsum++;
                ssd=CV_IMAGE_ELEM(fill_line,uchar,jy[pnum]+m,(jx[pnum]+n)*3)
                        -CV_IMAGE_ELEM(fill_line,uchar,sy[samth]+m,(sx[samth]+n)*3);
                sum_ssd+=ssd*ssd;
                ssd=CV_IMAGE_ELEM(fill_line,uchar,jy[pnum]+m,(jx[pnum]+n)*3+1)
                        -CV_IMAGE_ELEM(fill_line,uchar,sy[samth]+m,(sx[samth]+n)*3+1);
                sum_ssd+=ssd*ssd;
                ssd=CV_IMAGE_ELEM(fill_line,uchar,jy[pnum]+m,(jx[pnum]+n)*3+2)
                        -CV_IMAGE_ELEM(fill_line,uchar,sy[samth]+m,(sx[samth]+n)*3+2);
                sum_ssd+=ssd*ssd;
            }
        }

    double ei_x;
    ei_x=sum_ssd*1.0*ki/(pnsum*norl);
    es_x+=ei_x;
   // cout << "edge, Ei(x) " << ei_x << " , " ;
    cout << "plus Es(x) is " << es_x;

    cout << endl;

    return es_x;
}

double MainWindow::energe_two(int pnum, int samq, int samh)
{
    int di,dj;
    int sum_ssd=0;
    int dis;

    if(pnum==-1)
    {
        di=patch-abs(pn[samq].y - pn[samh].y);
        dj=patch-abs(pn[samq].x - pn[samh].x);
        samq=pn[samq].sam;
        samh=pn[samh].sam;
    }
    else
    {
        di=patch-abs(py[pnum]-py[pnum+1]);
        dj=patch-abs(px[pnum]-px[pnum+1]);
    }


    //cout << "pnode is " << pnum  << " and " << (pnum+1) << ", samq is " << samq  << " and samh is "  << samh << ", ";
    //cout << "di is " << di << ", dj is " << dj << ",    ";
    if(di==0||dj==0) return 0;

    //cai se !!!!!!!
    if(py[pnum]<py[pnum+1]&&px[pnum]<px[pnum+1])
    {
        for(int i=0;i<di;i++)
            for(int j=0;j<dj;j++)
            {
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]+radius-di,(sx[samq]+radius-dj)*3)
                        - CV_IMAGE_ELEM(src,uchar,sy[samh]-radius-1+di,(sx[samh]-radius-1+dj)*3);
                sum_ssd+=dis*dis;
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]+radius-di,(sx[samq]+radius-dj)*3+1)
                        - CV_IMAGE_ELEM(src,uchar,sy[samh]-radius-1+di,(sx[samh]-radius-1+dj)*3+1);
                sum_ssd+=dis*dis;
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]+radius-di,(sx[samq]+radius-dj)*3+2)
                        - CV_IMAGE_ELEM(src,uchar,sy[samh]-radius-1+di,(sx[samh]-radius-1+dj)*3+2);
                sum_ssd+=dis*dis;
            }
        //cout << "1 sum_ssd is"  << sum_ssd << ", ";
    }

    if(py[pnum]<py[pnum+1]&&px[pnum]>=px[pnum+1])
    {
        for(int i=0;i<di;i++)
            for(int j=0;j<dj;j++)
            {
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]+radius-di,(sx[samq]-radius-1+dj)*3)
                        -CV_IMAGE_ELEM(src,uchar,sy[samh]-radius-1+di,(sx[samh]+radius-dj)*3);
                sum_ssd+=dis*dis;
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]+radius-di,(sx[samq]-radius-1+dj)*3+1)
                        -CV_IMAGE_ELEM(src,uchar,sy[samh]-radius-1+di,(sx[samh]+radius-dj)*3+1);
                sum_ssd+=dis*dis;
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]+radius-di,(sx[samq]-radius-1+dj)*3+2)
                        -CV_IMAGE_ELEM(src,uchar,sy[samh]-radius-1+di,(sx[samh]+radius-dj)*3+2);
                sum_ssd+=dis*dis;
            }
        //cout << "2 sum_ssd is"  << sum_ssd << ", ";
    }

    if(py[pnum]>=py[pnum+1]&&px[pnum]<px[pnum+1])
    {
        for(int i=0;i<di;i++)
            for(int j=0;j<dj;j++)
            {
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]-radius-1+di,(sx[samq]+radius-dj)*3)
                        -CV_IMAGE_ELEM(src,uchar,sy[samh]+radius-di,(sx[samh]-radius-1+dj)*3);
                sum_ssd+=dis*dis;
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]-radius-1+di,(sx[samq]+radius-dj)*3+1)
                        -CV_IMAGE_ELEM(src,uchar,sy[samh]+radius-di,(sx[samh]-radius-1+dj)*3+1);
                sum_ssd+=dis*dis;
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]-radius-1+di,(sx[samq]+radius-dj)*3+2)
                        -CV_IMAGE_ELEM(src,uchar,sy[samh]+radius-di,(sx[samh]-radius-1+dj)*3+2);
                sum_ssd+=dis*dis;
            }
        //cout << "3 sum_ssd is"  << sum_ssd << ", ";
    }

    if(py[pnum]>=py[pnum+1]&&px[pnum]>=px[pnum+1])
    {
        for(int i=0;i<di;i++)
            for(int j=0;j<dj;j++)
            {
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]-radius-1+di,(sx[samq]-radius-1+dj)*3)
                        -CV_IMAGE_ELEM(src,uchar,sy[samh]+radius-di,(sx[samh]+radius-dj)*3);
                sum_ssd+=dis*dis;
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]-radius-1+di,(sx[samq]-radius-1+dj)*3+1)
                        -CV_IMAGE_ELEM(src,uchar,sy[samh]+radius-di,(sx[samh]+radius-dj)*3+1);
                sum_ssd+=dis*dis;
                dis=CV_IMAGE_ELEM(src,uchar,sy[samq]-radius-1+di,(sx[samq]-radius-1+dj)*3+2)
                        -CV_IMAGE_ELEM(src,uchar,sy[samh]+radius-di,(sx[samh]+radius-dj)*3+2);
                sum_ssd+=dis*dis;
            }
        //cout << "4 sum_ssd is"  << sum_ssd << ", ";
    }
    //cout << endl;

    double e2=sum_ssd*1.0/(di*dj*nor);
   // cout << "normalized is " << e2 << endl;
    return e2;
}


double MainWindow::energe_three(int t, int choose)
{
    int count=0;\
    int e3=0;

    for(int i=-radius;i<radius;i++)
        for(int j=-radius;j<radius;j++)
        {
            count++;

            e3+=(CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3) - CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3)) *
                     (CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3) - CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3)) +
                     (CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+1) - CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+1)) *
                     (CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+1) - CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+1)) +
                     (CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+2) - CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+2)) *
                    (CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+2) - CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+2));
        }
    //cout << endl;

    double en3=e3*1.0/count;
    return en3;
}

void MainWindow::on_circleButton_clicked()
{
    if(state==3)
    {
        later_circle=1;

        //reset
        node=0;
        circle=0;
        pnode=0;
        line_num=1;
        line_head[0]=0;
        samnum=0;

        patch=20;
        radius=patch/2;
        patchlap=5;
        samlap=5;

        //state=4;
    }
}

void MainWindow::on_fillcircleButton_clicked()
{
    if(state==3&&later_circle==1)
    { 
        flag=1;

        for(int j=0;j<w;++j)//fill_cir 初始化为黑色
            for(int i=0;i<h;++i)
                CV_IMAGE_ELEM(fill_cir,uchar,i,j)=0;

        cx[circle]=cx[0];cy[circle]=cy[0];

        cir_left=cx[0];cir_right=cx[0];
        cir_up=cy[0];cir_down=cy[0];

        for(int i=0;i<circle;i++)
        {
           cvLine(fill_cir, cvPoint(cx[i], cy[i]), cvPoint(cx[i+1], cy[i+1]), cvScalar(255,255,255));

           if(cx[i]<cir_left) cir_left=cx[i];
           if(cx[i]>cir_right) cir_right=cx[i];
           if(cy[i]<cir_up) cir_up=cy[i];
           if(cy[i]>cir_down) cir_down=cy[i];
        }

        cvFloodFill(fill_cir, cvPoint((cir_left+cir_right)/2,(cir_up+cir_down)/2), CV_RGB( 255, 255, 255));
        cvSaveImage("/home/chen/img/fill_cir.jpg",fill_cir);

        int left=cir_right, right=cir_left, up=cir_down, down=cir_up;
        //after choose select, find sam
        for(int j=cir_left;j<=cir_right;j+=samlap)
            for(int i=cir_up;i<=cir_down;i+=samlap)
            {
                if(CV_IMAGE_ELEM(fill_cir,uchar,i,j)==255)
                {
                    if(CV_IMAGE_ELEM(fill_line,uchar,i,j*3)==255 && CV_IMAGE_ELEM(fill_line,uchar,i,j*3+1)==255
                            && CV_IMAGE_ELEM(fill_line,uchar,i,j*3+2)==255)
                    {//patch
                        if(left>j) left=j;
                        if(right<j) right=j;
                        if(up>i) up=i;
                        if(down<i) down=i;
                    }
                    else//sample
                    {
                        int count=0;
                        for(int n=-radius;n<radius;n++)
                            for(int m=-radius;m<radius;m++)
                            {
                                if(CV_IMAGE_ELEM(fill_cir,uchar,i+m,j+n)!=255 || CV_IMAGE_ELEM(dst,uchar,i+m,j+n)==255)
                                    count++;
                            }

                        if(count==0)
                        {
                            sx[samnum]=j;sy[samnum]=i;
                            samnum++;
                        }
                    }
                }
            }
        cout << "circle : samnum is " << samnum << endl;

        //find pnode
        for(int j=left;j<=right+radius;j+=(patch-patchlap))
            for(int i=up;i<=down+radius;i+=(patch-patchlap))
            {
                //if(CV_IMAGE_ELEM(fill_cir,uchar,i,j)==255)
                {
                    int count=0;
                    for(int n=-radius;n<radius;n++)
                        for(int m=-radius;m<radius;m++)
                        {
                            if(CV_IMAGE_ELEM(fill_cir,uchar,i+m,j+n)==255 && CV_IMAGE_ELEM(dst,uchar,i+m,j+n)!=255)
                                count++;
                        }

                    if(count < patch*patch)
                    {
                        px[pnode]=j;py[pnode]=i;
                        pnode++;
                    }
                }
            }
        cout << "circle : pnode is " << pnode << endl;

        //fill circle
        for(int t=0;t<pnode;t++)
        {
            int choose=0;
            double e2;
            double min_e2=energe_three(t,0);
            for(int i=1;i<samnum;i++)
            {
                e2=energe_three(t,i);
                if(e2<min_e2)
                {
                    e2=min_e2;
                    choose=i;
                }
            }
            cout << "choose is " << choose << endl;

            for(int i=-radius;i<radius;i++)
                for(int j=-radius;j<radius;j++)
                {
                    if (CV_IMAGE_ELEM(fill_cir,uchar,py[t]+i,(px[t]+j))==255)
                    {
                        if (CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3)==255&&CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+1)==255
                                &&CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+2)==255)
                        {
                            CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3);
                            CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+1)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+1);
                            CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+2)=CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+2);
                        }
                        else//overlap
                        {
                            if(CV_IMAGE_ELEM(all_line,uchar,py[t]+i,px[t]+j)==0)
                            {
                                double rate=1-(i*i+j*j)/(2*(radius-patchlap*1.0/2)*(radius-patchlap*1.0/2));
                                if(rate<0) rate=0;

                                CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3) = ((1-rate) * CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3)
                                                                                      + rate *CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3));
                                CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+1) = ((1-rate) * CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+1)
                                                                                        + rate * CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+1));
                                CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+2) = ((1-rate) * CV_IMAGE_ELEM(fill_line,uchar,py[t]+i,(px[t]+j)*3+2)
                                                                                        + rate * CV_IMAGE_ELEM(src,uchar,sy[choose]+i,(sx[choose]+j)*3+2));
                            }
                        }
                    }
                }
            //cout << endl;
        }
    }

    cvSaveImage("/home/chen/img/fill_line_circle.jpg",fill_line);
    cout << "done!!" << endl;
}


int MainWindow::cross_line(int ln, int cr)
{
    double k1, k2, b1, b2;
    double clx, cly;
    joi_num=0;

    if(x[ln]!=x[ln+1])
    {
        k1=(y[ln+1]-y[ln])*1.0/(x[ln+1]-x[ln]);
        b1=(x[ln+1] * y[ln] -  y[ln+1] * x[ln]) * 1.0 / (x[ln+1]-x[ln]);
    }
    else  {k1=0.0; b1=x[ln]*1.0;}

    if(cx[cr]!=cx[cr+1])
    {
        k2=(cy[cr+1]-cy[cr])*1.0/(cx[cr+1]-cx[cr]);
        b2=(cx[cr+1] * cy[cr] -  cy[cr+1] * cx[cr]) * 1.0 / (cx[cr+1]-cx[cr]);
    }
    else {k2=0.0; b2=cx[cr]*1.0;}

    if(k1!=k2)
    {
        clx=(b2-b1)/(k1-k2);
        cly=(k1*b2-k2*b1)/(k1-k2);

        int lnmax, lnmin, crmax, crmin;

        if(x[ln]>x[ln+1]) {lnmax=x[ln]; lnmin=x[ln+1];}
        else {lnmax=x[ln+1]; lnmin=x[ln];}

        if(cx[cr]>cx[cr+1]) {crmax=cx[cr]; crmin=cx[cr+1];}
        else {crmax=cx[cr]; crmin=cx[cr+1];}

        if(clx>=lnmin&&clx<=lnmax&&clx>=crmin&&clx<=crmax)
        {
            joint[joi_num*2]=round(clx);
            joint[joi_num*2+1]=round(cly);
            //ui->label->setText(tr("(%1,%2)").arg(joint[joi_num*2]+30).arg(joint[joi_num*2+1]+30));
            joi_num++;
            return 1;
        }
        else return 0;
    }

    return 0;
}












































