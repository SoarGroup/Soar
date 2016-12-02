def unitTestArguments = "-s -c SMemFunctionalTests"

def names = nodeNames()
def builders = [:]

for (int i=0; i<names.size(); ++i) {
  def name = names[i]

  builders["node_" + name] = {
    node(name) {
      checkout scm

      if (isUnix()) {
        sh 'rm -f *.7zip'
        sh 'rm -rf out*'
        sh 'rm -rf build/Core/ClientSMLSWIG*'
        sh 'scons all --scu'
        //sh 'pushd out; ./Prototype-UnitTesting ' + unitTestArguments + '; popd'
        //junit 'out/TestResults.xml'
        sh 'pushd out; ./UnitTests; popd'
      } else {
        bat 'del /q /f *.7zip'
        bat 'del /q /f user-env*.bat'
        //bat 'del /q /f VS2013\\'
        bat 'del /q /f VS2015\\'

        def tcl="C:\\Tcl"
        if (name == "Windows32") {
          tcl="C:\\Tcl"
          bat 'echo set PYTHON_HOME=C:\\Python27>> user-env.bat'
        } else {
          tcl="C:\\Tcl-x86-64"
          bat 'echo set PYTHON_HOME=C:\\Python27-64>> user-env.bat'
        }

        bat 'echo set JAVA_HOME=C:\\Program Files\\Java\\jdk1.7.0_79>> user-env.bat'
        bat 'echo set SWIG_HOME=C:\\swigwin\\>> user-env.bat'

        //bat "%VS_2013% & call build.bat all --no-scu --tcl=" + tcl + " --build=build-VS2013 --out=VS2013"
        bat "%VS_2015% & call build.bat all --scu --tcl=" + tcl + " --build=build-VS2015 --out=VS2015"

        bat 'pushd VS2015 & UnitTests & popd'
        //bat 'pushd VS2013 & Prototype-UnitTesting ' + unitTestArguments + ' & popd'
        //bat 'pushd VS2015 & Prototype-UnitTesting ' + unitTestArguments + ' & popd'

        //junit 'VS2013\\TestResults.xml'
        //junit 'VS2015\\TestResults.xml'
      }

      withCredentials([[$class: 'UsernamePasswordMultiBinding', credentialsId: '099da30c-b551-4c0c-847d-28fa1c22c5cb',
                            usernameVariable: 'USERNAME', passwordVariable: 'PASSWORD']]) {
        if (isUnix()) {
          sh "export VERSION=\$(<soarversion); 7za a \${VERSION}-" + name + ".7zip out/"
          sh "export VERSION=\$(<soarversion); sshpass -p \${PASSWORD} scp \${VERSION}-" + name + ".7zip \${USERNAME}@soar-jenkins.eecs.umich.edu:/Users/Shared/Build/Nightlies/"
        } else {
          //bat 'for /f %%x in (soarversion) do "C:/Program Files/7-Zip/7z.exe" a %%x-' + name + '-VS2013.7zip VS2013/'
          bat 'for /f %%x in (soarversion) do "C:/Program Files/7-Zip/7z.exe" a %%x-' + name + '-VS2015.7zip VS2015/'
          //bat 'for /f %%x in (soarversion) do C:\\pscp.exe -pw %PASSWORD% %%x-' + name + '-VS2013.7zip %USERNAME%@soar-jenkins.eecs.umich.edu:/Users/Shared/Build/Nightlies/'
          bat 'for /f %%x in (soarversion) do C:\\pscp.exe -pw %PASSWORD% %%x-' + name + '-VS2015.7zip %USERNAME%@soar-jenkins.eecs.umich.edu:/Users/Shared/Build/Nightlies/'
        }
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
