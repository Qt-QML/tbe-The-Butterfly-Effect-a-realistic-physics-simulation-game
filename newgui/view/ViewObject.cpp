/* The Butterfly Effect
 * This file copyright (C) 2011 Klaas van Gend
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

#include "AbstractObject.h"
#include "ImageCache.h"
#include "PieMenu.h"
#include "ViewObject.h"

#include "tbe_global.h"

#include <QtCore/QFile>
#include <QtGui/QGraphicsColorizeEffect>
#include <QtGui/QGraphicsSceneMouseEvent>

ViewObject::ViewObject(AbstractObject* anAbstractObjectPtr) :
	QGraphicsPixmapItem(NULL), theAbstractObjectPtr(anAbstractObjectPtr)
{
	// nothing to do yet :-)
	DEBUG3ENTRY;
	Q_ASSERT(anAbstractObjectPtr!=NULL);
	initViewObjectAttributes();
}

ViewObject::ViewObject(AbstractObject* anAbstractObjectPtr, const QString& anImageName) :
	QGraphicsPixmapItem(NULL), theAbstractObjectPtr(anAbstractObjectPtr)
{
	DEBUG3ENTRY;
	Q_ASSERT(anAbstractObjectPtr!=NULL);

	QPixmap myTempPixmap;
	QStringList myImageNames = anImageName.split(";");
	foreach(QString i, myImageNames)
	{
		ImageCache::getPixmap(i, &myTempPixmap);
		thePixmapList.push_back(myTempPixmap);
	}
	Q_ASSERT(thePixmapList.count() > 0);
	setPixmap(thePixmapList[0]);

	thePixmapWidth = QGraphicsPixmapItem::boundingRect().width();
	thePixmapHeight= QGraphicsPixmapItem::boundingRect().height();

	adjustObjectDrawing();

	initViewObjectAttributes();
}

ViewObject::~ViewObject()
{
	// nothing to do yet :-)
	theAbstractObjectPtr = NULL;
}

void ViewObject::adjustObjectDrawing(qreal aWidth, qreal aHeight, const Position& aCenter)
{
	// set transformations: origin, scale, translate

	// Qt by default scales and rotates around (0,0),
	// which is a corner of the object

	// Problem: Qt merges transformations and handles them in the wrong order.
	// * it first uses a rotation set with setRotate.
	// * then it executes a transformation set with setTransform
	//   (whatever transformation is set here)
	// * then it executes a translation set with setPos
	//
	// Problem2: you cannot put a rotation and a translation and a scaling
	// into setTransform because the three together will automagically
	// result in an unwanted slanting of the object.
	//
	// Problem3: if you set a rotation and a scaling in the transform matrix
	// it still will look like you do the rotation first and the scaling second
	//
	// So the end result:
	// * we rotate and scale (in that order!) in the setTransform
	// * then we translate in the setPos to set the the midpoint at the
	//   defined position (instead of the top-left corner)

    if (fabs(aWidth-theOldWidth)>0.001 || fabs(aHeight-theOldHeight)>0.001 ||
            fabs(aCenter.angle-theOldPos.angle)>0.001)
    {
        QTransform theTransform;
         // Qt has the Y axis in opposite direction, so negate the angle...
         theTransform.rotateRadians(-aCenter.angle);
         theTransform.scale(THESCALE*aWidth/thePixmapWidth,
                            THESCALE*aHeight/thePixmapHeight);
        setTransform(theTransform,false);
        theOldWidth=aWidth;
        theOldHeight=aHeight;
        theOldPos.angle=aCenter.angle;
    }

	if (aCenter!=theOldPos)
	{
		// Now we need to calculate the translation, based on the scaled&rotated
		// object that happened around the top-left corner.
		// So, where is the middle of our object currently?
		// And -as always- don't forget to correct for Qt's wrong Y axis...
		Position myMiddle = Position(0,0, -aCenter.angle)
							+ Vector(aWidth/2, aHeight/2);
		myMiddle.y = - myMiddle.y;
		setPos(aCenter.toQPointF() - myMiddle.toQPointF());
		theOldPos.x = aCenter.x;
		theOldPos.y = aCenter.y;
	}
}

void ViewObject::adjustObjectDrawing(void)
{
	adjustObjectDrawing(theAbstractObjectPtr->getTheWidth(),
						theAbstractObjectPtr->getTheHeight(),
						theAbstractObjectPtr->getOrigCenter());
}

void ViewObject::hoverEnterEvent ( QGraphicsSceneHoverEvent* )
{
    // this looks great, but unfortunately it also affects all children
    QGraphicsEffect* myEffect = new QGraphicsColorizeEffect();
    setGraphicsEffect(myEffect);
}


void ViewObject::hoverLeaveEvent ( QGraphicsSceneHoverEvent* )
{
    setGraphicsEffect(NULL);
}

void ViewObject::initViewObjectAttributes(void)
{
//    setFlags(QGraphicsItem::ItemIsMovable |
//             QGraphicsItem::ItemIsSelectable);
    setFlags(QGraphicsItem::ItemIsFocusable);
    setAcceptsHoverEvents(true);
    theDecorator.setViewObject(this);
}




void ViewObject::mousePressEvent ( QGraphicsSceneMouseEvent* anEvent )
{
    hoverLeaveEvent(NULL);
    PieMenuSingleton::addPieMenuToViewObject(this, anEvent->pos());
}

void ViewObject::setNewGeometry(const Position& aNewPosition, qreal aNewWidth, qreal aNewHeight)
{
    DEBUG4("%s", ASCII(QString("setNewGeometry Pos: %1, WxH %2x%3")
                .arg(aNewPosition.toString())
                .arg(aNewWidth).arg(aNewHeight)));
    theAbstractObjectPtr->setTheWidth(aNewWidth);
    theAbstractObjectPtr->setTheHeight(aNewHeight);
    setNewGeometry(aNewPosition);
}

void ViewObject::setNewGeometry(const Position& aNewPosition)
{
    theAbstractObjectPtr->setOrigCenter(aNewPosition);
    adjustObjectDrawing();
}


void ViewObject::setNewImageIndex(unsigned int anIndex)
{
    if (anIndex >= thePixmapList.count())
        anIndex = thePixmapList.count()-1;
    setPixmap(thePixmapList[anIndex]);
}
