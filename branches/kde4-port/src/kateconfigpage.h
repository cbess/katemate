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
#ifndef KATECONFIGPAGE_H
#define KATECONFIGPAGE_H

#include "KateConfigPageWidget.h"

class KatePluginKateMate;

/* Avail. control instances
 * QButton btReloadSnippets
 *
 */

/**
 * Represents the configuration page shown within Kate::Settings.
 */
class KateConfigPage: public KateConfigPageWidget 
{
    Q_OBJECT
            
    private:
        KatePluginKateMate *mKateMatePlugin;        
    
    public:
        KateConfigPage(QWidget *parent = 0, const char *name = 0, KatePluginKateMate *katemate = 0L);
        
    public slots:
        virtual void btShowConfigDialog_clicked();
        virtual void btReloadSnippets_clicked();
};

#endif
