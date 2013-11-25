var url = "/cgi-bin/cgi";
//var url = "http://localhost/testweb3.php";

		
// JavaScript Document   kkk nnn
//通过此函数向CGI 发送数据 
//参数1，Request ：set setDefault get getDefault
//参数2，Section ：对应协议中的Section  如IMAGE   RECORDPLAN ……等
//参数3，为具体参数。get时  为码流 0表示码流1 1表示码流2... set时 则为各个具体参数如：brightness=-233||参数2=333  等不同参数用"||"分割。
//参数4，函数名 获取的后台数据将会传递给此函数处理
//参数5，是否有滑动条。缺省参数。 默认false
//参数6,是否是即时设置。缺省参数。默认false
function GetAndSet(cgiFncCmd, cgiContentType, Content,successfunc,TB) {
	if(typeof(TB) == undefined || !TB || TB == null)
	{
		TB = true;
	}
	else if(TB == true)
	{
		TB = false;
	}

	//var TB = TB && true;
    $.ajax({
        //url:"http://10.0.0.160:7777/cgi-bin/jscgi.cgi",
        url: url,
        //url:"http://test.webgui.com:8081/testphp2.php",
        //data: "cgiFncCmd=" + cgiFncCmd + "&cgiContentType=" + cgiContentType + "&Content=" + encodeURIComponent(Content),
		data: "cgiFncCmd=" + cgiFncCmd + "&cgiContentType=" + cgiContentType + "&Content=" + Content,
        type: "post",
        global: true,
		
		async: TB,
        beforeSend: function(XMLHttpRequest) {
//alert(encodeURIComponent("cgiFncCmd=" + cgiFncCmd + "&cgiContentType=" + cgiContentType + "&Content=" + Content));
        },
        complete: function(XMLHttpRequest, textStatus) {
            //ajax 请求结束。失败或成功均调用        
        },
        success: function(data) {
           if(successfunc && typeof successfunc === 'function')
		   {
			   successfunc(data);
		   }
        },
        error: function(e) {
           
        }
    });
}
/************************
*解析服务器端返回的数据
*
*
*
**************************/
function UnPackData(responseStr)
{
	var results=responseStr.split("&");
	if(results.length < 3)
	{
		//alert("返回数据格式错误："+responseStr);
		return;
	}
	else
	{
		  this.cgiFncCmd = results[0].split("=")[1];
		  this.cgiContentType=results[1].split("=")[1];
		  this.Content=results[2].substring(8);
		  
		  //拼接字符
		  if(results.length > 3)
		  {
				for(var i=3;i<results.length;i++)
				{
					this.Content += "&" + results[i];
				}  
		  }
		  if(this.cgiContentType=="json")
		  {
			  if(this.Content!=null)
			  {
				 this.Content= eval("("+this.Content+")");
			  }   
		  }
		  if(this.cgiContentType=="xml")
		  {
			  if(this.Content!=null)
			  {
				 //this.Content = this.Content.replace(/(^\s*)|(\s*$)|(\n)/g, "");
				 this.Content= eval("("+this.Content+")");
				 //this.Content.XmlBuf=""+this.Content;
				 //this.Content.XmlBuf=this.Content.XmlBuf.replace(/;/g,"");
				 this.Content.XmlBuf=xmlToJson.parser(this.Content.XmlBuf.replace(/;/g,""));  
				 
			  } 
		  }
			
	}
	//alert(results.length);
}


