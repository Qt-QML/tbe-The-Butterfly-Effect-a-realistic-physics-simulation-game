/* The Butterfly Effect
 * This file copyright (C) 2011  Klaas van Gend
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
#include "ChoosePhoneNumber.h"
#include "ImageCache.h"
#include "TriggerExplosion.h"
#include "ViewDetonatorBox.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QStyleOption>

// Constructors/Destructors
//

ViewDetonatorBox::ViewDetonatorBox (AbstractObject* aAbstractObjectPtr, const QString& anImageName)
	: ViewObject(aAbstractObjectPtr, anImageName),
	  theHandle(NULL)
{
	// Everything is done in the ViewObject constructor...
	DEBUG5ENTRY;

	// Except that fact that we have a child object...
	// Which is controlled in parent coordinates, by the way...
	theHandle.setParentItem(this);
	QPixmap myTempPixmap;
	ImageCache::getPixmap("DetonatorBoxHandle", &myTempPixmap);
	theHandle.setPixmap(myTempPixmap);
//	theHandle.setFlag(QGraphicsItem::ItemStacksBehindParent,true);

}

ViewDetonatorBox::~ViewDetonatorBox ( )
{
}

//
// Methods
//


// Accessor methods
//


// Other methods
//

void ViewDetonatorBox::displayChoosePhoneNumber(void)
{
	// display a combobox with those numbers
	ResizingGraphicsView* myViewPtr = dynamic_cast<ResizingGraphicsView*>(scene()->views()[0]);
	DetonatorBox* myDetBoxPtr = dynamic_cast<DetonatorBox*>(theAbstractObjectPtr);
	assert (myViewPtr != NULL);
	assert (myDetBoxPtr != NULL);
	ChoosePhoneNumber* myDialogPtr = new ChoosePhoneNumber(myDetBoxPtr, myViewPtr);
	myDialogPtr->appearAnimated();

	// The dialog will set the phone numbers itself, no need for us to worry
	// it will also clean up after itself, I hope :-)
}

void ViewDetonatorBox::mousePressEvent(QGraphicsSceneMouseEvent* anEvent)
{
	// depending on whether we're level editor or not, move or display
	if (theIsLevelEditor)
		ViewObject::mousePressEvent(anEvent);
	else
	{
		displayChoosePhoneNumber();
		anEvent->accept();
	}
}


void ViewDetonatorBox::updateHandlePosition(qreal aDistance)
{
	Vector myVector(0,-aDistance);
	theHandle.setPos(myVector.toQPointF());
}