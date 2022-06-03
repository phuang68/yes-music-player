/************************************************
* Author:Jing Xu, Lishan Huang, Xiaoxuan Yang, Hongjian Cui, Pu Huang
*  Description of file contents: The file consists the main window class that will display the music player and
*       the functions of the buttons and decides what the player does depending on what the user action is.
*  Date:Dec,02,2020
***********************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

/************************************************
 * @brief This class function initializes the music player
 * @param parent Using the Qwidget
 * @authors Jing Xu, Lishan Huang, Xiaoxuan Yang, Hongjian Cui, Pu Huang
 * @return Return an initialized mainwindow
 ***********************************************/
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setAutoFillBackground(true);

    QPixmap pixmap = QPixmap(":/buttons/background.png").scaled(this->size());  // Set the background picture for the music player 
    QPalette palette (this->palette());
    palette.setBrush(QPalette::Background, QBrush(pixmap));
    this->setPalette(palette);

    ui->stackedWidget_lists->setCurrentWidget(ui->page_playlist);

    QFile file(":/Style.qss");
    file.open(QFile::ReadOnly);
    ui->listWidget_playlist->verticalScrollBar()->setStyleSheet(file.readAll());    //Beautify the list widget
    ui->listWidget_playlist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFile file3(":/Style3.qss");
    file3.open(QFile::ReadOnly);
    ui->listWidget_history->verticalScrollBar()->setStyleSheet(file3.readAll()); //Beautify the list widget
    ui->listWidget_history->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFile fileSlider(":/slider2.qss");
    fileSlider.open(QFile::ReadOnly);
    ui->horizontalSlider_progress->setStyleSheet(fileSlider.readAll());   //Beautify the horizontal slider

    QFile fileSlider2(":/slider3.qss");
    fileSlider2.open(QFile::ReadOnly);
    ui->verticalSlider_volume->setStyleSheet(fileSlider2.readAll()); //Beautify the vertical slider

    QFile texteditfile(":/texteditfile.qss");   //Lyrics display
    texteditfile.open(QFile::ReadOnly);
    ui->textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->textEdit->setAlignment(Qt::AlignCenter); // Align the lyrics in the centre

    ui->listWidget_playlist->setFrameStyle(QFrame::NoFrame);   //Removing the frames
    ui->listWidget_history->setFrameStyle(QFrame::NoFrame);

    player = new QMediaPlayer; //Initialize the player object
    playerlist = new QMediaPlaylist; // Initialize the playlist
    playerlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop); // Set the default playmode to play all the songs in sequence 
    player->setPlaylist(playerlist); // Add the intial playlist to the player

    connect(player, SIGNAL(metaDataAvailableChanged(bool)), this, SLOT(OnMetaDataAvailableChanged(bool)));
    connect(player,&QMediaPlayer::metaDataAvailableChanged,this,&MainWindow::update_file);  //Update what's playing
    connect(player,&QMediaPlayer::metaDataAvailableChanged,this,&MainWindow::update_info);
    connect(player,&QMediaPlayer::positionChanged,this,&MainWindow::updatePosition);   //Update the position in the progress timeline
    connect(player,&QMediaPlayer::durationChanged,this,&MainWindow::updateDuration);   //Update the progress timeline

    connect(ui->horizontalSlider_progress,SIGNAL(valueChanged(int)),this,SLOT(on_horizontalSlider_progress_valueChanged(int)));
    connect(playerlist,&QMediaPlaylist::currentIndexChanged,this,&MainWindow::lrc);  //Display the lyrics
    connect(playerlist,&QMediaPlaylist::currentIndexChanged,this,&MainWindow::refresh_all);

    ui->verticalSlider_volume->hide(); // The volume bar is set invisile normally
    ui->verticalSlider_volume->setValue(50);  //The default value of the volme is set to 50
    connect(ui->verticalSlider_volume,SIGNAL(valueChanged(int)),this,SLOT(adjustVolume()));  //When volume is adjusted, this will be invoked

    connect(ui->listWidget_playlist,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(DoubleClicked(QListWidgetItem *))); //Double click the song name, the song will be played consequently
    connect(ui->listWidget_history,SIGNAL(itemDoubleClicked(QListWidgetItem *)),this,SLOT(DoubleClicked(QListWidgetItem *)));

    connect(ui->pushButton_close, SIGNAL(clicked()), this, SLOT(closewin()));   //Close the player window
    ui->pushButton_playlist->setToolTip(tr("Play List"));
    ui->pushButton_history->setToolTip(tr("History"));
    ui->pushButton_add_file->setToolTip(tr("Add Songs"));

    ui->stackedWidget_all->setCurrentWidget(ui->page_refresh);

    isPlay=false; // Boolean parameter to check if an audio is playing
    is_favorite=false; // Boolean parameter to check if an audio is in favourite list
    volume_slider_present=false; // Boolean parameter to check if the volume slider is visible/invisible

    QVector<LrcFormat>lrclist; // Vector to store lyrics

    currentPos=0; //There's 0 files in the normal list intially

    favor_index=0; //There's 0 files in the favourite list initially    currentPos=0; //There's 0 files in the normal list intially

    favor_index=0; //There's 0 files in the favourite list initially

}

