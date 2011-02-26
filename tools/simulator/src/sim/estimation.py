# Copyright (C) 2010 Maryland Robotics Club
# Copyright (C) 2010 Jonathan Sternberg <jsternbe@umd.edu>
# All rights reserved.
#
# Author: Jonathan Sternberg <jsternbe@umd.edu>
# File:  tools/simulator/src/sim/estimation.py

import threading as threading

# Project Imports
import ext.core as core
import ext.math as math
import ext.vehicle as vehicle
import ext.estimation as estimation

def convertToVector3(vType, vector):
    return vType(vector.x, vector.y, vector.z)

def convertToQuaternion(qType, quat):
    return qType(quat.x, quat.y, quat.z, quat.w)

class IdealStateEstimator(estimation.IStateEstimator):
    """
    Implementation of the state estimator. This implementation cheats
    and uses the simulation device to figure out the status of the robot.

    This estimator cannot function outside of the simulator.
    """
    def __init__(self, cfg, deps):
        estimation.IStateEstimator.__init__(
            self, cfg.get('name', 'StateEstimator'),
            core.Subsystem.getSubsystemOfType(core.EventHub, deps))

        self._vehicle = core.Subsystem.getSubsystemOfType(
            vehicle.IVehicle, deps)

        # Must have simulation device to use this estimator
        if self._vehicle is not None and \
                'SimulationDevice' in self._vehicle.getDeviceNames():
            simDevice = self._vehicle.getDevice('SimulationDevice')
            self.robot = simDevice.robot
        else:
            # Print out a warning and continue
            print 'Warning! No simulation device found.'
            self.robot = None

        self._obstacles = {}

        # For the position
        self.initialPos = math.Vector2(self.robot._main_part._node.position.x,
                                       -self.robot._main_part._node.position.y)
        self.oldPos = self.initialPos
        self.velocity = math.Vector2(0, 0)

    def getEstimatedPosition(self):
        """
        Gets the robot's relative position in the world. Position is
        relative to the starting location of the robot.
        """
        if self.robot is not None:
            return math.Vector2(self.robot._main_part._node.position.x,
                                -self.robot._main_part._node.position.y)
        else:
            return math.Vector2.ZERO

    def getEstimatedVelocity(self):
        """
        Gets the robot's velocity.
        """
        return self.velocity

    def getEstimatedLinearAcceleration(self):
        if self.robot is not None:
            baseAccel = convertToVector3(math.Vector3,
                                         self.robot._main_part.acceleration)
            
            # Add in gravity
            return baseAccel + math.Vector3(0, 0, -9.8)
        else:
            return math.Vector3.ZERO

    def getEstimatedAngularRate(self):
        if self.robot is not None:
            return convertToVector3(math.Vector3,
                                    self.robot._main_part.angular_accel)
        else:
            return math.Vector3.ZERO

    def getEstimatedOrientation(self):
        if self.robot is not None:
            return convertToQuaternion(math.Quaternion,
                                       self.robot._main_part._node.orientation)
        else:
            return math.Quaternion.IDENTITY

    def getEstimatedDepth(self):
        if self.robot is not None:
            # Down is positive for depth
            return -3.281 * self.robot._main_part._node.position.z
        else:
            return 0

    def getEstimatedDepthRate(self):
        return 0

    def getEstimatedMass(self):
        return 0

    def getEstimatedThrusterForces(self):
        print 'IdealStateEstimator: NOT_YET_IMPLEMENTED'
        return math.Vector3.ZERO

    def getEstimatedThrusterTorques(self):
        print 'IdealStateEstimator: NOT_YET_IMPLEMENTED'
        return math.Vector3.ZERO

    def addObstacle(self, name, obstacle):
        self._obstacles[name] = obstacle

    def getObstaclePosition(self, name):
        return self._obstacles[name].getPosition()

    def getObstacleDepth(self, name):
        return self._obstacles[name].getDepth()

    def background(self, interval):
        pass

    def unbackground(self, join):
        pass

    def backgrounded(self):
        return False

    def update(self, time):
        """
        Sends out events using the current estimated values
        """
        currentPos = self.getEstimatedPosition()
        self.velocity = (currentPos - self.oldPos) / time
        self.oldPos = currentPos

        # Package all events
        devent = math.NumericEvent()
        devent.number = self.getEstimatedDepth()

        pevent = math.Vector2Event()
        pevent.vector2 = self.getEstimatedPosition()

        vevent = math.Vector2Event()
        vevent.vector2 = self.getEstimatedVelocity()

        oevent = math.OrientationEvent()
        oevent.orientation = self.getEstimatedOrientation()

        # Send all events at the same time
        self.publish(estimation.IStateEstimator.ESTIMATED_DEPTH_UPDATE, devent)
        self.publish(estimation.IStateEstimator.ESTIMATED_POSITION_UPDATE,
                     pevent)
        self.publish(estimation.IStateEstimator.ESTIMATED_VELOCITY_UPDATE,
                     vevent)
        self.publish(estimation.IStateEstimator.ESTIMATED_ORIENTATION_UPDATE,
                     oevent)

core.registerSubsystem('IdealStateEstimator', IdealStateEstimator)