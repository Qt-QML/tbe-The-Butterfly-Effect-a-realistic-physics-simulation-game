/* The Butterfly Effect 
 * This file copyright (C) 2009  Klaas van Gend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation
 * applicable version is GPL version 2 only.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include "MainWindow.h"

#include "ode/ode.h"

#include <QtGui>

// the verbosity for all logging - by default defined at 5 (most logging)
int theVerbosity = 5;


int main(int argc, char **argv)
{
	// init Qt (graphics toolkit) - www.qtsoftware.com
	QApplication app(argc, argv);
	// init OpenDE (physics library) - opende.sf.net
    dInitODE ();

	// setup main window
	MainWindow myMain;
	myMain.show();

	// run the main display loop    
    int myReturn=app.exec();
    
    // clean up (QT cleans up after itself)
    dCloseODE();
    
    return myReturn;
}
