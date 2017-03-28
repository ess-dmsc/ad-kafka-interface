/*
 * ADKafkaInterface Jenkinsfile
 */

node('eee') {
    dir("code") {
        try {
            stage("Checkout projects") {
                checkout scm
            } 
        } catch (e) {
            slackSend color: 'danger', message: '@jonasn ad-kafka-interface: Checkout failed'
            throw e
        }
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
            dir("unit_tests"){
                stage("Run unit tests") {
                    sh "./unit_tests --gtest_output=xml:AllResultsUnitTests.xml"
                    junit '*Tests.xml'
                }
            }
        } catch (e) {
            slackSend color: 'danger', message: '@jonasn ad-kafka-interface: Unit tests failed'
            throw e
        }
 
    }
    
    dir("code/m-epics-ADKafka") {
        try {
            stage("Compile ADKafka") {
                sh "make -f EEEmakefile LIBRDKAFKA_LIB_PATH=/opt/dm_group/usr/lib LIBRDKAFKA_INC_PATH=/opt/dm_group/usr/include"
            } 
        } catch (e) {
            slackSend color: 'danger', message: '@jonasn ad-kafka-interface: ADKafka build failed'
            throw e
        }
    }
    
    dir("code/m-epics-ADPluginKafka") {
        try {
            stage("Compile ADPluginKafka") {
                sh "make -f EEEmakefile LIBRDKAFKA_LIB_PATH=/opt/dm_group/usr/lib LIBRDKAFKA_INC_PATH=/opt/dm_group/usr/include"
/*                sh "python /opt/epics/modules/environment/2.0.0/3.15.4/bin/centos7-x86_64/module_manager.py --prefix=`pwd` --assumeyes --builddir='builddir' install 'ADPluginKafka' '1.0.0-INTTEST'"*/
            } 
        } catch (e) {
            slackSend color: 'danger', message: '@jonasn ad-kafka-interface: ADPluginKafka build failed'
            throw e
        }
    }
    dir("code") {
        stage("Archive ADPluginKafka") {
            sh "tar czf ADPluginKafka.tar.gz m-epics-ADPluginKafka"
            archiveArtifacts "ADPluginKafka.tar.gz"
            } 

    }
    if (currentBuild.previousBuild.result == "FAILURE") {
        slackSend color: 'good', message: 'ad-kafka-interface: Back in the green!'
    }
}
