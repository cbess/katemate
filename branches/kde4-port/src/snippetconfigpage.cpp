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

#include "snippet.h"
#include "snippetprocessor.h"
#include "snippetlistviewitem.h"
#include "snippetconfigpage.h"
#include <qstringlist.h>
#include <qdir.h>

SnippetConfigPage::SnippetConfigPage(QWidget *parent, const char *name)
    :SnippetConfigWidget(parent, name)
{    
    mShouldRefreshSnippets = false;
}

SnippetConfigPage::~SnippetConfigPage()
{
    debug("snippetconfigpage destructed");
}

bool SnippetConfigPage::loadSnippets(const QString& path)
{
    // load snippet map
    if (!mSnippetProcessor->loadSnippets(path))
        return false;
    
    this->refreshSnippets();
}

void SnippetConfigPage::refreshSnippets()
{    
    // clear previous snippets items
    lstSnippets->clear();
    mPrevItem = 0L;
    mShouldRefreshSnippets = false;
    clearTextFields();
    
    // add the implied global snippet item
    (void)new QListViewItem(lstSnippets, "Global");    
    
    // iterate over every snippet
    SnippetMap *map = mSnippetProcessor->snippetCollection();
    SnippetMap::Iterator it;
    for ( it = map->begin(); it != map->end(); ++it )
    {
        Snippet snippet = it.data();
        // add the snippet to the list view
        addSnippetItem(snippet);
    } // end FOR (SnippetMap)
}

bool SnippetConfigPage::saveSnippets()
{
    QString path = QDir::homeDirPath()+"/katemate.conf.xml";
    return saveSnippets(path);
}

bool SnippetConfigPage::saveSnippets(const QString& path)
{    
    if (!mSnippetProcessor->saveSnippets(path))
        return false;
        
    // reload list view
    if (mShouldRefreshSnippets)
        refreshSnippets();
    
    mShouldRefreshSnippets = false;
    
    return true;
}

void SnippetConfigPage::loadSnippet(const QString& name)
{
    
}

void SnippetConfigPage::saveSnippet()
{
}

void SnippetConfigPage::createSnippet()
{
    SnippetMap *map = mSnippetProcessor->snippetCollection();
    QString name = txtTriggerText->text();
    
    // if a snippet with that name already exists
    if (map->contains(name))
    {
        return;
    }
        
    // create the snippet
    Snippet snippet;
    snippet.contents = txtTriggerContent->text();
    snippet.triggerText = txtTriggerText->text();
    snippet.types = txtTypes->text();
    snippet.name = name;
    
    QListViewItem * item = addSnippetItem(snippet);
    
    // show the newly added items
    if (item)
    {
        // insert the snippet into the collection
        map->insert(snippet.name, snippet);
        
        // save the collection of snippets to disk
        this->saveSnippets();
        
        // show the newly added snippet
        lstSnippets->ensureItemVisible(item);
        lstSnippets->setSelected(item, true);
    }
}

QListViewItem * SnippetConfigPage::addSnippetItem(const Snippet& snippet)
{
    if (snippet.types.isEmpty())
        return 0L;
        
    QString type = primaryDocumentType(snippet.types);
    
    if (type.isEmpty())
        return 0L;
    
    // get the specified root item    
    QListViewItem *rootItem = scopeRootItem(type);
    
    // if the type is not found
    if (rootItem == 0L)
    {
        // add the root item to the list
        rootItem = new QListViewItem(lstSnippets, type.lower());
        rootItem->setExpandable(true);
    }
    
    // add the sub items
    SnippetListViewItem *item = new SnippetListViewItem(rootItem, snippet.name);
    item->setContents(snippet.contents);
    item->setTriggerText(snippet.triggerText);
    item->setTypes(snippet.types);
    item->setRenameEnabled(0, false);
    item->setExpandable(false);
    
    return item;
}

/** EVENT METHODS **/

