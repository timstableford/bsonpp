pipeline {
    agent any

    stages {
        stage('Build') {
            steps {
                sh 'mkdir -p build'
                sh 'cd build'
                sh 'cmake ..'
                sh 'make -j8'
            }
        }

        stage('Test') {
            steps {
                sh 'cd build'
                sh './BSONPP_Test'
            }
        }
    }
}
