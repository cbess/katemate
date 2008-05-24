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

#include "katematehelpers.h"
#include "katematexml.h"
#include <qdom.h>
#include <qfile.h>

KateMateXml::KateMateXml()
{
    mXmlDocument = 0L;
}

KateMateXml::~KateMateXml()
{
    delete mXmlDocument;
}

bool KateMateXml::LoadFile(const QString& filePath )
{
    if (mXmlDocument != 0L)
        delete mXmlDocument;
    
    mXmlDocument = new QDomDocument("katematexml");
    QFile file(filePath);
    
    if ( !file.open(IO_ReadOnly) )
        return false;
    
    if ( !mXmlDocument->setContent(&file) ) 
    {
        file.close();
        return false;
    }
    
    file.close();
    
    return true;
}

SnippetMap * KateMateXml::ParseSnippetXml()
{
    SnippetMap *map = new SnippetMap;    
    QDomElement docElem = mXmlDocument->documentElement();

    // get the top level node
    QDomNode sNode = docElem.firstChild();
    
    // iterate over every child of the first node <katemate>
    // which as of right now is only <snippet> nodes    
    while( !sNode.isNull() ) 
    {
        Snippet snippet;
        QDomNode childNode = sNode.firstChild(); // child snippet node
        // iterate over every child of <snippet>
        while (!childNode.isNull())
        {
            QDomElement e = childNode.toElement(); // try to convert the node to an element.
        
            if( !e.isNull() ) 
            {
                QString tagName = e.tagName();
                QString innerText = e.text();
                
                // determine the snippet var to populate
                if (tagName == "name")
                    snippet.name = innerText;
                else if (tagName == "triggertext")
                    snippet.triggerText = innerText;
                else if (tagName == "contents")
                {
                    // remove the newline at the start of the content (convenience)
                    if (innerText.startsWith("\n"))
                        innerText.remove(0, 1);
                    snippet.contents = innerText;
                }
                else if (tagName == "types")
                    snippet.types = innerText;
            }
            
           childNode = childNode.nextSibling();
        } // end WHILE (childNode)
        
        if (snippet.name != QString::null)
        {
            printf("snippet name: %s\n", (const char*)snippet.name);
            
            // add the snippet to the collection
            map->insert(snippet.name, snippet);
        }
        
        sNode = sNode.nextSibling();
    } // end WHILE (sNode)
    
    printf("Snippets: %d", map->count());
    printDebugLine();
    
    return map;
}