/************************************************
 * @brief This is a destructor for the main window of the music player
 * @authors Pu Huang
 * @return no return type
 ***********************************************/
MainWindow::~MainWindow()
{
    delete ui;
}

/************************************************
 * @brief This fuction allow the user to adjust the progress bar of current audio file..
 * @authors Lishan Huang
 * @param value of int type
 * @return no return type
 ***********************************************/
void MainWindow::on_horizontalSlider_progress_valueChanged(int value)
{
    if( qAbs (value - player->position()) > 2000 && (ui->horizontalSlider_progress->value() !=0))
    {
        player->stop(); // First we stop the music
        player->setPosition(value); // Set the position that was dragged to
        player->play(); //Play the audio file from there
    }
}

/************************************************
 * @brief This fuction allow the user to see the duration of current audio file.
 * @authors Xiaoxuan Yang
 * @param availability of boolean type
 * @return no return type
 ***********************************************/
void MainWindow::OnMetaDataAvailableChanged(bool availability)
{
    // get audio file duration from metadata
    qint64 duration = player->metaData("Duration").toInt();
    if (availability && duration!=0)
    {
        int minute = (duration/1000) / 60;
        int second = (duration/1000) % 60;

        QString time;
        //formatting how to display duration
        if(second<10)
        {
            time = QString("%1:0%2").arg(minute).arg(second);
        }
        else
        {
            time = QString("%1:%2").arg(minute).arg(second);
        }
    }
}

/************************************************
 * @brief This fuction allow the user to play the audio file by double clicking the file.
 * @authors Jing Xu
 * @param item of QListWidgetItem type
 * @return no return type
 ***********************************************/
void MainWindow::DoubleClicked(QListWidgetItem * item)
{
    int index=0;
    QString text=item->text(); //Take in the double clicked file
    while(text!=audio_list[index]) //Find the file in the audio list
    {
       index++;
    }
    playerlist->setCurrentIndex(index); //Play the double clicked file once the index is found
}

/************************************************
 * @brief This fuction shows the playlist when "show playlist" is clicked.
 * @authors Pu Huang
 * @return no return type
 ***********************************************/
void MainWindow::on_pushButton_playlist_clicked()
{
    ui->stackedWidget_lists->setCurrentWidget(ui->page_playlist);
}

/************************************************
 * @brief This fuction shows the history when "show history" is clicked.
 * @authors Lishan Huang
 * @return no return type
***********************************************/
void MainWindow::on_pushButton_history_clicked()
{
    ui->stackedWidget_lists->setCurrentWidget(ui->page_history);
}

/************************************************
 * @brief This fuction allows the user to add audio files from his or her device.
 * Audio files selected by the users will be added to the playlist.
 * @authors Xiaoxuan Yang, Lishan Huang,  Jing Xu, Pu Huang
 * @return no return type
***********************************************/
void MainWindow::on_pushButton_add_file_clicked()
{
    int init_pos=0;
    bool not_exist=true;
    QString audioName;
    //get one or more existing files selected by the user
    QStringList audioPathList = QFileDialog::getOpenFileNames(this, tr("Open File"),"",tr("music(*.mp3 *.wav *.wma)"));
    
    //If there is at least one file selected
    if(!audioPathList.isEmpty())
    {
        int n=currentPos;
        for(int i=0; i<audioPathList.size(); ++i)
        {
            QString path=QDir::toNativeSeparators(audioPathList.at(i));
            for(int j=0;j<currentPos;j++)
            {
                //if the file is already imported
                if(path==arrPath[j])
                {
                    not_exist=false;
                    init_pos--;
                    break;
                }
            }
            //If the file is not imported, add it to the playlist and the list of audio names.
            if(not_exist)
            {
                playerlist->addMedia(QUrl::fromLocalFile(path));
                audioName = path.split("\\").last();
                QListWidgetItem *item = new QListWidgetItem(QIcon(":/buttons/logo.png"),audioName); //Add the file to the play list in the format of songname
                item->setToolTip(audioName);
                ui->listWidget_playlist->addItem(item);
                path_list[n+init_pos]=path;
                audio_list[n+init_pos]=audioName;
                arrPath[n+init_pos]=path;
                currentPos++;
            }
            init_pos++; //increment total number of audio files in the playlist by 1
        }
    }
}

