@rem
@rem Run with clean-all-windows
@rem Deletes all object files resulting from build.
@rem Note: Will delete both debug and release in one go.
@rem

@rem Set the soar version (used to find the soar files)
@set soarver=Soar-8.3

@echo Delete STI files
del /s STI\*.exe STI\*.dll STI\*.obj STI\*.lib STI\*.idb STI\*.sbr STI\*.exp STI\*.pch STI\*.res

@echo Delete Soar-8.3 files
@rem del /s %soarver%\*.exe %soarver%\*.dll %soarver%\*.obj %soarver%\*.lib %soarver%\*.idb %soarver%\*.sbr %soarver%\*.exp %soarver%\*.pch %soarver%\*.res

@echo Delete VisualSoar files
del /s VisualSoar\*.class VisualSoar\*.jar


