pipeline {
    agent any

    stages {
        stage('Build') {
            steps {
                parallel(
                    'PlatformIO': {
                        sh 'pio run'
                    },
                    'Linux': {
                        sh 'mkdir -p build'
                        dir('build') {
                            sh 'cmake -DBUILD_TESTS=ON ..'
                            sh 'make -j8'
                        }
                    },
                )
            }
        }

        stage('Test') {
            steps {
                dir('build') {
                    sh './BSONPP_Test'
                }
            }
        }
    }
}
