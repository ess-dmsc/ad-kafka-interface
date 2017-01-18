/*
 * ADKafkaInterface Jenkinsfile
 */

node('eee') {

    stage("Checkout projects") {
        checkout scm
    }

    dir("build") {
        stage("Run CMake for unit tests") {
            sh "cmake ../code"
        }

        stage("Build unit tests") {
            sh "make"
        }

        stage("Run unit tests") {
            sh "./unit_tests/AllUnitTests --gtest_output=xml:AllResultsUnitTests.xml"
            junit '*Tests.xml'
        }
 
    }
}