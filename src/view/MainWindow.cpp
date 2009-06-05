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

#include "tbe_global.h"
#include "MainWindow.h"
#include "Popup.h"

#include "Level.h"
#include "World.h"
#include "DrawWorld.h"
#include "SplashScreen.h"

//////////////////////////////////////////////////////////////////////////////
// constructors & destructors

MainWindow::MainWindow(QWidget *parent)
     : QMainWindow(parent), theSimStateMachine(&ui)
{                                      
	ui.setupUi(this);
	
	QGraphicsSvgItem* myTitlePagePtr = new SplashScreen("images/illustrations/title_page.svg");
	QGraphicsScene* mySplashScenePtr = new QGraphicsScene(NULL);

	mySplashScenePtr->addItem(myTitlePagePtr);
	
	// basic properties of the view
	ui.graphicsView->setInteractive(true);
	ui.graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
	ui.graphicsView->setRubberBandSelectionMode(Qt::ContainsItemShape);

	// set my splash screen scene in view and make it fit nicely
	ui.graphicsView->setScene(mySplashScenePtr);
	QRectF myRect = myTitlePagePtr->boundingRect ();
	// TODO: FIXME: this /32.0 is wrong. very wrong :-(
	myRect.setWidth(myRect.width()/20.0);
	myRect.setHeight(myRect.height()/20.0);
	ui.graphicsView->fitInView(myRect, Qt::KeepAspectRatio);
	connect(myTitlePagePtr, SIGNAL(clicked()), 
			this, SLOT(on_splashScreen_clicked()));
}                           

//////////////////////////////////////////////////////////////////////////////
// public slots - alphabetical

void MainWindow::on_actionAbout_activated()
{
	Popup::Info(tr("<b>The Butterfly Effect</b><br><br>"
				"An open source game that uses realistic physics"
				" simulations to combine lots of simple mechanical elements"
				" to achieve a simple goal in the most complex way possible.<br><br>"
				"(C) 2009 Peter van Ginneken and Klaas van Gend<br>"
				"Licensed under GPL version 2"), this);
}

void MainWindow::on_actionOpen_level_activated()
{
	// TODO this code is more-or-less temporary 
	// - it should be replaced by a dashboard style interface 
	QString myFileName = QFileDialog::getOpenFileName(this,
	     tr("Open Level"), ".", tr("TBE Levels (*.xml *.tbe)"));
	loadLevel(myFileName);
}
 
void MainWindow::on_splashScreen_clicked(void)
{
	// TODO: FIXME: hardcoded level name!!!
	loadLevel("levels/draft/001_bowling_for_butterflies.xml");
}

//////////////////////////////////////////////////////////////////////////////
// public accessor methods

void MainWindow::loadLevel(const QString& aFileName)
{
	// create level and display in main window
    Level* myLevelPtr = new Level();
    QString myErrorMessage = myLevelPtr->load(aFileName); 
    if (!myErrorMessage.isEmpty())
    {
    	// TODO: popup and such
    	DEBUG1("ERROR during reading file '%s': %s\n",
    			aFileName.toAscii().constData(),
    			myErrorMessage.toAscii().constData() );
    	exit(1);
    }
    myLevelPtr->getTheWorldPtr()->createScene(this);
}

void MainWindow::setScene(DrawWorld* aScene, const QString& aLevelName)
{
	ui.graphicsView->setScene(aScene);
	
	// Unfortunately, fitInView() causes a segfault :-(
//	ui.graphicsView->fitInView(aScene->theBackGroundRectPtr);

	// that's why I have to do it "by hand":
	{
		// calculate x scaling and y scaling
		QSize myViewSize = ui.graphicsView->size();
		float xScale = (myViewSize.width()-10) / aScene->getWidth();
		float yScale = (myViewSize.height()-10) / aScene->getHeight();
	
		// and use the lowest value for both X and Y to keep correct aspect ratio
		if (xScale > yScale)
			xScale = yScale;
		// X horizontal positive to right
		// Y vertical   positive up -> that's why we need the negative
		ui.graphicsView->scale(1.0*xScale, 1.0*xScale);
		ui.graphicsView->centerOn(aScene->getWidth()/2.0, -aScene->getHeight()/2.0);
	}
		
    QObject::connect(&theSimStateMachine, SIGNAL(startSim()), aScene, SLOT(startTimer()));
    QObject::connect(&theSimStateMachine, SIGNAL(stopSim()),  aScene, SLOT(stopTimer()));
    QObject::connect(&theSimStateMachine, SIGNAL(resetSim()), aScene, SLOT(resetWorld()));
    
    setWindowTitle(APPNAME " - " + aLevelName);
}

