/* The Butterfly Effect
 * This file copyright (C) 2009,2010  Klaas van Gend
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

#include "Anchors.h"
#include "ImageStore.h"
#include "DrawObject.h"
#include "BaseObject.h"
#include "UndoDeleteCommand.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <cassert>
#include <cmath>


EditObjectDialog* Anchors::theObjectDialogPtr = NULL;

//////////////////////////////////////////////////////////////////////////////
// first the implementation for class Anchors, below for class Anchor
// (note the missing 's')


Anchors::Anchors(DrawObject* anObjectPtr)
		: theDrawObjectPtr(anObjectPtr)
{
	DEBUG5("Anchors::Anchors for %p\n", anObjectPtr);

	// only create anchors when it is a player-inserted object
	BaseObject* myBOPtr = theDrawObjectPtr->getBaseObjectPtr();
	if (myBOPtr->isMovable()==false)
		return;

	// the below code creates the 8 anchors around the object
	AnchorType myMode = NONE;
	if ( (myBOPtr->isResizable()&BaseObject::HORIZONTALRESIZE) || theIsLevelEditor)
		myMode = RESIZEHORI;
	theAnchorList.push_back(new ResizeAnchor(myMode, Anchor::RIGHT, this));
	theAnchorList.push_back(new ResizeAnchor(myMode, Anchor::LEFT, this));

	if ( (myBOPtr->isResizable()&BaseObject::VERTICALRESIZE) || theIsLevelEditor)
		myMode = RESIZEVERTI;
	else
		myMode = NONE;
	theAnchorList.push_back(new ResizeAnchor(myMode, Anchor::TOP, this));
	theAnchorList.push_back(new ResizeAnchor(myMode, Anchor::BOTTOM, this));

	if (myBOPtr->isRotatable() || theIsLevelEditor)
		myMode = ROTATE;
	else
		myMode = NONE;
	theAnchorList.push_back(new RotateAnchor(myMode, Anchor::TOPRIGHT, this));
	theAnchorList.push_back(new RotateAnchor(myMode, Anchor::TOPLEFT, this));
	theAnchorList.push_back(new RotateAnchor(myMode, Anchor::BOTTOMLEFT, this));
	theAnchorList.push_back(new RotateAnchor(myMode, Anchor::BOTTOMRIGHT, this));

	// the DELETE button is displayed left of topleft
	theAnchorList.push_back(new Anchor(DELETE, Anchor::TOPLEFTLEFT, this));

	if (theIsLevelEditor)
	{
		if (theObjectDialogPtr == NULL)
		{
			theObjectDialogPtr = new EditObjectDialog(myBOPtr, QApplication::activeWindow());
			theObjectDialogPtr->show();
		}
		else
		{
			theObjectDialogPtr->readFromObject(myBOPtr);
			theObjectDialogPtr->show();
		}
	}
	updatePosition();
}

Anchors::~Anchors()
{
	while(!theAnchorList.isEmpty())
	{
		Anchor* myPtr = theAnchorList.first();
		delete myPtr;
		theAnchorList.pop_front();
	}
	theDrawObjectPtr->focusRemove(false);
}

BaseObject*   Anchors::getBOPtr(void) const
{
	return theDrawObjectPtr->getBaseObjectPtr();
}

bool Anchors::isAnchor(QGraphicsItem* anItem)
{
	for (int i=0; i<theAnchorList.size();i++)
	{
		if (theAnchorList[i]==anItem)
			return true;
	}
	return false;
}

QGraphicsScene* Anchors::getScenePtr()
{
	return theDrawObjectPtr->scene();
}

void Anchors::updatePosition()
{
	Position myC = theDrawObjectPtr->getBaseObjectPtr()->getOrigCenter();
	myC.angle = fmodf(myC.angle, PI*2.0);
	qreal myW = theDrawObjectPtr->getBaseObjectPtr()->getTheWidth();
	qreal myH = theDrawObjectPtr->getBaseObjectPtr()->getTheHeight();

	for (int i=0; i<theAnchorList.count();i++)
	{
		theAnchorList[i]->updatePosition(myC, myW, myH);
	}
}

//////////////////////////////////////////////////////////////////////////////

const int Anchor::theIconSize = 16;

Anchor::Anchor(Anchors::AnchorType aDirection, AnchorPosition anIndex, Anchors* aParent)
		: theParentPtr(aParent),
		  theDirection(aDirection),
		  theIndex(anIndex), theOldAngle(0)
{
	// get the QSvgRenderer for my icon
	switch (aDirection)
	{
		case Anchors::NONE:
			setSharedRenderer(ImageStore::getRenderer("ActionNone"));
			break;
		case Anchors::RESIZEHORI:
			setSharedRenderer(ImageStore::getRenderer("ActionResizeHori"));
			break;
		case Anchors::RESIZEVERTI:
			setSharedRenderer(ImageStore::getRenderer("ActionResizeVerti"));
			break;
		case Anchors::ROTATE:
			setSharedRenderer(ImageStore::getRenderer("ActionRotate"));
			break;
		case Anchors::DELETE:
			setSharedRenderer(ImageStore::getRenderer("ActionDelete"));
			break;
	}

	// calculate how to scale my icon to look like it should
	QRectF mySquare=theParentPtr->getScenePtr()->views()[0]->mapToScene(QRect(0,0,theIconSize,theIconSize)).boundingRect();
	DEBUG5("32 pix = %f hori / %f verti\n", mySquare.width(), mySquare.height());
	// so we now know that our image should be reduced to width&height
	scale(mySquare.width()/boundingRect().width(),
		  mySquare.height()/boundingRect().height());

	theParentPtr->getScenePtr()->addItem(this);
	setZValue(10.0);

	theOffset=mySquare.width()/2;
}


int Anchor::getDX()
{
	switch (theIndex)
	{
		case RIGHT:
		case TOPRIGHT:
		case BOTTOMRIGHT:
			return 1;

		case TOP:
		case BOTTOM:
			return 0;

		case TOPLEFTLEFT:
		case TOPLEFT:
		case LEFT:
		case BOTTOMLEFT:
			return -1;
	}
	return 0;
}

int Anchor::getDY()
{
	switch (theIndex)
	{
		case RIGHT:
		case LEFT:
			return 0;

		case TOPRIGHT:
		case TOP:
		case TOPLEFT:
		case TOPLEFTLEFT:
			return 1;

		case BOTTOMRIGHT:
		case BOTTOM:
		case BOTTOMLEFT:
			return -1;
	}
	return 0;
}



void Anchor::mouseMoveEvent ( QGraphicsSceneMouseEvent* event )
{
	DEBUG5("Anchor::mouseMoveEvent(%d)\n", event->type());
}

void Anchor::mousePressEvent ( QGraphicsSceneMouseEvent* /* event */)
{
	DEBUG5("Anchor::mousePressEvent\n");

	// only active this if there are real icons to be had
	if (theDirection==Anchors::NONE)
		return;

	if (theIndex == TOPLEFTLEFT)
	{
		// the DELETE button
		UndoDeleteCommand* myCommandPtr = new UndoDeleteCommand(theParentPtr->getBOPtr());
		myCommandPtr->push();
	}
}

