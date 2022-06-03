/************************************************
* Author:Jing Xu, Lishan Huang, Xiaoxuan Yang, Hongjian Cui, Pu Huang
*  Description of file contents: The file executes the music player and listens to user's actions and react correspondingly
*  Date:Dec, 02, 2020
***********************************************/
#include "mainwindow.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
