/* The Butterfly Effect
 * This file copyright (C) 2015  Klaas van Gend
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA.
 */

#include "AbstractObject.h"
#include "ChoosePhoneNumber.h"
#include "ImageCache.h"
#include "TriggerExplosion.h"
#include "ViewPingus.h"
#include "Pingus.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QStyleOption>

// Constructors/Destructors
//

ViewPingus::ViewPingus (AbstractObjectPtr aAbstractObjectPtr, const QString &anImageName)
    : ViewObject(aAbstractObjectPtr, anImageName)
{
    // Override everything from the ViewObject constructor because we are special, baby
    DEBUG5ENTRY;
    QPixmap myTempPixmap;
    // don't want the image to be antialiased
    ImageCache::getPixmap("pingus", QSize(512, 384), &myTempPixmap, QPainter::NonCosmeticDefaultPen);

    thePixmapWidth = 32;
    thePixmapHeight = 32;

    thePixmapList.clear();
    for (unsigned int i = 0; i <= Pingus::DEAD; i++) {
        theIndexInImageList[i] = thePixmapList.size();
        for (unsigned int j = 0; j < Pingus::FramesPerState[i]; j++) {
            QPixmap mySmallPixmap = myTempPixmap.copy(j * thePixmapWidth,
                                                      i * thePixmapHeight,
                                                      thePixmapWidth,
                                                      thePixmapHeight);
            thePixmapList.push_back(mySmallPixmap);
        }
    }
    setPixmap(thePixmapList[0]);
    theDecorator.setViewObject(this);
}


ViewPingus::~ViewPingus ( )
{
}


void ViewPingus::adjustObjectDrawing(qreal aWidth, qreal aHeight, const Position &aCenter)
{
    // override angle so penguin remains 'upright'
    Position myCenter(aCenter.x, aCenter.y + 0.02, 0.01);
    ViewObject::adjustObjectDrawing(aWidth + 0.04, aHeight + 0.04, myCenter);
}


void ViewPingus::setNewAnimationFrame(unsigned int aState, unsigned int aFrameIndex)
{
    Q_ASSERT(aState <= Pingus::DEAD);
    Q_ASSERT(aFrameIndex < Pingus::FramesPerState[aState]);
    setPixmap(thePixmapList[theIndexInImageList[aState] + aFrameIndex]);
}
