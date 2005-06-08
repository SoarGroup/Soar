del /S /Q *.class 
javac.exe -classpath .;..\soar-library\sml.jar -sourcepath Source Source\edu\umich\visualsoar\VisualSoar.java
jar cfm VisualSoar.jar Source\meta-inf\manifest.mf -C Source .

IF NOT "%1"=="--nopause" pause