 function getExpDate(days, hours, minutes)
    {
        var expDate = new Date();
        if(typeof(days) == "number" && typeof(hours) == "number" && typeof(hours) == "number")
        {
            expDate.setDate(expDate.getDate() + parseInt(days));
            expDate.setHours(expDate.getHours() + parseInt(hours));
            expDate.setMinutes(expDate.getMinutes() + parseInt(minutes));
            return expDate.toGMTString();
        }
    }

    //utility function called by getCookie()
    function getCookieVal(offset)
    {
        var endstr = document.cookie.indexOf(";", offset);
        if(endstr == -1)
        {
            endstr = document.cookie.length;
        }
        return unescape(document.cookie.substring(offset, endstr));
    }

    // primary function to retrieve cookie by name
    function getCookie(name)
    {
        var arg = name + "=";
        var alen = arg.length;
        var clen = document.cookie.length;
        var i = 0;
        while(i < clen)
        {
            var j = i + alen;
            if (document.cookie.substring(i, j) == arg)
            {
                return getCookieVal(j);
            }
            i = document.cookie.indexOf(" ", i) + 1;
            if(i == 0) break;
        }
        return;
    }

    // store cookie value with optional details as needed
    function setCookie(name, value, expires, path, domain, secure)
    {
        document.cookie = name + "=" + escape(value) +
            ((expires) ? "; expires=" + expires : "") +
            ((path) ? "; path=" + path : "") +
            ((domain) ? "; domain=" + domain : "") +
            ((secure) ? "; secure" : "");
    }

    // remove the cookie by setting ancient expiration date
    function deleteCookie(name,path,domain) 
    {
        if(getCookie(name))
        {
            document.cookie = name + "=" +
                ((path) ? "; path=" + path : "") +
                ((domain) ? "; domain=" + domain : "") +
                "; expires=Thu, 01-Jan-70 00:00:01 GMT";
        }
    }
	function Login(username,password)
	{
		var oldhost = window.location.host;
		if(isZY)
		{
			if(username=="admin"&&password=="pass")
			{
				setCookie(oldhost+"gmiusername",username);
				setCookie(oldhost+"gmipassword",password);
				return true;
			}
		
			else
			{
				return false;
				alert("用户名或密码错误");
			}
		}
		else
		{
			if((username=="inc"&&password=="inc")||username=="admin"&&password=="admin")
			{
				setCookie(oldhost+"gmiusername",username);
				setCookie(oldhost+"gmipassword",password);
				return true;
			}
		
			else
			{
				return false;
				alert("用户名或密码错误");
			}
		}
		/*var aresult=false;
		var oldhost = window.location.host;
		 //登录认证开始
				var section = "AUTH";
                var postStr="user="+username+"||pass="+password+"||";
				$.ajax({
                    url: url,
					async: false,
                    data: "Request=set&Section=" + section + "&exsection=0&postDataStr=" + postStr,
                    type: "post",
                    success: function(data) {
                        var data2 = data.replace("\n", "");
                        data2 = $.trim(data2);
                        var result = data2.split('&');
                        var Request;
                        var Section;
                        var Flag;
                        var Info;
                        if (result.length != 4) {
                            return;
                        } else {
                            Request = result[0];
                            Section = result[1];
                            Flag = result[2];
                            Info = result[3];
                            if (Flag == 0) {	
							setCookie(oldhost+"gmiusername",username);
							setCookie(oldhost+"gmipassword",password);
							aresult=true;
							}
							else
							{
								if(Flag=="-404")
								{
									alert("用户名或密码错误");
								
								}
								else
								{
									alert(Info);
								}
							}
							
                        }
				
                    },
                    error: function() {
				
                    }
                });
				return aresult;*/
				 //登录认证结束
	}
	function outLogin()
	{
		try
		{
			var oldhost = window.location.host;
			deleteCookie(oldhost+"gmiusername");
			deleteCookie(oldhost+"gmipassword");
			deleteCookie(oldhost+"currenturl");
			top.location.href="login.html";
		}
		catch(e){alert(e);}
		
	}
	function isLogin()
	{
		var oldhost = window.location.host;
		var user=getCookie(oldhost+"gmiusername");
		var pas=getCookie(oldhost+"gmipassword");
		if(user!=null&&user!="")
		{
			return true;
		}
		else
		{
			   top.location.href="login.html";
		}
	}
