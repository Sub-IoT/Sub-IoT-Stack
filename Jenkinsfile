#!/usr/bin/env groovy

node {
    stage('Pull changes') {
        checkout scm
        sh 'rm -rf build'
        sh 'mkdir build'
    }

    stage('Build B_L072Z_LRWAN1 platform') {
        dir('build') {
            sh '''
                cmake ../stack/ -DAPP_GATEWAY=y -DAPP_MODEM=y -DAPP_SENSOR_PUSH=y -DPLATFORM=B_L072Z_LRWAN1 -DTOOLCHAIN_DIR=/opt/gcc-arm-none-eabi-6-2017-q2-update -DCMAKE_TOOLCHAIN_FILE=../stack/cmake/toolchains/gcc-arm-embedded.cmake
                make
               '''
        }
    }

    stage('Build NUCLEO_L073RZ platform') {
        sh 'rm -rf build'
        sh 'mkdir build'
        dir('build') {
            sh '''
                cmake ../stack/ -DAPP_GATEWAY=y -DAPP_MODEM=y -DAPP_SENSOR_PUSH=y -DPLATFORM=NUCLEO_L073RZ -DTOOLCHAIN_DIR=/opt/gcc-arm-none-eabi-6-2017-q2-update -DCMAKE_TOOLCHAIN_FILE=../stack/cmake/toolchains/gcc-arm-embedded.cmake
                make
               '''
        }
    }

    stage('Build EZR32LG_WSTK6200A platform') {
        sh 'rm -rf build'
        sh 'mkdir build'
        dir('build') {
            sh '''
                cmake ../stack/ -DAPP_GATEWAY=y -DAPP_MODEM=y -DAPP_SENSOR_PUSH=y -DPLATFORM=EZR32LG_WSTK6200A -DTOOLCHAIN_DIR=/opt/gcc-arm-none-eabi-6-2017-q2-update -DCMAKE_TOOLCHAIN_FILE=../stack/cmake/toolchains/gcc-arm-embedded.cmake
                make
               '''
        }
    }

   
}
