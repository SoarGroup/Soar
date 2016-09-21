
def names = nodeNames()
def builders = [:]

for (int i=0; i<names.size(); ++i) {
  def name = names[i]

  builders["node_" + name] = {
    node(name) {
      checkout scm

      if (isUnix()) {
        sh 'rm -f *.7zip'
        sh 'scons all --no-scu'
        sh 'pushd out; ./Prototype-UnitTesting -s -c SMemFunctionalTests; popd'
      } else {
        bat 'del /q /f *.7zip'
        bat 'del /q /f user-env.bat'

        def tcl="C:\\Tcl"
        if (name == "Windows32") {
          bat 'echo set PYTHON_HOME=C:\\Python27>> user-env.bat'
        } else {
          tcl="C:\\Tcl-x86-64"
          bat 'echo set PYTHON_HOME=C:\\Python27-64>> user-env.bat'
        }

        bat 'echo set JAVA_HOME=C:\\Program Files\\Java\\jdk1.7.0_79>> user-env.bat'
        bat 'echo set SWIG_HOME=C:\\swigwin\\>> user-env.bat'

        if (tcl.exists()) {
          bat '%VS_2015% & call build.bat all --no-scu --tcl=' + tcl
        } else {
          bat '%VS_2015% & call build.bat all --no-scu --tcl=' + tcl
        }

        bat 'pushd out & Prototype-UnitTesting -s -c SMemFunctionalTests & popd'
      }

      junit 'out/TestResults.xml'

      if (isUnix()) {
        sh "export VERSION=\$(<soarversion); 7za a \${VERSION}-" + name + ".7zip out/"
      } else {
        bat 'set /p VERSION=<soarversion & "C:/Program Files/7-Zip/7z.exe" a %VERSION%-' + name + '-VS2015.7zip out/'
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
