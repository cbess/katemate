/***************************************************************************
 *   Copyright (C) 2008 by Christopher Bess   *
 *   cbess@quantumquinn.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

// matches any whitepsace at the beginning of the string
#define INDENT_REGEX "^\\s*"
// matches gets any of the target chars
#define SPECIAL_TRIGGER_CHARS "> - + = : ; . - ? & [ ] %"
// helps match/detect all possible trigger text/chars
#define SPECIAL_WORD_REGEX "(^|\\(|\\s)"

#include "snippetprocessor.h"
#include "snippetconfigpage.h"
#include "plugin_katemate.h"

#include <kaction.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

#include <qregexp.h>
#include <qlayout.h>
#include <qlineedit.h>
//Added by qt3to4:
#include <Q3VBoxLayout>

#include "katematehelpers.h"
#include <stdlib.h>
#include "kateconfigpage.h"
#include "katemateconfigdia.h"

class PluginView : public KXMLGUIClient
{
    friend class KatePluginKateMate;

public:
    Kate::MainWindow *win;
};

extern "C"
{
    void* init_libkatemateplugin()
    {
        KGlobal::locale()->insertCatalogue("katekatemate");
        return new KatePluginFactory;
    }
}

KatePluginFactory::KatePluginFactory()
{
    s_instance = new KInstance( "kate" );
}

KatePluginFactory::~KatePluginFactory()
{
    delete s_instance;
}

QObject* KatePluginFactory::createObject( QObject* parent, const char* name, const char*, const QStringList & )
{
    return new KatePluginKateMate( parent, name );
}

KInstance* KatePluginFactory::s_instance = 0L;

KatePluginKateMate::KatePluginKateMate( QObject* parent, const char* name )
    : Kate::Plugin ( (Kate::Application*)parent, name )
{
    debug("Started KateMate plugin (debug mode)...\n");
    printDebugLine();
    
    mIsTriggerActive = false;
    mSnippetConfigPage = 0L;
    mSnippetToolView = 0L;
    mSnippetProcessor = new SnippetProcessor;
    mIndentRegex = new QRegExp(INDENT_REGEX);
    mSpecialTextRegex = new QRegExp(SPECIAL_WORD_REGEX);
    mSpecialCharRegex = 0L;
}

KatePluginKateMate::~KatePluginKateMate()
{
    delete mSnippetProcessor;
    delete mIndentRegex;
    delete mSpecialTextRegex;
}

void KatePluginKateMate::addView(Kate::MainWindow *win)
{
    if (mSnippetToolView == 0L)
    {   
        // setup toolview
        Kate::ToolViewManager *obj = win->toolViewManager();
        mSnippetToolView = obj->createToolView(QString("snippetmanager"),
                                                Kate::ToolViewManager::Bottom,
                                                0L,
                                                QString("QuKateMate"));
        // add child widget  
        Q3VBoxLayout* lo = new Q3VBoxLayout(mSnippetToolView, 0, 0, "katemate_config_layout");
        lo->setSpacing(KDialogBase::spacingHint()); 
        mSnippetConfigPage = new SnippetConfigPage(mSnippetToolView, "snippetconfigpage"); 
        lo->addWidget(mSnippetConfigPage);
        
        // load the snippets
        this->loadSnippets();
    }
    
    PluginView *view = new PluginView ();

    // notify if kate mate has been activated
    (void) new KAction ( i18n("Expand Text"), 
                        KShortcut(KKey(QKeySequence(Key_Tab))),
                        this,
                        SLOT( textTriggerActivated() ), 
                        view->actionCollection(),
                        "trigger_katemate" );
    
    // notify if kate mate has been alternatively activated
    (void) new KAction ( i18n("Alt Expand Text"), 
                    KShortcut(KKey("SHIFT+Tab")),
                    this,
                    SLOT( altTextTriggerActivated() ), 
                    view->actionCollection(),
                    "alt_trigger_katemate" );
    
    view->setInstance (new KInstance("kate"));    
    view->setXMLFile("plugins/katemate/plugin_katemate.rc");
    win->guiFactory()->addClient (view);
    view->win = win;
    
    { // remove the toolbar items from the KateMate toolbar
        QStringList *toolbarItemNames = new QStringList;
        toolbarItemNames->append("alt_trigger_katemate");
        toolbarItemNames->append("trigger_katemate");
            
        for ( QStringList::Iterator it = toolbarItemNames->begin(); it != toolbarItemNames->end(); ++it )
        {
            QWidget* w = win->guiFactory()->container("KateMate", view, false);
            if (w && view->actionCollection()->action(*it))
                view->actionCollection()->action(*it)->unplug(w);
        }
        
        // cleanup
        delete toolbarItemNames;
    }
    
    m_views.append(view);
}

void KatePluginKateMate::removeView(Kate::MainWindow *win)
{
    for (uint z=0; z < m_views.count(); z++)
    {
        if (m_views.at(z)->win == win)
        {
            PluginView *view = m_views.at(z);
            m_views.remove (view);
            win->guiFactory()->removeClient (view);
            delete view;
        }
    }
    
    if (mSnippetToolView)
    {
        delete mSnippetToolView;
        mSnippetToolView = 0L;
    }
}

/** CUSTOM METHODS **/