/************************************************
 * @brief This fuction handles the event of "palying next file." 
 * If current mode is "random", then randomly select a file to play.
 * If current mode is "repeat all" or "repeat once", get the previous audio file's index.
 * @authors Xiaoxuan Yang, Hongjian Cui, Pu Huang
 * @return no return type
 ***********************************************/
void MainWindow::on_pushButton_previous_clicked()
{
    int current_index=playerlist->currentIndex();
    int total_num_of_audio_file=playerlist->mediaCount();

    // If current mode is random, select a random audio file as the previous audio file
    if(playerlist->playbackMode()==QMediaPlaylist::Random) 
    {
        //generate a random number between 0 and (total_num_of_audio_file-1) as the index of next audio file
        current_index=qrand()%total_num_of_audio_file;  
    }
    else
    {
        //If this is the first audio file in the playlist, get the last audio file as the previous file
        if(current_index == 0)
        {
            current_index=playerlist->mediaCount();
        }
        --current_index;
    }
    playerlist->setCurrentIndex(current_index);     //set previous audio file
}

/************************************************
 * @brief This fuction handles the event of current index change. 
 * The current index will change when the user clicks "next" or "previouis."
 * Whenever the current index chancges we update the favorite icon and the history.
 * @authors Hongjian Cui, Pu Huang
 * @return no return type
 ***********************************************/
void MainWindow::refresh_all()
{
    ui->stackedWidget_all->setCurrentWidget(ui->page_refresh);
    int current_index=playerlist->currentIndex();

    //If the current audio file is not marked as played (i.e not in the history list), add it to the history.
    if(check_in_history[current_index]!=1)
    {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/buttons/haveplayed.png"),audio_list[current_index]);
        item->setToolTip(audio_list[current_index]);
        ui->listWidget_history->addItem(item);
        check_in_history[current_index]=1;
    }

    //hide volume bar
    ui->verticalSlider_volume->hide();
}

/************************************************
 * @brief This fuction handles the event of "palying next file." 
 * If current mode is "random", then randomly select a file to play.
 * If current mode is "repeat all" or "repeat once", get the next audio file's index.
 * @author Jing Xu
 * @return no return type
 ***********************************************/
void MainWindow::on_pushButton_next_clicked()
{
    int current_index=playerlist->currentIndex();
    int total_num_of_audio_file=playerlist->mediaCount();
    
    // If current mode is random, select a random audio file as the next audio file
    if(playerlist->playbackMode()==QMediaPlaylist::Random)
    {
        current_index = qrand()%total_num_of_audio_file; //generate a random number between 0 and (total_num_of_audio_file-1) as the index of next audio file
    }
    else {
         if(current_index == (total_num_of_audio_file-1))
         {
             current_index = -1;
         }
             ++current_index;
    }
    playerlist->setCurrentIndex(current_index);
}

/************************************************
 * @brief This fuction handles the event of "switching playback mode" and "changing the playback mode icon"
 * If the current playback mode is shuffle, playback mode will change to repeat all after the button is clicked. 
 * If the current playback mode is repeat all, playback mode will change to repeat one after the button is clicked. 
 * If the current playback mode is repeat one, playback mode will change to shuffle after the button is clicked. 
 * @author Xiaoxuan Yang, Hongjian Cui, Pu Huang
 * @return no return type
 ***********************************************/
void MainWindow::on_pushButton_mode_clicked() //Play mode in sequence
{
    switch (mode_select)    //switch playback mode
    {
    // switch to different playback mode with the value of play_mode variable "mode_select"
    case shuffle:
        ui->pushButton_mode->setStyleSheet("border-image:url(:/buttons/random.png)"); // set the playback icon to shuffle
        playerlist->setPlaybackMode(QMediaPlaylist::Random); //Repeat the all the songs in the playlist
        mode_select=repeat_all;
        break;
    case repeat_all:
        ui->pushButton_mode->setStyleSheet("border-image:url(:/buttons/loop.png)"); // set the playback icon to repeat
        playerlist->setPlaybackMode(QMediaPlaylist::Loop); //Repeat the all the songs in the playlist
        mode_select=repeat_one;
        break;
    case repeat_one:
        ui->pushButton_mode->setStyleSheet("border-image:url(:/buttons/playloop.png)"); // set the playback icon to repeat
        playerlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop); //Repeat the selected song
        mode_select=shuffle;
        break;    
    default:
        break;
    }
}

