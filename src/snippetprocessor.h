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


class QString;
class QRegExp;
class QStringList;

#ifndef __TRIGGER_OBJECT__
#define __TRIGGER_OBJECT__

#include "snippet.h"

/**
 * Represents the activated "tab trigger" object.
 * @remark This object is passed between the front-end,
 * and the text processor
 */
struct TriggerObject
{
    /**
     * Update: setLastTriggerObject(...), if this struct is updated
     */
    
    /**
    * the trigger text to process using a user defined action (text snippet), set from view
    */
    QString triggerText;
    /**
    * the indention text (only tabs or spaces), set from view
    */
    QString indentText;
    /**
     * True if tab stops exists, false otherwise
     */
    bool hasTabStop;
    /**
     * the line the trigger object was activated on, set from the view
     */
    int triggerLine;
    /**
     * the column the trigger object was activated on, set from the view
     */
    int triggerColumn;
};
#endif

/**
 * Represents the core snippet processing engine.
 */
class SnippetProcessor
{
    private:
        QRegExp *mTabStopRegex;
        SnippetMap *mSnippets;
        TriggerObject mLastTriggerObject;
        /**
         * collection of tab stop objects, associated with the LastTriggerObject()
         */
        TabStopList * mTabStopList;
        /**
         * the current tab stop index (active), -1 otherwise (inactive)
         */
        uint mCurrentTabStop;
        /**
         * the current tab stop number '$1 = 1'
         */
        uint mCurrentTabStopNumber;
        
    public:
        SnippetProcessor();
        virtual ~SnippetProcessor();
        
        /**
         * Gets the current snippet collection
         */
        inline SnippetMap * snippetCollection() const
        { return mSnippets; }
        
        /**
         * Loads the snippets from the specified path (replaces current collection)
         * @return true upon successful load, false otherwise (or fatal if XML parse error)
         */
        bool loadSnippets(const QString&);
        /**
         * Process the specified trigger object
         * @return: the processed version of the text
         */
        QString processText(TriggerObject& triggerObject);
        /**
        * Determines if the specified text has a processor action for the specified document type
        * @return true if text can be processed, false otherwise
        */
        inline bool canProcess(const QString& text, const QString& docType)
        { return this->hasAction(text, docType); }
        /**
         * Gets the last trigger object processed
         */
        inline TriggerObject lastTriggerObject()
        { return mLastTriggerObject; }        
        /**
         * Gets the collection of tab stops, associated with LastTriggerObject()
         */
        inline TabStopList * tabStops() const
        { return mTabStopList; }
        
        /**
         * Gets the first tab stop position
         */
        inline TabStop firstTabStop() const
        { return *mTabStopList->getFirst(); }
            
        /**
         * Gets the tab stop position, or current tab stop position [if (tabNum < 0)]
         */
        bool getTabStopPosition(int * col, int * line, int tabNum = -1);
        
        /**
         * Gets the next tab stop position, advances tab stop cursor
         * @return true if the cursor advanced, false if not other tab stops available
         */
        bool getNextTabStopPosition(int * col, int * line);
        
        /**
         * Gets the last tab stop position.
         */
        bool getLastTabStopPosition(int * line, int * col);
        
        /**
         * Determines if there are any tab stops left
         * @return true if more tab stops are available, false otherwise
         */
        inline bool canAdvanceTabStop()
        { return ((mCurrentTabStop + 1) < mTabStopList->count()); }
        
        /**
         * Updates the tab stop positions, called whenever the user types while at a tab stop
         * @return true if the tab stops were updated successfully
         */
        bool updateTabStopPositions(int newLine, int newColumn);
        
        /**
         * Saves the current snippet collection to the specified path
         * @param path 
         * @return true upon successful save, false otherwise
         */
        bool saveSnippets(const QString& path);
    protected:
        /**
        * Parses the specified text for tab stops and adds them to the tab stop collection
        */
        void parseTabStops(const QString &);
        /**
        * Determines if the target text has a defined action (can be processed)
        */
        bool hasAction(const QString &, const QString&);
        /**
         * sets the tab stop positions
         * @param TabStop the tab stop to process
         * @param QString the tab stop number "$n" to process within lines
         * @param QStringList the lines to evaluate
         */
        void processTabStops(TabStop * obj, const QString& phString, QStringList& lines);
    
    private:
        void setLastTriggerObject(const TriggerObject&);
        /**
         * adds the specified tab stop to the TabStopList collection
         * @return the newly added TabStop instance
         */
        TabStop * addTabStop(const QString& tabStopNum) const;
    
};