/**
 * Called if KateMate "alt text trigger" has been activated (pressing 'Shift+Tab')
 */
void KatePluginKateMate::altTextTriggerActivated()
{    
    Kate::View *view = activeView();
    
    if (isSnippetPageFocused())
        return;
    
    // if the user has selecting text
    if (view->getDoc()->selection().length() > 0)
    {
        view->unIndent();
        deactivateTrigger();
        return;
    }
    else if (mIsTriggerActive)
    {
        advanceCursorPosition(true); // jump to end of expansion
        deactivateTrigger();
    }
    else
        view->unIndent();
}

/**
 * Called if KateMate "text trigger" has been activated (pressing 'Tab')
 * - This triggers "text expansion"
 */
void KatePluginKateMate::textTriggerActivated()
{    
    Kate::View *view = activeView();
    
    if (isSnippetPageFocused())
        return;
    
    // if the user is selecting text
    if (view->getDoc()->selection().length() > 0)
    {
        view->indent();
        deactivateTrigger();
        return;
    }
    
    if (mIsTriggerActive)
    {
        if (advanceCursorPosition())
        {
            // can it advance again?
            if (!mSnippetProcessor->canAdvanceTabStop())
                deactivateTrigger();
        }
    }
    else
    { // can only process on snippet at a time
        // handles the trigger text expansion (if needed)
        if (processTriggerText(view))
        { // the trigger text was expanded
            
        }
        else
            insertTabulators();
    }
}

QString KatePluginKateMate::grabTriggerText(Kate::View *view)
{    
    // get the target  word
    QString text = view->currentTextLine();
    uint col;
    view->cursorPositionReal(0L, &col);
    text.remove(col, text.length() - col);

    // locate the special text
    mSpecialTextRegex->searchRev(text);
    text = text.remove(0, mSpecialTextRegex->pos()).stripWhiteSpace();
    if (text.startsWith("("))
        text.remove(0, 1);
    
    // temp, to clear debug output
    if (text == "reset")
    {
        system("reset");
    }
    
    // output debug info
    debug("word = '%s'", (const char*)text);
    printDebugLine();
    
    return text;
}

bool KatePluginKateMate::processTriggerText(Kate::View *view)
{    
    bool textHandled = false;
    int nextCursorLine = 0;
    int nextCursorColumn = 0;
    bool hasTabStop = false;
    
    // get the target triggerText
    QString triggerText = this->grabTriggerText(view);
    if (triggerText.isEmpty()
       || triggerText == QString::null)
        return false;
    
    QString newText;
    Kate::Document *document = activeDocument();
    // get the current document type (scope)
    QString docType = document->hlModeName(document->hlMode());
    
//     debug("highlight mode = '%s'", (const char*)docType);
    if (mSnippetProcessor->canProcess(triggerText, docType))
    {
        // get the indent level string
        mIndentRegex->search(view->currentTextLine());
        QString indentText = mIndentRegex->cap();
    
        uint column = 0;
        uint line = 0;
        // gets the current cursor position (zero based)
        view->cursorPositionReal(&line, &column);
    
        // get the initial column
        uint textLength = triggerText.length();
        uint startColumn = column - textLength;
        
        // process the triggerText
        TriggerObject tObj;
        tObj.triggerText = triggerText;
        tObj.indentText = indentText;
        tObj.triggerLine = line;
        tObj.triggerColumn = startColumn;
        // - processes then, populates the object 
        // with the next cursor position data
        newText = mSnippetProcessor->processText(tObj);
        
        // remove trigger text
        uint endColumn = column;
        document->removeText(line, startColumn, line, endColumn);
        
        // engage tab stop
        int cursorColumn;
        int cursorLine;
        mSnippetProcessor->getTabStopPosition(&cursorColumn, &cursorLine);
       
        // set the initial next cursor position, to the current
        nextCursorLine = cursorLine;
        nextCursorColumn = cursorColumn;
     
        if (!tObj.hasTabStop)
        {
            deactivateTrigger();
        }
        else
            hasTabStop = true;
        
        // output debug info
        printf("triggerText = %s\npos = %d, %d\n", (const char*)triggerText, column, line);
        //printf("highlight mode = '%s'", (const char*)document->hlModeName(document->hlMode()));
        printDebugLine();
        
        textHandled = true;
    }
    else
    { // could not process the text
        deactivateTrigger();
    }
    
    // has the text trigger been handled, 
    if (textHandled)
    {
        view->insertText(newText);
        view->setCursorPositionReal(nextCursorLine, nextCursorColumn);
        
        if (hasTabStop)
            activateTrigger();
    }
    
    return textHandled;
}

