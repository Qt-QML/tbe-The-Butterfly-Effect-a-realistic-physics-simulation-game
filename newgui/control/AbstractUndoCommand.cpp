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

#include "AbstractUndoCommand.h"
#include "UndoSingleton.h"
#include "ViewObjectActionDectorator.h"
#include "ViewObject.h"

AbstractUndoCommand::AbstractUndoCommand(
        ViewObject* anViewObjectPtr,
        const QString& anUndoName,
        QUndoCommand* parent)
    : QUndoCommand(parent),
      theViewObjPtr(anViewObjectPtr)
{
    // This is the undo action: %1 will contain e.g. "Move"
    // and %2 might contain BowlingBall
    setText( QString("%1 %2").arg(anUndoName)
             .arg(anViewObjectPtr->getAbstractObjectPtr()->getName()) );
    theOrigPos = anViewObjectPtr->getAbstractObjectPtr()->getOrigCenter();
}



void AbstractUndoCommand::redo()
{
    qDebug() << Q_FUNC_INFO;
    // TODO/FIXME: implement!
}


void AbstractUndoCommand::setupProxyImage(const QString& anImageName)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(anImageName.isEmpty()==false);
    Q_ASSERT(theViewObjPtr!=NULL);
    theVOADPtr = new ViewObjectActionDecorator(theViewObjPtr, anImageName, this);
}


void AbstractUndoCommand::undo()
{
    qDebug() << Q_FUNC_INFO;
    // TODO/FIXME: implement!
}
