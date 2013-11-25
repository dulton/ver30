// JavaScript Document

function setTab(name,cursel,n){
for(i=1;i<=n;i++){
var menu=document.getElementById(name+i);/* zzjs1 */
//var con=document.getElementById("con_"+name+"_"+i);/* con_zzjs_1 */
menu.className=i==cursel?"hover":"";/*三目运算 等号优先*/
//con.style.display=i==cursel?"block":"none";
}
}
//-->
