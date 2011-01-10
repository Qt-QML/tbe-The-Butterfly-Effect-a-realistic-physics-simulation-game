/* The Butterfly Effect
 * This file copyright (C) 2010  Klaas van Gend
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

#include "BalloonCactus.h"
#include "tbe_global.h"
#include "Box2D.h"
#include "Property.h"


//
//  This source file lists 3 classes:
//    * Balloon
//    * Cactus
//    * BedOfNails
//

///---------------------------------------------------------------------------
///------------------------ Balloon ------------------------------------------
///---------------------------------------------------------------------------


//// this class' ObjectFactory
class BalloonObjectFactory : public ObjectFactory
{
public:
	BalloonObjectFactory(void)
	{	announceObjectType("Balloon", this); }
	virtual BaseObject* createObject(void) const
	{	return fixObject(new Balloon()); }
};
static BalloonObjectFactory theBalloonObjectFactory;


const qreal Balloon::POPPING_TIME = 0.3;
const qreal Balloon::POPPED_MASS  = 0.6;
const qreal Balloon::POPPED_TIME  = 2.0;


Balloon::Balloon()
		: PolyObject(QObject::tr("Balloon"),
					 QObject::tr("a Helium Balloon. Lighter than air, it moves up."),
					 "Balloon;BalloonPoof;BalloonRest;Empty",
					 "(-0.018,0.18)=(-0.07,0.16)=(-0.12,0.1)=(-0.13,0.017)=(-0.1,-0.08)"
					 "=(-0.03,-0.16)=(0.006,-0.17)=(0.039,-0.16)=(0.10,-0.08)"
					 "=(0.13,0.015)=(0.11,0.11)=(0.07,0.16)=(0.01,0.18)",
					 0.27, 0.36, 0.1, 0.7)
{
	theState = BALLOON;
}

Balloon::~Balloon()
{
}


void Balloon::callbackStep (qreal aDeltaTime, qreal aTotalTime)
{
	DEBUG6("Balloon receives callback\n");

	switch(theState)
	{
	case BALLOON:
		callbackStepBalloon(aDeltaTime, aTotalTime);
		break;
	case POPPING:
		callbackStepPopping(aDeltaTime, aTotalTime);
		break;
	case POPPED:
		callbackStepPopped(aDeltaTime, aTotalTime);
		break;
	case GONE:
		break;
	}
}

void Balloon::callbackStepBalloon(qreal aDeltaTime, qreal aTotalTime)
{
	// the upward force...
	theB2BodyPtr->ApplyForce(b2Vec2(0,0.8), (getTempCenter()+Vector(0,0.1)).toB2Vec2());

	// the downward force...
	// i.e. the "drag" or "air restance"
	if (aDeltaTime >= aTotalTime)
	{
		// on the first call, set the previous position
		thePreviousPosition = getTempCenter();
	}

	Vector mySpeedVector = (getTempCenter().toVector() - thePreviousPosition.toVector());
	float myForce = -0.05 * (mySpeedVector.dx*mySpeedVector.dx + mySpeedVector.dy*mySpeedVector.dy)/aDeltaTime/aDeltaTime;

	if (mySpeedVector.length()/aDeltaTime < 0.01)
		myForce = 0;

	Vector myForceVector = myForce * Vector(mySpeedVector.toAngle());
	theB2BodyPtr->ApplyForce(myForceVector.toB2Vec2(), (getTempCenter()).toB2Vec2());
//	DEBUG1("speed: %f@%f / force: %f\n", mySpeedVector.length()/aDeltaTime,
//		   mySpeedVector.toAngle(), myForceVector.length());

	thePreviousPosition = getTempCenter();
}

void Balloon::callbackStepPopped(qreal /*aDeltaTime*/, qreal aTotalTime)
{
	// during the Popped state, which lasts for POPPED_TIME (only 2-3 seconds),
	// we reduce the mass step by step
	// once the time runs out, we switch to "GONE" state

	assert(thePoppingTimeStart>0.1);
	qreal myDelta = aTotalTime - thePoppingTimeStart - POPPING_TIME;
	if ( myDelta >= POPPED_TIME)
		goToState(GONE);
	else
	{
//		struct b2MassData myData;
//		myData.mass  = POPPED_MASS - POPPED_MASS/(POPPED_TIME-myDelta);
//		myData.center= b2Vec2(0,0);
//		myData.I     = theB2BodyPtr->GetInertia();
//		theB2BodyPtr->SetMass(&myData);
	}
}

