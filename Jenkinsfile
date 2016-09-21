stage('Build') {
  node('unix') {
    checkout scm
    sh 'scons all --no-scu'
    sh './Prototype-UnitTesting -s -c SMemFunctionalTests'

    archive 'out/**'

    version=sh('cat soarversion')
    sh "7za a " + version + "-" + nodeName + ".7zip out/"

    archive '*.7zip'
  }
  node('windows') {
    checkout scm

    def folder = new File('C:/Tcl')

    if (folder.exists()) {
      sh 'call build.bat all --no-scu --tcl=C:/Tcl'
    } else {
      sh 'call build.bat all --no-scu --tcl=C:/Tcl-x86-64'
    }

    sh 'Prototype-UnitTesting -s -c SMemFunctionalTests'

    archive 'out/**'

    sh 'set /p VERSION=<soarversion'
    sh '"C:/Program Files/7-Zip/7z.exe" a %VERSION%-%BUILD_ID%-' + nodeName + '-VS2015.7zip out/'

    archive '*.7zip'
  }
}
