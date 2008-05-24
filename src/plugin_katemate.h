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


#ifndef _PLUGIN_KATEMATE_H_
#define _PLUGIN_KATEMATE_H_

#define TAB_TEXT "    "

#include <kate/application.h>
#include <kate/documentmanager.h>
#include <kate/document.h>
#include <kate/mainwindow.h>
#include <kate/plugin.h>
#include <kate/view.h>
#include <kate/viewmanager.h>
#include <kate/pluginconfiginterface.h>
#include <kate/pluginconfiginterfaceextension.h>
#include <kate/toolviewmanager.h>

#include <klibloader.h>
#include <klocale.h>

#include <qobjectlist.h> 

class KateMateConfigPage;
class SnippetProcessor;
class QRegExp;
class SnippetConfigPage;

class KatePluginFactory : public KLibFactory
{
    Q_OBJECT

public:
    KatePluginFactory();
    virtual ~KatePluginFactory();

    virtual QObject* createObject( QObject* parent = 0, const char* pname = 0, const char* name = "QObject", const QStringList &args = QStringList() );

private:
    static KInstance* s_instance;
};

/**
 * Represents the main plugin interface.
 */
class KatePluginKateMate : public Kate::Plugin, Kate::PluginViewInterface, Kate::PluginConfigInterfaceExtension
{
    Q_OBJECT
            
    private:
        SnippetProcessor *mSnippetProcessor;
        /**
         * Gets the string at the start of the line
         */
        QRegExp *mIndentRegex;
        /**
         * Matches any special trigger chars
         */
        QRegExp *mSpecialCharRegex;
        /**
         * True if tab stops still exists within the text expansion, false otherwise
         */
        bool mIsTriggerActive;
        /**
         * Matches special trigger words/text
         */
        QRegExp *mSpecialTextRegex;
        /**
         * The config page used to manage the snippets (resides within the toolview).
         */
        SnippetConfigPage *mSnippetConfigPage;
        /**
         * The toolview that manages the snippets
         */
        QWidget *mSnippetToolView;
        
public:
    KatePluginKateMate( QObject* parent = 0, const char* name = 0 );
    virtual ~KatePluginKateMate();

    void addView (Kate::MainWindow *win);
    void removeView (Kate::MainWindow *win);

    /** overwrite some functions  */
    uint configPages () const { return 1; }
    Kate::PluginConfigPage *configPage (uint , QWidget *w, const char *name=0);
    QString configPageName(uint) const { return i18n("KateMate"); };
    QString configPageFullName(uint) const { return i18n("Configure KatePluginKateMate"); };
    QPixmap configPagePixmap (uint number = 0, int size = KIcon::SizeSmall) const { return 0L; };

        /**
         * Loads the snippet collection and populates the snippet config page list
         */
        void loadSnippets();
        
public slots:
    void textTriggerActivated();
    void slotApplyConfig(KateMateConfigPage*);
    void altTextTriggerActivated();
    void activeDocumentTextChanged();
    
    private:
        /**
        * Grabs the valid word/char to the left of the current cursor position
        */
        QString grabTriggerText(Kate::View *view);
    
        /**
         * Attempts to process trigger text (performs snippet expansion)
         * @return true if the text was expanded, false otherwise
         */
        bool processTriggerText(Kate::View *view);
        
        /**
         * attempts to advance the cursor to the next tab stop position
         * @param true if the cursor will skip all tab stops and advance to the end of the expansion
         * @return true if the cursor advanced its position, false otherwise
         */
        bool advanceCursorPosition(bool jumpToEnd = false);        
        
        void activateTrigger();
        void deactivateTrigger();
    
        inline Kate::View * activeView() const
        {            
            // get the currently active document
            return application()->activeMainWindow()
                    ->viewManager()->activeView();
        }
        
        inline Kate::Document * activeDocument() const
        {
            // get the currently active document
            Kate::Document *doc = application()->activeMainWindow()
            ->viewManager()->activeView()->getDoc();
            
            return doc;
        }
        
        inline Kate::ToolViewManager * toolViewManager() const
        { 
            return application()->activeMainWindow()
                ->toolViewManager(); 
        }
        
        inline void insertTabulators()
        {
            if (!mIsTriggerActive)
                activeView()->insertText(TAB_TEXT); // insert tabulators
        }
        
        /**
         * Gets the SnippetConfigPage instance
         * @return 
         */
        inline SnippetConfigPage * snippetConfigPage()
        { return mSnippetConfigPage; }
        
        /**
         * Determines snippet toolview is the focused widget.
         * @return 
         */
        bool isSnippetPageFocused();
                
private:
    void initConfigPage( KateMateConfigPage* );

private:
    QPtrList<class PluginView> m_views;
};


class KateMateConfigPage : public Kate::PluginConfigPage
{
    Q_OBJECT
    friend class KatePluginKateMate;

public:
    KateMateConfigPage (QObject* parent = 0L, QWidget *parentWidget = 0L);
    ~KateMateConfigPage ();

    /** Reimplemented from Kate::PluginConfigPage; just emits configPageApplyRequest( this ).  */
    virtual void apply();

    virtual void reset () { ; };
    virtual void defaults () { ; };

signals:
    /** Ask the plugin to set initial values  */
    void configPageApplyRequest( KateMateConfigPage* );
    /** Ask the plugin to apply changes  */
    void configPageInitRequest( KateMateConfigPage* );

private: // variables

};

#endif // _PLUGIN_KATEMATE_H_