void Balloon::callbackStepPopping(qreal /*aDeltaTime*/, qreal aTotalTime)
{
	// is this the first callback in Popping state???
	if (thePoppingTimeStart<0.0001)
	{
		thePoppingTimeStart=aTotalTime;
		switchToSmallShape();
	}
	if (aTotalTime-thePoppingTimeStart > POPPING_TIME)
		goToState(POPPED);
}


Balloon::States Balloon::goToState(Balloon::States aNewState)
{
	DEBUG5("Balloon change state request from %d to %d.\n", theState, aNewState);

	switch (theState)
	{
	case BALLOON:
		if (aNewState==POPPING)
		{
			theState = POPPING;
			// everything else will be handled in callbackStepPopping
		}
		if (aNewState==POPPED)
		{
			// typical case of "this should never happen"
			assert(false);
		}
		break;
	case POPPING:
		if (aNewState==POPPED)
		{
			theState = POPPED;
		}
		break;
	case POPPED:
		if (aNewState==GONE)
		{
			theState=GONE;
			deletePhysicsObject();
		}
		break;
	case GONE:
		// nothing to be done here...
		break;
	}
	return theState;
}

void Balloon::notifyJoints(JointInterface::JointStatus aStatus)
{
	if (theState != BALLOON && aStatus==JointInterface::CREATED)
		return;
	BaseObject::notifyJoints(aStatus);
}


void Balloon::reportNormalImpulseLength(qreal anImpulseLength)
{
	// also pop the balloon if it is maltreated
	// WARNING: Magic number here!!!
	if (anImpulseLength > 2.2 && theState==BALLOON)
		goToState(POPPING);
}


void Balloon::createPhysicsObject(void)
{
	theState = BALLOON;
	thePoppingTimeStart = 0;
	clearShapeList();
	fillShapeList();
	PolyObject::createPhysicsObject();
	theWorldPtr->registerCallback(this);
}

void Balloon::stung(void)
{
	if (theState == BALLOON)
		goToState(POPPING);
}

void Balloon::switchToSmallShape(void)
{
	// save the current position - as it is only stored within the B2Body
	Position myCurrentPos = getTempCenter();

	deletePhysicsObject();
	clearShapeList();

	b2PolygonShape* myRestShape = new b2PolygonShape();
	myRestShape->SetAsBox(0.05, 0.05);
	b2FixtureDef* myRestDef = new b2FixtureDef();
	myRestDef->density= 0.001 / (0.1 * 0.1);
	myRestDef->userData = this;
	myRestDef->shape   = myRestShape;
	theShapeList.push_back(myRestDef);

	PolyObject::createPhysicsObject(myCurrentPos);
}


///---------------------------------------------------------------------------
///------------------------ Cactus -------------------------------------------
///---------------------------------------------------------------------------


//// this class' ObjectFactory
class CactusObjectFactory : public ObjectFactory
{
public:
	CactusObjectFactory(void)
	{	announceObjectType("Cactus", this); }
	virtual BaseObject* createObject(void) const
	{	return fixObject(new Cactus()); }
};
static CactusObjectFactory theCactusObjectFactory;


Cactus::Cactus() : PolyObject(QObject::tr("Cactus"),
							  QObject::tr("Cactacea Bulbuous Stingus - a Cactus has spines!"),
							  "Cactus",
							  // first the pot:
							  "(-0.11,-0.038)=(-0.053,-0.2)=(0.053,-0.2)=(0.11,-0.038);"
							  // then the plant itself:
							  "(0.045,-0.036)=(0.085,0.012)=(0.092,0.089)=(0.046,0.156)=(0.00,0.171)"
							  "=(-0.046,0.156)=(-0.092,0.089)=(-0.085,0.012)=(-0.045,-0.036)"
							  ,
							  0.25, 0.4, 1.0, 0.3)
{
}

Cactus::~Cactus()
{
}

void Cactus::callBackSensor(const ContactInfo& aPoint)
{
	BaseObject* myOtherObject=NULL;

	// which one of the two shapes is not me?
	if (aPoint.myFixtureA->GetUserData()==this)
		myOtherObject = reinterpret_cast<BaseObject*>(aPoint.myFixtureB->GetUserData());
	if (aPoint.myFixtureB->GetUserData()==this)
		myOtherObject = reinterpret_cast<BaseObject*>(aPoint.myFixtureA->GetUserData());
	if (myOtherObject==NULL)
		return;

	// is it a Balloon?
	// then pop it!
	Balloon* myBalloonPtr = dynamic_cast<Balloon*>(myOtherObject);
	if (myBalloonPtr!=NULL)
		myBalloonPtr->stung();
}

