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

#include "SnippetConfigWidget.h"
#include "plugin_katemate.h"
#include "katemateconfigdia.h"
#include "kateconfigpage.h"
#include <qpushbutton.h>

KateConfigPage::KateConfigPage(QWidget *parent, const char *name, KatePluginKateMate *katemate)
    :KateConfigPageWidget(parent, name)
{
    mKateMatePlugin = katemate;
}

void KateConfigPage::btShowConfigDialog_clicked()
{
    //KateMateConfigDia *dia = new KateMateConfigDia;
    //dia->show();
}

void KateConfigPage::btReloadSnippets_clicked()
{
    btReloadSnippets->setEnabled(false);
    
    // reload the snippets from disk file
    mKateMatePlugin->loadSnippets();
    
    debug("reloaded snippets from disk");
    
    // update visuals
    btReloadSnippets->setText("Reloaded Snippets");
}


#include "kateconfigpage.moc"
