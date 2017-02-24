#include "musicplaylistfoundwidget.h"
#include "musicsourcedownloadthread.h"
#include "musicdownloadquerywyplaylistthread.h"
#include "musicplaylistfoundinfowidget.h"
#include "musicdownloadqueryfactory.h"
#include "musictinyuiobject.h"

#include <QPushButton>
#include <QGridLayout>
#include <QScrollArea>
#include <QStackedWidget>

#define MIN_LABEL_SIZE  150
#define MAX_LABEL_SIZE  200

MusicPlaylistFoundItemWidget::MusicPlaylistFoundItemWidget(QWidget *parent)
    : QLabel(parent)
{
    setFixedSize(MIN_LABEL_SIZE, MAX_LABEL_SIZE);

    m_topListenButton = new QPushButton(this);
    m_topListenButton->setGeometry(0, 0, MIN_LABEL_SIZE, 20);
    m_topListenButton->setIcon(QIcon(":/tiny/btn_listen_hover"));
    m_topListenButton->setText(" - ");
    m_topListenButton->setStyleSheet(MusicUIObject::MBackgroundStyle08 + MusicUIObject::MColorStyle01);

    m_playButton = new QPushButton(this);
    m_playButton->setGeometry(110, 110, 30, 30);
    m_playButton->setCursor(Qt::PointingHandCursor);
    m_playButton->setStyleSheet(MusicUIObject::MKGTinyBtnPlaylist);
    connect(m_playButton, SIGNAL(clicked()), SLOT(currentPlayListClicked()));

    m_iconLabel = new QLabel(this);
    m_iconLabel->setGeometry(0, 0, MIN_LABEL_SIZE, MIN_LABEL_SIZE);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setGeometry(0, 150, MIN_LABEL_SIZE, 25);
    m_nameLabel->setText(" - ");

    m_creatorLabel = new QLabel(this);
    m_creatorLabel->setGeometry(0, 175, MIN_LABEL_SIZE, 25);
    m_creatorLabel->setText("by anonymous");
}

MusicPlaylistFoundItemWidget::~MusicPlaylistFoundItemWidget()
{
    delete m_topListenButton;
    delete m_playButton;
    delete m_iconLabel;
    delete m_nameLabel;
    delete m_creatorLabel;
}

QString MusicPlaylistFoundItemWidget::getClassName()
{
    return staticMetaObject.className();
}

void MusicPlaylistFoundItemWidget::setMusicPlaylistItem(const MusicPlaylistItem &item)
{
    m_itemData = item;
    m_nameLabel->setText(item.m_name);
    m_creatorLabel->setText("by " + item.m_nickname);
    m_topListenButton->setText(item.m_playCount);

    MusicSourceDownloadThread *download = new MusicSourceDownloadThread(this);
    connect(download, SIGNAL(downLoadByteDataChanged(QByteArray)), SLOT(downLoadFinished(QByteArray)));
    download->startToDownload(item.m_coverUrl);
}

void MusicPlaylistFoundItemWidget::downLoadFinished(const QByteArray &data)
{
    QPixmap pix;
    pix.loadFromData(data);
    m_iconLabel->setPixmap(pix.scaled(m_iconLabel->size()));
    m_topListenButton->raise();
    m_playButton->raise();
}

void MusicPlaylistFoundItemWidget::currentPlayListClicked()
{
    emit currentPlayListClicked(m_itemData);
}



MusicPlaylistFoundWidget::MusicPlaylistFoundWidget(QWidget *parent)
    : MusicFoundAbstractWidget(parent)
{
    m_container = new QStackedWidget(this);
    layout()->removeWidget(m_mainWindow);
    layout()->addWidget(m_container);
    m_container->addWidget(m_mainWindow);

    m_firstInit = false;
    m_infoWidget = nullptr;
    m_downloadThread = M_DOWNLOAD_QUERY_PTR->getPlaylistThread(this);
    connect(m_downloadThread, SIGNAL(createPlaylistItems(MusicPlaylistItem)),
                              SLOT(queryAllFinished(MusicPlaylistItem)));
}

MusicPlaylistFoundWidget::~MusicPlaylistFoundWidget()
{
    delete m_infoWidget;
//    delete m_container;
    delete m_downloadThread;
}

QString MusicPlaylistFoundWidget::getClassName()
{
    return staticMetaObject.className();
}

void MusicPlaylistFoundWidget::setSongName(const QString &name)
{
    MusicFoundAbstractWidget::setSongName(name);
    m_downloadThread->startSearchSong(MusicDownLoadQueryThreadAbstract::MovieQuery, QString());
}

void MusicPlaylistFoundWidget::resizeWindow()
{
    if(!m_resizeWidget.isEmpty())
    {
        QGridLayout *grid = MStatic_cast(QGridLayout*, m_mainWindow->layout());
        if(grid)
        {
            for(int i=0; i<m_resizeWidget.count(); ++i)
            {
                grid->removeWidget(m_resizeWidget[i]);
            }

            int lineNumber = width()/MAX_LABEL_SIZE;
            for(int i=0; i<m_resizeWidget.count(); ++i)
            {
                grid->addWidget(m_resizeWidget[i], i/lineNumber,
                                                   i%lineNumber, Qt::AlignCenter);
            }
        }
    }
}

void MusicPlaylistFoundWidget::queryAllFinished(const MusicPlaylistItem &item)
{
    delete m_statusLabel;
    m_statusLabel = nullptr;

    if(!m_firstInit)
    {
        m_container->removeWidget(m_mainWindow);
        QScrollArea *scrollArea = new QScrollArea(this);
        scrollArea->setStyleSheet(MusicUIObject::MScrollBarStyle01);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setAlignment(Qt::AlignLeft);
        scrollArea->setWidget(m_mainWindow);
        m_container->addWidget(scrollArea);

        m_firstInit = true;
        delete m_mainWindow->layout();
        QGridLayout *gridLayout = new QGridLayout(m_mainWindow);
        gridLayout->setVerticalSpacing(35);
        m_mainWindow->setLayout(gridLayout);
    }

    QGridLayout *grid = MStatic_cast(QGridLayout*, m_mainWindow->layout());

    MusicPlaylistFoundItemWidget *label = new MusicPlaylistFoundItemWidget(this);
    connect(label, SIGNAL(currentPlayListClicked(MusicPlaylistItem)), SLOT(currentPlayListClicked(MusicPlaylistItem)));
    label->setMusicPlaylistItem(item);

    int lineNumber = width()/MAX_LABEL_SIZE;
    grid->addWidget(label, m_resizeWidget.count()/lineNumber,
                           m_resizeWidget.count()%lineNumber, Qt::AlignCenter);
    m_resizeWidget << label;
}

void MusicPlaylistFoundWidget::currentPlayListClicked(const MusicPlaylistItem &item)
{
    delete m_infoWidget;
    m_infoWidget = new MusicPlaylistFoundInfoWidget(this);
    m_infoWidget->setQueryInput(M_DOWNLOAD_QUERY_PTR->getPlaylistThread(this));
    m_infoWidget->setMusicPlaylistItem(item, this);
    m_container->addWidget(m_infoWidget);
    m_container->setCurrentIndex(1);
}

void MusicPlaylistFoundWidget::backToPlayListMenu()
{
    m_container->setCurrentIndex(0);
}

void MusicPlaylistFoundWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    resizeWindow();
}