void Cactus::fillShapeList(void)
{
	PolyObject::fillShapeList();

	// And add the sensor to the shapes
	b2PolygonShape* mySensorShape = new b2PolygonShape();
	mySensorShape->m_vertexCount = 10;
	mySensorShape->m_vertices[0]=b2Vec2( 0.078, -0.036);
	mySensorShape->m_vertices[1]=b2Vec2( 0.116,  0.012);
	mySensorShape->m_vertices[2]=b2Vec2( 0.125,  0.085);
	mySensorShape->m_vertices[3]=b2Vec2( 0.099,  0.147);
	mySensorShape->m_vertices[4]=b2Vec2( 0.038,  0.198);
	mySensorShape->m_vertices[5]=b2Vec2(-0.038,  0.198);
	mySensorShape->m_vertices[6]=b2Vec2(-0.099,  0.147);
	mySensorShape->m_vertices[7]=b2Vec2(-0.125,  0.085);
	mySensorShape->m_vertices[8]=b2Vec2(-0.116,  0.012);
	mySensorShape->m_vertices[9]=b2Vec2(-0.078, -0.036);
	b2FixtureDef* mySensorDef = new b2FixtureDef();
	mySensorDef->shape    = mySensorShape;
	mySensorDef->isSensor = true;
	mySensorDef->userData = this;
	theShapeList.push_back(mySensorDef);
}


///---------------------------------------------------------------------------
///------------------------ Bed Of Nails -------------------------------------
///---------------------------------------------------------------------------


// this class' ObjectFactory
class BedOfNailsObjectFactory : public ObjectFactory
{
public:
	BedOfNailsObjectFactory(void)
	{	announceObjectType("BedOfNails", this); }
	virtual BaseObject* createObject(void) const
	{	return fixObject(new BedOfNails()); }
};
static BedOfNailsObjectFactory theBedOfNailsObjectFactory;


BedOfNails::BedOfNails()
		: PolyObject(QObject::tr("BedOfNails"),
					 QObject::tr("Do not touch a bed of nails - it stings!"),
					 "BedOfNails",
					 // first the bar:
					 "(-0.4,-0.075)=(0.4,-0.075)=(0.4,0.006)=(-0.4,0.006);"
					 // then the nails block:
					 "(-0.34,-0.05)=(0.34,-0.05)=(0.34,0.06)=(-0.34,0.06)"
					 ,
					 0.8, 0.15, 0.0, 0.2)
{
}

BedOfNails::~BedOfNails()
{
}

void BedOfNails::callBackSensor(const ContactInfo& aPoint)
{
	BaseObject* myOtherObject=NULL;

	// which one of the two shapes is not me?
	if (aPoint.myFixtureA->GetUserData()==this)
		myOtherObject = reinterpret_cast<BaseObject*>(aPoint.myFixtureB->GetUserData());
	if (aPoint.myFixtureB->GetUserData()==this)
		myOtherObject = reinterpret_cast<BaseObject*>(aPoint.myFixtureA->GetUserData());
	if (myOtherObject==NULL)
		return;

	// is it a Balloon?
	// then pop it!
	Balloon* myBalloonPtr = dynamic_cast<Balloon*>(myOtherObject);
	if (myBalloonPtr!=NULL)
		myBalloonPtr->stung();
}

void BedOfNails::fillShapeList(void)
{
	PolyObject::fillShapeList();

	// And add the sensor to the shapes
	b2PolygonShape* mySensorShape = new b2PolygonShape();
	mySensorShape->m_vertexCount = 4;
	mySensorShape->m_vertices[0]=b2Vec2(-0.335, -0.04);
	mySensorShape->m_vertices[1]=b2Vec2( 0.335, -0.04);
	mySensorShape->m_vertices[2]=b2Vec2( 0.335,  0.07);
	mySensorShape->m_vertices[3]=b2Vec2(-0.335,  0.07);
	b2FixtureDef* mySensorDef = new b2FixtureDef();
	mySensorDef->shape    = mySensorShape;
	mySensorDef->isSensor = true;
	mySensorDef->userData = this;
	theShapeList.push_back(mySensorDef);
}

