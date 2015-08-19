/* The Butterfly Effect
 * This file copyright (C) 2010,2013 Klaas van Gend
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

#include "tbe_global.h"
#include "LocalString.h"

#include <QtCore/QLocale>

// ###################################################

LocalString::LocalString(void)
{
	QLocale mySysLocale = QLocale::system();
	the5Char = mySysLocale.name();
}

void LocalString::add(const QString& aValue, const QString& aLangCode)
{
		theStringList.insert(aLangCode, aValue);
}

QString LocalString::all() const
{
    QString myResult = "{";
    LocalStringList::const_iterator i = theStringList.begin();
    while (i != theStringList.end())
    {
        myResult += QString("%1='%2'; ").arg(i.key()).arg(i.value());
        i++;
    }
    myResult += "}; ";
    return myResult;
}


void LocalString::clear()
{
	theStringList.clear();
}

QString LocalString::english() const
{
	if (theStringList.count()==0)
		return "";
	else
		return theStringList.find("").value();
}

void
LocalString::fillFromDOM(
		const QDomNode& aNode,
		const QString& aTagString,
		const QString& aDefault)
{
	theStringList.insert("", aDefault);
	QDomElement myE = aNode.firstChildElement(aTagString);
	while (myE.isNull()==false)
	{
		theStringList.insert(myE.attribute("lang",""), myE.text());
		myE = myE.nextSiblingElement(aTagString);
	}
}

void LocalString::serializeTo(QDomElement& aParentElement) const
{
    QDomDocument myDomDocument = aParentElement.ownerDocument();
    for(auto i : theStringList.toStdMap())
    {
        if (i.first.isEmpty())
            continue;
        QDomElement myStringNode = myDomDocument.createElement("name");
        QDomText myStringValue = myDomDocument.createTextNode(i.second);
        myStringNode.setAttribute("lang", i.first);
        myStringNode.appendChild(myStringValue);
        aParentElement.appendChild(myStringNode);
    }
}


QString LocalString::result() const
{
	QString myReturn;
	QString myLang;

	LocalStringList::const_iterator i = theStringList.constBegin();
	while (i != theStringList.constEnd())
	{
		QString aLangCode = i.key();
		QString aValue = i.value();

		// we follow these rules:
		// 1) if nothing set and we find a <title> without a language - let's
		//    use it
		// 2) if we find a title with the corresponding language - let's use
		//    it instead of rule 1
		// 3) if we find a title with a full corresponding "language_country",
		//    let's use that one instead of 1 or 2

		// rule 1
		if (aLangCode.isEmpty() && myLang.isEmpty())
			myReturn = aValue;

		// rule 2
		if (myLang.size()<3 && aLangCode.left(2) == the5Char.left(2))
		{
			// but not if we already have "nl", are looking for "nl_NL" and we now get "nl_BE"
			if ( !(aLangCode.size()==5 && myLang.size()==2) )
			{
				myReturn = aValue;
				myLang = aLangCode.left(2);
			}
		}

		// rule 3
		if (myLang.size()<3 && aLangCode == the5Char)
		{
			myReturn = aValue;
			myLang = aLangCode;
		}

		++i;
	}
	return myReturn;
}
