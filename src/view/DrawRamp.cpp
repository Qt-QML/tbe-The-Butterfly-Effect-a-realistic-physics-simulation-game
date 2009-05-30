#include "DrawRamp.h"
#include "BaseObject.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>

// Constructors/Destructors
//  

DrawRamp::DrawRamp (BaseObject* aBaseObjectPtr)
	: DrawObject(aBaseObjectPtr)
{
	// everything is done in the DrawObject constructor
}

DrawRamp::~DrawRamp ( ) { }

//  
// Methods
//  


// Accessor methods
//  


// Other methods
//  

void DrawRamp::advance(int step)
{
	// this object never moves - it shouldn't need to advance...
	//applyPosition();
}

void DrawRamp::applyPosition(void)
{
	// TODO FIXME: Add rotation here
    const dReal *pos1 = dGeomGetPosition (theBaseObjectPtr->getTheGeomID());
    const dReal *ang  = dGeomGetRotation (theBaseObjectPtr->getTheGeomID());
    
    qreal myAngle = atan2(ang[1], ang[0]);
    
    // Qt has Y positive downwards, whereas all of the model has Y upwards.
    // that's what the minus is for :-)
    setPos(pos1[0], -pos1[1]);
    rotate((myAngle-theOldAngle)*180/3.14);
    theOldAngle=myAngle;
}

QRectF DrawRamp::boundingRect() const
{
	Ramp* myRampPtr = reinterpret_cast<Ramp*>(theBaseObjectPtr);
	qreal myWidth = myRampPtr->getSlabLength()*theScale;
	qreal myHeight= myRampPtr->theSlabThickness*theScale;
	const qreal adjust = 0.1;
    return QRectF(-myWidth/2-adjust, -myHeight/2-adjust, myWidth+2*adjust, myHeight+2*adjust);
}

void DrawRamp::initAttributes ( ) 
{
	
}

void DrawRamp::paint(QPainter* myPainter, const QStyleOptionGraphicsItem *, QWidget *)
{
	Ramp* myRampPtr = reinterpret_cast<Ramp*>(theBaseObjectPtr);
	qreal myWidth = myRampPtr->getSlabLength()*theScale;
	qreal myHeight= myRampPtr->theSlabThickness*theScale;
	
//	DEBUG5("void DrawRamp::paint - %fx%f\n",
//			   myRampPtr->getSlabLength(), 
//			   myRampPtr->theSlabThickness);
	
	QColor color(qrand() % 256, qrand() % 256, qrand() % 256);
    myPainter->setBrush(color);
    myPainter->drawRect(-myWidth/2, -myHeight/2, myWidth, myHeight);
    applyPosition();
}
