// POSIX headers
#include <unistd.h>

// MythTV headers
#include <mythuibutton.h>
#include <mythuibuttonlist.h>
#include <mythmainwindow.h>
#include <mythcontext.h>
#include <mythdirs.h>
#include <mythtv/mythdialogs.h>
#include <QSettings>

// MythMPD headers
#include "libmpdclient.h"
#include "mythmpd.h"
#include "mythmpd_playqueue.h"

MythMPD_PlayQueue::MythMPD_PlayQueue(MythScreenStack *parent,
                                   MythScreenType *previous,
                                   const char *name) :
                   MythScreenType(parent, name)
                 , m_generalScreen(previous)
                 , m_buttonBack(NULL)
{
    m_generalScreen->Hide();
}

bool MythMPD_PlayQueue::Create()
{
    bool err = false;
    bool foundtheme = false;
    mpd_InfoEntity *entity;
    mpd_Song *song;
    int counter = 0;

    foundtheme = LoadWindowFromXML("mythmpd-ui.xml", "PlayQueue", this);
    if (!foundtheme)
        return -1;
    UIUtilE::Assign(this, m_buttonlistPlayQueue, "buttonlist_PlayQueue", &err);
    UIUtilE::Assign(this, m_buttonBack,          "button_Back"         , &err);

    // Get current playlist contents.
    VERBOSE(VB_IMPORTANT, "MythMPD_PlayQueue: Getting current playlist");
    mpd_sendPlaylistInfoCommand(conn,-1);
    entity = mpd_getNextInfoEntity(conn);
    VERBOSE(VB_IMPORTANT, "MythMPD_PlayQueue: Showing playlist");
    m_buttonlistPlayQueue->Reset();
    while (entity)
    {
        song = entity->info.song;
        int thisPosition = song->pos;
        counter++;
        char buf[256];
        sprintf(buf, "%s / %s / %s / %s", song->title, song->album, song->artist, song->file);
        MythUIButtonListItem *playlist_item = new MythUIButtonListItem(m_buttonlistPlayQueue,QString(buf));
        VERBOSE(VB_IMPORTANT, buf);
        entity = mpd_getNextInfoEntity(conn);
    };

    BuildFocusList();

    connect(m_buttonBack,          SIGNAL(Clicked()),                          this, SLOT(clicked_Back()));
    connect(m_buttonlistPlayQueue, SIGNAL(itemClicked(MythUIButtonListItem*)), this, SLOT(clicked_track(MythUIButtonListItem*)));
    return true;
}

bool MythMPD_PlayQueue::keyPressEvent(QKeyEvent *event)
{
    if (GetFocusWidget() && GetFocusWidget()->keyPressEvent(event))
        return true;
    
    bool handled = false;
    QStringList actions;
    handled = GetMythMainWindow()->TranslateKeyPress("Music", event, actions);

    if (!handled && MythScreenType::keyPressEvent(event))
        handled = true;

    return handled;
}


void MythMPD_PlayQueue::clicked_track(MythUIButtonListItem *playlist_item)
{
    int id = m_buttonlistPlayQueue->GetCurrentPos();
    VERBOSE(VB_IMPORTANT, "MythMPD_PlayQueue: selected track " + QString::number(id));
    mpd_sendPlayIdCommand(conn,id);
}


void MythMPD_PlayQueue::Close()
{
    if (m_generalScreen)
        m_generalScreen->Show();
    MythScreenType::Close();
}


void MythMPD_PlayQueue::clicked_Back(void)
{
    VERBOSE(VB_IMPORTANT, "MythMPD_PlayQueue: Going back to main screen");
    m_generalScreen->Show();
    Close();
}
