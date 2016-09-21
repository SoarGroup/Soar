
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
        def folder = new File('C:/Tcl')

        bat 'del /q /f *.7zip'
        bat 'del /q /f user-env.bat'

        bat 'echo set PYTHON_HOME=%PYTHON_HOME%>> user-env.bat & echo set JAVA_HOME=%JAVA_HOME%>> user-env.bat & echo set SWIG_HOME=%SWIG_HOME%>> user-env.bat'

        if (folder.exists()) {
          bat '%VS_2015% & call build.bat all --no-scu --tcl=C:/Tcl'
        } else {
          bat '%VS_2015% & call build.bat all --no-scu --tcl=C:/Tcl-x86-64'
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