void Anchor::mouseReleaseEvent ( QGraphicsSceneMouseEvent*)
{
	DEBUG5("Anchor::mouseReleaseEvent\n");

	// delete has no release event, clicking is enough.
	// so that one is missing here...
}

void Anchor::updatePosition(Position myC, qreal myW, qreal myH)
{
	// calculate the position of the center of the anchor
	qreal myDX = (0.5*myW+1.5*theOffset) * static_cast<qreal>(getDX()) - theOffset;
	qreal myDY = (0.5*myH+1.5*theOffset) * static_cast<qreal>(getDY()) + theOffset;
	if (theIndex==Anchor::TOPLEFTLEFT)
		myDX = myDX -2.5*theOffset;

	myC = myC + Vector(myDX,myDY);

	// the final rotate/move
	rotate((-myC.angle-theOldAngle)*180/3.14);
	setPos(myC.x, -myC.y);
	theOldAngle=-myC.angle;
}


// #########################################################################
// #########################################################################
// #########################################################################
// #########################################################################

ResizeAnchor::ResizeAnchor(Anchors::AnchorType aDirection, AnchorPosition anIndex, Anchors* aParent)
		: Anchor(aDirection, anIndex, aParent), theUndoResizePtr(NULL)
{
}

void ResizeAnchor::mouseMoveEvent ( QGraphicsSceneMouseEvent* event )
{
	DEBUG5("ResizeAnchor::mouseMoveEvent(%d)\n", event->type());

	if(theUndoResizePtr==NULL)
		return;

	QPointF aCursorPos = event->scenePos();
	Position myOldCenter = theUndoResizePtr->getOldPosition();
	Position myNewCenter;
	Vector myOldSize = theUndoResizePtr->getOldSize();
	Vector myNewSize;

	// Essentially, we're going to project the current cursor position
	// across the axis through the object center and the anchor
	// the length of the projection is used to calculate the new
	// width of the object.
	// The goal is to keep the anchor at the other end of the object at the
	// same position.

	Vector myVectorToCenter = Vector(aCursorPos) - myOldCenter.toVector();
	float myAngle = myVectorToCenter.toAngle();
	float myAxisAngle = theIndex*PI/4 + myOldCenter.angle;
	float myLengthAcrossAxis = myVectorToCenter.length()*cos(myAngle-myAxisAngle);

	// This works - even for larger angles!
	if (theIndex==RIGHT || theIndex==LEFT)
	{
		float myOldWidth = myOldSize.dx/2.0;
		if (myLengthAcrossAxis+myOldWidth < 0.1)
			return;
		myNewSize = Vector(myLengthAcrossAxis+myOldWidth, myOldSize.dy);
		if (theIndex==Anchor::RIGHT)
			myNewCenter = myOldCenter + Vector(0.5*(myLengthAcrossAxis-myOldWidth),0);
		if (theIndex==Anchor::LEFT)
			myNewCenter = myOldCenter + Vector(-0.5*(myLengthAcrossAxis-myOldWidth),0);
	}
	if (theIndex==TOP || theIndex==BOTTOM)
	{
		float myOldHeight= myOldSize.dy/2.0;
		if (myLengthAcrossAxis+myOldHeight < 0.1)
			return;
		myNewSize = Vector(myOldSize.dx, myLengthAcrossAxis+myOldHeight);
		if (theIndex==Anchor::TOP)
			myNewCenter = myOldCenter + Vector(0, 0.5*(myLengthAcrossAxis-myOldHeight));
		if (theIndex==Anchor::BOTTOM)
			myNewCenter = myOldCenter + Vector(0, -0.5*(myLengthAcrossAxis-myOldHeight));
	}

	theUndoResizePtr->update(myNewCenter, myNewSize);
//	theParentPtr->updatePosition();
}