function motionSlider() {

    var valids = ["55roi"+cindex+"_sensitivity", "55roi"+cindex+"_threshold"];
    for (var i = 0; i < valids.length; i++) {
        var dragbgid = valids[i].substr(2,3)+valids[i].substr(6);
        var value = document.getElementById(valids[i]).value;
        if (dragbgid == "roi_sensitivity") //取值范围 0-10；
        {
			sensitivity=value;
            value = parseInt((value / 10 * 100), 10);
        }
        if (dragbgid == "roi_threshold") //取值范围-15-15；
        {
			threshold=value;
            value = parseInt((value / 100 * 100), 10);
        }
        $("#" + dragbgid + " a")[0].style.left = value + "%";
    }

}
function getCTCFG(time, elem1, elem2) {
    var h = parseInt(time, 10) / 60;
    h = parseInt(h, 10);
    var m = (parseInt(time, 10) - h * 60);
    m = parseInt(m, 10);
    elem1.val(h);
    elem2.val(m);

}
function initPrivate() {
    $("#masklist").empty(); //清空列表
    var mask1, mask2, mask3, mask4;
    //alert(Info);
    //第一次出现pm1，pm2。。。。索引位置
    var pm1Index = pInfo.indexOf("pm1");
    var pm2Index = pInfo.indexOf("pm2");
    var pm3Index = pInfo.indexOf("pm3");
    var pm4Index = pInfo.indexOf("pm4");
    mask1 = pInfo.substring(pm1Index, pm2Index);
    mask2 = pInfo.substring(pm2Index, pm3Index);
    mask3 = pInfo.substring(pm3Index, pm4Index);
    mask4 = pInfo.substring(pm4Index);
    $("#mask1").val(mask1);
    $("#mask2").val(mask2);
    $("#mask3").val(mask3);
    $("#mask4").val(mask4);
    //alert($("#mask1").val())
    posStr = "";
    createList(1, mask1);
    createList(2, mask2);
    createList(3, mask3);
    createList(4, mask4);
    if (posStr != "") {
        posStr = posStr + posL;
        //alert(posStr);
        document.getElementById("GMIIPCmrWebPlugIn1").SetMaskPosition(posStr);
    }
    //alert($("#32pm1_color").val());
    //显示隐私遮挡样式。
    $("#32pm_color").val($("#32pm1_color").val());
}
///移动侦测窗口 类  获取 开始。
function motionGet(Info, Index) {
    var _this = this;
    _this.roi_enable = '0';
    _this.roi_h = '2';
    _this.roi_index = '1';
    _this.roi_name = "new";
    _this.roi_sensitivity = '6';
    _this.roi_threshold = '50';
    _this.roi_w = '2';
    _this.roi_x = '0';
    _this.roi_y = '1';
    _this.ocxStr = "";
    var init = function() {
        var arrs = Info.split("||");
        _this.roi_index = arrs[0].split("=")[1];
        _this.roi_name = arrs[1].split("=")[1];
        _this.roi_enable = arrs[2].split("=")[1];
        _this.roi_x = arrs[3].split("=")[1];
        _this.roi_y = arrs[4].split("=")[1];
        _this.roi_w = arrs[5].split("=")[1];
        _this.roi_h = arrs[6].split("=")[1];
        _this.roi_threshold = arrs[7].split("=")[1];
        _this.roi_sensitivity = arrs[8].split("=")[1];
        if (_this.roi_enable != '0') {
            _this.ocxStr = "roi_x = " + _this.roi_x + "\nroi_y = " + _this.roi_y + "\nroi_w = " + _this.roi_w + "\nroi_h = " + _this.roi_h + "\nroi_threshold = " + _this.roi_threshold + "\nroi_sensitivity = " + _this.roi_sensitivity + "\nroi_index = " + _this.roi_index + "\nroi_enable = " + _this.roi_enable + "\nroi_name = " + _this.roi_name + "\nroi_nums = 3" + "\n#";
        }
    };
    init();
}
///移动侦测窗口 类 获取  结束。
function createList(index, maskinfo) {
    var maskenable;
    var maskname;
    var enableindex = maskinfo.indexOf("pm" + index + "_enable");
    var enable = maskinfo.substr(enableindex, 12);
    var temp = enable.split("=");
    maskenable = temp[1];
    var nameindex = maskinfo.indexOf("pm" + index + "_name");
    var namelast = maskinfo.substring(nameindex);
    var templ = namelast.indexOf("||");
    var masknametemp = namelast.substring(0, templ);
    maskname = masknametemp.split("=")[1];
    if (maskenable == 1) {
        document.getElementById("masklist").options.add(new Option(maskname, index));
        //alert(maskinfo);
        var maskarr = maskinfo.split("||");
        var name = maskarr[6].split("=")[1];
        var top = maskarr[2].split("=")[1];
        var left = maskarr[1].split("=")[1];
        var w = maskarr[3].split("=")[1]; //宽
        var h = maskarr[4].split("=")[1]; //高
        var right = parseInt(left, 10) + parseInt(w, 10) - parseInt("1", 10);
        var bottom = parseInt(top, 10) + parseInt(h, 10) - parseInt("1", 10);
        if (posStr == "") {
            posStr = name + "," + top + "," + left + "," + right + "," + bottom + "," + index + ",";
        } else {
            posStr += "#," + name + "," + top + "," + left + "," + right + "," + bottom + "," + index + ",";
        }
        ////posStr = "mas1,1,1,10,10,1,#,mas2,2,2,20,20,2,#,mas3,3,3,30,30,3,#,mas4,4,4,40,40,4,\0" ; 
        //document.getElementById("GMIIPCmrWebPlugIn1").SetMaskPosition(this.value);   
    } else {
        //删除
        document.getElementById("GMIIPCmrWebPlugIn1").DelPmWindow(index);
    }
}
//预览页面状态
var isplaying = true;
var isstreamlive = true;
/*
function LoadImg(hostname, streamId) {
	//debugger;
	 $(".btn-set", window.top.document)[0].disabled=true;
	 $(".btn-set", window.top.document)[1].disabled=true;
    if (window.ActiveXObject) {
        try {
            if (typeof(isstreamlive) == 'boolean' && isstreamlive == false) {
                if (document.getElementById("GMIIPCmrWebPlugIn1")) document.getElementById("GMIIPCmrWebPlugIn1").style.display = "none";
                //alert("当前码流已关闭.");
                return;
            }
		    var activeX = document.getElementById("GMIIPCmrWebPlugIn1");
			//activeX.SetUserName(getCookie(oldhost+"gmiusername"),getCookie(oldhost+"gmipassword"));
				if (activeX.SetRecvType(0)) {
					
					activeX.SetHostname(hostname);
					activeX.SetStreamId(streamId);
					activeX.ShowStat(1);
					activeX.ShowDPTZ(0);
				
				}
				else
				{
					alert("设置传输协议失败！");
					location.reload();
				}
				if(parent.tport!="")
			{
				activeX.SetCmdPort(parent.tport);
				//成功返回 1
				//是否硬件加速记录到cookie
				var isHardwareEnabled=getCookie("isHardwareEnabled");
				if(isHardwareEnabled==null||isHardwareEnabled==undefined)
				{
					setCookie("isHardwareEnabled","1");
					$("#liIsHardware").css("display","block");
					var activex = document.getElementById("GMIIPCmrWebPlugIn1");		
					activex.SetAcceleration(true);
					$("#isHardwareEnabled").val("1");
				}
				var i = activeX.Play();
				
				setBuffer(streamId);//设置buffer
			
				
			}
			else
			{
				$.ajax({
                    url: url,
                    data: "Request=get&Section=PROTOINFO&exsection= 0&postDataStr=" + encodeURIComponent("||"),
                    type: "post",
                    success: function(data) {
                        //alert(data);
                        var data2 = data.replace("\n", "");
                        data2 = $.trim(data2);
                        var result = data2.split('&');
                        var Request;
                        var Section;
                        var Flag;
                        var Info;
                        if (result.length != 4) {
                            alert("返回数据解析错误");
                            return;
                        } else {
                            Request = result[0];
                            Section = result[1];
                            Flag = result[2];
                            Info = result[3];
                            //alert(Info);
                            if (Flag == 0) {
								var index=Info.indexOf("camera_server_port");
								parent.tport=Info.substring(index).split("||")[0].split("=")[1];
							    activeX.SetCmdPort(parent.tport);
								//是否硬件加速记录到cookie
								var isHardwareEnabled=getCookie("isHardwareEnabled");
								if(isHardwareEnabled==null||isHardwareEnabled==undefined)
								{
									setCookie("isHardwareEnabled","1");
									$("#liIsHardware").css("display","block");
									var activex = document.getElementById("GMIIPCmrWebPlugIn1");		
									activex.SetAcceleration(true);
									$("#isHardwareEnabled").val("1");
								}
								//成功返回 1
								var i = activeX.Play();
								if(i!=1)
								{
									location.reload();
								}
								setBuffer(streamId);//设置buffer
								
								
                            }
                        }
                    },
                    error: function() {
                        	if(isAlertUrlError)
			                               {
                                           //   alert("util.js-->GetAndSet()--调用ajax 出错！\n");
			                               }
                    }
                });
			}
				
			
        } catch(e) {
            location.reload();
			//alert(e);
            return;
        }
    }
	$(".btn-set", window.top.document)[0].disabled=false;
	$(".btn-set", window.top.document)[1].disabled=false;
}*/
//视频遮挡加载控件方法
function LoadMaskActiveX(hostname, streamId) {

    if (window.ActiveXObject) {
        if (typeof(isstreamlive) == 'boolean' && isstreamlive == false) {
            if ($("GMIIPCmrWebPlugIn1")) $("GMIIPCmrWebPlugIn1").style.display = "none";
            //alert("当前码流已关闭.");
            return;
        }
        try {
            var activeX = document.getElementById("GMIIPCmrWebPlugIn1");
			activeX.SetUserName(getCookie(oldhost+"gmiusername"),getCookie(oldhost+"gmipassword"));
            var mask = true;
            mask = activeX.showMaskDlg();
			
			
            if (activeX.SetRecvType(0)) {
                activeX.SetHostname(hostname);
                activeX.SetStreamId(streamId);
            }
			else
				{
					alert("设置传输协议失败！");
					location.reload();
				}
			if(parent.tport!="")
			{
			   activeX.SetCmdPort(parent.tport);
			   //是否硬件加速记录到cookie
				var isHardwareEnabled=getCookie("isHardwareEnabled");
				if(isHardwareEnabled==null||isHardwareEnabled==undefined)
				{
					setCookie("isHardwareEnabled","1");
					$("#liIsHardware").css("display","block");
					var activex = document.getElementById("GMIIPCmrWebPlugIn1");		
					activex.SetAcceleration(true);
					$("#isHardwareEnabled").val("1");
				}
               activeX.MaskPlay();
			   setBuffer(streamId);//设置buffer
			}
			else
			{
				$.ajax({
                    url: url,
                    data: "Request=get&Section=PROTOINFO&exsection= 0&postDataStr=" + encodeURIComponent("||"),
                    type: "post",
                    success: function(data) {
                        //alert(data);
                        var data2 = data.replace("\n", "");
                        data2 = $.trim(data2);
                        var result = data2.split('&');
                        var Request;
                        var Section;
                        var Flag;
                        var Info;
                        if (result.length != 4) {
                            alert("返回数据解析错误");
                            return;
                        } else {
                            Request = result[0];
                            Section = result[1];
                            Flag = result[2];
                            Info = result[3];
                            //alert(Info);
                            if (Flag == 0) {
								var index=Info.indexOf("camera_server_port");
								parent.tport=Info.substring(index).split("||")[0].split("=")[1];
							
								activeX.SetCmdPort(parent.tport);
								//是否硬件加速记录到cookie
								var isHardwareEnabled=getCookie("isHardwareEnabled");
								if(isHardwareEnabled==null||isHardwareEnabled==undefined)
								{
									setCookie("isHardwareEnabled","1");
									$("#liIsHardware").css("display","block");
									var activex = document.getElementById("GMIIPCmrWebPlugIn1");		
									activex.SetAcceleration(true);
									$("#isHardwareEnabled").val("1");
								}
                                activeX.MaskPlay();
								setBuffer(streamId);//设置buffer
								
                            }
                        }
                    },
                    error: function() {
                        	if(isAlertUrlError)
			                               {
                                           //   alert("util.js-->GetAndSet()--调用ajax 出错！\n");
			                               }
                    }
                });
			}
        } catch(e) {
            alert(e);
            return;
        }
    }
}

