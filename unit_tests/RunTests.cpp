//
//  RunTests.cpp
//  KafkaPlugin
//
//  Created by Jonas Nilsson on 2017-01-04.
//  Copyright © 2017 European Spallation Source. All rights reserved.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>


int main(int argc, char **argv) {
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

