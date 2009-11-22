/* The Butterfly Effect
 * This file copyright (C) 2009  Klaas van Gend
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation
 * applicable version is GPL version 2 only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef GOAL_H
#define GOAL_H

#include <QMap>

// forward declarations
class World;
class BaseObject;


/** interface for all Goal classes
 */
class Goal
{
public:
    Goal();
	virtual ~Goal();

	/** Pure virtual member - check if this goal is met.
	  * Note that it is up to the class to decide if a goal is met forever or needs to be met
	  * (that makes a difference if you have more than one goal)
	  *
	  * @returns true if this goal has been met
	  */
	virtual bool checkForSuccess(void) = 0;

	/// set property aKey to aValue
	virtual void  setProperty(const QString& aKey, const QString& aValue)
	{ theProperties[aKey] = aValue; }

	/// @returns the value for property aKey
	/// - or an empty string if it does not exist
	QString getProperty(const QString& aKey)
	{ return theProperties.value(aKey, ""); }

	/// remove a property
	void removeProperty(const QString& aKey)
	{ theProperties.remove(aKey); }

	/** after all properties are set, call this function
	  * to have the goal actually try to understand the
	  * properties.
	  * @param aWorldPtr
	  */
	virtual bool parseProperties(World* aWorldPtr) = 0;

protected:
	typedef QMap<QString,QString> PropertyMap;
	PropertyMap theProperties;
};

/// implemented Goal - distance between two objects grows larger than or is less than a limit
class GoalDistance : public Goal
{
public:
	enum DistanceType
	{
		NOTYPE,
		LESSTHAN,
		MORETHAN,
		// future?: MORETHANRELATIVE
	};

	/** constructor
	  * NOTE: this class is only fully usable after parseProperties has been run
	  */
	GoalDistance();
	virtual ~GoalDistance();

	virtual bool parseProperties(World* aWorldPtr);

	bool checkForSuccess(void);

private:
	DistanceType theType;
	qreal theLimit;
	BaseObject* theFirstPtr;
	BaseObject* theSecondPtr;
};

/// implemented Goal - win if Position of an Object changes
//class GoalPositionChange : public Goal
//{
//public:
//	enum PositionType
//	{
//		NOTYPE,
//		XCHANGED,
//		YCHANGED,
//		ACHANGED
//	};
//
//	/** constructor
//	  * NOTE: this class is only fully usable after parseProperties has been run
//	  */
//	GoalPositionChange();
//	virtual ~GoalPositionChange();
//
//	virtual bool parseProperties(World* aWorldPtr);
//
//	bool checkForSuccess(void);
//
//private:
//	PositionType theType;
//	BaseObject* theBOPtr;
//};

// future goal classes will include isHit, and maybe things like isAlive.

#endif // GOAL_H
