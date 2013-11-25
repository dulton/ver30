
 //自动关闭提示框, 请用新页面替换
            function timeoutalert(getStrfunc, secs, closefunc) {
                if (typeof(getStrfunc) !== "function" || typeof(secs) !== "number") return;
                //function timeoutalert(msg) {
                var msgw, msgh, bordercolor;
                msgw = 500; //提示窗口的宽度
                msgh = 120; //提示窗口的高度
                titleheight = 25 //提示窗口标题高度
                bordercolor = "#000"; //提示窗口的边框颜色
                titlecolor = "#99CCFF"; //提示窗口的标题颜色
				background ="#242424";
                var sWidth, sHeight;
                //获取当前窗口尺寸
                sWidth = document.body.offsetWidth;
				//sWidth=900;
                sHeight = document.body.offsetHeight > document.body.clientHeight ? document.body.offsetHeight: document.body.clientHeight;
                sHeight = sHeight > window.screen.availHeight ? sHeight: window.screen.availHeight;
				
                //    //背景div
                var bgObj = document.createElement("div");
                bgObj.setAttribute('id', 'alertbgDiv');
                bgObj.style.position = "absolute";
                bgObj.style.top = "0";
                bgObj.style.background = "#E8E8E8";
                bgObj.style.filter = "progid:DXImageTransform.Microsoft.Alpha(style=3,opacity=25,finishOpacity=75";
                bgObj.style.opacity = "0.6";
                bgObj.style.left = "0";
                bgObj.style.width = sWidth + "px";
                bgObj.style.height = sHeight + "px";
                bgObj.style.zIndex = "10000";
                document.body.appendChild(bgObj);
                //创建提示窗口的div
                var msgObj = document.createElement("div");
				msgObj.setAttribute("id", "alertmsgDiv");
                msgObj.setAttribute("align", "center");
                msgObj.style.background = background;
                msgObj.style.border = "1px solid " + bordercolor;
                msgObj.style.position = "absolute";
                msgObj.style.left = "50%";
				msgObj.style.color = "#ccc";
                msgObj.style.font = "12px/1.6em Verdana, Geneva, Arial, Helvetica, sans-serif";
                //窗口距离左侧和顶端的距离
                msgObj.style.marginLeft = "-225px";
                //窗口被卷去的高+（屏幕可用工作区高/2）-150
                msgObj.style.top = document.body.scrollTop + (window.screen.availHeight / 2) - 150 + "px";
                msgObj.style.width = msgw + "px";
                msgObj.style.height = msgh + "px";
                msgObj.style.textAlign = "center";
                msgObj.style.lineHeight = "25px";
                msgObj.style.zIndex = "10001";
                document.body.appendChild(msgObj);
                //提示信息标题
                var title = document.createElement("h4");
                title.setAttribute("id", "alertmsgTitle");
                title.setAttribute("align", "left");
                title.style.margin = "0";
                title.style.padding = "3px";
                title.style.background = bordercolor;
                title.style.filter = "progid:DXImageTransform.Microsoft.Alpha(startX=20, startY=20, finishX=100, finishY=100,style=1,opacity=75,finishOpacity=100);";
                title.style.opacity = "0.75";
                title.style.border = "1px solid " + bordercolor;
                title.style.height = "18px";
                title.style.font = "12px Verdana, Geneva, Arial, Helvetica, sans-serif";
                title.style.color = "white";
                title.innerHTML = "提示信息";
                document.getElementById("alertmsgDiv").appendChild(title);
                //提示信息
                var txt = document.createElement("p");
                txt.setAttribute("id", "msgTxt");
                txt.style.margin = "16px 0";
                txt.innerHTML = getStrfunc(); //msg;
                document.getElementById("alertmsgDiv").appendChild(txt);
                //设置关闭时间
                var chmsgid = 0;
                var chmsg = function() {
                    var txt = document.getElementById("msgTxt");
                    if (txt) txt.innerHTML = getStrfunc();
                    chmsgid = setTimeout(chmsg, 1000);
                }
                chmsgid = setTimeout(chmsg, 1000);
                var closeid = 0,
                closefuncid = 0;
                var closewin = function() {
                    try {
                        document.body.removeChild(document.getElementById("alertbgDiv"));
                        document.getElementById("alertmsgDiv").removeChild(document.getElementById("alertmsgTitle"));
                        document.body.removeChild(document.getElementById("alertmsgDiv"));
                    } catch(e) {
                        //ignore this error
                    }
                    if (chmsgid !== 0) window.clearTimeout(chmsgid);
                    if (closeid !== 0) window.clearTimeout(closeid);
                    if (closefuncid !== 0) window.clearTimeout(closefuncid);
                }
                if (typeof(closefunc) === 'function') closefuncid = window.setTimeout(closefunc, secs * 1000 - 10);
                //make sure closewin run later than closefunc for normal case
                closeid = window.setTimeout(closewin, secs * 1000);
                return closewin;
            }
            //交互数据打包
            function Pack() {
                this.mem = '';
                this.add = function(name, value) {
                    this.mem += name + "=" + value + "||"
                }
            }
            //响应数据解包
            function RequestResult(Request, Section, Flag, Info, Extension) {
                this.Request = Request;
                this.Section = Section;
                this.Flag = Flag;
                this.Info = Info;
                this.Extension = Extension;
				this.params = null;
				this.getParams = function(){ //解析参数
					if(this.params != null)return this.params;
					this.params = new Array();
					var pairs = this.Info.split('||');
					for (var i = 0; i < pairs.length; i++){
						var pair = pairs[i].split('=');
						if(pair.length == 2)
							this.params[pair[0]] = pair[1];
					}
					return this.params;
				}
				this.filterPageParam = function(Param){	//转换为页面参数
					var pairs = this.Info.split('||');
					for (var i = 0; i < pairs.length; i++){
						var pair = pairs[i].split('=');
						if(pair.length == 2){
							//this.params[pair[0]] = pair[1];
							for(var j = 0; j < Param.count; j++){
								if(Param.mem[j][0] == pair[0])
									Param.mem[j][1] = pair[1];
							}
						}
					}
				}
            }
            function UnPack(responesStr) {
                var result = responesStr.split('&');
                if (result.length == 4) return new RequestResult(result[0], result[1], result[2], result[3], -1);
                else if (result.length > 4) return new RequestResult(result[0], result[1], result[2], result[3], result[4]);
                else return new RequestResult('', '', -1, '', -1);
            }
            //与CGI交互
            function FlySwapData(Request, Section, postDataStr, successfunc, exsection) {
                var exsection = exsection || "0";

                $.ajax({
                    url:url,
                    data: "Request=" + Request + "&Section=" + Section + "&exsection=" + exsection + "&postDataStr=" + encodeURIComponent(postDataStr + "||"),
                    type: "post",

                    success: function(data) {
                        var data2 = data.replace("\n", "");
                        data2 = $.trim(data2);
                        if (successfunc && typeof successfunc === 'function') {
                            successfunc(data2);
							//alert(data2);
                        }
                    }
                });
            }
		
		
