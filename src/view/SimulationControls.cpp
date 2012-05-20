/* The Butterfly Effect
 * This file copyright (C) 2011,2012 Klaas van Gend
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

#include "tbe_global.h"
#include "SimulationControls.h"
#include "ui_SimulationControls.h"
#include "ImageCache.h"

#include <cstdio>

void SimState::onEntry ( QEvent * event )
{
	DEBUG4("SimulationControls-SimState %s onEntry!\n", ASCII(theName));
	QState::onEntry(event);
        isActive = true;
}

void SimState::onExit ( QEvent * event )
{
        QState::onExit(event);
        isActive = false;
}


SimulationControls::SimulationControls(QWidget *parent) :
    QLabel(parent),
    ui(new Ui::SimulationControls)
{
    ui->setupUi(this);

    QPixmap myPixmap;
    ImageCache::getPixmap("SimControlsBackground", size(), &myPixmap);
    this->setPixmap(myPixmap);

    const QSize myIconSize(36,36);
    theForwardIcon= ImageCache::getQIcon("ActionFastForward", myIconSize);
    thePauseIcon  = ImageCache::getQIcon("ActionMenuPause", myIconSize);
    thePlayIcon   = ImageCache::getQIcon("ActionMenuPlay", myIconSize);
    theResetIcon  = ImageCache::getQIcon("ActionUndo", myIconSize);

    ImageCache::getPixmap("StatusFail",   QSize(64,64), &theFailedStatusPixmap);
    ImageCache::getPixmap("StatusFF",     QSize(64,64), &theForwardStatusPixmap);
    ImageCache::getPixmap("StatusPlay",   QSize(64,64), &theRunningStatusPixmap);
    ImageCache::getPixmap("StatusPause",  QSize(64,64), &thePausedStatusPixmap);
    ImageCache::getPixmap("StatusProblem",QSize(64,64), &theProblemStatusPixmap);
    ImageCache::getPixmap("StatusStop",   QSize(64,64), &theStoppedStatusPixmap);
    ui->statusLabel->setPixmap(theStoppedStatusPixmap);
}


SimulationControls::~SimulationControls()
{
    delete ui;
}


void SimulationControls::hideYourself()
{
    // todo: remove entirely
    emit hide();
}


void SimulationControls::hookSignalsUp(ViewWorld* aViewWorld)
{
    DEBUG1ENTRY;

    emit internalReset();

    // hook up states to signals for ViewWorld/World
    // note: no need to hook up theProblemState
    connect(theFailedState, SIGNAL(entered()), aViewWorld,
            SLOT(slot_signalPause()));
    connect(theForwardState, SIGNAL(entered()), aViewWorld,
            SLOT(slot_signalFF()));
    connect(thePausedState, SIGNAL(entered()), aViewWorld,
            SLOT(slot_signalPause()));
    connect(theRunningState, SIGNAL(entered()), aViewWorld,
            SLOT(slot_signalPlay()));
    connect(theStoppedState, SIGNAL(entered()), aViewWorld,
            SLOT(slot_signalReset()));

    connect(this, SIGNAL(go_quadspeed()), aViewWorld,
            SLOT(slot_signal4F()));
}


void SimulationControls::parentResize(const QSize& aSize)
{
    // TODO/FIXME: magic numbers here
    // I bet these have to be different for Windows and MacOSX :-(
    move(aSize.width()-size().width()-2,-8);
}

void SimulationControls::setup(QMenu* aMenuPtr)
{
	theFailedState  = new SimState(&theSimStateMachine, "Failed");
	theForwardState = new SimState(&theSimStateMachine, "Forward");
        theHiddenState  = new SimState(&theSimStateMachine, "Hidden");
        thePausedState  = new SimState(&theSimStateMachine, "Paused");
        theProblemState = new SimState(&theSimStateMachine, "Problem");
        theRunningState = new SimState(&theSimStateMachine, "Running");
	theStoppedState = new SimState(&theSimStateMachine, "Stopped");
	theSimStateMachine.setInitialState(theStoppedState);

        // add actions and their quick keys
        theForwardAction = new QAction(theForwardIcon, tr("&Forward"), NULL);
        //: translators: 'f' is for (fast) forward
        theForwardAction->setShortcut(QKeySequence(tr("f")));

        // add actions and their quick keys
        thePauseAction = new QAction(thePauseIcon, tr("&Pause"), NULL);
        //: translators: TODO: I want to use Space here, too but for now 'p' is for pause
        thePauseAction->setShortcut(QKeySequence(tr("p")));

        // add actions and their quick keys
        thePlayAction = new QAction(thePlayIcon, tr("Play"), NULL);
	//: translators: space is for start/pause the sim - no need to translate
        thePlayAction->setShortcut(QKeySequence(tr("Space")));

        // add actions and their quick keys
        theResetAction = new QAction(theResetIcon, tr("&Reset"), NULL);
        //: translators: 'r' is for reset
        theResetAction->setShortcut(QKeySequence(tr("r")));

//	QKeySequence myFwdKey(tr("f"));
//	QKeySequence myResetKey(tr("r"));

        //: translators: really-fast-forward is only available as a key shortcut
        //: it should be shift-<normal fast-forward>...
        QKeySequence my4FwdKey(tr("Shift+f"));
        the4FAction = new QAction(NULL);
        the4FAction->setShortcut(my4FwdKey);
        //Qt::ApplicationShortcu
        this->addAction(the4FAction);
        connect(the4FAction, SIGNAL(triggered()), this, SLOT(slot_4SpeedForward()));

        aMenuPtr->addAction(thePauseAction);
        aMenuPtr->addAction(thePlayAction);
        aMenuPtr->addAction(theResetAction);

        ui->buttonForward->setDefaultAction(theForwardAction);
        ui->buttonPause->setDefaultAction(thePauseAction);
        ui->buttonPlay->setDefaultAction(thePlayAction);
        ui->buttonReset->setDefaultAction(theResetAction);

	QLabel* myLabelPtr = ui->statusLabel;

	// add transitions here
        theStoppedState->addTransition(thePlayAction, SIGNAL(triggered()), theRunningState);
//	theStoppedState->addTransition(this, SIGNAL(internalReset()), theStoppedState);
	theStoppedState->addTransition(this, SIGNAL(internalCrossPresent()), theProblemState);
        theStoppedState->addTransition(this, SIGNAL(hide()), theHiddenState);

        theRunningState->addTransition(thePauseAction, SIGNAL(triggered()), thePausedState);
        theRunningState->addTransition(theForwardAction, SIGNAL(triggered()), theForwardState);
	theRunningState->addTransition(this, SIGNAL(internalFailed()), theFailedState);
//	theRunningState->addTransition(this, SIGNAL(internalReset()), theStoppedState);
        theRunningState->addTransition(this, SIGNAL(hide()), theHiddenState);

        theProblemState->addTransition(this, SIGNAL(internalCrossGone()), theStoppedState);
        theProblemState->addTransition(this, SIGNAL(hide()), theHiddenState);

        thePausedState ->addTransition(thePlayAction, SIGNAL(triggered()), theRunningState);
        thePausedState ->addTransition(theResetAction, SIGNAL(triggered()), theStoppedState);
//	thePausedState->addTransition(this, SIGNAL(internalReset()), theStoppedState);
        thePausedState->addTransition(this, SIGNAL(hide()), theHiddenState);

        theHiddenState ->addTransition(this, SIGNAL(show()), theStoppedState);

        theFailedState ->addTransition(theResetAction, SIGNAL(triggered()), theStoppedState);
//	theFailedState->addTransition(this, SIGNAL(internalReset()), theStoppedState);
        theFailedState->addTransition(this, SIGNAL(hide()), theHiddenState);

        theForwardState->addTransition(thePauseAction, SIGNAL(triggered()), thePausedState);
        theForwardState->addTransition(thePlayAction, SIGNAL(triggered()), theRunningState);
        theForwardState->addTransition(this, SIGNAL(internalFailed()), theFailedState);
//	theForwardState->addTransition(this, SIGNAL(internalReset()), theStoppedState);
        theForwardState->addTransition(this, SIGNAL(hide()), theHiddenState);


	// set the start conditions for the icons for each state
        theStoppedState->assignProperty(theForwardAction,"enabled", false);
        theStoppedState->assignProperty(thePauseAction,  "enabled", false);
        theStoppedState->assignProperty(thePlayAction,   "enabled", true);
        theStoppedState->assignProperty(theResetAction,  "enabled", false);
        theStoppedState->assignProperty(myLabelPtr,      "pixmap",  theStoppedStatusPixmap);
        // upon entering running state, Top = Pause/enabled, Bottom = FF/enabled
        theRunningState->assignProperty(theForwardAction,"enabled", true);
        theRunningState->assignProperty(thePauseAction,  "enabled", true);
        theRunningState->assignProperty(thePlayAction,   "enabled", false);
        theRunningState->assignProperty(theResetAction,  "enabled", false);
        theRunningState->assignProperty(myLabelPtr,      "pixmap",  theRunningStatusPixmap);
        // upon entering problem state, Top = Play/disabled, Bottom = FF/disabled
        theProblemState->assignProperty(theForwardAction,"enabled", false);
        theProblemState->assignProperty(thePauseAction,  "enabled", false);
        theProblemState->assignProperty(thePlayAction,   "enabled", false);
        theProblemState->assignProperty(theResetAction,  "enabled", false);
        theProblemState->assignProperty(myLabelPtr,      "pixmap",  theProblemStatusPixmap);
        // upon entering paused  state, Top = Play/enabled, Bottom = Reset/enabled
        thePausedState->assignProperty(theForwardAction,"enabled", false);
        thePausedState->assignProperty(thePauseAction,  "enabled", false);
        thePausedState->assignProperty(thePlayAction,   "enabled", true);
        thePausedState->assignProperty(theResetAction,  "enabled", true);
        thePausedState->assignProperty(myLabelPtr,      "pixmap",  thePausedStatusPixmap);
	// upon entering forward state, Top = Pause/enabled, Bottom = Play/enabled
        theForwardState->assignProperty(theForwardAction,"enabled", false);
        theForwardState->assignProperty(thePauseAction,  "enabled", true);
        theForwardState->assignProperty(thePlayAction,   "enabled", true);
        theForwardState->assignProperty(theResetAction,  "enabled", false);
        theForwardState->assignProperty(myLabelPtr,      "pixmap",  theForwardStatusPixmap);
	// upon entering failed  state, Top = Play/disabled, Bottom = Reset/enabled
        theFailedState->assignProperty(theForwardAction,"enabled", false);
        theFailedState->assignProperty(thePauseAction,  "enabled", false);
        theFailedState->assignProperty(thePlayAction,   "enabled", false);
        theFailedState->assignProperty(theResetAction,  "enabled", true);
        theFailedState->assignProperty(myLabelPtr,      "pixmap",  theFailedStatusPixmap);

	emit theSimStateMachine.start();
}

void SimulationControls::showYourself()
{
	emit show();
}


void SimulationControls::slotNumberOfCrossesChanged(int aNewNumber)
{
	if (aNewNumber==0)
		emit internalCrossGone();
	else
		emit internalCrossPresent();
}

void SimulationControls::slot_4SpeedForward(void)
{
    // if in play mode, fake going to fast forward
    // but secretly emit a second signal to go a lot faster
    if (theRunningState->isActive==true)
    {
        emit theForwardAction->trigger();
        emit go_quadspeed();
    }
}
