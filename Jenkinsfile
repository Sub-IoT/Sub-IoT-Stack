#!/usr/bin/env groovy

node {
    parameters
    {
        string(name: 'BRANCH_PASSED_OVER', defaultValue: '${env.BRANCH_NAME}', description: 'pass branch value')
    }
    stage('Pull changes') {
        properties([[$class: 'CopyArtifactPermissionProperty', projectNames: '*']])
        sh '''
        if [ -d .git ]; then
         git clean -dfx
        fi;

        '''
        checkout scm
        sh '''
            rm -rf B_L072Z_LRWAN1
            rm -rf NUCLEO_L073RZ
            rm -rf EZR32LG_WSTK6200A
            rm -rf cortus_fpga

            mkdir B_L072Z_LRWAN1
            mkdir NUCLEO_L073RZ
            mkdir EZR32LG_WSTK6200A
            mkdir cortus_fpga
         '''

    }

    stage('Build B_L072Z_LRWAN1 platform') {
        dir('B_L072Z_LRWAN1') {
            sh 'mkdir build'
            dir('build') {
                try {
                    sh '''
                        platform="B_L072Z_LRWAN1"
                        toolchain_file="../../stack/cmake/toolchains/gcc-arm-embedded.cmake"
                        toolchain_dir="/opt/gcc-arm-none-eabi-7-2017-q4-major"
                        cmake ../../stack/ -DAPP_GATEWAY=y -DAPP_MODEM=y -DAPP_SENSOR_PUSH=y -DPLATFORM=$platform -DTOOLCHAIN_DIR=$toolchain_dir -DCMAKE_TOOLCHAIN_FILE=$toolchain_file
                        make
                       '''
                    setBuildStatus("B_L072Z_LRWAN1","Build","SUCCESS")
                    echo 'B_L072Z_LRWAN1 success '
                }
                catch (exc) 
                {
                    setBuildStatus("B_L072Z_LRWAN1","Build","FAILURE")
                    echo 'B_L072Z_LRWAN1 failed'
                    currentBuild.result = 'FAILURE'
                }
            }
        }
    }
    
    stage('Build NUCLEO_L073RZ platform') {
        dir('NUCLEO_L073RZ') {
            sh 'mkdir build'
            dir('build') {
                try {
                    sh '''
                        platform="NUCLEO_L073RZ"
                        toolchain_file="../../stack/cmake/toolchains/gcc-arm-embedded.cmake"
                        toolchain_dir="/opt/gcc-arm-none-eabi-7-2017-q4-major"
                        cmake ../../stack/ -DAPP_GATEWAY=y -DAPP_MODEM=y -DAPP_SENSOR_PUSH=y -DPLATFORM=$platform -DTOOLCHAIN_DIR=$toolchain_dir -DCMAKE_TOOLCHAIN_FILE=$toolchain_file
                        make
                       '''
                    setBuildStatus("NUCLEO_L073RZ","Build","SUCCESS")
                    echo 'NUCLEO_L073RZ success '
                }
                catch (exc) 
                {
                    setBuildStatus("NUCLEO_L073RZ","Build","FAILURE")
                    echo 'NUCLEO_L073RZ failed'
                    currentBuild.result = 'FAILURE'
                }
            }
        }
    }

    // stage('Build EZR32LG_WSTK6200A platform') {
    //     dir('EZR32LG_WSTK6200A') {
    //         sh 'mkdir build'
    //         dir('build') {
    //             try {
    //                 sh '''
    //                     platform="EZR32LG_WSTK6200A"
    //                     toolchain_file="../../stack/cmake/toolchains/gcc-arm-embedded.cmake"
    //                     toolchain_dir="/opt/gcc-arm-none-eabi-7-2017-q4-major"
    //                     cmake ../../stack/ -DAPP_GATEWAY=y -DAPP_MODEM=y -DAPP_SENSOR_PUSH=y -DPLATFORM=$platform -DTOOLCHAIN_DIR=$toolchain_dir -DCMAKE_TOOLCHAIN_FILE=$toolchain_file
    //                     make
    //                    '''
    //                 setBuildStatus("EZR32LG_WSTK6200A","Build","SUCCESS")
    //                 echo 'EZR32LG_WSTK6200A success'
    //             }
    //             catch (exc) 
    //             {
    //                 setBuildStatus("EZR32LG_WSTK6200A","Build","FAILURE")
    //                 echo 'EZR32LG_WSTK6200A failed'
    //                 currentBuild.result = 'FAILURE'
    //             }
    //         }
    //     }
    // }

     stage('Build cortus_fpga platform') {
        dir('cortus_fpga') {
            sh 'mkdir build'
            dir('build') {
                try{
                    sh '''
                        platform="cortus_fpga"
                        toolchain_dir="/opt/cortus"
                        toolchain_file="../../stack/cmake/toolchains/aps-gcc.cmake"
                        cmake ../../stack/ -DAPP_GATEWAY=y -DAPP_MODEM=y -DAPP_SENSOR_PUSH=y -DPLATFORM=$platform -DTOOLCHAIN_DIR=$toolchain_dir -DCMAKE_TOOLCHAIN_FILE=$toolchain_file
                        make
                       '''
                    setBuildStatus("cortus_fpga","Build","SUCCESS")
                    echo 'cortus_fpga success'
                }
                catch (exc) 
                {
                    setBuildStatus("cortus_fpga","Build","FAILURE")
                    echo 'cortus_fpga failed'
                    currentBuild.result = 'FAILURE'
                }
            }
        }
    }

//    stage ('Save Artifacts'){
//            archiveArtifacts '**'
//    }
//    stage ('Flash B_L072Z_LRWAN1'){
//           build job: 'FlashRPI_P_B_L072Z_LRWAN1_param', wait: false, parameters: [string(name: 'PASS_BRANCH_NAME', value: env.BRANCH_NAME)]
//    }
    
}
def setBuildStatus(String context,String message, String state) {
  step([
      $class: "GitHubCommitStatusSetter",
      reposSource: [$class: "ManuallyEnteredRepositorySource", url: 'https://github.com/MOSAIC-LoPoW/dash7-ap-open-source-stack'],
      contextSource: [$class: "ManuallyEnteredCommitContextSource", context: context],
      errorHandlers: [[$class: "ChangingBuildStatusErrorHandler", result: "UNSTABLE"]],
      statusResultSource: [ $class: "ConditionalStatusResultSource", results: [[$class: "AnyBuildResult", message: message, state: state]] ]
  ]);

  def color = "danger"
  if(state == 'SUCCESS')
    color = "good"

  def msg = "Build ${context} ${state} (${env.BUILD_NUMBER})"
  slackSend(color: color, message: msg)
}
