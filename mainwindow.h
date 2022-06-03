#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/************************************************
* Author:Jing Xu, Lishan Huang, Xiaoxuan Yang, Hongjian Cui, Pu Huang
*  Description of file contents: The header file contains the main window class that will display the music player and
*       the functions of the buttons and decides what the player does depending on what the user action is.
*  Date: Dec, 02, 2020
***********************************************/
#include <QApplication>
#include <QMainWindow>
#include<QIcon>
#include<QPainter>
#include<QPixmap>
#include<QPalette>
#include<QFrame>
#include<QMediaPlayer>
#include<QMediaPlaylist>
#include<QGraphicsOpacityEffect>
#include<QListWidget>
#include<QListWidgetItem>
#include<QFileInfo>
#include<QFile>
#include<QFileDialog>
#include<QMessageBox>
#include<QScrollBar>
#include<QListView>
#include<QPushButton>
#include<QSlider>
#include<QMouseEvent>
#include<option.h>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; } // Intialize UI namespace
QT_END_NAMESPACE


typedef struct _tLrcFormat //Lyrics format
{
    unsigned int ms;
    QString lrc;
} LrcFormat;


class MainWindow : public QMainWindow
{
    Q_OBJECT // Moc compiler will read C++ header file and create a MOC file for the mainwindow when they find Q_OBJECT

public:
    //UI class constructor
    MainWindow(QWidget *parent = nullptr);
    //UI class desctructor
    ~MainWindow();

    QMediaPlayer *player;
    QMediaPlaylist *playerlist;

private:

    Ui::MainWindow *ui;
    //Data members
    bool isPlay; // Boolean parameter to check if an audio is playing
    bool is_favorite; // Boolean parameter to check if an audio is in favourite list
    play_mode mode_select=shuffle; // Enum parameter to determine the play mode
    bool volume_slider_present; // Boolean parameter to check if the volume slider is visible/invisible

    QVector<LrcFormat>lrclist; // Vector to store lyrics

    QString path_list[1000]; // String array to store the audio file paths
    QString audio_list[1000]; // String array to store the audio file name

    QString arrPath[1000]; //To check if the the imported files are duplicated
    int currentPos; //There's 0 files in the normal list intially

    int check_in_favor[1000]; //To check if the audio file is in the favourite list
    int check_in_history[1000]; //To check if the audio file is duplicated played in the history

    QString favor_list[1000]; //To store the names of the favourite audio files.
    int favor_index; //There's 0 files in the favourite list initially

    QPoint mousePosition;

    bool mouse_pressed;

    void update_file();

    void update_info();

    void updatePosition(qint64 position);

    void update_lyric_position(qint64 position);

    void updateDuration(qint64 duration);

    void refresh_all();


private slots:
    void lrc();

    void on_pushButton_playlist_clicked();

    void on_pushButton_history_clicked();

    void on_pushButton_add_file_clicked();

    void on_pushButton_previous_clicked();

    void on_pushButton_play_clicked();

    void on_pushButton_pause_clicked();

    void on_pushButton_next_clicked();

    void on_pushButton_mode_clicked();

    void OnMetaDataAvailableChanged(bool available);

    void on_pushButton_volume_clicked();

    void adjustVolume();

    void DoubleClicked(QListWidgetItem *);

    void on_horizontalSlider_progress_valueChanged(int);

    void closewin();

    
};
#endif // MAINWINDOW_H
