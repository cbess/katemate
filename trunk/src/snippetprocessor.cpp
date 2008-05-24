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

// matches any tabstop ($n)
#define TAB_STOP_REGEX "\\$([0-9]+)"

#include "snippetprocessor.h"
#include "katematexml.h"
#include "katematehelpers.h"
#include <qstring.h>
#include <qregexp.h>
#include <kdebug.h>
#include <qdir.h>
#include <qstringlist.h>
#include <stdlib.h>
#include <qfile.h>
#include <qtextstream.h>

SnippetProcessor::SnippetProcessor()
{
    mTabStopRegex = new QRegExp(TAB_STOP_REGEX);
    mTabStopList = new TabStopList();
    mTabStopList->setAutoDelete(true);
    mSnippets = 0L;
    mCurrentTabStop = 0;
    mCurrentTabStopNumber = 0;
}

SnippetProcessor::~SnippetProcessor()
{
    delete mTabStopRegex;
    delete mTabStopList;
    if (mSnippets)
        delete mSnippets;
}

bool SnippetProcessor::loadSnippets(const QString& path)
{        
    if (!QFile::exists(path))
        return false;
    
    KateMateXml *xml = new KateMateXml;
    
    if (!xml->LoadFile(path))
        return false;
    
    if (mSnippets)
        delete mSnippets;
    
    // load the snippet collection
    mSnippets = xml->ParseSnippetXml();
    
    // cleanup
    delete xml;
    
    return true;
}

// initially expands trigger text and populates tab stop collection
QString SnippetProcessor::processText(TriggerObject& obj)
{
    if (!mSnippets)
        return QString::null;
    
    // find the respective text expansion for this trigger text
    QString newText = mSnippets->find(obj.triggerText).data().contents;
    
    // clear the previous tab stop collection
    mTabStopList->clear();
    
    // parse the tab stops
    parseTabStops(newText);
    
    debug("tabstops: %d", mTabStopList->count());
    printDebugLine();
    
    // add the specified indentation level to the new text
    newText.replace("\n", "\n"+obj.indentText);
    
    QStringList lines = QStringList::split('\n', newText);
    
    // iterate over all the tab stops
    QPtrListIterator<TabStop> it(*mTabStopList);
    TabStop *tabstop;
    while ( (tabstop = it.current()) != 0L ) 
    {        
        QString phString = "$"+tabstop->tabNumString; // placeholder
        // try to find the tab stop (cursor position) 'placeholder'
        int phStartIndex = newText.find(phString);
    
        // determine the tab stop positions
        processTabStops(tabstop, phString, lines);
        
        // debug output
        debug("tab: col = %d, line = %d, num = %s\n", tabstop->cursorColumn, tabstop->cursorLine, (const char*)phString);
        
        // now replace the placeholder
        if (phStartIndex >= 0)
        {
            newText.remove(phStartIndex, phString.length());
            // update the lines value
            lines = QStringList::split('\n', newText);
        }
        
        // next object
        ++it;
    } // end WHILE
    
    printDebugLine();
    
    mCurrentTabStop = 0;
    mCurrentTabStopNumber = mTabStopList->getFirst()->tabNumber;
    obj.hasTabStop = (mTabStopList->count() > 1);
    
    // store last trigger object
    this->setLastTriggerObject(obj);
    
    return newText;
}

bool SnippetProcessor::getTabStopPosition(int * col, int * line, int tabNum)
{    
    if (tabNum < 0)
        tabNum = mCurrentTabStop;
    
    TabStop *stop = mTabStopList->at(tabNum);
    if (stop)
    {
        // only add the column if on the first line of the expanded text
        *col = stop->cursorColumn + ( stop->cursorLine == 0 ? mLastTriggerObject.triggerColumn : 0);
        *line = stop->cursorLine + mLastTriggerObject.triggerLine;
        return true;
    }
    else
        return false;
}

bool SnippetProcessor::getNextTabStopPosition(int * col, int * line)
{    
    bool canAdvance = canAdvanceTabStop();
    
    // can the position advance?
    if (canAdvance)
    {
        ++mCurrentTabStop;
        ++mCurrentTabStopNumber;
    }
    
    return getTabStopPosition(col, line, mCurrentTabStop);
}

/**
 * Updates the tab stop positions
 * @param newLine 
 * @param newColumn 
 */
bool SnippetProcessor::updateTabStopPositions(int newLine, int newColumn)
{
    int diff;
    int tabStopTextLength;
    int tabStopLineDepth;
  
    // get the current typed text length
    // from tab stop to current column position
    int curStopLine;
    int curStopColumn;
    getTabStopPosition(&curStopColumn, &curStopLine);
    tabStopTextLength = newColumn - curStopColumn; // the length of the typed text
    tabStopLineDepth = newLine - curStopLine; // the number of lines the current edit spans
    
    // iterate over all the tab stops
    QPtrListIterator<TabStop> it(*mTabStopList);
    TabStop *tabstop;
    while ((tabstop = it.current()) != 0L) 
    {
        // only update every tabstop after the current one
        if (!(tabstop->tabNumber > mCurrentTabStopNumber))
            continue;
    
        // move iterator back one, to get prev tabstop, then forward to current       
        --it; TabStop *prevStop = it.current(); ++it;
        
        if (prevStop != 0L)
        {
            // get original column distance
            diff = tabStopLineDepth + tabstop->origCursorLine;
            
            if (diff != 0)
                tabstop->cursorLine = diff;
            
            // update column if the tabstop's column is on the updated line
            if ((tabstop->cursorLine + mLastTriggerObject.triggerLine) == newLine)
            {         
                    // get original column distance
                    diff = tabstop->origCursorColumn - prevStop->origCursorColumn;
                    // add the position from the prev tab stop plus the newly entered text length
                    diff += prevStop->cursorColumn + tabStopTextLength;
                    
                    // update the tab stop
                    tabstop->cursorColumn = diff;
            } // end IF (cursorLine)            
        } // end IF (prevStop)   
    
        // next object
        ++it;  
    } // end WHILE
    
    return true;
}

