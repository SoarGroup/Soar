def branches = [:]
def names = nodeNames()

for (int i=0; i<names.size(); ++i)
{
  def nodeName = names[i];
  // Into each branch we put the pipeline code we want to execute
  branches["node_" + nodeName] = {

    stage name: 'Checkout'
    node(nodeName){
      if (nodeName != "master"){
        checkout scm
      }
    }

    stage name: 'Build'
    node(nodeName){
      if (nodeName != "master"){
        if (isUnix()){
          sh('scons all --no-scu')
        } else {
          def folder = new File('C:/Tcl')

          sh('%VS_2015%')

          if (folder.exists()){
            sh('call build.bat all --no-scu --tcl=C:/Tcl')
          } else {
            sh('call build.bat all --no-scu --tcl=C:/Tcl-x86-64')
          }
        }
      }
    }

    stage name: 'Testing'
    node(nodeName){
      if (isUnix()){
        sh('./Prototype-UnitTesting -c SMemFunctionalTests -f SMemFunctionalTests::testReadCSoarDB -f SMemFunctionalTests::testDbBackupAndLoadTests')
      } else {
        sh('Prototype-UnitTesting -c SMemFunctionalTests -f SMemFunctionalTests::testReadCSoarDB -f SMemFunctionalTests::testDbBackupAndLoadTests')
      }
    }

    stage name: 'Archive'
    node(nodeName) {
      archive 'out/**'
      
      if (isUnix()){
        sh('export VERSION=$(<soarversion)')
        sh("7za a ${VERSION}-${BUILD_ID}-" + nodeName + ".7zip out/")
      } else {
        sh('set /p VERSION=<soarversion')
        sh('"C:/Program Files/7-Zip/7z.exe" a %VERSION%-%BUILD_ID%-' + nodeName + '-VS2015.7zip out/')
      }

      archive '*.7zip'
    }

  }
}

// Now we trigger all branches
parallel branches

// This method collects a list of Node names from the current Jenkins instance
@NonCPS
def nodeNames() {
  return jenkins.model.Jenkins.instance.nodes.collect { node -> node.name }
}