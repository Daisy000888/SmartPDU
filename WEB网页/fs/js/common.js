/*表格*/
var alt = document.getElementById('alternatecolor');

var rows = alt.getElementsByTagName("tr");

for(var i = 0; i < rows.length; i++) {
    if (i % 2 == 0) {
        rows[i].className = "evenrowcolor";
    } else {
        rows[i].className = "oddrowcolor";
    }
}

/*表格1*/
var alt1 = document.getElementById('alternatecolor1');

var rows1 = alt1.getElementsByTagName("tr");

for(i = 0; i < rows1.length; i++) {
    if (i % 2 == 0) {
        rows1[i].className = "evenrowcolor";
    } else {
        rows1[i].className = "oddrowcolor";
    }
}

/*表格2*/
var alt2 = document.getElementById('alternatecolor2');

var rows2 = alt2.getElementsByTagName("tr");

for(i = 0; i < rows2.length; i++) {
    if (i % 2 == 0) {
        rows2[i].className = "evenrowcolor";
    } else {
        rows2[i].className = "oddrowcolor";
    }
}

/*表格3*/
var alt3 = document.getElementById('alternatecolor3');

var rows3 = alt3.getElementsByTagName("tr");

for(i = 0; i < rows3.length; i++) {
    if (i % 2 == 0) {
        rows3[i].className = "evenrowcolor";
    } else {
        rows3[i].className = "oddrowcolor";
    }
}


/**数据导出为Excel格式**/

    var idTmr;
    function  getExplorer() {
        var explorer = window.navigator.userAgent ;
        //ie
        if (explorer.indexOf("MSIE") >= 0 ||!!window.ActiveXObject || "ActiveXObject" in window) {
            return 'ie';
        }
        //firefox
        else if (explorer.indexOf("Firefox") >= 0) {
            return 'Firefox';
        }
        //Chrome
        else if(explorer.indexOf("Chrome") >= 0){
            return 'Chrome';
        }
        //Opera
        else if(explorer.indexOf("Opera") >= 0){
            return 'Opera';
        }
        //Safari
        else if(explorer.indexOf("Safari") >= 0){
            return 'Safari';
        }
    }
    function method5(tableid) {
        if(getExplorer()=='ie')
        {
            var curTbl = document.getElementById(tableid);
            var oXL = new ActiveXObject("Excel.Application");

            var oWB = oXL.Workbooks.Add();

            var xlsheet = oWB.Worksheets(1);

            var sel = document.body.createTextRange();
            sel.moveToElementText(curTbl);

            sel.select;

            sel.execCommand("Copy");

            xlsheet.Paste();

            oXL.Visible = true;

            try {
                var fname = oXL.Application.GetSaveAsFilename("Excel.xls", "Excel Spreadsheets (*.xls), *.xls");
            } catch (e) {
                print("Nested catch caught " + e);
            } finally {
                oWB.SaveAs(fname);

                oWB.Close(savechanges = false);

                oXL.Quit();
                oXL = null;

                idTmr = window.setInterval("Cleanup();", 1);
            }
        }
        else
        {
            tableToExcel(tableid);
        }
    }
    function Cleanup() {
        window.clearInterval(idTmr);
        CollectGarbage();
    }
    var tableToExcel = (function() {
        var uri = 'data:application/vnd.ms-excel;base64,',
            template = '<html><head><meta charset="UTF-8"></head><body><table>{table}</table></body></html>',
            base64 = function(s) { return window.btoa(unescape(encodeURIComponent(s))) },
            format = function(s, c) {
                return s.replace(/{(\w+)}/g,
                    function(m, p) { return c[p]; }) };
        return function(table, name) {
            if (!table.nodeType) table = document.getElementById(table);
            var ctx = {worksheet: name || 'Worksheet', table: table.innerHTML};
            window.location.href = uri + base64(format(template, ctx))
        }
    })();





/**电源数据页**/
/*每个插座电能记录*/
/*document.getElementById('every_button').onclick = function(){
    document.getElementById('top_div').style.display = 'none';
    document.getElementById('el_his').style.display = 'none';
};*/


/**网络管理页**/
/*验证输入的MAC*/
function is_mac(){
    var mac = document.getElementById('mac').value;
    var re =/[A-F\d]{1,2}-[A-F\d]{1,2}-[A-F\d]{1,2}-[A-F\d]{1,2}-[A-F\d]{1,2}-[A-F\d]{1,2}/;

    if(mac != ''){
        if(re.test(mac) == true){
            return true;
        }else{
            alert('MAC格式不合法或大小写错误');
            return false;
        }
    }else{
        alert('MAC地址不能为空');
        return false;
    }
}