bool SnippetProcessor::getLastTabStopPosition(int * line, int * col)
{
    return getTabStopPosition(col, line, mTabStopList->count() - 1);
}

bool SnippetProcessor::saveSnippets(const QString& path)
{
    QFile file(path);
    if (QFile::exists(path))
        QFile::remove(path);
    file.open(IO_WriteOnly | IO_Append);
    QTextStream stream(&file);
    
    // write the header
    stream << "<katemate version=\"0.1\">\n";
    // iterate over every snippet
    SnippetMap *map = this->snippetCollection();
    SnippetMap::Iterator it;
    for ( it = map->begin(); it != map->end(); ++it )
    {
        Snippet snippet = it.data();
        stream << "\t<snippet>\n";
        stream << "\t\t<name><![CDATA[" << snippet.name << "]]></name>\n";
        stream << "\t\t<triggertext><![CDATA[" << snippet.triggerText << "]]></triggertext>\n";
        stream << "\t\t<contents><![CDATA[\n" << snippet.contents << "]]></contents>\n";
        stream << "\t\t<types><![CDATA[" << snippet.types << "]]></types>\n";
        stream << "\t</snippet>\n";        
    } // end FOR
    // write the footer
    stream << "</katemate>";
    
    file.close();
    
    return true;
}

/** PRIVATE METHODS **/

// private
void SnippetProcessor::processTabStops(TabStop * obj, const QString& phString, QStringList& lines)
{    
    // find the next cursor position
    obj->cursorLine = -1;
    obj->cursorColumn = -1;
    int posLine = -1;
    // iterate every line of the expansion, to get new cursor position
    for ( QStringList::Iterator it = lines.begin() ; it != lines.end() ; ++it ) 
    {
        // count line
        ++posLine;
        
        // get line
        QString line = *it;
        int pos = line.find(phString);
        
        if (pos >= 0)
        {
            obj->cursorColumn = pos;
            break;
        }
        else // if tab stop not found, default to end of line
            obj->cursorColumn = line.length();
    } // end FOR
    
    // set next line position
    obj->cursorLine = posLine;
    
    // set the initial line/column values
    obj->origCursorColumn = obj->cursorColumn;
    obj->origCursorLine = obj->cursorLine;
}    


// private, only happens once per expansion
void SnippetProcessor::parseTabStops(const QString& text)
{
    bool addFinalTabStop = true; // true if $0 not found
    int pos = 0;
    while (pos >= 0) 
    {
        pos = mTabStopRegex->search(text, pos);
        if (pos > -1) 
        {
            QString stop = mTabStopRegex->cap(1);
            
            TabStop * tStop = addTabStop(stop);
            
            if (tStop->tabNumber == 0)
                addFinalTabStop = false;
            
            // advance the regex position
            pos  += mTabStopRegex->matchedLength();
        }
    } // end WHILE
    
    if (mTabStopList->count() == 0)        
        addTabStop("0"); // add the default
    else if (addFinalTabStop)
    {
        // add the implied 'end of expansion' tab stop
        addTabStop(QString::number(mTabStopList->getLast()->tabNumber + 1));
    }
}

// private
TabStop * SnippetProcessor::addTabStop(const QString& tabStopNum) const
{    
    TabStop *tStop = new TabStop;
    tStop->tabNumber = tabStopNum.toInt();
    tStop->tabNumString = tabStopNum;
            
    // add the tab stop to the collection
    mTabStopList->append(tStop);        
        
    return tStop;
}

// private, accessed via 'canProcess(QString, QString)'
bool SnippetProcessor::hasAction(const QString& text, const QString& docType)
{
    if (!mSnippets)
        return false;
    
    // find the corresponding trigger snippet
    QMapIterator<QString, Snippet> it = mSnippets->find(text);
    if (it != mSnippets->end())
    {
        Snippet snippet = it.data();
        // get all possible document types
        QStringList types = QStringList::split(";", snippet.types);
        
        // test the global scope
        if (snippet.types.find("global", 0, false) >= 0)
            return true;

        // iterate every associated document type for the snippet
        for (QStringList::Iterator it = types.begin() ; it != types.end() ; ++it) 
        {
            // get type
            QString type = *it;
            
            // compare the doc type to the scope of the snippet
            if (docType.find(type.stripWhiteSpace(), 0, false) >= 0)
            {
                debug("'%s' available to scope '%s'", (const char*)snippet.name, (const char*)docType);
                return true;
            }
        } // end FOR
    } // end IF
    
    return false;
}

// private
void SnippetProcessor::setLastTriggerObject(const TriggerObject& obj)
{
    // copy the trigger object
    mLastTriggerObject.triggerText = QString(obj.triggerText);
    mLastTriggerObject.indentText = QString(obj.indentText);   
    mLastTriggerObject.hasTabStop = bool(obj.hasTabStop);
    mLastTriggerObject.triggerLine = int(obj.triggerLine);
    mLastTriggerObject.triggerColumn = int(obj.triggerColumn);
}
