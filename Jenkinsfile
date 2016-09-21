
def names = nodeNames()
def builders = [:]

for name in names {
  builders["node_" + name] = {
    node(name + " && unix") {
        checkout scm
        sh 'scons all --no-scu'
        sh 'pushd out; ./Prototype-UnitTesting -s -c SMemFunctionalTests; popd'
        junit 'out/TestResults.xml'

        archive 'out/**'

        version=sh('cat soarversion')
        sh "7za a " + version + "-" + nodeName + ".7zip out/"

        archive '*.7zip'
      }
      node(name + " && windows") {
        checkout scm

        def folder = new File('C:/Tcl')

        if (folder.exists()) {
          sh 'call build.bat all --no-scu --tcl=C:/Tcl'
        } else {
          sh 'call build.bat all --no-scu --tcl=C:/Tcl-x86-64'
        }

        sh 'pushd out; Prototype-UnitTesting -s -c SMemFunctionalTests; popd'
        junit 'out/TestResults.xml'

        archive 'out/**'

        sh 'set /p VERSION=<soarversion'
        sh '"C:/Program Files/7-Zip/7z.exe" a %VERSION%-%BUILD_ID%-' + nodeName + '-VS2015.7zip out/'

        archive '*.7zip'
      }
    }
  }
}

parallel builders

@NonCPS
def nodeNames() {
  return jenkins.model.Jenkins.instance.nodes.collect { node -> node.name }
}
