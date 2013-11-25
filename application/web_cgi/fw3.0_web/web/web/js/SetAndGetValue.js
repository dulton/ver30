// JavaScript Document
// 设置页面元素的值。根据cgi层返回的数据，直接填充到页面。页面元素的id 与cgi层返回的参数名一致即可。
//cgi返回字符串。值格式为 ElementId=1||ElementId2=33||ElementId=3434
//2012-3-8测试马军港通过
var streamSection="STREAM0"; //全局变量 设置码流的section
var globalStream='0';//码流全局变量
function SetFormValue(section, str, hasDrag) {
    var hasDrag = hasDrag || false;
    var elem;
    var values = str.split('||');
    var radioname;
    for (var m = 0; m < values.length; m++) {
        var value = values[m].split('=');
        var tempValue; //元素ID名称
        if (section == "IMAGE") {
            tempValue = "01" + value[0];
        }
        if (section == "IPINFO") {
            tempValue = "11" + value[0];
        }
        if (section == "DPTZ") {
            tempValue = "02" + value[0];
        }
        if (section == "PTZ") {
            tempValue = "03" + value[0];
        }
        if (section == "ENCODE") {
            tempValue = "04" + value[0];
        }
        if (section == "STREAM0"||section == "STREAM1"||section == "STREAM2"||section == "STREAM3") {
            tempValue = "20" + value[0];
        }
     
        if (section == "OSD") {
            tempValue = "31" + value[0];
        }
        if (section == "PMSETS") {
            tempValue = "32" + value[0];
        }
        if (section == "IPINFO") {
            tempValue = "40" + value[0];
        }
        if (section == "SMTP") {
            tempValue = "41" + value[0];
        }
        if (section == "UPNP") {
            tempValue = "42" + value[0];
        }
        if (section == "FTP") {
            tempValue = "43" + value[0];
        }
        if (section == "EVENTRESPONSE") {
            tempValue = "50" + value[0];
        }
        if (section == 'MDPLAN' || section == 'CTPLAN' || section == 'INPUTPORT1PLAN' || section == 'RECORDPLAN') {
            tempValue = "51" + value[0];
        }
        if (section == "CTCFG") {
            tempValue = "52" + value[0];
        }
        if (section == "DIGITALIO") {
            tempValue = "53" + value[0];
        }
        if (section == "OUTPORT") {
            tempValue = "54" + value[0];
        }
        if (section == "MDROIS") {
            tempValue = "55" + value[0];
        }
		if(section=="SDINFO")
		{
			  tempValue = "60" + value[0];
		}
		if(section=="UARTCTRL")
		{ 
		      tempValue = "61" + value[0];
		}
		if(section=="PTZ_PROTOCOL")
		{ 
		      tempValue = "62" + value[0];
		}
		if(section=="UPCENTERINFO")
		{ 
		      tempValue = "63" + value[0];
		}
		if(section=="DDNSCENTERINFO")
		{ 
		      tempValue = "64" + value[0];
		}
		if(section=="PROTOINFO")
		{ 
		      tempValue = "65" + value[0];
		}
		if(section=="VINVOUT")
		{ 
		      tempValue = "66" + value[0];
		}
		if(section=="gb28181")
		{ 
		      tempValue = "67" + value[0];
		}
        var trueValue = value[1]; //元素值
        elem = null;

        if (tempValue == '' || tempValue == 'null' || tempValue == null) {
            //如果id名为空 则跳出本次循环
            continue;
        }
        elem = document.getElementById(tempValue);
        if (elem == null) {
            //如果没有对应的控件，则跳过本次循环 
            continue;
        }
        if (trueValue == '' || trueValue == 'null' || trueValue == null) {
            //如果当前值为空，则跳过本次循环 
            continue;
        }
        if (elem.tagName == 'SELECT') {
            //select控件另外处理 
            for (var j = 0; j < elem.length; j++) {

                if (trueValue == elem.options[j].value) {
                    //找到对应元素，让其选中 
                    elem.options[j].selected = true;
                    //并让其不可选 
                    //elem.disabled=true; 
                    //退出循环 
					//alert(elem.id+";"+trueValue);
                    break;
                }
            }
        }
        //复选框
        if (elem.tagName == 'INPUT' && elem.type == 'checkbox') {
            //alert(elem.id);
            if (trueValue == '1') {
                elem.checked = true;

            } else {
                elem.checked = false;
            }
            elem.value = trueValue;

        }
        //单选框
        if (elem.tagName == 'INPUT' && elem.type == 'radio') {
            var radios = document.getElementsByName(elem.id);

            if (radios != null) {
                for (var i = 0; i < radios.length; i++) {
                    var radio = radios[i];
                    if (radio.value == trueValue) {
                        //alert("i===="+i+"=="+trueValue);
                        radio.checked = true;
                        break;
                    }
                }
            }
        }

        else if (elem.tagName == 'INPUT' || elem.tagName == 'TEXTAREA') {

            //alert(elem.tagName);

            elem.value = trueValue;
            //特殊处理部分开始
            if (elem.id == "50rec_sd_enable" || elem.id == "50rec_ftp_enable") {
                if (elem.value == 1) {
                    var tempElem = document.getElementById("rec_enable");
                   if(tempElem!=null)
				   {
                    tempElem.checked = true;
				   }

                }
            }
            //特殊处理部分结束

            //elem.readOnly=true; 
        }
    }
    //处理隐藏域开始
    if (section == "EVENTRESPONSE") {
        var stream, delay, type;
        if ($("#50rec_ftp_enable").val() == '1')

        {
            stream = $("#50rec_ftp_stream_id").val();
            delay = $("#50rec_ftp_delay").val();
            type = '0';

        }

        if ($("#50rec_sd_enable").val() == '1') {
            stream = $("#50rec_sd_stream_id").val();
            delay = $("#50rec_sd_delay").val();
            type = '1';
        }
        $("#stream_id option[value=" + stream + "]").attr("selected", "selected");
        $("#delay option[value=" + delay + "]").attr("selected", "selected");
        $("#save_type option[value=" + type + "]").attr("selected", "selected");
    }
    //处理隐藏域结束
    //处理布防设置开始
    if (section == 'MDPLAN' || section == 'CTPLAN' || section == 'INPUTPORT1PLAN' || section == 'RECORDPLAN') {
        getStarttime($("#51t1_start").val(), $("#t1starth"), $("#t1startm"));
        getEndtime($("#51t1_end").val(), $("#t1endh"), $("#t1endm"));
		getStarttime($("#51t2_start").val(), $("#t2starth"), $("#t2startm"));
        getEndtime($("#51t2_end").val(), $("#t2endh"), $("#t2endm"));
		getStarttime($("#51t3_start").val(), $("#t3starth"), $("#t3startm"));
        getEndtime($("#51t3_end").val(), $("#t3endh"), $("#t3endm"));
		getStarttime($("#51t4_start").val(), $("#t4starth"), $("#t4startm"));
        getEndtime($("#51t4_end").val(), $("#t4endh"), $("#t4endm"));
		getStarttime($("#51t5_start").val(), $("#t5starth"), $("#t5startm"));
        getEndtime($("#51t5_end").val(), $("#t5endh"), $("#t5endm"));
		getStarttime($("#51t6_start").val(), $("#t6starth"), $("#t6startm"));
        getEndtime($("#51t6_end").val(), $("#t6endh"), $("#t6endm"));
        var str = $("#t1starth").val() + "||" + $("#t1startm").val() + "||" + $("#t1endh").val() + "||" + $("#t1endm").val() + "||" + $("#t2starth").val() + "||" + $("#t2startm").val() + "||" + $("#t2endh").val() + "||" + $("#t2endm").val() + "||" + $("#t3starth").val() + "||" + $("#t3startm").val() + "||" + $("#t3endh").val() + "||" + $("#t3endm").val() + "||" + $("#t4starth").val() + "||" + $("#t4startm").val() + "||" + $("#t4endh").val() + "||" + $("#t4endm").val() + "||" + $("#t5starth").val() + "||" + $("#t5startm").val() + "||" + $("#t5endh").val() + "||" + $("#t5endm").val() + "||" + $("#t6starth").val() + "||" + $("#t6startm").val() + "||" + $("#t6endh").val() + "||" + $("#t6endm").val();
        //alert("Set"+str);
        $("#holdStr").val(str);
    }
    //处理布防设置结束

    

}
//根据页面元素ID的数组 生产post 字符串。
function GetFormValue(para, value) {
    var poststr = "";
    if (para != null && value != null) {
        if (para != "" && value != "") {
            poststr = para + "=" + value + "||";
        }
    }
    return poststr;
}
//根据Section 获取页面设置参数
function GetFormIds(section) {
    //var ids;
    var sectionid; //section编号 如：IMAGE 01  
    var trueid; //参数名
    var poststr = ""; //返回值
    if (section == 'IMAGE') {
        sectionid = '01';
    }

    if (section == 'IPINFO') {
        sectionid = '11';
    }
    if (section == 'DPTZ') {
        sectionid = '02';
    }
    if (section == 'PTZ') {
        sectionid = '03';
    }
    if (section == 'ENCODE') {
        sectionid = '04';
    }
    if (section == "STREAM0"||section == "STREAM1"||section == "STREAM2"||section == "STREAM3") {
        sectionid = '20';
    }
    if (section == 'OSD') {
        sectionid = '31';
    }
    if (section == 'PMSETS') {
        sectionid = '32';
    }
    if (section == 'IPINFO') {
        sectionid = '40';
    }
    if (section == 'SMTP') {
        sectionid = '41';
    }
    if (section == 'UPNP') {
        sectionid = '42';
    }
    if (section == 'FTP') {
        sectionid = '43';
    }
    if (section == 'EVENTRESPONSE') {
        sectionid = '50';
    }
    if (section == 'MDPLAN' || section == 'CTPLAN' || section == 'INPUTPORT1PLAN' || section == 'RECORDPLAN') {
        sectionid = '51';
    }
    if (section == "CTCFG") {
        sectionid = '52';
    }
    if (section == "DIGITALIO") {
        sectionid = '53';
    }
    if (section == "OUTPORT") {
        sectionid = '54';
    }
    if (section == "MDROIS") {
        sectionid = '55';
    }
	if (section == "UARTCTRL") {
        sectionid = '61';
    }
	if (section == "PTZ_PROTOCOL") {
        sectionid = '62';
    }
    if (section == "UPCENTERINFO") {
        sectionid = '63';
    }
	if (section == "DDNSCENTERINFO") {
        sectionid = '64';
    }
	if (section == "PROTOINFO") {
        sectionid = '65';
    }
	if (section == "VINVOUT") {
        sectionid = '66';
    }
	if (section == "gb28181") {
        sectionid = '67';
    }
    var inputids = document.getElementsByTagName('INPUT');
    if (inputids != null) {
        for (var i = 0; i < inputids.length; i++) {
            var id = inputids[i];
            if (id.id.length > 2) {

                var isvalid = id.id.substring(0, 2);
                trueid = id.id.substring(2);
                if (isvalid == sectionid) {
                    //ids.push(trueid);
                    if (id.type == 'radio') {

                        if (id.checked == true) {

                            poststr += trueid + "=" + id.value + "||";
                            //alert(poststr);
                        }
                    } else {
                        if (id.type == 'checkbox') {
                            if (id.checked == true) {
                                poststr += trueid + "=" + 1 + "||";
                            } else {
                                poststr += trueid + "=" + 0 + "||";
                            }
                        } else {
							if(id.value!="")
							{
                            poststr += trueid + "=" + id.value + "||";
							}
							else
							{  
									poststr += trueid + "= ||";//空时候 传空格
							}
                        }
                    }
                }

            }
        }
    }
    var selectids = document.getElementsByTagName('select');
    if (selectids != null) {
        for (var i = 0; i < selectids.length; i++) {
            var id = selectids[i];
            if (id.id.length > 2) {
                var isvalid = id.id.substring(0, 2);
                trueid = id.id.substring(2);
                if (isvalid == sectionid) {
                    for (var j = 0; j < id.length; j++) {
                        if (id.options[j].selected == true) {
                            poststr += trueid + "=" + id.options[j].value + "||";
                            break;
                        }
                    }
                }
            }
        }
    }
    return poststr;
}