/************************************************
 * @brief This fuction handles the event of "setting the name of current playing music to label component of Qt UI" 
 * This function reads the "Title" from the metaData and set the title name to the UI label component.
 * If the title of the music cannot be found, the label will show "Could not find file name!"
 * @author Jing Xu, Lishan Huang
 * @return no return value
 ***********************************************/
void MainWindow::update_file()
{
    QString file_name="Title: ";
    if(player->isMetaDataAvailable())
    {
        QString name=player->metaData(QStringLiteral("Title")).toString();
        if(name.isEmpty())
            file_name+=("Could not find file name!");
        else
            file_name+=name;
    }
    ui->label_file->setText(file_name);
}

/************************************************
 * @brief This fuction handles the event of "setting the artist name of current playing music to lineEdit component of Qt UI" 
 * This function reads the "Author" from the metaData and set the artist name to the UI lineEdit component.
 * If the artist name of the music cannot be found, the label will show "Could not find artist name!"
 * @author Xiaoxuan Yang, Hongjian Cui, Pu Huang
 * @return no return value
 ***********************************************/
void MainWindow::update_info() 
{
    QString file_info="Artist: ";;
    if(player->isMetaDataAvailable())
    {
        QString artist=player->metaData(QStringLiteral("Author")).toString();
        if(artist.isEmpty())
            file_info+=("Could not find artist name!");
        else
            file_info+=artist;
    }
    ui->lineEdit_info->setText(file_info);
}

/************************************************
 * @brief This fuction handles the event of "Converting the format time showing beside the playing progress bar"
 * This function will convert the time input into format of "minute:second"
 * @author Hongjian Cui, Pu Huang
 * @return no return value
 ***********************************************/
static QString Time(qint64 time)
{
    int second=time/1000;
    int minute = second/60;
    second = second % 60;
    return QStringLiteral("%1:%2")
            .arg(minute, 2, 10, QLatin1Char('0'))
            .arg(second, 2, 10, QLatin1Char('0'));
}

/************************************************
 * @brief This fuction handles the event of "updating the position of progress bar and showing time on the lineEdit UI component" 
 * after the prograss slider bar has been adjusted to new position.
 * This function will set the profress bar to assigned position. 
 * Then, this function will convert the duration with time covert function and set to lineEdit UI component.
 * @author Jing Xu, Lishan Huang
 * @return no return value
 ***********************************************/
void MainWindow::updatePosition(qint64 slider_position)
{
    //int maxHeight=ui->textEdit->document()->size().height();
    ui->horizontalSlider_progress->setValue(slider_position);
    ui->lineEdit_time->setText(Time(slider_position)+"/"+Time(player->duration()));
    update_lyric_position(slider_position);
}

/************************************************
 * @brief This fuction handles the event of "updating the lyric window" after the progress bar been adjusted to a new position.
 * This function reads the total height of the lyric file and divide the lyric file evenly by the duration of the music.
 * Then, once the progree bar been adjusted to a new position, the lyric UI scroll window will be updated to the new position.
 * @author Xiaoxuan Yang, Hongjian Cui, Pu Huang
 * @return no return value 
 ***********************************************/
void MainWindow::update_lyric_position(qint64 slider_position)
{
    int total_Height=ui->textEdit->document()->size().height();
    int curPostition;
    curPostition= static_cast<int>(slider_position);
    for(QVector<LrcFormat>::iterator start_point=lrclist.begin();start_point!=lrclist.end();start_point++)
    {
        if(Time(slider_position)==Time(start_point->ms))
        {
            if(curPostition!=0)
            {
                float shown_line=(curPostition*total_Height/player->duration());
                ui->textEdit->verticalScrollBar()->setValue(shown_line);
            }
        }
    }
}

/************************************************
 * @brief This fuction handles the event of "updating the duration of all UI component" with a new playing music.
 * This function updates the range of playing pprogress bar and lyric scroll bar with new 'duration' input value
 * @author Jing Xu, Lishan Huang
 * @param duration of qint64 type
 * @return no return value
 ***********************************************/
void MainWindow::updateDuration(qint64 duration)
{
    ui->horizontalSlider_progress->setRange(0,duration);
    ui->horizontalSlider_progress->setEnabled(duration>0);
    ui->horizontalSlider_progress->setPageStep(duration/10);

    ui->textEdit->verticalScrollBar()->setRange(0,duration);
    ui->textEdit->verticalScrollBar()->setPageStep(duration/10);
}

