/* The Butterfly Effect
 * This file copyright (C) 2013 Klaas van Gend
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
#include "GameResources.h"
#include "InsertUndoCommand.h"
#include "Popup.h"
#include "Position.h"
#include "ToolboxGroupIcon.h"
#include "ViewObject.h"


ToolboxGroupIcon::ToolboxGroupIcon(ToolboxGroup* aTBGPtr, GameResources* aGRPtr, QWidget*)
    : QWidget(NULL),
      theTBGPtr(aTBGPtr)
{
    Q_ASSERT(aTBGPtr!=NULL);

    if (theTBGPtr->count() > 0)
    {
        AbstractObject* myAOPtr = theTBGPtr->first();
        ViewObject* myVOPtr = myAOPtr->createViewObject();

        theOriginalM11 = aGRPtr->theTransformMatrix.m11();
        theIconSize.setWidth(( myAOPtr->getTheWidth()* (float)THESCALE *
                           theOriginalM11));
        theIconSize.setHeight(( myAOPtr->getTheHeight()* (float)THESCALE *
                           aGRPtr->theTransformMatrix.m22()));

        thePixmap = myVOPtr->pixmap();
    }
    theTransformMatrix = aGRPtr->theTransformMatrix;
}


void ToolboxGroupIcon::paintEvent ( QPaintEvent * )
{
    QPainter painter(this);
    painter.setBackgroundMode(Qt::TransparentMode);
//    painter.setBackground(QBrush(QColor(0,0,0,255)));
//    painter.eraseRect(painter.viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPixmap(painter.viewport(), thePixmap);
}


void ToolboxGroupIcon::updateCount(void)
{
    setMinimumSize(theIconSize);
    setMaximumSize(theIconSize);
}