function get_page_data(Param)
{
	for (var i = 0; i < Param.count; i++) {
		var obj = document.getElementById(Param.mem[i][0]);
		if (obj != undefined) {
			switch (obj.type) {
			case "text" :
			case "textarea":
			case "select-one" :
				Param.mem[i][1] = obj.value;
				break;
			case "radio" :
				var radio = document.getElementsByName(/*"n_" +*/ Param.mem[i][0]);
				for (var j = 0; j < radio.length; j++) {
					if (radio[j].checked) {
						Param.mem[i][1] = radio[j].value;
					}
				}
				break;
			case "checkbox" :
				if (obj.checked == true) {
					Param.mem[i][1] = 1;
				} else {
					Param.mem[i][1] = 0;
				}
				break;
			default :
				if (obj.tagName == 'LABEL'){
					Param.mem[i][1] = obj.innerHTML;
				}
				else
					alert("Unknown obj name : [" + Param.mem[i][0] + "], type is : ["+obj.type+"].\n");
				break;
			}
		}
	}
}	
function set_page_data(Param)
{
	var i;
	for (i = 0; i < Param.count; i++) {
		var obj = document.getElementById(Param.mem[i][0]);
		if (obj != undefined) {
			switch (obj.type) {
			case "text" :
			case "textarea":
			case "select-one" :
				obj.value = Param.mem[i][1];
				break;
			case "radio" :
				var radio = document.getElementsByName(/*"n_" + */Param.mem[i][0]);
				for (var j = 0; j < radio.length; j++) {
					if (Param.mem[i][1] == radio[j].value) {
						radio[j].checked = true;
					} else {
						radio[j].checked = false;
					}
				}
				break;
			case "checkbox" :
				if (Param.mem[i][1] == 1) {
					obj.checked = true;
				} else {
					obj.checked = false;
				}
				break;
			default :
				if (obj.tagName == 'LABEL'){
					obj.innerHTML = Param.mem[i][1];
				}
				else
					alert("Unknown type is : ["+obj.type+"], id is " + Param.mem[i][0] + "\n");
				break;
			}
		}
	}
}

			
//以下三个函数显示自动增加时间		
var autoDateCount = new Array();
function timePerSecond(indexstr, dateEle, timeEle){
	var dateT = autoDateCount[indexstr];
	if(dateT == null) return;
	setTimeout("timePerSecond('" + indexstr + "', '" + dateEle + "', '" + timeEle + "')", 1000);
	dateT.setTime(dateT.getTime() + 1000); //增加1秒
	autoDateCount[indexstr] = dateT;
	
	y = dateT.getFullYear();
	M = dateT.getMonth() + 1;
	M = "0" + M;
	d = "0" + dateT.getDate();
	h = "0" + dateT.getHours();
	m = "0" + dateT.getMinutes();
	s = "0" + dateT.getSeconds();
	M = M.substring(M.length - 2, M.length + 1);
	d = d.substring(d.length - 2, d.length + 1);
	h = h.substring(h.length - 2, h.length + 1);
	m = m.substring(m.length - 2, m.length + 1);
	s = s.substring(s.length - 2, s.length + 1);
	strdate = y + "-" + M + "-" + d;
	strtime = h + ":" + m + ":" + s;
	if($('#'+dateEle).attr('value') != undefined)
		$('#'+dateEle).val(strdate);
	if($('#'+dateEle).attr('innerHTML') != undefined)
		$('#'+dateEle).html(strdate);
	if($('#'+timeEle).attr('value') != undefined)
		$('#'+timeEle).val(strtime);
	if($('#'+timeEle).attr('innerHTML') != undefined)
		$('#'+timeEle).html(strtime);
	
}
function autoTime(startDate, dateEle, timeEle){ //返回一个handle，需要停止时间时通过它调用stopAutoTime
	var prifix = "date_";
	var index;
	do{
		index = prifix + Math.round(Math.random()*(100000));
	} while(autoDateCount[index] != null);
	autoDateCount[index] = startDate;
	timePerSecond(index, dateEle, timeEle);
	return index;
}
function stopAutoTime(indexstr){
	if(autoDateCount[indexstr] != null)
		autoDateCount[indexstr] = null;
}
function parseTime(datestr, timestr){ //解析时间为DATE对象
	var c = datestr + ' ' + timestr;
	return new Date(Date.parse(c.replace(/-/g,"/")));
}
