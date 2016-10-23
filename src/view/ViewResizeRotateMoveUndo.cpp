/* The Butterfly Effect
 * This file copyright (C) 2016  Klaas van Gend
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

#include "ViewResizeRotateMoveUndo.h"
#include "ViewItem.h"
#include "UndoSingleton.h"
#include "AbstractQUndoCommand.h"

ViewResizeRotateMoveUndo::ViewResizeRotateMoveUndo(QQuickItem *parent)
    : QQuickItem(parent),
      theUndoPtr(nullptr)
{
    assert(nullptr == parent);
}

ViewResizeRotateMoveUndo::~ViewResizeRotateMoveUndo()
{
    // Flush the last existing undo.
    startNewUndo("", nullptr);
}

QQuickItem *ViewResizeRotateMoveUndo::activeHandle()
{
    if (nullptr == theUndoPtr)
        return nullptr;
    return theUndoPtr->theHandlePtr;
}

void ViewResizeRotateMoveUndo::commitChanges()
{
    if (theUndoPtr->isChanged())
        theUndoPtr->commit();
    else
        delete theUndoPtr;
    theUndoPtr = nullptr;
}


qreal ViewResizeRotateMoveUndo::vector2AngleDegrees(qreal dx, qreal dy)
{
    Vector myVector(dx, dy);
    return myVector.toAngle()* 180/PI;
}

void ViewResizeRotateMoveUndo::startNewUndo(const QString& aType, QQuickItem *aHandlePtr)
{
    DEBUG1ENTRY;
    // do we need to take care of the previous undo first?
    if (theUndoPtr)
    {
        commitChanges();
        theUndoPtr = nullptr;
    }
    if (aHandlePtr)
    {
        theUndoPtr = UndoSingleton::createQUndoCommand(theDecoratedPtr, aHandlePtr, aType);
        emit theActiveHandleChanged();
    }
}