<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
	<head>
		<title>Speedy {dash_title}</title>
		
		<link href="common/public/speedy.css" rel="stylesheet" type="text/css" media="all" />
		<link rel="shortcut icon" href="common/public/favico.ico" >
		
		<link type="text/css" href="common/public/jquery/jquery-ui-1.7.2.custom.css" rel="Stylesheet" />
		
		<script type="text/javascript" src="http://www.google.com/jsapi"></script>
		<script type="text/javascript">
			google.load( "jquery", "1.3.2" );
			google.load( "jqueryui", "1.7.2" );
		</script>
		
		
		{head}
	</head>
	
	<body>
		<div id="content">
			
			<div id="header">
				<div style="text-align: {align}"><a href="index.php"><img src="common/public/logo.png" /></a></div>
				<div style="text-align: {align}" class="nav">&nbsp;{nav}</div>
			</div>
			
			<div id="title">
				{title}
			</div>
			<br />
		
			{content}
			
		</div>
	</body>
	
</html>