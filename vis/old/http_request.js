var request = null;

function httpRequest( reqType, url, asynch, respHandle )
{
    if ( window.XMLHttpRequest )
    {
        request = new XMLHttpRequest();
    }
    else if ( window.ActiveXObject )
    {
        request = new ActiveXObject("MSxml2.XMLHTTP");
        if ( !request )
        {
            request = new ActiveXObject("Microsoft.XMLHTTP");
        }
    }
    
    if ( request )
    {
        if ( reqType.toLowerCase() != "post" )
        {
            initReq( reqType, url, asynch, respHandle );
        }
        else
        {
            var args = arguments[4];
            if ( args != null && args.length > 0 )
            {
                initReq( reqType, url, asynch, respHandle, args );
            }
        }
    }
    else
    {
        alert("Your browser is not supported!");
    }
}

function initReq( reqType, url, bool, respHandle )
{
    try
    {
        request.onreadystatechange = respHandle;
        request.open( reqType, url, bool );
        
        if ( reqType.toLowerCase() == "post" )
        {
            request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");
            request.send(arguments[4]);
        }
        else
        {
            request.send(null);
        }
    }
    catch (errv)
    {
        alert("Application cannot contact the server: " + errv.message);
    }
}
