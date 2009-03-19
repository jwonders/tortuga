# Copyright (C) 2008 Maryland Robotics Club
# Copyright (C) 2008 Joseph Lisee <jlisee@umd.edu>
# All rights reserved.
#
# Author: Joseph Lisee <jlisee@umd.edu>
# File:  packages/python/ram/ai/state.py

# STD Imports

# Project Imports
import ext.core as core
import ram.ai.state as state

# Special event that denotes 
TIMEOUT = core.declareEventType('TIMEOUT')        

class Next(state.State):
    """
    Special state denotes that the next task should be moved to
    """
    pass

class End(state.State):
    """
    Special state that denotes the complete end of the state machine
    """
    pass

class Task(state.State):
    """
    Encapsulates a single AI task, like completing an objective.  It allows for
    the implementation and testing of such tasks without concern for what comes
    before, or after said task.
    """
    def __init__(self, config = None, **subsystems):
        # Call the super class
        state.State.__init__(config, **subsystems)
        
        # Dynamically create our event
        self._timeoutEvent = core.declareEventType(
            'TIMEOUT' + self.__class__.__name__)
        
        # Read in config information to allow 
        self._nextState = None
    
        
    def transitions():
        """
        A dynamic transition function which allows you to wire together a 
        missions dynamically.
        """
        baseTrans = self._transitions()
        newTrans = {}
        for eventType, nextState in baseTrans:
            # Catch the timeout event and replace with our class specific 
            # timeout event type
            if eventType == TIMEOUT:
                eventType = self._timeoutEvent
                
            # If the next state is the special Next marker state, swap it out
            # for the real next state
            if nextState == Next:
                nextState = self._nextState
            
            # Store the event
            newTrans[eventType] = nextState 