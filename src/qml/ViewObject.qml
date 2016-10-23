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
import QtQuick.Controls 1.4
import TBEView 1.0

// The ViewItem is an object we own from C++, it contains all info we need.
// We use an embedded Image (and in the future probably also a Text) to
// handle the display of what we want.
ViewItem {
    id: viewItem

    property var tooltip;
    property var imageName;

    signal signalUpdateVars(real anXM, real aYM, real aRotDegrees, real aWidthM, real aHeightM);

    function restoreBindings() {
        x        = Qt.binding(function() { return m2xwh(xInM); })
        y        = Qt.binding(function() { return m2y(yInM); })
        width    = Qt.binding(function() { return m2xwh(widthInM); })
        height   = Qt.binding(function() { return m2xwh(heightInM); })
        rotation = Qt.binding(function() { return angleInDegrees})
    }

    /// Update using a function+signal instead of 5 existing signals to save
    /// execution overhead.
    function updateVars() {
        signalUpdateVars( xwh2m(x+width/2),
                          y2m(y+height/2),
                          rotation,
                          xwh2m(width),
                          xwh2m(height));
    }

    // Any variables that we modify using the ResizeRotateMoveDecorator will
    // lose their binding to the ViewItem properties. The above restoreBindings()
    // function 'fixes' that. The above updateVars() function updates the undo
    // object with the current values (that wind up in the ViewItem via the undo).
    x: m2xwh(xInM);
    y: m2y(yInM);
    width: m2xwh(widthInM);
    height: m2xwh(heightInM);
    rotation: angleInDegrees;


    Image {
        anchors.fill: parent
        source: img(imageName)
        // performance optimization: do not ask for redraw of images while resizing/rotating/dragging
        sourceSize.width:  (selectedItem != undefined && selectedItem.theDecorated===viewItem) ? selectedItem.oldWidth : width
        sourceSize.height: (selectedItem != undefined && selectedItem.theDecorated===viewItem) ? selectedItem.oldHeight: height
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            if(selectedItem)
                if (selectedItem.theDecorated != parent) {
                    selectedItem.destroy();
                    selectedItem = undefined
                }
            if(!selectedItem) {
                var component = Qt.createComponent("ResizeRotateMoveDecorator.qml");
                if (component.status == Component.Ready) {
                    selectedItem = component.createObject(gameView, {
                                                           "theDecorated": parent,
                                                           "oldWidth": parent.width,
                                                           "oldHeight": parent.height,
                                                           "oldZ": parent.z,
                                                           "width": parent.width,
                                                           "height": parent.height})
                    // we explicitly set the decorator to be a child of the decorated
                    selectedItem.parent = viewItem;
                    // raise to the front while moving/resizing/rotating
                    viewItem.z = 9999;
                }
                else
                    console.log("Error creating component: '" + component.errorString() + "'");
            }
        }
    }

    // Disabled: current implementation is too crude.
    // Unfortunately, it will steal all hover/click events away from other MouseAreas.
//    Tooltip {
//        text: parent.tooltip
//    }

}
