/*
 * ADKafkaInterface Jenkinsfile
 */

node('eee') {
    try {
        stage("Checkout projects") {
            checkout scm
        } 
    } catch (e) {
        slackSend color: 'danger', message: '@jonasn ad-kafka-interface: Checkout failed'
        throw e
    }

    dir("build") {
        try {
            stage("Run CMake for unit tests") {
                sh "cmake ../code"
            } 
        } catch (e) {
            slackSend color: 'danger', message: '@jonasn ad-kafka-interface: CMake failed'
            throw e
        }
        
        try {
            stage("Build unit tests") {
                sh "make"
            }
        } catch (e) {
            slackSend color: 'danger', message: '@jonasn ad-kafka-interface: Unit tests build failed'
            throw e
        }
        
        try {
            stage("Run unit tests") {
                sh "./unit_tests/unit_tests --gtest_output=xml:AllResultsUnitTests.xml"
                junit '*Tests.xml'
            }
        } catch (e) {
            slackSend color: 'danger', message: '@jonasn ad-kafka-interface: Unit tests failed'
            throw e
        }
 
    }
    
    dir("code/m-epics-ADKafka") {
        try {
            stage("Compile ADKafka") {
                sh "make -f EEEmakefile"
            } 
        } catch (e) {
            slackSend color: 'danger', message: '@jonasn ad-kafka-interface: ADKafka build failed'
            throw e
        }
    }
    
    dir("code/m-epics-ADPluginKafka") {
        try {
            stage("Compile ADPluginKafka") {
                sh "make -f EEEmakefile"
            } 
        } catch (e) {
            slackSend color: 'danger', message: '@jonasn ad-kafka-interface: ADPluginKafka build failed'
            throw e
        }
    }
}
