
def names = nodeNames()
def builders = [:]

for (int i=0; i<names.size(); ++i) {
  def name = names[i]

  builders["node_" + name] = {
    node(name) {
      checkout scm

      if (isUnix()) {
        sh 'scons all --no-scu'
        sh 'pushd out; ./Prototype-UnitTesting -s -c SMemFunctionalTests; popd'
      } else {
        def folder = new File('C:/Tcl')

        if (folder.exists()) {
          bat 'call build.bat all --no-scu --tcl=C:/Tcl'
        } else {
          bat 'call build.bat all --no-scu --tcl=C:/Tcl-x86-64'
        }

        bat 'pushd out; Prototype-UnitTesting -s -c SMemFunctionalTests; popd'
      }

      junit 'out/TestResults.xml'

      if (isUnix()) {
        sh "export VERSION=\$(<soarversion); 7za a \${VERSION}-" + name + ".7zip out/"
      } else {
        bat 'set /p VERSION=<soarversion; "C:/Program Files/7-Zip/7z.exe" a %VERSION%-%BUILD_ID%-' + name + '-VS2015.7zip out/'
      }

      archive '*.7zip'
    }
  }
}

parallel builders

@NonCPS
def nodeNames() {
  return jenkins.model.Jenkins.instance.nodes.collect { node -> node.name }
}
