/*
 * Copyright (C) 2007 Robotics at Maryland
 * Copyright (C) 2007 Joseph Lisee <jlisee@umd.edu>
 * All rights reserved.
 *
 * Author: Joseph Lisee <jlisee@umd.edu>
 * File:  packages/vision/include/Vehicle.h
 */

#ifndef RAM_VEHICLE_VEHICLE_06_11_2007
#define RAM_VEHICLE_VEHICLE_06_11_2007

// STD Includes
#include <string>

// Library Includes
#include "boost/tuple/tuple.hpp"

// Project Includes
#include "core/include/ConfigNode.h"
#include "core/include/EventPublisher.h"
#include "core/include/ReadWriteMutex.h"
#include "core/include/Updatable.h"

#include "vehicle/include/Common.h"
#include "vehicle/include/IVehicle.h"
#include "vehicle/include/estimator/IStateEstimator.h"

#include "math/include/MatrixN.h"
#include "math/include/VectorN.h"
#include "math/include/Vector3.h"

namespace ram {
namespace vehicle {

typedef boost::tuple<math::Vector3, math::Vector3, math::Vector3,
                     math::Vector3, math::Vector3, math::Vector3> Tuple6Vector3;

class Vehicle : public IVehicle, public core::Updatable
{
public:
    Vehicle(core::ConfigNode config, 
            core::SubsystemList deps = core::SubsystemList());
        
    virtual ~Vehicle();
    
    /** Gets the device associated with the given name  <b>NOT THREAD SAFE</b>*/
    virtual device::IDevicePtr getDevice(std::string name);

    virtual std::vector<std::string> getDeviceNames();
    
    virtual double getDepth(std::string obj = "vehicle");

    virtual math::Vector2 getPosition(std::string obj = "vehicle");
    
    virtual math::Vector2 getVelocity(std::string obj = "vehicle");

    math::Vector3 getLinearAcceleration();

    math::Vector3 getAngularRate();
    
    math::Quaternion getOrientation(std::string obj = "vehicle");

    bool hasObject(std::string obj);

    bool hasMagBoom();

    virtual void safeThrusters();

    virtual void unsafeThrusters(); 

    virtual void dropMarker();

    virtual void fireTorpedo();

    virtual void releaseGrabber();
    
    virtual void applyForcesAndTorques(const math::Vector3& force,
                                       const math::Vector3& torque);

    /** This is <b>NOT</b> thread safe */
    /** return 0 indicates success, everything else indicates failure */
    virtual int _addDevice(device::IDevicePtr device);

    /** Sets the priority to all background devices threads */
    virtual void setPriority(core::IUpdatable::Priority priority);

    virtual core::IUpdatable::Priority getPriority() {
        return Updatable::getPriority();
    }

    /** Sets the affinity for all background devices threads */
    virtual void setAffinity(size_t affinity);

    virtual int getAffinity() {
        return Updatable::getAffinity();
    }
         
    /** Backgrounds all devices with the given update interval */
    virtual void background(int interval);

    /** Unbackgrounds all devices */
    virtual void unbackground(bool join = false);
    
    virtual bool backgrounded() {
        return Updatable::backgrounded();
    };

    /** Currently just manually grabs depth */
    virtual void update(double timestep);
    

    /** Get the depth directly from the depth sensor */
    virtual double getRawDepth();

    /** Get the position directly from the position sensor */
    virtual math::Vector2 getRawPosition();

    /** Get the velocity directly from the velocity sensor */
    virtual math::Vector2 getRawVelocity();

    /** Get the orientation directly from the orientation sensor */
    virtual math::Quaternion getRawOrientation();

    /** Get the state estimator */
    virtual estimator::IStateEstimatorPtr getStateEstimator();

    /** compute vector of forces to apply to thrusters at given offsets
       so that there is no net torque.  This assumes that the thrusters
       are applying a torque in opposite directions*/

    math::MatrixN createControlSignalToThrusterForcesMatrix(
        Tuple6Vector3 thrusterLocations);

protected:    
    /** Returns true if all IThrusterPtrs now contain valid thrusters */
    bool lookupThrusterDevices();

    /** Called when the depth sensor has an update, publishes vehicle update */
    void onDepthUpdate(core::EventPtr event);

    /** Called when the IMU has an update, publishes vehicle update */
    void onOrientationUpdate(core::EventPtr event);

    /** Called when the position sensor has an update, publises vehicle up. */
    void onPositionUpdate(core::EventPtr event);

    /** Called when the velocity sensor has an update, publises vehicle up. */
    void onVelocityUpdate(core::EventPtr event);
    
private:
    core::ConfigNode m_config;
    
    NameDeviceMap m_devices;

    std::string m_starboardThrusterName;
    vehicle::device::IThrusterPtr m_starboardThruster;
    std::string m_portThrusterName;
    vehicle::device::IThrusterPtr m_portThruster;
    std::string m_foreThrusterName;
    vehicle::device::IThrusterPtr m_foreThruster;
    std::string m_aftThrusterName;
    vehicle::device::IThrusterPtr m_aftThruster;
    std::string m_topThrusterName;
    vehicle::device::IThrusterPtr m_topThruster;
    std::string m_bottomThrusterName;
    vehicle::device::IThrusterPtr m_bottomThruster;

    double m_topThrusterThrottle;

    std::string m_stateEstimatorName;
    vehicle::device::IStateEstimatorPtr m_stateEstimator;
    
    std::string m_imuName;
    vehicle::device::IIMUPtr m_imu;
    
    bool m_hasMagBoom;
    std::string m_magBoomName;
    vehicle::device::IIMUPtr m_magBoom;

    core::EventConnectionPtr m_orientationConnection;
    
    std::string m_depthSensorName;
    vehicle::device::IDepthSensorPtr m_depthSensor;
    core::EventConnectionPtr m_depthConnection;
    
    std::string m_velocitySensorName;
    vehicle::device::IVelocitySensorPtr m_velocitySensor;
    core::EventConnectionPtr m_velocityConnection;
    
    std::string m_positionSensorName;
    vehicle::device::IPositionSensorPtr m_positionSensor;
    core::EventConnectionPtr m_positionConnection;
    
    std::string m_markerDropperName;
    vehicle::device::IPayloadSetPtr m_markerDropper;

    std::string m_torpedoLauncherName;
    vehicle::device::IPayloadSetPtr m_torpedoLauncher;

    std::string m_grabberName;
    vehicle::device::IPayloadSetPtr m_grabber;

    estimator::IStateEstimatorPtr stateEstimator;

    math::MatrixN m_controlSignalToThrusterForces;
    bool m_controlSignalToThrusterForcesCreated;
    
    enum thrusters {STAR = 0, PORT, BOT, TOP, FORE, AFT};
};
    
} // namespace vehicle
} // namespace ram
    
#endif // RAM_VEHICLE_VEHICLE_06_11_2007
