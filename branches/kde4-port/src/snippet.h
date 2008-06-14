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

#include <qstring.h>
#include <qmap.h>
#include <qptrlist.h>

#if 0
example snippet XML:

	<snippet>
        <name>test</name>
        <triggertext><![CDATA[test]]></triggertext>
        <contents>
            <![CDATA[this is the snippet $0 expansion $1 here]]>
        </contents>
        <types>global</types>
    </snippet>
#endif

#ifndef __SNIPPET__
#define __SNIPPET__

/**
 * Represents a single snippet object.
 <pre>
 example snippet XML:
	<snippet>
        <name>test</name>
        <triggertext><![CDATA[test]]></triggertext>
        <contents>
            <![CDATA[this is the snippet $0 expansion $1 here]]>
        </contents>
        <types>global</types>
    </snippet>
   </pre>
 */
struct Snippet
{  
    public:
        /**
        * the name of this snippet
        */
        QString name;
        /**
        * the text that should trigger this snippet to expand
        */
        QString triggerText;
        /**
        * the contents of this snippet (once expanded)
        */
        QString contents;
        /**
        * comma separated list of supported types (php, xml, etc)
        */
        QString types;
};

/**
 * Represents a single tab stop placeholder object.
 */
struct TabStop
{
    /**
     * the number of the tab stop ($1, $2, etc)
     */
    uint tabNumber;
    /**
     * the string representation of the tabNumber (for convienence)
     */
    QString tabNumString;
    /**
     * the line of this tab stop
     */
    int cursorLine;
    /**
     * the column of this tab stop
     */
    int cursorColumn;
    
    /**
     * Stores the initial column position of this TabStop, 
     * (for the snippet processor [updateTabStopPositions)
     */
    int origCursorColumn;
    /**
     * Stores the initial line position of this TabStop, 
     * (for the snippet processor [updateTabStopPositions)
     */
    int origCursorLine;
};

// create a snippet collection object
typedef QMap<QString, Snippet> SnippetMap;
typedef QPtrList<TabStop> TabStopList;

#endif