function OnLoadMotionDetectActiveX(hostname, streamId) {
    if (window.ActiveXObject) {
        try {
            if (typeof(isstreamlive) == 'boolean' && isstreamlive == false) {
                if ($("GMIIPCmrWebPlugIn1")) $("GMIIPCmrWebPlugIn1").style.display = "none";
                //alert("当前码流已关闭.");
                return;
            }

            var activeX = document.getElementById("GMIIPCmrWebPlugIn1");
			activeX.SetUserName(getCookie(oldhost+"gmiusername"),getCookie(oldhost+"gmipassword"));
            activeX.MotionDetection();
            if (activeX.SetRecvType(0)) {
                activeX.SetHostname(hostname);
                activeX.SetStreamId(streamId);
            }
			else
				{
					alert("设置传输协议失败！");
					location.reload();
				}
			if(parent.tport!="")
			{
			   activeX.SetCmdPort(parent.tport);
			  //是否硬件加速记录到cookie
				var isHardwareEnabled=getCookie("isHardwareEnabled");
				if(isHardwareEnabled==null||isHardwareEnabled==undefined)
				{
					setCookie("isHardwareEnabled","1");
					$("#liIsHardware").css("display","block");
					var activex = document.getElementById("GMIIPCmrWebPlugIn1");		
					activex.SetAcceleration(true);
					$("#isHardwareEnabled").val("1");
				}
                activeX.MotionPlay();
				setBuffer(streamId);//设置buffer
			}
			else
			{
				
				$.ajax({
                    url: url,
                    data: "Request=get&Section=PROTOINFO&exsection= 0&postDataStr=" + encodeURIComponent("||"),
                    type: "post",
                    success: function(data) {
                        //alert(data);
                        var data2 = data.replace("\n", "");
                        data2 = $.trim(data2);
                        var result = data2.split('&');
                        var Request;
                        var Section;
                        var Flag;
                        var Info;
                        if (result.length != 4) {
                            alert("返回数据解析错误");
                            return;
                        } else {
                            Request = result[0];
                            Section = result[1];
                            Flag = result[2];
                            Info = result[3];
                            //alert(Info);
                            if (Flag == 0) {
								var index=Info.indexOf("camera_server_port");
								parent.tport=Info.substring(index).split("||")[0].split("=")[1];
							
								activeX.SetCmdPort(parent.tport);
								//是否硬件加速记录到cookie
								var isHardwareEnabled=getCookie("isHardwareEnabled");
								if(isHardwareEnabled==null||isHardwareEnabled==undefined)
								{
									setCookie("isHardwareEnabled","1");
									$("#liIsHardware").css("display","block");
									var activex = document.getElementById("GMIIPCmrWebPlugIn1");		
									activex.SetAcceleration(true);
									$("#isHardwareEnabled").val("1");
								}
                                 activeX.MotionPlay();
								 setBuffer(streamId);//设置buffer
								
                            }
                        }
					
                    },
                    error: function() {
                        	if(isAlertUrlError)
			                               {
                                           //   alert("util.js-->GetAndSet()--调用ajax 出错！\n");
			                               }
                    }
                });
			}
           
        } catch(e) {
            alert(e);
            return;
        }
    }
}
//0-255 验证
function checkValue(value) {
    //alert(value);
    var exp = /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
    //var exp = /^(\d{-2,2}|[-1,1]\d\d|[-2,2][0-4]\d|[-25,25][0-5])$/;
    var reg = value.match(exp);
    var ErrMsg = "请按照要求输入！"
    //var Msg = "你输入的是一个合法的IP地址段！"
    if (reg == null) {
        alert(ErrMsg);
        return false;
    } else {
        return true;
    }
}
function check_ip(ip) {
    var i;
    i = ip.length;
    if (i == 0) {
        return false;
        // alert("请输入IP地址"); 
    }
    var re = /^(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9])\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[1-9]|0)\.(25[0-5]|2[0-4][0-9]|[0-1]{1}[0-9]{2}|[1-9]{1}[0-9]{1}|[0-9])$/;
    if (re.test(ip)||ip=="0.0.0.0") {
        return true;
    } else {
        alert("请输入正确的地址!");
        return false;
    }
}
function check_date(date) {
    var re = /^(?:(?!0000)[0-9]{4}-(?:(?:0[1-9]|1[0-2])-(?:0[1-9]|1[0-9]|2[0-8])|(?:0[13-9]|1[0-2])-(?:29|30)|(?:0[13578]|1[02])-31)|(?:[0-9]{2}(?:0[48]|[2468][048]|[13579][26])|(?:0[48]|[2468][048]|[13579][26])00)-02-29)$/
    if (re.test(date)) {
        return true;
    } else {
        alert("请输入正确的日期格式:yyyy-mm-dd");
        return false;
    }

}
function check_time(time) {
    var re = /([01][0-9]|2[0-3]):([0-5][0-9]):([0-5][0-9])/;
    if (re.test(time)) {
        return true;
    } else {
        alert("请输入正确的时间格式:hh:mm:ss");
        return false;
    }
}

function isIntcheckValue(value, isalert) {
    var isalert = isalert || true; //是否弹出提示框
    //alert(value);
    var exp = /^\d+$/;
    //var exp = /^(\d{-2,2}|[-1,1]\d\d|[-2,2][0-4]\d|[-25,25][0-5])$/;
    var reg = value.match(exp);
    var ErrMsg = "请按照要求输入！"
    //var Msg = "你输入的是一个合法的IP地址段！"
    if (reg == null) {
        if (isalert) {
            alert(ErrMsg);
        }
        return false;
    } else {
        return true;
    }
}

