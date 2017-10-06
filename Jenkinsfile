/*
 * ADKafkaInterface Jenkinsfile
 */

def failure_function(exception_obj, failureMessage) {
    def toEmails = [[$class: 'DevelopersRecipientProvider']]
    emailext body: '${DEFAULT_CONTENT}\n\"' + failureMessage + '\"\n\nCheck console output at $BUILD_URL to view the results.', recipientProviders: toEmails, subject: '${DEFAULT_SUBJECT}'
    slackSend color: 'danger', message: "@jonasn ad-kafka-interface: " + failureMessage
    throw exception_obj
}

node('eee') {
    cleanWs()

    dir("code") {
        try {
            stage("Checkout projects") {
                checkout scm
            }
        } catch (e) {
            failure_function(e, 'Checkout failed')
        }
    }

    dir("build") {
        try {
            stage("Run CMake for unit tests") {
                sh "cmake ../code"
            }
        } catch (e) {
            failure_function(e, 'CMake failed')
        }

        try {
            stage("Build unit tests") {
                sh "make"
            }
        } catch (e) {
            failure_function(e, 'Unit tests build failed')
        }

        dir("unit_tests"){
            stage("Run unit tests") {
                try {
                    sh "./unit_tests --gtest_output=xml:AllResultsUnitTests.xml"
                } catch (e) {
                    junit '*Tests.xml'
                    failure_function(e, 'Unit tests failed')
                }
                junit '*Tests.xml'
            }
        }

    }

    dir("code/m-epics-ADKafka") {
        try {
            stage("Compile ADKafka") {
                sh "make -f EEEmakefile LIBRDKAFKA_LIB_PATH=/opt/dm_group/usr/lib LIBRDKAFKA_INC_PATH=/opt/dm_group/usr/include"
            }
        } catch (e) {
            failure_function(e, 'ADKafka build failed')
        }
    }

    dir("code/m-epics-ADPluginKafka") {
        try {
            stage("Compile ADPluginKafka") {
                sh "make -f EEEmakefile LIBRDKAFKA_LIB_PATH=/opt/dm_group/usr/lib LIBRDKAFKA_INC_PATH=/opt/dm_group/usr/include"
/*                sh "python /opt/epics/modules/environment/2.0.0/3.15.4/bin/centos7-x86_64/module_manager.py --prefix=`pwd` --assumeyes --builddir='builddir' install 'ADPluginKafka' '1.0.0-INTTEST'"*/
            }
        } catch (e) {
            failure_function(e, 'ADPluginKafka build failed')
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

node('clang-format') {
    cleanWs()
    
    dir("code") {
        try {
            stage("Checkout projects") {
                checkout scm
            }
        } catch (e) {
            failure_function(e, 'Checkout failed')
        }

        try {
            stage("Check formatting") {
                sh "find . \\( -name '*.cpp' -or -name '*.h' -or -name '*.hpp' \\) \
                    -exec $DM_ROOT/usr/bin/clangformatdiff.sh {} +"
            }
        } catch (e) {
            failure_function(e, 'Formatting check failed')
        }
    }
}
