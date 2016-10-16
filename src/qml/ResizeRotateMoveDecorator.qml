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

import QtQuick 2.0
import TBEView 1.0

ViewResizeRotateMoveUndo {
    id: theDecorator;
    objectName: "theDecorator";
    property real minSize: ResizeInfo.pixPerMeter * 0.1

    property real oldWidth;
    property real oldHeight;
    property real oldZ;

    Component.onDestruction: {
        theDecorated.z = oldZ;
    }

    Rectangle {
        anchors.fill: parent;
        border {
            width: 1
            color: "black"
        }
        color: /* theDecorated.isColliding ? "#80FF5050" :*/ "#8050FF50"
        visible: true

        HandleResizeHorizontal {
            id: left
            sign: -1
        }

        HandleResizeHorizontal {
            id: right
            sign: 1
        }

        HandleResizeVertical {
            id: up
            sign: -1
        }

        HandleResizeVertical {
            id: down
            sign: 1
        }

        HandleRotate {
            id: topleft
            hsign: -1
            vsign: -1
        }

        HandleRotate {
            id: topright
            hsign:  1
            vsign: -1
        }

        HandleRotate {
            id: bottomright
            hsign: 1
            vsign: 1
        }

        HandleRotate {
            id: bottomleft
            hsign: -1
            vsign:  1
        }

    }
}