//控制码流可选  页面顶部的码流单选按钮控制
function controlEnable(maxStreams) {
	if(parent.isZY&&parent.$("#iframe1").attr("src")=="ylanzkai_1.html")
	{
		for (var streamid = 1; streamid < maxStreams; streamid++) {
			$("#radiostream" + streamid).attr("disabled", "disabled");
			$("#radiostream" + streamid).parent().css("color", "#333");		
		}
	}
	else
	{
		/*
		$.ajax({
			url: url,
			data: "Request=get" + "&Section=ENCODE" + "&exsection=0" + "&postDataStr=" + encodeURIComponent("||"),
			type: "post",
			success: function(data) {
				//通用处理data  
				//处理后的数据 data2
				//返回数据中包含"\n"字符 。
				//$("#load").css("display","none");
				var data2 = data.replace("\n", "");
				data2 = $.trim(data2);
				//异步返回数据  经处理 传递给相应的函数处理。
				var result = data2.split('&');
				var Request;
				var Section;
				var Flag;
				var Info;
				if (result.length != 4) {
					alert("返回数据解析错误");
					return;
				} else {
					Request = result[0];
					Section = result[1];
					Flag = result[2];
					Info = result[3];
					//alert(Info);
				}
				if (Flag == "0") {
					for (var streamid = 0; streamid < maxStreams; streamid++) {
						var index = Info.indexOf("s" + streamid + "_type");
						var temp = Info.substr(index, 9);
						var isopen = temp.split("=")[1];
						// alert(isopen);
						//关闭
						if (isopen == "0") {
							$("#radiostream" + streamid).attr("disabled", "disabled");
							$("#radiostream" + streamid).parent().css("color", "#333");
						}
					}
				}
	
			},
			error: function() {
				if (isAlertUrlError) {
					alert("util.js-->GetAndSet()--调用ajax 出错！\n");
				}
			}
		});*/
	}
}

//切换码流调用

function changeStream_util(streamId, hostname) {
    //切换动作
    //var activex = document.getElementById("GMIIPCmrWebPlugIn1");
        //activex.Stop();
        //activex.SetStreamId(streamid);
      //activex.Play();
    //切换动作结束
    if (window.ActiveXObject) {
        try {

            var activeX = document.getElementById("GMIIPCmrWebPlugIn1");
			activeX.SetUserName(getCookie(oldhost+"gmiusername"),getCookie(oldhost+"gmipassword"));
            //location.reload();
            //activeX.Stop();
            if (activeX.SetRecvType(0)) {
                //alert("ok");
                activeX.SetHostname(hostname);
                //test 
                //alert('streamid : ' + streamId);
                activeX.SetStreamId(streamId);
                activeX.ShowStat(1);
                activeX.ShowDPTZ(1);
            }
			activeX.SetCmdPort(parent.tport);
			//是否硬件加速记录到cookie
				var isHardwareEnabled=getCookie("isHardwareEnabled");
				if(isHardwareEnabled==null||isHardwareEnabled==undefined)
				{
					setCookie("isHardwareEnabled","1");
					$("#liIsHardware").css("display","block");
					var activex = document.getElementById("GMIIPCmrWebPlugIn1");		
					activex.SetAcceleration(true);
					$("#isHardwareEnabled").val("1");
				}
            activeX.Play();
			setBuffer(streamId);//设置buffer
        } catch(e) {
            alert(e);
            return;
        }
    }

}

//获取查询字符串值
function getQueryString(key) {
    var value = "";
    //获取当前文档的URL,为后面分析它做准备
    var sURL = window.document.URL;
    //URL中是否包含查询字符串
    if (sURL.indexOf("?") > 0)

    {
        //分解URL,第二的元素为完整的查询字符串
        //即arrayParams[1]的值为【first=1&second=2】
        var arrayParams = sURL.split("?");
        //分解查询字符串
        //arrayURLParams[0]的值为【first=1 】
        //arrayURLParams[2]的值为【second=2】
        var arrayURLParams = arrayParams[1].split("&");

        //遍历分解后的键值对
        for (var i = 0; i < arrayURLParams.length; i++) {
            //分解一个键值对
            var sParam = arrayURLParams[i].split("=");
            if ((sParam[0] == key) && (sParam[1] != "")) {
                //找到匹配的的键,且值不为空
                value = sParam[1];
                break;
            }
        }
    }
    return value;
}
//获取字符串字节数
function getlengthB(str) {
    return str.replace(/[^\x00-\xff]/g, "***").length;
}

//数组是否包含某个元素 添加contains 属性
Array.prototype.contains = function(element) {
    for (var i = 0; i < this.length; i++) {
        if (this[i] == element) {
            return true;
        }
    }
    return false;
}
//事件设置中是否显示 视频录像可用与否。
function isorNoShowRec() {
    var sect = 'SDINFO';
    var postDataStr = "";
    $.ajax({
        url: url,
        data: "Request=get&Section=" + sect + "&exsection= 0&postDataStr=" + encodeURIComponent(postDataStr + "||"),
        type: "post",
        success: function(data) {
            //alert(data);
            var data2 = data.replace("\n", "");
            data2 = $.trim(data2);
            var result = data2.split('&');
            var Request;
            var Section;
            var Flag;
            var Info;
            if (result.length != 4) {
                alert("返回数据解析错误");
                return;
            } else {
                Request = result[0];
                Section = result[1];
                Flag = result[2];
                Info = result[3];
                //alert(Info);
                if (Flag == 0) {
                    var infos = Info.split("||");
                    var status = infos[0].split("=")[1];
                    var rw = infos[1].split("=")[1];
                    var space = infos[2].split("=")[1];
                    var free_space = infos[3].split("=")[1];
                    var CNstatus = "";
                    if (status == 0) //无SDK
                    {
                        $("#rec_enable").attr("disabled", "disabled");
                        $("#rec_enable").parent().css("color", "#333");
                    }
                }
            }
        }
    });
}
//定时录像是否可用
function isorNoTimeRecEnable() {
    var sect = 'SDINFO';
    var postDataStr = "";
    $.ajax({
        url: url,
        data: "Request=get&Section=" + sect + "&exsection= 0&postDataStr=" + encodeURIComponent(postDataStr + "||"),
        type: "post",
        success: function(data) {
            //alert(data);
            var data2 = data.replace("\n", "");
            data2 = $.trim(data2);
            var result = data2.split('&');
            var Request;
            var Section;
            var Flag;
            var Info;
            if (result.length != 4) {
                alert("返回数据解析错误");
                return;
            } else {
                Request = result[0];
                Section = result[1];
                Flag = result[2];
                Info = result[3];
                //alert(Info);
                if (Flag == 0) {
                    var infos = Info.split("||");
                    var status = infos[0].split("=")[1];
                    var rw = infos[1].split("=")[1];
                    var space = infos[2].split("=")[1];
                    var free_space = infos[3].split("=")[1];
                    var CNstatus = "";
                    if (status == 0) //无SDK
                    {
						$("#infodiv").css("display","block");
                        $.modal("<div></div>", {
					closeHTML: "",
					dataCss:{background:'none'},
					overlayCss: {
						backgroundColor:"#242424"
						},
					containerCss: {
						height: 0,
						//padding:0,
						width: 0,
						border:0
					}
				});
                    }
                }
            }
        }
    });
}