/**电源管理页,电源行程预约**/
document.getElementById('close_appointment').onclick = function(){
    document.getElementById('top_div').style.display = 'none';
    document.getElementById('appointment').style.display = 'none';
};


/*验证输入的IP*/
function is_ip(){
    var ip = document.getElementById('ip').value;
    var re =/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;

    if(ip != ''){
        if(re.test(ip) == true){
            if(RegExp.$1<256 && RegExp.$2<256 && RegExp.$3<256 && RegExp.$4<256){
                return true;
            }else{
                alert('IP格式不合法');
                return false;
            }
        }else{
            alert('IP格式不合法');
            return false;
        }
    }else{
        alert('IP地址不能为空');
        return false;
    }
}

/*验证输入的子网掩码*/
function is_mask(){
    var mask = document.getElementById('netmask').value;

    var re=/^(254|252|248|240|224|192|128|0)\.0\.0\.0$|^(255\.(254|252|248|240|224|192|128|0)\.0\.0)$|^(255\.255\.(254|252|248|240|224|192|128|0)\.0)$|^(255\.255\.255\.(254|252|248|240|224|192|128|0))$/;

    if(mask != ''){
        if(re.test(mask) == true){
            return true;
        }else{
            alert('子网掩码不合法');
            return false;
        }
    }else{
        alert('子网掩码不能为空');
        return false;
    }
}

/*验证输入网关*/
function is_gateway(){
    var gateway = document.getElementById('gateway').value;
    var re =/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;

    if(gateway != ''){
        if(re.test(gateway) == true){
            if(RegExp.$1<256 && RegExp.$2<256 && RegExp.$3<256 && RegExp.$4<256){
                return true;
            }else{
                alert('网关格式不合法');
                return false;
            }
        }else{
            alert('网关格式不合法');
            return false;
        }
    }else{
        alert('网关不能为空');
        return false;
    }
}

/*验证输入DNS服务器*/
function is_dns(){
    var dns = document.getElementById('dnsserver').value;
    var re =/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;

    if(dns != ''){
        if(re.test(dns) == true){
            if(RegExp.$1<256 && RegExp.$2<256 && RegExp.$3<256 && RegExp.$4<256){
                return true;
            }else{
                alert('DNS服务器格式不合法');
                return false;
            }
        }else{
            alert('DNS服务器格式不合法');
            return false;
        }
    }else{
        alert('DNS服务器不能为空');
        return false;
    }
}

/*验证输入端口号*/
function is_port(){
    var port = document.getElementById('serverport').value;
    var re = /^(\d+)$/;

    if(port != ''){
        if(re.test(port) && port>=0 && port<=65536){
           return true;
        }else{
            alert('端口号不合法');
            return false;
        }
    }else{
        alert('端口号不能为空');
        return false;
    }
}


/**用户管理页**/
/*新增用户*/
var ad_bu = document.getElementById('add_button');
var add = document.getElementById('add');
var reset = document.getElementById('reset');

ad_bu.onclick = function(){
    add.style.display= 'block';
};

reset.onclick = function () {
    add.style.display = 'none';
};


/*删除用户*/
function del(){
    var a = confirm('是否确认删除该用户？');

    if(a == true){
        alert('向服务器发送请求，删除成功！');
    }
}

/**用户退出,js实现post提交**/
function login_out(){
    clearCookie();

    var myform = document.createElement("form");
    myform.method = "post";
    myform.action = "exit.cgi";

    var myinput = document.createElement("input");
    myinput.setAttribute("name","login_out");
    myinput.setAttribute("value","login_out");
    myform.appendChild(myinput);

    document.getElementById('admins').appendChild(myform);
    myform.submit();
    document.getElementById('admins').removeChild(myform);
}


/*全局弹框历史记录*/
/*document.getElementById('top_div').onclick = function(){
    document.getElementById('power_history').style.display = 'none';
    document.getElementById('top_div').style.display = 'none';
    document.getElementById('el_his').style.display = 'none';
};*/


/*设置cookie*/
function setCookie(cname, cvalue, exdays) {
    var d = new Date();
    d.setTime(d.getTime() + (exdays*24*60*60*1000));
    var expires = "expires="+d.toUTCString();
    document.cookie = cname + "=" + cvalue + "; " + expires;
}

/*清除cookie*/
function clearCookie(name) {
    setCookie(name, "", -1);
}