/**
 * - called if the trigger is still active (advances cursor to 
 * the next tab stop position
 */
bool KatePluginKateMate::advanceCursorPosition(bool jumpToEnd)
{
    Kate::View *view = activeView();
    bool didAdvance = false; // did the cursor move?
    int nextCursorLine = 0;
    int nextCursorColumn = 0;
    // get the next tab position
    if (jumpToEnd)
        didAdvance = mSnippetProcessor->getLastTabStopPosition(&nextCursorLine, &nextCursorColumn);
    else
        didAdvance = mSnippetProcessor->getNextTabStopPosition(&nextCursorColumn, &nextCursorLine);
    
    if (nextCursorColumn < 0)
        didAdvance = false;
    
    // add current line position
    debug("advanced cursor to: %d, %d", nextCursorColumn, nextCursorLine);
    printDebugLine();
    
    // set the cursor position
    view->setCursorPositionReal(nextCursorLine, nextCursorColumn);
    
    if (!didAdvance)
        deactivateTrigger();
    
    return didAdvance;
}

void KatePluginKateMate::activeDocumentTextChanged()
{
    Kate::View *view = activeView();
    
    uint column = 0;
    uint line = 0;
    // gets the current cursor position (zero based)
    view->cursorPositionReal(&line, &column);
        
    // update all tab stops positions
    if (!mSnippetProcessor->updateTabStopPositions(line, column))
    {
            
    }
}


void KatePluginKateMate::deactivateTrigger()
{ 
    // disconnect from active doc text changes
    disconnect(activeDocument(), SIGNAL(textChanged()), 
               this, SLOT(activeDocumentTextChanged()));
            
    mIsTriggerActive = false; 
    mSnippetProcessor->tabStops()->clear();
}
        
void KatePluginKateMate::activateTrigger()
{
    // connect to text changed event for this active document
    connect(activeDocument(), SIGNAL(textChanged()),
            this, SLOT(activeDocumentTextChanged())); 
            
    mIsTriggerActive = true; 
}

void KatePluginKateMate::loadSnippets()
{    
    // load the snippets within the toolview
    mSnippetConfigPage->setSnippetProcessor(mSnippetProcessor);
    QString path = QDir::homeDirPath()+"/katemate.conf.xml";
    mSnippetConfigPage->loadSnippets(path);
}

bool KatePluginKateMate::isSnippetPageFocused()
{
    // if the user is in the toolview do not respond to tabs
    QString focusedName = QString(this->snippetConfigPage()->focusWidget()->name());
    if (!focusedName.isEmpty())
    {        
        debug("focusedName: %s", (const char*)focusedName);
        return true;
    }
            
    return false;
}

/** END CUSTOM METHODS **/

Kate::PluginConfigPage* KatePluginKateMate::configPage (uint, QWidget *w, const char* name)
{
    debug("config page opened");
    printDebugLine();
    
    KateMateConfigPage* p = new KateMateConfigPage(this, w);
    initConfigPage( p );
    connect( p, SIGNAL(configPageApplyRequest(KateMateConfigPage*)), this, SLOT(slotApplyConfig(KateMateConfigPage*)) );
    return (Kate::PluginConfigPage*)p;
}

void KatePluginKateMate::initConfigPage( KateMateConfigPage* p )
{
    // TODO: initialize KateMateConfigPage here
    // NOTE: KatePluginKateMate is friend of KateMateConfigPage
}

void KatePluginKateMate::slotApplyConfig( KateMateConfigPage* p )
{
    // TODO: save KateMateConfigPage here
    // NOTE: KatePluginKateMate is friend of KateMateConfigPage
}


/**
 * KateMateConfigPage
 */
KateMateConfigPage::KateMateConfigPage (QObject* parent /*= 0L*/, QWidget *parentWidget /*= 0L*/)
    : Kate::PluginConfigPage( parentWidget )
{
    Q3VBoxLayout* lo = new Q3VBoxLayout( this, 0, 0, "config_page_layout" );
    lo->setSpacing(KDialogBase::spacingHint());

    KateConfigPage *page = new KateConfigPage(this, "kateconfigpage", (KatePluginKateMate*)parent);
    
//     SnippetConfigPage *page = new SnippetConfigPage(this, "snippetconfigpage");
    
    lo->addWidget(page);

    // TODO: add connection to emit SLOT( changed() )
}

KateMateConfigPage::~KateMateConfigPage()
{
    debug("config page gone");
}

void KateMateConfigPage::apply()
{
    emit configPageApplyRequest( this );
}

#include "plugin_katemate.moc"

