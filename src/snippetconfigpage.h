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
#ifndef SNIPPETCONFIGPAGE_H
#define SNIPPETCONFIGPAGE_H

#include "SnippetConfigWidget.h"
#include <qlineedit.h>
#include <ktextedit.h>
#include <q3listview.h>

/* Inherited members:
 *
 * QLabel* textLabel1; // tab trigger text label
 * QPushButton* btDeleteSnippet;
 * KTextEdit* txtTriggerContent;
 * QLineEdit* txtTypes;
 * QLineEdit* txtTriggerText;
 * QPushButton* btNewSnippet;
 * QListView* lstSnippets;
 * 
 */

/**
 * Represents the configuration UI for the snippet operations.
 */
class SnippetConfigPage: public SnippetConfigWidget 
{
    Q_OBJECT
            
    private:
        SnippetProcessor * mSnippetProcessor;
        class SnippetListViewItem *mPrevItem;
        /**
         * True if the list view should refresh, upon save
         */
        bool mShouldRefreshSnippets;
        
    public:
        SnippetConfigPage(QWidget *parent = 0, const char *name = 0);
        virtual ~SnippetConfigPage();
      
        /**
         * Sets the internal SnippetProcessor instance.
         */
        inline void setSnippetProcessor(SnippetProcessor * proc)
        { mSnippetProcessor = proc; }
        
        /**
         * Loads the snippets from the specified path.
         * @return true upon successful load, false otherwise
         */
        bool loadSnippets(const QString& path);
        /**
         * Saves the snippets to the specified path.
         * @return true upon successful save, false otherwise
         */
        bool saveSnippets(const QString& path);
        bool saveSnippets(void);
        
        /**
         * Save the currently selected snippet.
         */
        void saveSnippet();
        /**
         * Loads the snippet associated with specified name.
         */
        void loadSnippet(const QString& name);
        /**
         * Creates a new snippet from the current field values, then selects it.
         */
        void createSnippet();
        /**
         * Adds the specifed snippet to the snippet list view.
         */
        Q3ListViewItem * addSnippetItem(const Snippet& snippet);
        /**
         * Gets the current snippet processor.
         * @return current snippet processor instance
         */
        inline SnippetProcessor * snippetProcessor()
        { return mSnippetProcessor; }
        
        /**
         * Refreshes the snippets list view
         */
        void refreshSnippets();
        
        /**
         * Clears the snippet text fields
         */
        inline void clearTextFields()
        {
            // clear fields
            txtTriggerText->clear();
            txtTriggerContent->clear();
            txtTypes->clear();
        }
        
        /**
         * Gets the first document type
         * @param string document types (';' separated)
         */
        inline QString primaryDocumentType(const QString& types)
        { return QStringList::split(";", types).first().stripWhiteSpace(); }
        
        /**
         * Gets the scope list item instance (ex: global)
         */
        inline Q3ListViewItem * scopeRootItem(const QString& type)
        { return lstSnippets->findItem(type, 0, ExactMatch); }
        
        /**
         * Determines if the target triggerText is already assigned to another snippet
         * @return true if the trigger text is in use, false otherwise
         */
        bool snippetExists(const QString& triggerText);
        
public slots:
    virtual void lstSnippets_itemRenamed(Q3ListViewItem*, int, const QString&);
    virtual void lstSnippets_selectionChanged(Q3ListViewItem*);
public slots:
    virtual void btNewSnippet_clicked();
public slots:
    virtual void btSaveSnippets_clicked();
    virtual void btDeleteSnippet_clicked();
    virtual void lstSnippets_clicked(Q3ListViewItem*);
    virtual void lstSnippets_pressed(Q3ListViewItem*);
    virtual void lstSnippets_mouseButtonPressed(int,Q3ListViewItem*,const QPoint&,int);
};

#endif