function getStarttime(time, elem1, elem2) {
    var h = parseInt(time, 10) / 3600;
    h = parseInt(h, 10);
    var m = (parseInt(time, 10) - h * 3600) / 60;
    m = parseInt(m, 10);
    elem1.val(h);
    elem2.val(m);

}

function getEndtime(time, elem1, elem2) {
    if (time > 0) {
        var h = parseInt(time, 10) / 3600;
        h = parseInt(h, 10);
        var m = (parseInt(time, 10) - h * 3600) / 60;
        m = parseInt(m, 10);
        elem1.val(h);
        elem2.val(m);
    }
}

//初始化码流选择 查看当前那些码流打开。添加到select option
function initStreamSelect()
			{
				//alert("da");
				  var section = "ENCODE";
                var postDataStr = "";
                $.ajax({
                    url: url,
                    data: "Request=get&Section=" + section + "&exsection=0" + "&postDataStr=" + encodeURIComponent(postDataStr + "||"),
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
                            alert("返回数据解析错误");
                            return;
                        } else {
                            Request = result[0];
                            Section = result[1];
                            Flag = result[2];
                            Info = result[3];
                            if (Flag == 0) {
                              var index0=Info.indexOf("s0_type");
						      var index1=Info.indexOf("s1_type");
							  var index2=Info.indexOf("s2_type");
							  var index3=Info.indexOf("s3_type");
							  var s0_type=Info.substr(index0,9).split("=")[1];
							  var s1_type=Info.substr(index1,9).split("=")[1];
							  var s2_type=Info.substr(index2,9).split("=")[1];
							  var s3_type=Info.substr(index3,9).split("=")[1];
							  
							  //alert(Info);
							  var values=new Array();
							  if(s0_type!="0")
							  {
								  values.push(0);
								   if(s1_type !="0")
								  {
									 values.push(1);
									  if(s2_type!="0")
									  {
										   values.push(2);
										    if(s3_type!="0")
											  {
												values.push(3);
											  }
									  }
								  }
							  }
							 
                               $("#stream_id").empty();
							   if(parent.isZY)
							   {
								   var option = new Option("码流1", values[0]);
	
										$("#stream_id")[0].options.add(option);
							   }
							   else
							   {
									for (var i = 0; i < values.length; i++) {
										var option = new Option("码流"+(values[i]+1), values[i]);
	
										$("#stream_id")[0].options.add(option);
									}
							   }
								if(document.getElementById("50dptz_source")!=null)
								{
									
									$("#50dptz_source").empty();
									   if(parent.isZY)
									   {
										   var option = new Option("码流1", values[0]);
										   $("#50dptz_source")[0].options.add(option);
									   }
									   else
									   {
											for (var i = 0; i < values.length; i++) {
												var option = new Option("码流"+(values[i]+1), values[i]);
												$("#50dptz_source")[0].options.add(option);
											}
									   }
								}
                            }
                            //
                        }
                    },
                    error: function() {
                        alert("util.js-->GetAndSet()--调用ajax 出错！\n");
                    }
                });
			}