void SnippetConfigPage::btDeleteSnippet_clicked()
{
    debug("delete snippet");
    
    // get the selected item
    QListViewItem * sItem = lstSnippets->selectedItem();
    SnippetListViewItem *item = dynamic_cast<SnippetListViewItem*>(sItem);
    
    if (!item)
        return;
    
    // remove the snippet from the collection
    mSnippetProcessor->snippetCollection()->remove(sItem->text(0));
    
    // get the primary type
    QString type = primaryDocumentType(item->types());
    
    // get the associated root item
    QListViewItem *rootItem = scopeRootItem(type);
    if (rootItem != 0L)
    {
        // highlight the root item
        lstSnippets->ensureItemVisible(rootItem);
        lstSnippets->setSelected(rootItem, true);
    }
    
    delete sItem;
    mPrevItem = 0L;
    
    clearTextFields();
}

void SnippetConfigPage::btNewSnippet_clicked()
{
    mPrevItem = 0L;
    
    createSnippet();
}


void SnippetConfigPage::btSaveSnippets_clicked()
{
    saveSnippets();
}

void SnippetConfigPage::lstSnippets_selectionChanged(QListViewItem* i)
{
}

void SnippetConfigPage::lstSnippets_clicked(QListViewItem* i)
{
}

void SnippetConfigPage::lstSnippets_pressed(QListViewItem* i)
{    
    SnippetListViewItem *item = dynamic_cast<SnippetListViewItem*>(i);
    
    if (!item)
        return;
        
    if (mPrevItem)
    {       
//         debug("prev = '%s' - now = '%s'", (const char*)mPrevItem->triggerText(), (const char*)txtTriggerText->text());
        QString triggerText = txtTriggerText->text();
        QString contents = txtTriggerContent->text();
        QString types = txtTypes->text();
        
        if (triggerText.isEmpty())
            return;
        
        // if nothing changed, make sure it doesn't overwrite another snippet
//         if (triggerText == mPrevItem->text()
//             && contents == mPrevItem->contents()
//             && types == mPrevItem->types())
        {
            if (snippetExists(triggerText))
                return;
        }
        
        SnippetMap *map = mSnippetProcessor->snippetCollection();
        QString name = mPrevItem->text();
        
        // create new snippet
        Snippet snippet;
        snippet.name = name;
        snippet.contents = contents;
        snippet.types = types;
        snippet.triggerText = triggerText;
        
        // replace the old snippet
        map->replace(name, snippet);
        
        // if the types have changed, reload the list view
        if (!mShouldRefreshSnippets)
            mShouldRefreshSnippets = (snippet.types != mPrevItem->types());
        
        // store the item values
        mPrevItem->setContents(contents);
        mPrevItem->setTypes(types);
        mPrevItem->setTriggerText(triggerText);
    }
    
    if (item)
        mPrevItem = item;
}

void SnippetConfigPage::lstSnippets_mouseButtonPressed(int button, QListViewItem* i, const QPoint&, int)
{ // emitted after 'item_pressed'
    SnippetListViewItem *item = dynamic_cast<SnippetListViewItem*>(i);
    if (item)
    {
        txtTriggerText->setText(item->triggerText());
        txtTypes->setText(item->types());
        txtTriggerContent->setText(item->contents());
    }
    else
    {
        clearTextFields();
        
        if (i != 0L)
            txtTypes->setText(i->text(0).lower());
    }
}

void SnippetConfigPage::lstSnippets_itemRenamed(QListViewItem* i,int,const QString& newName)
{
    return;
    
    SnippetListViewItem *item = dynamic_cast<SnippetListViewItem*>(i);
    if (item)
    {
        // create the snippet
        Snippet snippet;
        snippet.contents = item->contents();
        snippet.types = item->types();
        snippet.triggerText = item->triggerText();
        snippet.name = newName;
        
        SnippetMap *map = mSnippetProcessor->snippetCollection();
        // remove the old snippet
        map->remove(item->text());
        // add the renamed snippet
        map->insert(newName, snippet);
        
        debug("rename %s to %s", (const char*)mPrevItem->text(), (const char*)newName);
    }
}

// temp function, until I find a better way to preventing the snippets from
// overwriting each other (during the store snippet operation)
bool SnippetConfigPage::snippetExists(const QString& triggerText)
{
    // iterate over every snippet
    SnippetMap *map = mSnippetProcessor->snippetCollection();
    SnippetMap::Iterator it;
    for ( it = map->begin(); it != map->end(); ++it )
    {
        if (it.data().triggerText == triggerText)
            return true;
    }
    
    return false;
}

#include "snippetconfigpage.moc"
