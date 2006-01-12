function  OnFinish(selProj, selObj)
{
	try
	{
		var strProjectPath = wizard.FindSymbol("PROJECT_PATH");
		var strProjectName = wizard.FindSymbol("PROJECT_NAME");
		var selProj = CreateProject(strProjectName, strProjectPath);

		AddCommonConfig(selProj, strProjectName);
		AddSpecificConfig(selProj, strProjectName);
		
		SetupFilters(selProj);

		AddFilesToProjectWithInfFile(selProj, strProjectName);

		selProj.Object.Save();
	}
	catch(e)
	{
		if( e.description.length > 0 )
		SetErrorInfo(e);

		return e.number;
	}
} 

function AddSpecificConfig(selProj, strProjectName)
{
	try
	{
		selProj.Object.Keyword="Win32Proj";
	
		var config = selProj.Object.Configurations("Debug");
		config.OutputDirectory="build";
		config.IntermediateDirectory="$(OutDir)/Debug";
		config.ConfigurationType="1";
		config.CharacterSet="2";
		config.UseManagedExtensions="FALSE";
		config.WholeProgramOptimization="FALSE";
		
		var CLTool = config.Tools("VCCLCompilerTool");
		CLTool.Optimization="0";
		CLTool.AdditionalIncludeDirectories="../ClientSML/include;../ConnectionSML/include;../ElementXML/include;../KernelSML/include";
		CLTool.PreprocessorDefinitions="WIN32;_DEBUG;_CONSOLE";
		CLTool.MinimalRebuild="TRUE";
		CLTool.BasicRuntimeChecks="3";
		CLTool.RuntimeLibrary="1";
		CLTool.UsePrecompiledHeader="0";
		CLTool.AssemblerListingLocation="$(IntDir)/";
		CLTool.ProgramDataBaseFileName="$(IntDir)/";
		CLTool.WarningLevel="4";
		CLTool.Detect64BitPortabilityProblems="TRUE";
		CLTool.DebugInformationFormat="4";

		var LinkTool = config.Tools("VCLinkerTool");
		LinkTool.OutputFile="../../soar-library/$(ProjectName).exe";
		LinkTool.LinkIncremental="2";
		LinkTool.AdditionalLibraryDirectories="../../soar-library";
		LinkTool.GenerateDebugInformation="TRUE";
		LinkTool.ProgramDatabaseFile="$(OutDir)/TestClientSML.pdb";
		LinkTool.SubSystem="1";
		LinkTool.TargetMachine="1";
		
		config = selProj.Object.Configurations("Release");
		config.OutputDirectory="build";
		config.IntermediateDirectory="$(OutDir)/Release";
		config.ConfigurationType="1";
		config.CharacterSet="2";
		config.UseManagedExtensions="FALSE";
		config.WholeProgramOptimization="TRUE";
		
		CLTool = config.Tools("VCCLCompilerTool");
		CLTool.AdditionalIncludeDirectories="../ClientSML/include;../ConnectionSML/include;../ElementXML/include;../KernelSML/include";
		CLTool.PreprocessorDefinitions="WIN32;NDEBUG;_CONSOLE";
		CLTool.RuntimeLibrary="0";
		CLTool.UsePrecompiledHeader="0";
		CLTool.AssemblerListingLocation="$(IntDir)/";
		CLTool.ProgramDataBaseFileName="$(IntDir)/";
		CLTool.WarningLevel="3";
		CLTool.Detect64BitPortabilityProblems="TRUE";
		CLTool.DebugInformationFormat="3";

		LinkTool = config.Tools("VCLinkerTool");
		LinkTool.OutputFile="../../soar-library/$(ProjectName).exe";
		LinkTool.LinkIncremental="1";
		LinkTool.AdditionalLibraryDirectories="../../soar-library";
		LinkTool.GenerateDebugInformation="TRUE";
		LinkTool.ProgramDatabaseFile="$(OutDir)/TestClientSML.pdb";
		LinkTool.SubSystem="1";
		LinkTool.OptimizeReferences="2";
		LinkTool.EnableCOMDATFolding="2";
		LinkTool.TargetMachine="1";
		
		// None of this commented out code below works
		//var Solution = dte.Solution;
		//var ClientSML = Solution.FindProjectItem("ClientSML");
		//var ConnectionSML = Solution.FindProjectItem("ConnectionSML");
		//var ElementXML = Solution.FindProjectItem("ElementXML");
		//var KernelSML = Solution.FindProjectItem("KernelSML");
		//var refmanager = selProj.Object.References;
		//refmanager.AddProject(ClientSML);
		//refmanager.AddProject(ConnectionSML);
		//refmanager.AddProject(ElementXML);
		//refmanager.AddProject(KernelSML);
		// Need to get project objects for ClientSML, ConnectionSML,
		// ElementXML, KernelSML and then pass them to AddProject()
		//var refmanager = selProj.Object.References;
		//refmanager.AddProject(projectObject);
	}
	catch(e)
	{
		throw e;
	}
}

function GetTargetName(strName, strProjectName, strResPath, strHelpPath)
{
	return strName; 
}

function SetFileProperties(projfile, strName)
{
	return false;
}