/************************************************
 * @brief This function handles the event of "reading and converting lyric files for sending to lyric window"
 * This function reads through the lyric file and convert the lyric to string.
 * After the lyric been converted, it will be sent to textEdit UI component.
 * @author Jing Xu, Lishan Huang, Xiaoxuan Yang, Hongjian Cui, Pu Huang
 * @return no return value
 ***********************************************/
void MainWindow::lrc()
{
    int current=playerlist->currentIndex(); // Let current takes the current audio file
    ui->textEdit->clear();   //Clear the lyrics panel
    lrclist.clear();    //Clear the lyrics container
    QString lrcPath = arrPath[current].mid(0, arrPath[current].lastIndexOf(".")) + QString(".lrc");  // Take in the path of the lyrics

    QFile file(lrcPath);
    if( file.open(QIODevice::ReadOnly | QIODevice::Text) )
    {
        QTextStream stream(&file); //Tale in the file
        QString readStr = stream.readAll(); // Read the file

        QRegularExpression rex("\\[(\\d+)?:(\\d+\\.\\d+)?\\]"); // Regular expression
        QRegularExpressionMatch match = rex.match(readStr);

        readStr = readStr.mid(match.capturedStart(), readStr.length() - match.capturedStart()); //Ignore the first line

        while(true)
        {
            LrcFormat lrc;
            lrc.ms = static_cast<unsigned int>((match.captured(1).toInt() * 60000 + match.captured(2).toDouble() * 1000)); // Get the milliesecond
            lrc.lrc = readStr.mid(match.capturedLength(), readStr.indexOf("\n") - match.capturedLength() + 1).simplified(); // Get the lyrics 
            readStr = readStr.mid(readStr.indexOf("\n") + 1, readStr.length() - readStr.indexOf("\n")); // Get the trimmed string

            match = rex.match(readStr);
            lrclist.append(lrc); // QVector<LrcFormat> 

            if( readStr.indexOf("\n") < 0 ) // Reach to the end of the string
            {
                LrcFormat end;
                end.ms = static_cast<unsigned int>((match.captured(1).toInt() * 60000 + match.captured(2).toDouble() * 1000));
                end.lrc = readStr.mid(match.capturedLength(), readStr.indexOf("\n") - match.capturedLength() + 1).simplified();
                lrclist.append(end);
                break;
            }
        }

        file.close();
        for(QVector<LrcFormat>::iterator iter1=lrclist.begin();iter1!=lrclist.end();iter1++)
        {
            ui->textEdit->append(iter1->lrc);  //Display the lyrics
            ui->textEdit->setAlignment(Qt::AlignCenter); //Align the lyrics to the centre
        }
    }
}

/************************************************
 * @brief This fuction handles the event of "showing and hiding volume adjustment slider after the volume button been clicked".
 * If the volume slider bar is shown, the volume slider bar will be hidden after the button is clicked. 
 * If the volume silder bar is shown, the volume slider bar will be shown after the button is clicked.
 * @author Jing Xu, Lishan Huang
 * @return no return value
 ***********************************************/
void MainWindow::on_pushButton_volume_clicked() 
{
    if(volume_slider_present) // Check if the volume slider is visible
    {
        ui->verticalSlider_volume->show();
        volume_slider_present=false; // Make it false, next time when it's click, we hid the volume bar
    }
    else
    {
        ui->verticalSlider_volume->hide();
        volume_slider_present=true;
    }
}

/************************************************
 *@brief This fuction handles the event of "palying the selected music" once the buttons been clicked.
 *@author Xiaoxuan Yang, Hongjian Cui, Pu Huang
 *@return no return value
 ***********************************************/
void MainWindow::on_pushButton_play_clicked(){
    player->play();
}

/************************************************
 * @brief This fuction handles the event of "pausing the selected music" once the buttons been clicked.
 * @author Jing Xu
 * @return no return value
 ***********************************************/
void MainWindow::on_pushButton_pause_clicked(){
    player->pause();
}

/************************************************
 * @brief This fuction handles the event of "setting the value of vlolume with the position of the volume slider bar"
 * @author Lishan Huang
 * @return no return value
 ***********************************************/
void MainWindow::adjustVolume()   //Adjust the value of volume
{
    player->setVolume(ui->verticalSlider_volume->value());
}

/************************************************
 * @brief This function handles the event of "closing the window and terminate the program"
 * @author Hongjian Cui
 * @return no return value
 ***********************************************/
void MainWindow::closewin()
{
    close();
}

