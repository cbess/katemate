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

#include <q3listview.h>

class QString;

/**
 * Represents the snippet as a list view item.
 */
class SnippetListViewItem : public Q3ListViewItem 
{
    private:
        QString mContents;
        QString mTriggerText;
        QString mTypes;
        
    public:
        SnippetListViewItem(Q3ListView* parent, const QString& label);
        SnippetListViewItem(Q3ListViewItem* parent, const QString& name);
        
        /**
         * Sets the contents (the expanded text)
         * @param text 
         */
        void setContents(const QString& text);
        
        inline QString contents()
        { return mContents; }
        
        /**
         * Sets the trigger text
         * @param text 
         */
        void setTriggerText(const QString& text);
        
        inline QString triggerText()
        { return mTriggerText; }
        
        /**
         * Sets the types associated with this snippet
         * @param types 
         */
        void setTypes(const QString& types);
        
        inline QString types()
        { return mTypes; }
        
        /**
         * Sets the name of the snippet
         * @param name 
         */
        inline void setName(const QString& name)
        { this->setText(0, name); }
        
        inline QString text(int col=0)
        { return this->Q3ListViewItem::text(col); }
};
