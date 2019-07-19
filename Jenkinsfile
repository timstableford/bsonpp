pipeline {
    agent any

    stages {
        stage('Build') {
            steps {
                sh 'mkdir -p build'
                dir('build') {
                    sh 'cmake ..'
                    sh 'make -j8'
                }
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