//JS 弹出文件保存窗口

function  saveFile() 
{ 
      fileDialog.CancelError=true; 
      try{ 
      fileDialog.Filter="录像文件 (*.264)|*.264|All Type   (*.*)|*.*"; 
      fileDialog.ShowSave(); 
	  return fileDialog.filename;
      } 
      catch(e){
			  //取消
			  if(e.message=="Cancel was selected.")
			  {
				return null;
			  }
			  else
			  {
				return e.message;
			  }
		  } 
} 
//根据码流数。动态设置顶部码流切换信息
function maxStreamsStyle(maxStreams,webindex)
{
	//var curUrl=getCookie(window.location.host + "currenturl");
	//if(curUrl.indexOf("inf_superimposed_1.html")<0)
	if(webindex == 0)//ylanzkai_1.html
	{
	var stream2html='<dd><input type="radio" name="bitstream" value="1" id="radiostream1"/><label id="2stream">码流</label><label>2</label></dd>';
	var stream3html='<dd><input type="radio" name="bitstream" value="2" id="radiostream2"/><label id="3stream">码流</label><label>3</label></dd>';
	var stream4html='<dd><input type="radio" name="bitstream" value="3" id="radiostream3"/><label id="4stream">码流</label><label>4</label></dd>';
	var obj=$("#streamInfo");
	if(obj!=null)
	{
		if(parent.StreamNum_NLJ==2)
		{
			obj.html(obj.html()+stream2html);
		}
		if(parent.StreamNum_NLJ==3)
		{
			obj.html(obj.html()+stream2html+stream3html);
		}
		if(parent.StreamNum_NLJ==4)
		{
			obj.html(obj.html()+stream2html+stream3html+stream4html);
		}
		var smax = parseInt(maxStreams);
		switch(smax)
		{
			case 1:
			$("#radiostream" + 1).attr("disabled", "disabled");
			$("#radiostream" + 1).parent().css("color", "#333");	
			$("#radiostream" + 2).attr("disabled", "disabled");
			$("#radiostream" + 2).parent().css("color", "#333");	
			$("#radiostream" + 3).attr("disabled", "disabled");
			$("#radiostream" + 3).parent().css("color", "#333");		
			break;
			case 2:
			$("#radiostream" + 2).attr("disabled", "disabled");
			$("#radiostream" + 2).parent().css("color", "#333");	
			$("#radiostream" + 3).attr("disabled", "disabled");
			$("#radiostream" + 3).parent().css("color", "#333");	
			break;
			case 3:
			$("#radiostream" + 3).attr("disabled", "disabled");
			$("#radiostream" + 3).parent().css("color", "#333");
			break;
			case 4:	
			default:
			break;
		}

	}
	 var streamid=getQueryString("streamid");
	 $("#radiostream" + streamid).attr("checked", "checked");
	//绑定事件
	 $("input[type=radio][name='bitstream']").bind("click",
                function(e) {
                    var streamvalue = this.value;
                    changeStream(streamvalue);
      });
	//控制码流可选  页面顶部的码流单选按钮控制
     //controlEnable(maxStreams);
	}
	else //if(webindex == 1)//camera_1.html
	{
		var stream2html='<dd><input type="radio" name="bitstream" value="1" id="radiostream1"/><label id="2stream">码流</label><label>2</label></dd>';
	var stream3html='<dd><input type="radio" name="bitstream" value="2" id="radiostream2"/><label id="3stream">码流</label><label>3</label></dd>';
	var stream4html='<dd><input type="radio" name="bitstream" value="3" id="radiostream3"/><label id="4stream">码流</label><label>4</label></dd>';
	var cvbshtml='<dd><input type="radio" name="bitstream" value="4" id="radiostream4"/><label id="5stream"></label><label>cvbs</label></dd>';
	var obj=$("#streamInfo");
	if(obj!=null)
	{
		if(parent.StreamNum_NLJ==2)
		{
			obj.html(obj.html()+stream2html);
		}
		if(parent.StreamNum_NLJ==3)
		{
			obj.html(obj.html()+stream2html+stream3html);
		}
		if(parent.StreamNum_NLJ==4)
		{
			obj.html(obj.html()+stream2html+stream3html+stream4html+cvbshtml);
		}
		var smax = parseInt(maxStreams);
		switch(smax)
		{
			case 1:
			$("#radiostream" + 1).attr("disabled", "disabled");
			$("#radiostream" + 1).parent().css("color", "#333");	
			$("#radiostream" + 2).attr("disabled", "disabled");
			$("#radiostream" + 2).parent().css("color", "#333");	
			$("#radiostream" + 3).attr("disabled", "disabled");
			$("#radiostream" + 3).parent().css("color", "#333");		
			break;
			case 2:
			$("#radiostream" + 2).attr("disabled", "disabled");
			$("#radiostream" + 2).parent().css("color", "#333");	
			$("#radiostream" + 3).attr("disabled", "disabled");
			$("#radiostream" + 3).parent().css("color", "#333");	
			break;
			case 3:
			$("#radiostream" + 3).attr("disabled", "disabled");
			$("#radiostream" + 3).parent().css("color", "#333");
			break;
			case 4:	
			default:
			break;
		}
	}
	 var streamid=getQueryString("streamid");
	 $("#radiostream" + streamid).attr("checked", "checked");
	//绑定事件
	 $("input[type=radio][name='bitstream']").bind("click",
                function(e) {
					
                    var streamvalue = this.value;
                    changeStream(streamvalue);
      });
	//控制码流可选  页面顶部的码流单选按钮控制
     controlEnable(maxStreams);
	}
}

