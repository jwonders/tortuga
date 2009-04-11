/*
 * Copyright (C) 2009 Robotics at Maryland
 * Copyright (C) 2009 Joseph Lisee <jlisee@umd.edu>
 * All rights reserved.
 *
 * Author: Joseph Lisee <jlisee@umd.edu>
 * File:  packages/core/test/src/TestProperty.cxx
 */


// Library Includes
#include <UnitTest++/UnitTest++.h>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

// Project Includes
#include "core/include/PropertySet.h"

using namespace ram;

SUITE(PropertySet) {

struct Fixture {

    Fixture()
    {
        prop = core::PropertyPtr(
            new core::VariableProperty<int>("val", "A test property", 5,
                                            &value));
    }

    core::PropertySet propSet;
    core::PropertyPtr prop;
    int value;
};

    
TEST_FIXTURE(Fixture, addProperty)
{
    // Add the property
    propSet.addProperty(prop);
}

void setter(int* ref, int val)
{
    *ref = val;
}

int getter(int* ref)
{
    return *ref;
}


TEST_FIXTURE(Fixture, addPropertySpecial)
{
    // Add the property
    propSet.addProperty(prop);

    // Have it build a variable property for us
    int value2;
    propSet.addProperty("val2", "A test auto var", 5, &value2);

    // Have it build a function property for us
    int value3;
    propSet.addProperty<int>("val3", "A test auto func", 5,
                             boost::bind(getter, &value3),
                             boost::bind(setter, &value3, _1));
}


TEST_FIXTURE(Fixture, hetProperty)
{
    // Add the property
    propSet.addProperty(prop);

    // Get it back and make sure its the right one
    core::PropertyPtr prop2 = propSet.getProperty("val");
    CHECK_EQUAL(prop.get(), prop2.get());
}

TEST_FIXTURE(Fixture, hasProperty)
{
    // Make sure its not there
    CHECK_EQUAL(false, propSet.hasProperty("val"));
    
    // Add the property
    propSet.addProperty(prop);

    // Now make sure its there
    CHECK(propSet.hasProperty("val"));
}

TEST_FIXTURE(Fixture, getPropertyNames)
{
    core::PropertyPtr prop2(
        new core::VariableProperty<int>("val 2", "property", 6, &value));

    // Add the properties
    propSet.addProperty(prop);
    propSet.addProperty(prop2);

    std::vector<std::string> propNames = boost::assign::list_of("val")("val 2");
    CHECK_ARRAY_EQUAL(propNames, propSet.getPropertyNames(), propNames.size());
}




} // SUITE(PropertySet)