/*
 * ADKafkaInterface Jenkinsfile
 */

node('eee') {
    try {
        stage("Checkout projects") {
            checkout scm
        } 
    } catch (e) {
        slackSend color: 'danger', message: '@jonasn: PICS Kafka interface checkout failed'
        throw e
    }

    dir("build") {
        try {
            stage("Run CMake for unit tests") {
                sh "cmake ../code"
            } 
        } catch (e) {
            slackSend color: 'danger', message: '@jonasn: EPICS Kafka interface CMake failed'
            throw e
        }
        
        try {
            stage("Build unit tests") {
                sh "make"
            }
            } catch (e) {
                slackSend color: 'danger', message: '@jonasn: EPICS Kafka interface build failed'
                throw e
            }
        
            try {
                stage("Run unit tests") {
                    sh "./unit_tests/unit_tests --gtest_output=xml:AllResultsUnitTests.xml"
                    junit '*Tests.xml'
                }
                } catch (e) {
                    slackSend color: 'danger', message: '@jonasn: EPICS Kafka interface unit tests failed'
                    throw e
                }
 
    }
}