//根据设备能力集码流数设置界面码流按钮
function SetStreamNum_NLJ(StreamNum_NLJ)
{
	if(!isNaN(StreamNum_NLJ))
	{
		return;	
	}	
	switch(StreamNum_NLJ)
	{
		case 1:
			$("#radiostream" + 1).style.display = "none";
			$("#radiostream" + 2).style.display = "none";
			$("#radiostream" + 3).style.display = "none";
		break;
		case 2:
			$("#radiostream" + 2).style.display = "none";
			$("#radiostream" + 3).style.display = "none";
		break;
		case 3:
			$("#radiostream" + 3).style.display = "none";
		break;
		case 4:
		break;
		default:
		break;
	}
}
//---3.0获取码流
function getMaxStreamNum(onlyData,SessionId,AuthValue,webindex)
{
/*	var onlyData=onlyData||false;
	var maxStreams=parent.maxStreams;
	if(parent.maxStreams!=null)
	{
		if(onlyData)
		{
			return maxStreams;
		}
		 maxStreamsStyle(maxStreams,webindex);
	}
	else
	{*/
		var sect = 'DEVCAP';
    	var postDataStr = "";
		var  content={};
		content.SessionId=SessionId;
		content.AuthValue=AuthValue;

		contentstr=jsonToString(content);
		GetAndSet("GetEncodeStreamNum","json",contentstr, sucess_getStream);
//	}	
}
//3.0获取码流数目成功函数
function sucess_getStream(data)
{
            //alert(data);
            var data2 = data.replace("\n", "");
            data2 = $.trim(data2);
            var result = data2.split('&');
            var Request;
            var Section;
            var Flag;
            var Info;
            if (result.length != 3)
			{
                alert("返回数据解析错误");
                return;
            }
			else
			{

				var PackDate=new UnPackData(data);
				if(PackDate.Content!=null)
				{
					if(PackDate.Content.RetCode=="0")
					{
						//提取码流数目
						var num = PackDate.Content.StreamNum;
						parent.maxStreams = num;
						//
						//默认是ylanzkai_1.html
						maxStreamsStyle(num,0);
					}
				}

            }	
}
//获取当前设备码流数
function getMaxStreams(onlyData)
{
	var onlyData=onlyData||false;
	var maxStreams=parent.maxStreams;
	if(parent.maxStreams!=null)
	{
		if(onlyData)
		{
			return maxStreams;
		}
		 maxStreamsStyle(maxStreams);
	}
	else
	{
	var sect = 'DEVCAP';
    var postDataStr = "";
    $.ajax({
        url: url,
        data: "Request=get&Section=" + sect + "&exsection= 0&postDataStr=" + encodeURIComponent(postDataStr + "||"),
        type: "post",
        success: function(data) {
            //alert(data);
            var data2 = data.replace("\n", "");
            data2 = $.trim(data2);
            var result = data2.split('&');
            var Request;
            var Section;
            var Flag;
            var Info;
            if (result.length != 4) {
                alert("返回数据解析错误");
                return;
            } else {
                Request = result[0];
                Section = result[1];
                Flag = result[2];
                Info = result[3];
                //alert(Info);
                if (Flag == 0) {
                    var infos = Info.split("||");
                    var maxsteams = infos[0].split("=")[1];//支持码流数
					parent.maxStreams=maxsteams;
					if(onlyData)
					{
						return maxStreams;
					}
                    maxStreamsStyle(maxsteams);
                }
            }
        }
    });
	}
}
//时间戳
function getLocalFrormatTime()
{
	var date=new Date();
	var year= date.getFullYear();
	var month=date.getMonth()+1;
	var day=date.getDate();
	var hour=date.getHours();
	var minute=date.getMinutes();
	var sec=date.getSeconds();
	var strmonth=(month.toString().length==2 ? month:"0"+month);
	var strday=(day.toString().length==2 ? day:"0"+day);
	var strhour=(hour.toString().length==2 ? hour:"0"+hour);
	var strminute=(minute.toString().length==2 ? minute:"0"+minute);
	var strsec=(sec.toString().length==2 ? sec:"0"+sec);
	return date.getFullYear()+""+strmonth+strday+strhour+strminute+strsec+"_"+date.getMilliseconds();
}


//设置缓冲buffer
function setBuffer(streamId)
			{
				var rateItem=[[60, 8533333], [30, 17066667],[25,20480000],[24,21333333],[23,22260869],[22,23272727],[21,24380952],[20,25600000],[19,26947368],[18,28444444],[17,30117647],[16,32000000],[15,34133333],[14,36571428],[13,39384615],[12,42666666],[11,46545454],[10,51200000],[9,56888888],[8,64000000],[7,73142857],[6,85333333],[5,102400000],[4,128000000],[3,170666667],[2,256000000],[1,512000000]];
				//获取帧率
			var section = 'ENCODE';
                postDataStr = "";
                $.ajax({
                    url: url,
                    data: "Request=get&Section=" + section + "&exsection= 0&postDataStr=" + encodeURIComponent(postDataStr + "||"),
                    type: "post",
                    success: function(data) {
                        //alert(data);
                        var data2 = data.replace("\n", "");
                        data2 = $.trim(data2);
                        if (data2.substring(0, 3) == "<br") {
                            alert("返回数据出错：\n" + data2);
                            return;
                        }
                        var result = data2.split('&');
                        var Request;
                        var Section;
                        var Flag;
                        var Info;
                        if (result.length != 4) {
                            alert("返回数据解析错误");
                            return;
                        } else {
                            Request = result[0];
                            Section = result[1];
                            Flag = result[2];
                            Info = result[3];
                            //alert(Info);
                            if (Flag == 0) {
                               var splitIndexItem="s"+streamId+"_enc_fps";
							   var index=Info.indexOf(splitIndexItem);
							   var rate=30;//帧率默认30
							   var fps=Info.substring(index).split("||")[0].split("=")[1];
							   for(var i=0;i<rateItem.length;i++)
							   {
								   if(rateItem[i][1]==fps||rateItem[i][1]+""==fps)
								   {
									   rate=rateItem[i][0];
									   break;
								   }
							   }
                            }
                        }
                        //
						var activex = document.getElementById("GMIIPCmrWebPlugIn1");
					   // if (activex != null && activex != 'null') {
						//if(activex)
						//	activex.Play();
					     // }
						
						   var frame=getCookie("frame");
							if(isNaN(frame)||frame==undefined)
							{
								
								//frame=activex.GetBufferSizeEx();
								frame=2;
								
							}
						   if(frame!=null)
						   {
							   if(frame==1)
							   {
								   frame=1;
							   }
							   else
							   {
                                   frame=Math.ceil(frame*(rate/30));
							   }
						   }
						   else
						   {
							   frame=Math.ceil(frame*(rate/30));
							   //frame=activex.GetBufferSize();
						   }
						   //启用流畅性
						   if(!isNaN(frame))
						   {
							
						   		var res=activex.SetBufferSize(frame);
								//alert("设置值="+frame+"==设置成功？："+res+"==获取的值："+activex.GetBufferSizeEx());
						   }
						  // alert(res+"==="+frame);
                    },
                    error: function() {
                        if (isAlertUrlError) {
                           // alert("util.js-->GetAndSet()--调用ajax 出错！\n");
                        }
                    }
                });
			//获取帧率结束
			}
			