void ResizeAnchor::mousePressEvent ( QGraphicsSceneMouseEvent* event)
{
	DEBUG5("ResizeAnchor::mousePressEvent\n");

	// only active this if there are real icons to be had
	if (theDirection==Anchors::NONE)
		return;

	// reset cursor to hori/verti/rotate
	// and setup the corresponding undo class
	assert(theUndoResizePtr==NULL);

	if (theIndex==RIGHT || theIndex==LEFT)
		setCursor(Qt::SizeHorCursor);
	if (theIndex==TOP   || theIndex==BOTTOM)	
		setCursor(Qt::SizeVerCursor);
	
	// store this position - calculate differential angles later
	theUndoResizePtr = UndoObjectChange::createUndoObject(
			UndoObjectChange::RESIZE, theParentPtr->getBOPtr(), event->scenePos());
}

void ResizeAnchor::mouseReleaseEvent ( QGraphicsSceneMouseEvent*)
{
	DEBUG5("ResizeAnchor::mouseReleaseEvent\n");

	if (theUndoResizePtr==NULL)
		return;

	if (theUndoResizePtr->pushYourself()==true)
	{
		DEBUG5("PUSHED UNDO\n");
	}
	else
	{
		DEBUG5("CLEARED UNDO\n");
		theUndoResizePtr->undo();
		delete theUndoResizePtr;
	}
	theUndoResizePtr = NULL;

}

// #########################################################################
// #########################################################################
// #########################################################################
// #########################################################################

RotateAnchor::RotateAnchor(Anchors::AnchorType aDirection, AnchorPosition anIndex, Anchors* aParent)
		: Anchor(aDirection, anIndex, aParent), theUndoRotatePtr(NULL)
{
}

float RotateAnchor::getCurrentAngle(Vector aHotspot) const
{
	return (aHotspot - theParentPtr->getBOPtr()->getOrigCenter().toVector()).toAngle();
}


void RotateAnchor::mouseMoveEvent ( QGraphicsSceneMouseEvent* event )
{
	DEBUG5("RotateAnchor::mouseMoveEvent(%d)\n", event->type());

	if(theUndoRotatePtr==NULL)
		return;
	theUndoRotatePtr->update(getCurrentAngle(event->scenePos())-theHotspotAngle);
//	theParentPtr->updatePosition();
}

void RotateAnchor::mousePressEvent ( QGraphicsSceneMouseEvent* event)
{
	DEBUG5("RotateAnchor::mousePressEvent\n");

	// only active this if there are real icons to be had
	if (theDirection==Anchors::NONE)
		return;

	// reset cursor to hori/verti/rotate
	// and setup the corresponding undo class
	assert(theUndoRotatePtr==NULL);

	// TODO: add cursor ROTATE shape
	// store this position - calculate differential angles later
	theUndoRotatePtr = UndoObjectChange::createUndoObject(
			UndoObjectChange::ROTATE, theParentPtr->getBOPtr(), event->scenePos());

	theHotspotAngle = getCurrentAngle(event->scenePos());
}

void RotateAnchor::mouseReleaseEvent ( QGraphicsSceneMouseEvent*)
{
	DEBUG5("RotateAnchor::mouseReleaseEvent\n");

	if (theUndoRotatePtr==NULL)
		return;

	if (theUndoRotatePtr->pushYourself()==true)
	{
		DEBUG5("PUSHED UNDO\n");
	}
	else
	{
		DEBUG5("CLEARED UNDO\n");
		theUndoRotatePtr->undo();
		delete theUndoRotatePtr;
	}
	theUndoRotatePtr = NULL;

}
