//µÃµ½xmlÉÏÏÂÎÄ 
function getXMLHandler(xmlFile){ 
  var xmlDoc;
  if(window.ActiveXObject){ 
    xmlDoc=new ActiveXObject("Microsoft.XMLDOM"); 
    xmlDoc.onreadystatechange = function() { 
      if(xmlDoc.readyState == 4);// doAction(); 
    } 
    
  }
  else if(document.implementation&&document.implementation.createDocument){ 
    // string ÀàÐÍ
	 xmlDoc=document.implementation.createDocument("","",null);

	//ÎÄ±¾ÀàÐÍ
	//var parser = new DOMParser();
    //xmlDoc = parser.parseFromString(xmlFile, "text/xml");

  }
  else 
  {
	 alert("´íÎó:Parameter Error in function LoadXML£¡ä¯ÀÀÆ÷²»Ö§³Ö¼ÓÔØxml£¡\n--Your browser cannot handle this script")
    return null; 
  }
  xmlDoc.async=false; 
  xmlDoc.load(xmlFile); 
  return xmlDoc; 
} 

//µÃµ½½ÚµãÖµ
//xmlFile:XMLÎÄ¼þ
//language:ÓïÑÔ
//tagName:ÔªËØÃû
function getNodeValue(xmlFile,language,tagName){
	//tagName  第一个字母不能为数字
	var firstchar=tagName[0];
	if(!isNaN(firstchar))
	{
		 return "";
	}
  var xmlDoc = getXMLHandler(xmlFile);
  try
  {
  if( xmlDoc.getElementsByTagName(tagName) != null)
  {
	  var btype=0;  //IE ºÍFirefox  xmlÖÐ childNodesË÷Òý²»Í¬  IE:0123...Firefox  1 3 5 7
	    if(navigator.userAgent.indexOf("MSIE")>0) 
		{   
        	btype=0;   
 	    }   
 	   if(isFirefox=navigator.userAgent.indexOf("Firefox")>0)
	   {   
            btype=1; 
       }   
      if(isSafari=navigator.userAgent.indexOf("Safari")>0) 
      {   
       		btype=0;  
      }    
      if(isCamino=navigator.userAgent.indexOf("Camino")>0)
	  {   
        	btype=0; 
      }   
     // if(isMozilla=navigator.userAgent.indexOf("Gecko/")>0)
	 // {   
      //  	btype=0;   
      //}  
	  if(xmlDoc.getElementsByTagName(tagName)[0]!=null)
	  {
		  if(language == "en")
		  {
			 return xmlDoc.getElementsByTagName(tagName)[0].childNodes[btype].firstChild.nodeValue;
		  }
		  else
		  {
			 return xmlDoc.getElementsByTagName(tagName)[0].childNodes[btype*2+1].firstChild.nodeValue;
		  }
	  }
  }
  }
  catch(e)
  {}
  
}

//×ª±äÔªËØÓïÑÔ
//targetDocument:ÎÄµµ,tag:±êÇ©Ãû,propertyToSet:·½Ê½,language:ÓïÑÔ
function TranslateElements( targetDocument, tag, propertyToSet, language )
{
	//debugger;
        var e = targetDocument.getElementsByTagName(tag) ;
        for ( var i = 0 ; i < e.length ; i++ )
        {
                var sKey = e[i].getAttribute('id') ;
                
                if ( sKey )
                {
                        var s = getNodeValue('xmlFile.xml',language,(sKey.substring(1,sKey.length)).toLowerCase());
                        if ( s ) 
                                eval( 'e[i].' + propertyToSet + ' = s' ) ;
                        
                }
        }
}

function TranslatePage( language )
{
        //this.TranslateElements( targetDocument, 'INPUT', 'value' ) ;
        //this.TranslateElements( targetDocument, 'SPAN', 'innerHTML' ) ;
        this.TranslateElements( document, 'LABEL', 'innerHTML',language) ;
		this.TranslateElements( document, 'A', 'innerHTML',language) ;
		this.TranslateElements( document, 'BUTTON', 'innerHTML',language);
		this.TranslateElements( document, 'DT', 'innerHTML',language); 
		this.TranslateElements( document, 'INPUT', 'value',language); 
		this.TranslateElements( document, 'DIV', 'innerHTML',language); 
		this.TranslateElements( document, 'LEGEND', 'innerHTML',language);
		this.TranslateElements( document, 'H3', 'innerHTML',language);
	    this.TranslateElements( document, 'B', 'innerHTML',language);
		this.TranslateElements( document, 'OPTION', 'innerHTML',language);
        //this.TranslateElements( targetDocument, 'OPTION', 'innerHTML' ) ;
}
function initLang()
{
	  var lang=getCookie('lang');
	  if(lang!=null)
	 {
		 $("#select_language").val(lang);
		 TranslatePage( lang );
	 }
	 else
	{
		 TranslatePage( 'zh-cn' );
	}
}
  