/*
* 端口号校验
*/			
function f_check_port(obj)
 {
     if(!f_check_number(obj))
         return false;
     if(obj.value < 65536&&obj.value>-1)
         return true;
     alert("请输入合法的计算机ip地址端口号");
     return false; 
}
/* 
* 判断是否为数字，是则返回true,否则返回false 
*/
 function f_check_number(obj)   
{          
    if(/^[0-9]*$/.test(obj.value))   
    {   
       return true;   
    }    
    else    
    {   
       return false;   
    }   
}   
/* 
* 判断是否含有特殊字符e 
*/
 function f_check_valid(obj)   
{          
    if(/^[^='"&\r\n\[\]]*$/.test(obj.value))
    {   
       return true;   
    }    
    else    
    {   
	   alert("内容不要包括特殊字符：[,],=,&,\',\",等特殊字符。");
       return false;   
    }   
} 
	function disableElement(obj,disabled)
			{
				$("input,select",obj).each(function()
				{
					if(disabled)
					{
						this.setAttribute("disabled","disabled");
					}
					else
					{
						if(this.id!="54out_levels")
						this.disabled=false;
					}
				});
			}
			
			
/**  
* 时间对象的格式化  
*/  
Date.prototype.format = function(format)  
{  
/*  
* format="yyyy-MM-dd hh:mm:ss";  
*/  
var o = {  
"M+" : this.getMonth() + 1,  
"d+" : this.getDate(),  
"h+" : this.getHours(),  
"m+" : this.getMinutes(),  
"s+" : this.getSeconds(),  
"q+" : Math.floor((this.getMonth() + 3) / 3),  
"S" : this.getMilliseconds()  
}  
  
if (/(y+)/.test(format))  
{  
format = format.replace(RegExp.$1, (this.getFullYear() + "").substr(4  
- RegExp.$1.length));  
}  
  
for (var k in o)  
{  
if (new RegExp("(" + k + ")").test(format))  
{  
format = format.replace(RegExp.$1, RegExp.$1.length == 1  
? o[k]  
: ("00" + o[k]).substr(("" + o[k]).length));  
}  
}  
return format;  
}  

Date.prototype.DateAdd = function(strInterval, Number) 
{ 
var dtTmp = this; 
switch (strInterval) { 
case 's' :return new Date(Date.parse(dtTmp) + (1000 * Number)); 
case 'n' :return new Date(Date.parse(dtTmp) + (60000 * Number)); 
case 'h' :return new Date(Date.parse(dtTmp) + (3600000 * Number)); 
case 'd' :return new Date(Date.parse(dtTmp) + (86400000 * Number)); 
case 'w' :return new Date(Date.parse(dtTmp) + ((86400000 * 7) * Number)); 
case 'q' :return new Date(dtTmp.getFullYear(), (dtTmp.getMonth()) + Number*3, dtTmp.getDate(), dtTmp.getHours(), dtTmp.getMinutes(), dtTmp.getSeconds()); 
case 'm' :return new Date(dtTmp.getFullYear(), (dtTmp.getMonth()) + Number, dtTmp.getDate(), dtTmp.getHours(), dtTmp.getMinutes(), dtTmp.getSeconds()); 
case 'y' :return new Date((dtTmp.getFullYear() + Number), dtTmp.getMonth(), dtTmp.getDate(), dtTmp.getHours(), dtTmp.getMinutes(), dtTmp.getSeconds()); 
} 
} 




xmlToJson={
    parser:function(xmlcode,ignoretags,debug){
        if(!ignoretags){ignoretags=""};
        xmlcode=xmlcode.replace(/\s*\/>/g,'/>');
        xmlcode=xmlcode.replace(/<\?[^>]*>/g,"").replace(/<\![^>]*>/g,"");
        if (!ignoretags.sort){ignoretags=ignoretags.split(",")};
        var x=this.no_fast_endings(xmlcode);
        x=this.attris_to_tags(x);
        x=escape(x);
        x=x.split("%3C").join("<").split("%3E").join(">").split("%3D").join("=").split("%22").join("\"");
        for (var i=0;i<ignoretags.length;i++){
            x=x.replace(new RegExp("<"+ignoretags[i]+">","g"),"*$**"+ignoretags[i]+"**$*");
            x=x.replace(new RegExp("</"+ignoretags[i]+">","g"),"*$***"+ignoretags[i]+"**$*")
        };
        x='<JSONTAGWRAPPER>'+x+'</JSONTAGWRAPPER>';
        this.xmlobject={};
        var y=this.xml_to_object(x).jsontagwrapper;
        if(debug){y=this.show_json_structure(y,debug)};
        return y
    },
    xml_to_object:function(xmlcode){
        var x=xmlcode.replace(/<\//g,'?');
        x=x.split("<");
        var y=[];
        var level=0;
        var opentags=[];
        for (var i=1;i<x.length;i++){
            var tagname=x[i].split(">")[0];
            opentags.push(tagname);
            level++
            y.push(level+"<"+x[i].split("?")[0]);
            while(x[i].indexOf("?"+opentags[opentags.length-1]+">")>=0){level--;opentags.pop()}
        };
        var oldniva=-1;
        var objname="this.xmlobject";
        for (var i=0;i<y.length;i++){
            var preeval="";
            var niva=y[i].split("<")[0];
            var tagnamn=y[i].split("<")[1].split(">")[0];
            tagnamn=tagnamn.toLowerCase();
            var rest=y[i].split(">")[1];
            if(niva<=oldniva){
                var tabort=oldniva-niva+1;
                for (var j=0;j<tabort;j++){objname=objname.substring(0,objname.lastIndexOf("."))}
            };
            objname+="."+tagnamn;
            var pobject=objname.substring(0,objname.lastIndexOf("."));
            if (eval("typeof "+pobject) != "object"){preeval+=pobject+"={value:"+pobject+"};\n"};
            var objlast=objname.substring(objname.lastIndexOf(".")+1);
            var already=false;
            for (k in eval(pobject)){if(k==objlast){already=true}};
            var onlywhites=true;
            for(var s=0;s<rest.length;s+=3){
                if(rest.charAt(s)!="%"){onlywhites=false}
            };
            if (rest!="" && !onlywhites){
                if(rest/1!=rest){
                    rest="'"+rest.replace(/\'/g,"\\'")+"'";
                    rest=rest.replace(/\*\$\*\*\*/g,"</");
                    rest=rest.replace(/\*\$\*\*/g,"<");
                    rest=rest.replace(/\*\*\$\*/g,">")
                }
            } 
            else {rest="{}"};
            if(rest.charAt(0)=="'"){rest='unescape('+rest+')'};
            if (already && !eval(objname+".sort")){preeval+=objname+"=["+objname+"];\n"};
            var before="=";after="";
            if (already){before=".push(";after=")"};
            var toeval=preeval+objname+before+rest+after;
            eval(toeval);
            if(eval(objname+".sort")){objname+="["+eval(objname+".length-1")+"]"};
            oldniva=niva
        };
        return this.xmlobject
    },
    show_json_structure:function(obj,debug,l){
        var x='';
        if (obj.sort){x+="[\n"} else {x+="{\n"};
        for (var i in obj){
            if (!obj.sort){x+=i+":"};
            if (typeof obj[i] == "object"){
                x+=this.show_json_structure(obj[i],false,1)
            }
            else {
                if(typeof obj[i]=="function"){
                    var v=obj[i]+"";
                    //v=v.replace(/\t/g,"");
                    x+=v
                }
                else if(typeof obj[i]!="string"){x+=obj[i]+",\n"}
                else {x+="'"+obj[i].replace(/\'/g,"\\'").replace(/\n/g,"\\n").replace(/\t/g,"\\t").replace(/\r/g,"\\r")+"',\n"}
            }
        };
        if (obj.sort){x+="],\n"} else {x+="},\n"};
        if (!l){
            x=x.substring(0,x.lastIndexOf(","));
            x=x.replace(new RegExp(",\n}","g"),"\n}");
            x=x.replace(new RegExp(",\n]","g"),"\n]");
            var y=x.split("\n");x="";
            var lvl=0;
            for (var i=0;i<y.length;i++){
                if(y[i].indexOf("}")>=0 || y[i].indexOf("]")>=0){lvl--};
                tabs="";for(var j=0;j<lvl;j++){tabs+="\t"};
                x+=tabs+y[i]+"\n";
                if(y[i].indexOf("{")>=0 || y[i].indexOf("[")>=0){lvl++}
            };
            if(debug=="html"){
                x=x.replace(/</g,"&lt;").replace(/>/g,"&gt;");
                x=x.replace(/\n/g,"<BR>").replace(/\t/g,"&nbsp;&nbsp;&nbsp;&nbsp;")
            };
            if (debug=="compact"){x=x.replace(/\n/g,"").replace(/\t/g,"")}
        };
        return x
    },
    no_fast_endings:function(x){
        x=x.split("/>");
        for (var i=1;i<x.length;i++){
            var t=x[i-1].substring(x[i-1].lastIndexOf("<")+1).split(" ")[0];
            x[i]="></"+t+">"+x[i]
        }    ;
        x=x.join("");
        return x
    },
    attris_to_tags: function(x){
        var d=' ="\''.split("");
        x=x.split(">");
        for (var i=0;i<x.length;i++){
            var temp=x[i].split("<");
            for (var r=0;r<4;r++){temp[0]=temp[0].replace(new RegExp(d[r],"g"),"_jsonconvtemp"+r+"_")};
            if(temp[1]){
                temp[1]=temp[1].replace(/'/g,'"');
                temp[1]=temp[1].split('"');
                for (var j=1;j<temp[1].length;j+=2){
                    for (var r=0;r<4;r++){temp[1][j]=temp[1][j].replace(new RegExp(d[r],"g"),"_jsonconvtemp"+r+"_")}
                };
                temp[1]=temp[1].join('"')
            };
            x[i]=temp.join("<")
        };
        x=x.join(">");
        x=x.replace(/ ([^=]*)=([^ |>]*)/g,"><$1>$2</$1");
        x=x.replace(/>"/g,">").replace(/"</g,"<");
        for (var r=0;r<4;r++){x=x.replace(new RegExp("_jsonconvtemp"+r+"_","g"),d[r])}    ;
        return x
    }
};


if(!Array.prototype.push){
    Array.prototype.push=function(x){
        this[this.length]=x;
        return true
    }
};

if (!Array.prototype.pop){
    Array.prototype.pop=function(){
          var response = this[this.length-1];
          this.length--;
          return response
    }
};


function jsonToString (obj){   
        var THIS = this;    
        switch(typeof(obj)){   
            case 'string':   
                return '"' + obj.replace(/(["\\])/g, '\\$1') + '"';   
            case 'array':   
                return '[' + obj.map(THIS.jsonToString).join(',') + ']';   
            case 'object':   
                 if(obj instanceof Array){   
                    var strArr = [];   
                    var len = obj.length;   
                    for(var i=0; i<len; i++){   
                        strArr.push(THIS.jsonToString(obj[i]));   
                    }   
                    return '[' + strArr.join(',') + ']';   
                }else if(obj==null){   
                    return 'null';   
  
                }else{   
                    var string = [];   
                    for (var property in obj) string.push(THIS.jsonToString(property) + ':' + THIS.jsonToString(obj[property]));   
                    return '{' + string.join(',') + '}';   
                }   
            case 'number':   
                return obj;   
            case false:   
                return obj;   
        }   
    }
	
/*************************************************
Function:		parseXml
Description:	从xml文件中解析xml
Input:			无
Output:			无
return:			xmlDoc				
*************************************************/
function parseXml(fileRoute)
{
	var st = "";
	/*
	$.ajax({
		url: fileRoute,
		dataType: "xml",
		type: "get",
		async: false,
        success: function(data) {
			st = data;
        },
        error: function(e) {
         st = "<FileVersion><Platform name=win32><PreviewOCX.ocx>1,0,0,1</PreviewOCX.ocx><GIPCQuartz.dll>2,5,4,20629</GIPCQuartz.dll><GIPCPlayer.dll>2,5,4,30621</GIPCPlayer.dll><GIPCamera.dll>3,0,0,30918</GIPCamera.dll></Platform></FileVersion>";  
        }
	});
	*/
         st = "<xml version='1.0' encoding='utf-8'><FileVersion><Platform name=win32><PreviewOCX.ocx>1,0,0,7</PreviewOCX.ocx><GIPCQuartz.dll>2,5,4,20629</GIPCQuartz.dll><GIPCPlayer.dll>2,5,4,30621</GIPCPlayer.dll><GIPCamera.dll>3,0,0,30918</GIPCamera.dll></Platform></FileVersion>"; 
	return st;
}
/*************************************************
Function:		xmlToStr
Description:	xml转换字符串
Input:			Xml xml文档
Output:			无
return:			字符串				
*************************************************/
function xmlToStr(Xml)
{
	if(Xml == null)
	{
	    return;	
	}
	var XmlDocInfo = "";
	if(navigator.appName == "Netscape" || navigator.appName == "Opera")
	{
		var oSerializer = new XMLSerializer();
		XmlDocInfo = oSerializer.serializeToString(Xml);
	}
	else
	{
		XmlDocInfo = Xml;
	}
	//if(XmlDocInfo.indexOf('<?xml') == -1)
	//{
	//	XmlDocInfo = "<?xml version='1.0' encoding='utf-8'?>" + XmlDocInfo;
	//}
	return XmlDocInfo;
}
/*************************************************
Function:		UpdateTips
Description:	更新提示
Input:			无
Output:			无
return:			无
*************************************************/
function UpdateTips()
{
	/*
	var bUpdateTips = $.cookie('updateTips');
	var szUpdate = '';
	if(bUpdateTips == 'true'){
		if(navigator.platform == "Win32") {
			szUpdate = getNodeValue('jsUpdatePlugin');
			Warning =confirm(szUpdate);
			if (Warning) {
				window.open("../../codebase/WebComponents.exe","_self");
			} else {
				$.cookie('updateTips', 'false');
			}
		} else {
			szUpdate = getNodeValue('jsUpdateNotWin32');
			setTimeout(function() {alert(szUpdate);},20);
			$.cookie('updateTips', 'false');
		}
	}
	*/
	alert("控件需要更新请点击下载安装，安装过程中请关闭浏览器，安装完成之后请重启浏览器！");
	window.open("/activeX/WebPluginSetup.exe","_self");
	//window.close();
}
	//判断控件版本
	function ActiveXCtr()
	{
		var stmp = CompareFileVersion();
		//需要更新控件
		if(!stmp)
		{
			UpdateTips();
			return false;	
		}
		//不需要更新控件
		else(stmp)
		{
			
		}
		return true;
	}
	function CompareFileVersion()
	{
		var activeX = document.getElementById("GMIIPCmrWebPlugIn1");
		if(activeX == null)
		{
			return false;
		}
		//检测控件是否正确安装
		//try
		//{
			//activeX.SetLoginInfo(getCookie("gmiwebusername"),getCookie("gmiwebpassword"),parent.ip,parent.port);
			//activeX.AboutBox();
		//}
		//catch(e)
		//{
		//	alert("Login error");
		//	return false;	
		//}

		var xmlDoc=parseXml("/xml/version.xml");
		var szXml = xmlToStr(xmlDoc);
		var bRes = -2;
		try
		{
			//alert("ComparePlatform begin");
			bRes = activeX.ComparePlatform(szXml);
			//alert("ComparePlatform end");
			//版本一致
			if(bRes == 0)
			{
				return true;
			}
			//比PC版本低 控件高一点
			else if(bRes == -1)
			{
				return true;
			}
			//比PC版本高 设备高一点
			else if(bRes == 1)
			{
				return false;	
			}
			
		}
		catch(e)
		{
			//alert("ComparePlatform error");
			return false;	
		}
	}




