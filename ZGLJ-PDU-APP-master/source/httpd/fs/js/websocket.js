/*检查浏览器是否支持websocket*/
if(window.WebSocket){

}else{
    alert('该浏览器不支持WebSocket');
}

var websocket_ip;
var websocket;

loadXMLDoc();

function loadXMLDoc()
{
    var xmlhttp;
    if (window.XMLHttpRequest)
    {
        xmlhttp=new XMLHttpRequest();
    }
    else
    {
        xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlhttp.onreadystatechange=function()
    {
        if (xmlhttp.readyState==4 && xmlhttp.status==200)
        {
            websocket_ip = xmlhttp.responseText;

            if(websocket_ip != null){
                websocket_link();
            }else{
                setTimeout(loadXMLDoc,2000);
            }
        }
    };
    xmlhttp.open("POST","/linkip.cgi",true);
    xmlhttp.send();
}


function websocket_link(){
    /**websocket连接**/
    var  wsServer = websocket_ip;
    websocket = new WebSocket(wsServer);

    websocket.onopen = function (evt) { onOpen(evt) };
    websocket.onclose = function (evt) { onClose(evt) };
    websocket.onmessage = function (evt) { onMessage(evt) };
    websocket.onerror = function (evt) { onError(evt) };

    function onOpen(evt) {
        websocket.send('username');
    }
    function onClose(evt) {
        console.log("Disconnected");
        websocket = new WebSocket(wsServer);
    }


    function onMessage(evt) {

        console.log(evt.data);


        ws_data = evt.data;

        var head_data = ws_data.substring(0,2);

        /**登录用户**/
        if(head_data == 'us'){
            admin_login();

            websocket.send('PDU');
        }


        /*最新告警*/
        if(ws_data == 'new_alarm'){
            document.getElementById('new_alarm').style.display = 'block';
        }


        /**实时监测页**/
        /*PDU名称下拉框*/
        if(head_data == 'PD'){
            select();

            page = 'home_page';
            sen(page);
        }

        /*spd状态和漏流保护开关状态*/
        if(head_data == 'sp'){
            spd_lcp();

            page1_setinterval = setInterval(pdu_data,1000);
        }

        /*服务器返回的实时数据*/
        if(head_data == 'vo'){
            got_data();
        }

        /*切换漏流保护开关*/
        if(head_data == 'lc'){
            lcp_sw();
        }

        /*总电能清零*/
        if(head_data == 'po'){
            if(ws_data == 'power_clean ok'){
                alert('清零成功！');
            }else{
                alert('清零失败！');
            }
        }

        /**总电能历史数据**/
        var phistory_first = ws_data.split(" ")[0];

        if(phistory_first == 'sum'){

            phistory_do();

            power_history();

            document.getElementById('power_history_prev').style.backgroundColor= 'dodgerblue';
            document.getElementById('power_history_prev').disabled= false;
            document.getElementById('power_history_next').style.backgroundColor='dodgerblue';
            document.getElementById('power_history_next').disabled = false;

        }else if(ws_data == 'end_prev_page'){

            document.getElementById('power_history_prev').style.backgroundColor= '#ccc';
            document.getElementById('power_history_prev').disabled= true;

        }else if(ws_data=='end_next_page'){

            document.getElementById('power_history_next').style.backgroundColor= '#ccc';
            document.getElementById('power_history_next').disabled = true;
        }




        /**电源数据页**/
        /*电源管理发送页识别之后返回*/
        if(head_data == 'ok'){
            if(ws_data == 'ok'){
                page2_setinterval = setInterval(outlet_data,1000);
            }else{
               return false;
            }
        }

        var outlet_three = ws_data.substring(0,6);
        if(outlet_three == 'status'){

            outlet_do();

            one_outlet_history();
        }

        /**每个插座的电能记录**/
        var outlet_first = ws_data.split(" ")[0];

        if(outlet_first == 'branch'){

            outlet_historydo();

            every_outlet_history();

            document.getElementById('outlet_prev').style.backgroundColor= 'dodgerblue';
            document.getElementById('outlet_prev').disabled= false;
            document.getElementById('outlet_next').style.backgroundColor='dodgerblue';
            document.getElementById('outlet_next').disabled = false;

        }else if(ws_data == 'end_outlet_prev_page'){

            document.getElementById('outlet_prev').style.backgroundColor= '#ccc';
            document.getElementById('outlet_prev').disabled= true;

        }else if(ws_data=='end_outlet_next_page'){

            document.getElementById('outlet_next').style.backgroundColor= '#ccc';
            document.getElementById('outlet_next').disabled = true;
        }

        /**环境信息管理页**/
        /*环境信息管理页首次返回的数据*/
        if(head_data == 'tu'){

            environment_give();
        }

        /*环境实时数据*/
        if(head_data == 'te'){
            environment_timely();
        }

        /*温度传感器开关切换*/
        if(head_data == 't_'){
            page = 'environment';
            sen(page);
        }

        /*湿度传感器开关切换*/
        if(head_data == 'h_'){
            page = 'environment';
            sen(page);
        }

        /*烟雾传感器开关切换*/
        if(head_data == 's_'){
            smo_switch();
        }

        /*门禁传感器开关切换*/
        if(head_data == 'd_'){
            dr_switch();
        }

        /*输出传感器开关切换*/
        if(head_data == 'o_'){
            ou_switch();
        }

        /*设置温度上限下限，湿度上限*/
        if(head_data == 'se'){
            if(ws_data == 'set_temp ok' || ws_data == 'set_humi ok'){
                page = 'environment';
                sen(page);
            }
        }

        /**历史告警记录**/
        var alarm_first = ws_data.split(" ")[0];

        if(alarm_first == 'alarm'){
            alarm_data();
        }


        /**设备管理页**/
        if(head_data == 'lo'){
            device_data();
        }

        /*设备位置设置*/
        if(ws_data == 'set_location ok'){
            alert('设备位置设置成功');
            page='device_manage';
            sen(page);
        }else if(ws_data == 'set_location error'){
            alert('设备位置设置失败！');
        }

        /*设备工作模式*/
        if(ws_data == 'set_mode ok'){
            alert('工作模式设置成功！');
            page='device_manage';
            sen(page);
        }else if(ws_data == 'set_mode error'){
            alert('工作模式设置失败!!');
        }

        /*软件升级*/
        if(ws_data == 'startpack ok'){
            do_upload();
        }

        if(ws_data == 'file not correct'){
            alert('请选择正确的升级文件');
        }

        if(ws_data == 'datapack ok'){

            cuLoaded = cuLoaded + step;
            do_upload();
        }

        if(ws_data == 'endpack ok'){
            alert('软件升级成功,请等待10s后重新连接');
            document.getElementById('upload_top_div').style.display = 'none';
            document.getElementById('upload_progress').style.display = 'none';
        }else if(ws_data == 'endpack error'){
            alert('软件升级失败');
            cuLoaded = 0;
            document.getElementById('upload_top_div').style.display = 'none';
            document.getElementById('upload_progress').style.display = 'none';
        }


        /**电源管理**/
        if(head_data == 'tp'){
            power_do();

            each_appointment();
        }

        if(ws_data == 'set_outlet ok'){
            alert('电源阀值配置成功');

            page = 'power_manage';
            sen(page);
        }else if(ws_data == 'set_outlet error'){
            alert('电源阀值配置失败');
        }

        if(ws_data == 'outlet_switch ok'){
            swit_success();

        }else if(ws_data == 'outlet_switch error'){
            alert('电源开关切换失败');
        }

        /*电源行程预约*/
       var appoint_data = ws_data.substring(0,7);

        if(appoint_data == 'outlet='){

            every_outlet_appointment();
        }

        if(ws_data == 'outlet_order ok'){

            alert('电源行程预约成功');
            document.getElementById("appoint_date").value='';

            outlet_appoint_do();
            websocket.send('outlet_order:list');

        }else if(ws_data == 'outlet_order error'){

            alert('电源行程预约失败');
        }else if(ws_data == 'delete_num ok'){

            alert('电源行程预约删除成功');

            outlet_appoint_do();
            websocket.send('outlet_order:list');

        }else if(ws_data == 'delete_num error'){

            alert('电源行程预约删除失败');
        }



        /**网络管理**/
        if(head_data == 'ma'){
            internet_data();
        }

        /*配置网络参数*/
        if(ws_data == 'set_netconfig ok'){
            alert('网络配置成功！正在重启，请等待10秒后重新连接');
            page = 'netconfig';
            sen(page);
        }else if(ws_data == 'set_netconfig error'){
            alert('网络配置失败!');
        }

        /*时间同步*/
        if(ws_data == 'set_syncdate ok'){
            alert('时间同步成功！');

        }else if(ws_data == 'set_syncdate error'){
            alert('时间同步失败!');
        }

        /**用户管理**/
        if(head_data == 'na'){
            admin_data();
            admin_del();
        }
        /*用户添加*/
        if(ws_data == 'add_user ok'){
            alert('用户添加成功');

            admin_do();

            page='username';
            sen(page);
        }else if(ws_data == 'add_user error'){
            alert('用户添加失败');
        }

        /*用户删除*/
        if(ws_data == 'delete_user ok'){
            alert('用户删除成功');

            admin_do();

            page='username';
            sen(page);
        }

    }
    function onError(evt) {
        console.log('Error occured: ' + evt.data);
    }
}


/*当前登陆用户*/
var this_admin_type;

/*接收到的websocket信息*/
var pdu = document.getElementById('pdu_names');
var search1 = document.getElementById('search1');

var ws_data;
var pdu_arr;

var page;

var lcp_switch;
var page1_setinterval;
var page2_setinterval;
var page3_setinterval;

var power_history_num = 0;
var outlet_history_num = 0;
var outlet_text;

var temp_switch_data;
var hum_switch_data;
var smoke_switch_data;
var door_switch_data;
var out_switch_data;

var intime = null;


/**登陆用户及状态**/
function admin_login(){
    var admin_arr = ws_data.split(" ");

    var username = admin_arr[0].split("=")[1];
    var user_type = admin_arr[1].split("=")[1];

    /*当前用户状态*/
    this_admin_type = user_type;

    document.getElementById('this_admin').textContent = username;
}


/*最新告警*/
document.getElementById('alarm_text').onclick = function(){
    new_alarm();

    document.getElementById('page4').onclick();
};

function new_alarm(){
    websocket.send('read_alarm');
    document.getElementById('new_alarm').style.display = 'none';
}



/*设备名称下拉框*/
function select(){

    pdu_arr = ws_data.split(" ");

    for(var i=0;i<pdu_arr.length;i++){
        pdu.options.add(new Option(pdu_arr[i],pdu_arr[i]));
    }
}

/*向服务端发送对应页的页识别*/
function sen(page){

    var index = pdu.selectedIndex;

    if(index >= 0){
        var va = pdu.options[index].value;

        websocket.send('pdu_name:'+page +'=' +va);
    }else{
        return false;
    }
}

/*发送了页识别之后返回的数据,返回spd状态和漏流保护开关状态*/
function spd_lcp(){
    var spd_arr = ws_data.split(" ");

    var spd = spd_arr[0];
    lcp_switch = spd_arr[1];

    if(spd == 'spd=1'){
        document.getElementById('spd').textContent = '正常';
    }else{
        document.getElementById('spd').textContent = '故障';
    }

    if(lcp_switch == 'lcp_switch=1'){
        lcp_switch =1;
        document.getElementById('electric_switch').textContent= '开';
    }else{
        lcp_switch =0;
        document.getElementById('electric_switch').textContent= '关';
    }

}

/*实时数据获取*/
function pdu_data(){
    websocket.send('pdu_data');
}

/*实时数据处理*/
var voltage_value;
var current_value;
var power_value;
var energy_value;

function got_data(){
    var put_data = ws_data;

    var put_arr =put_data.split(" ");

    var voltage = put_arr[0].split("=")[1];
    var current = put_arr[1].split("=")[1];
    var power = put_arr[2].split("=")[1];
    var energy = put_arr[3].split("=")[1];
    var leakcurrent = put_arr[4].split("=")[1];
    var freq = put_arr[5].split("=")[1];
    var powerfac = put_arr[6].split("=")[1];

    /*漏流*/
    document.getElementById("leakcurrent").textContent = leakcurrent + 'mA';
    /*功率因素*/
    document.getElementById("powerfac").textContent = powerfac;
    /*频率*/
    document.getElementById("freq").textContent = freq + 'Hz';

    var voltage1 = new Number(voltage);
    voltage_value = voltage1.toFixed(2);

    var current1 = new Number(current);
    current_value = current1.toFixed(2);

    var power1 = new Number(power);
    power_value = power1.toFixed(2);

    var energy1 = new Number(energy);
    energy_value = energy1.toFixed(2);

}



/*选择设备名称*/
pdu.onchange = function(){
    var index = pdu.selectedIndex;
    var va = pdu.options[index].value;

    websocket.send('pdu_name:' + page + '='+va);
};

/*漏流保护开关切换*/
var es = document.getElementById('electric_switch');
es.onclick = function(){
    websocket.send('lcp_switch');
};

function lcp_sw(){
    if(ws_data == 'lcp_switch ok'){
        if(lcp_switch == 1){
            lcp_switch = 0;
        }else{
            lcp_switch = 1;
        }
    }else{
        alert('漏流开关切换失败');
    }

    if(lcp_switch ==1){
        document.getElementById('electric_switch').textContent= '开';
    }else{
        document.getElementById('electric_switch').textContent= '关';
    }
}

/*总电能清零*/
var d = document.getElementById('del');

d.onclick = function(){
    var aq = confirm('是否确定进行总电能清零操作？');
    if(aq == true){
        websocket.send('power_clean');
    }
};

/**以点击菜单栏的方式进入主页**/
var page1 = document.getElementById('page1');

page1.onclick = function(){
    menu_page();
    document.getElementById('page_one').style.display = 'block';
    page1.className = 'on';


    /*需要显示搜索框*/
    search1.style.display = 'block';
    /*清除计时器*/
    clearInterval(page2_setinterval);
    clearInterval(page3_setinterval);

    page = 'home_page';

    sen(page);
};

/*总电能历史记录*/
var p_button = document.getElementById('power_history_button');

p_button.onclick = function(){
    clearInterval(page1_setinterval);

    document.getElementById('power_history').style.display = 'block';
    document.getElementById('top_div').style.display = 'block';

    websocket.send('power_history');
};


function phistory_do(){
    var phtb = document.getElementById('power_history_table');

    phtb.innerHTML = '';

    power_history_num = 0;
}

/*总电能历史数据处理*/
function power_history(){
    var table = document.getElementById('power_history_table');

    if(table.getElementsByClassName('history_head').length == 0){
        var oneRow1 = table.insertRow();

        oneRow1.innerHTML = ' <tr>'
        +'<th>编号</th>'
        +'<th>时间</th>'
        +'<th>总电能(kWh)</th>'
        +'</tr>';

        oneRow1.className = 'history_head';
        oneRow1.style.backgroundColor = '#C3DDE0';
    }

    var phistory1 = ws_data.replace("sum ","");


    var phistory = phistory1.split("&");

    for(var i=0;i<phistory.length-1;i++){
        var phistory_data = phistory[i].split(" ");

        var time_ym = phistory_data[0];
        var time_hs1 = phistory_data[1];
        var sum_power = phistory_data[2];

        if(time_hs1 != undefined){
            var time_hs2 = time_hs1.replace(/-/g,":");
        }

        var oneRow2 = table.insertRow();
        oneRow2.style.backgroundColor = '#D4E3E5';

        var cell1= oneRow2.insertCell();
        var cell2= oneRow2.insertCell();
        var cell3= oneRow2.insertCell();

       power_history_num = power_history_num + 1 ;

        cell1.textContent =  power_history_num;
        cell2.textContent = time_ym+' '+time_hs2;
        cell3.textContent = sum_power;
    }
}

/**总电能历史数据的分页**/
var history_prev_button = document.getElementById('power_history_prev');
var history_next_button = document.getElementById('power_history_next');

/*上一页*/
history_prev_button.onclick = function(){

    websocket.send('power_history:prev_page');
};

/*下一页*/
history_next_button.onclick = function(){

    websocket.send('power_history:next_page');
};


/**实时电压折线图**/
var st_v = setInterval(start_v,1000);

function start_v(){
    if(voltage_value != null){
        v_charts();
        clearInterval(st_v);
    }else{
        return false;
    }
}


/***折线图***/
function v_charts(){

     setInterval(data_change,1000);

    var v_data = new Array();

    var datas = new  Array();
    var datas2 = new  Array();


    var can1 = document.getElementById('can1');

    var ctx = can1.getContext("2d");

    function draw_v(){
        /*给画布设置宽度与高度*/
        var c1_width = document.getElementById('voltage_data').clientWidth;
        var c1_height = document.getElementById('voltage_data').clientHeight-20;

        can1.setAttribute('width',c1_width);
        can1.setAttribute('height',c1_height);

        var cwidth = document.getElementById('can1').clientWidth;
        var cheight = document.getElementById('can1').clientHeight;

        var y_height = cheight-40;

        //画出坐标线
        function drawBorder(){
            ctx.beginPath();
            ctx.moveTo(60,20);
            ctx.lineTo(60,y_height);
            ctx.moveTo(60,y_height);
            ctx.lineTo(cwidth-10,y_height);
            ctx.strokeStyle = "#C0C0C0";
            ctx.closePath();
            ctx.stroke();
        }

        var nums = v_data;
        var max_nums = Math.max.apply(null,nums);

        /*获取当前时间*/
        var time = new Date();

        var m = time.getMonth() + 1;
        var dt = time.getFullYear() + "-" + m + "-"
            + time.getDate();
        var dt2 = time.getHours() + " " + ":"+" "+
            + time.getMinutes() +" " +":" + " " + time.getSeconds();

        if(datas.length<5){
            datas.push(dt);
            datas2.push(dt2);
        }else{
            for(var i =0;i<datas.length;i++){
                if(i<4){
                    datas[i] = datas[i+1];
                    datas2[i] = datas2[i+1];
                }else if(i == 4){
                    datas[4] = dt;
                    datas2[4] = dt2;
                }
            }
        }


        //画出折线
        function drawLine(){
            for (var i = 0;i < nums.length-1;i++){
                //起始坐标
                var numsY;
                var numsNY;

                if(max_nums === 0){
                    numsY = y_height;
                }else{
                    numsY = (y_height-40)-(y_height-40)/max_nums*nums[i]+40;
                }

                var numsX = i*((cwidth-60)/5)+80;
                //终止坐标
                if(max_nums === 0){
                    numsNY = y_height;
                }else{
                    numsNY = (y_height-40)-(y_height-40)/max_nums*nums[i+1]+40;
                }

                var numsNX = (i+1)*((cwidth-60)/5)+80;

                ctx.beginPath();
                ctx.moveTo(numsX,numsY);
                ctx.lineTo(numsNX,numsNY);
                ctx.lineWidth = 2;
                ctx.strokeStyle = "#488ADE";
                ctx.closePath();
                ctx.stroke();
            }
        }
        //绘制折线点的菱形和数值，横坐标值，纵坐标值
        function drawBlock(){
            for (var i = 0;i < nums.length;i++){
                var numsY;
                if(max_nums === 0){
                    numsY = y_height;
                }else{
                    numsY = (y_height-40)-(y_height-40)/max_nums*nums[i]+40;
                }

                var numsX = i*((cwidth-60)/5)+80;

                ctx.beginPath();
                // 画出折线上的方块
                ctx.moveTo(numsX-2,numsY);
                ctx.lineTo(numsX,numsY-2);
                ctx.lineTo(numsX+2,numsY);
                ctx.lineTo(numsX,numsY+2);
                ctx.fill();
                ctx.font = "10px scans-serif";
                ctx.fillStyle = "#666";
                //折线上的点值
                var text = ctx.measureText(nums[i]);
                ctx.fillText(nums[i],numsX-text.width+20,numsY-8);
                //绘制纵坐标
                var v_width = ctx.measureText('电压/v').width;
                ctx.fillText('电压/v',60-v_width,14);

                //控制小数位数
                var v_num = new Number((nums.length-i)*(max_nums/4));
                var v_value = v_num.toFixed(2);

                var t_width = ctx.measureText(v_value).width;

                if(i==0){

                }else{
                    if(max_nums === 0){
                        ctx.fillText(v_value,60-t_width-2,y_height);
                    }else{
                        ctx.fillText(v_value,60-t_width-2,(y_height-40)/max_nums*(i-1)*(max_nums/4)+40+2);
                    }
                }

                //绘制横坐标并判断
                if (i < 5){
                    ctx.fillText(datas[i],i*((cwidth-70)/5)+60,y_height+20);
                    ctx.fillText(datas2[i],i*((cwidth-70)/5)+60,y_height+36);
                }else if(i == 5) {
                    return;
                }

                ctx.closePath();
                ctx.stroke();
            }
        }
        drawBorder();
        drawLine();
        drawBlock();
    }

    function data_change(){

        if(v_data.length < 5){

            v_data.push(voltage_value);
        }else{
            for(var i =0;i<v_data.length;i++){
                if(i<4){
                    v_data[i] = v_data[i+1];
                }else if(i == 4){
                    v_data[4] =  voltage_value;
                }
            }
        }
        clearCanvas();
        draw_v();
    }

    /*清除画布内容，重绘*/
    function clearCanvas()
    {
        var c=document.getElementById('can1');
        var cxt=c.getContext("2d");
        cxt.clearRect(0,0, c.width,c.height);
    }
}


/**实时电流折线图**/
var st_a = setInterval(start_a,1000);

function start_a(){
    if(current_value != null){
        a_charts();
        clearInterval(st_a);
    }else{
        return false;
    }
}

function a_charts(){

    setInterval(data_change,1000);

    var a_data = new Array();

    var datas = new  Array();
    var datas2 = new  Array();


    var can2 = document.getElementById('can2');


    var ctx = can2.getContext("2d");

    function draw_a(){
        /*给画布设置宽度与高度*/
        var c1_width = document.getElementById('current_data').clientWidth;
        var c1_height = document.getElementById('current_data').clientHeight-20;

        can2.setAttribute('width',c1_width);
        can2.setAttribute('height',c1_height);

        var cwidth = document.getElementById('can2').clientWidth;
        var cheight = document.getElementById('can2').clientHeight;

        var y_height = cheight-40;

        //画出坐标线
        function drawBorder(){
            ctx.beginPath();
            ctx.moveTo(60,20);
            ctx.lineTo(60,y_height);
            ctx.moveTo(60,y_height);
            ctx.lineTo(cwidth-10,y_height);
            ctx.strokeStyle = "#C0C0C0";
            ctx.closePath();
            ctx.stroke();
        }

        var nums = a_data;
        var max_nums = Math.max.apply(null,nums);

        /*获取当前时间*/
        var time = new Date();

        var m = time.getMonth() + 1;
        var dt = time.getFullYear() + "-" + m + "-"
            + time.getDate();
        var dt2 = time.getHours() + " " + ":"+" "+
            + time.getMinutes() +" " +":" + " " + time.getSeconds();

        if(datas.length<5){
            datas.push(dt);
            datas2.push(dt2);
        }else{
            for(var i =0;i<datas.length;i++){
                if(i<4){
                    datas[i] = datas[i+1];
                    datas2[i] = datas2[i+1];
                }else if(i == 4){
                    datas[4] = dt;
                    datas2[4] = dt2;
                }
            }
        }


        //画出折线
        function drawLine(){
            for (var i = 0;i < nums.length-1;i++){
                //起始坐标
                var numsY;
                var numsNY;
                if(max_nums === 0){
                    numsY = y_height;
                }else{
                    numsY = (y_height-40)-(y_height-40)/max_nums*nums[i]+40;
                }
                var numsX = i*((cwidth-60)/5)+80;

                //终止坐标
                if(max_nums === 0){
                    numsNY = y_height;
                }else{
                    numsNY = (y_height-40)-(y_height-40)/max_nums*nums[i+1]+40;
                }
                var numsNX = (i+1)*((cwidth-60)/5)+80;

                ctx.beginPath();
                ctx.moveTo(numsX,numsY);
                ctx.lineTo(numsNX,numsNY);
                ctx.lineWidth = 2;
                ctx.strokeStyle = "#87D57A";
                ctx.closePath();
                ctx.stroke();
            }
        }
        //绘制折线点的菱形和数值，横坐标值，纵坐标值
        function drawBlock(){
            for (var i = 0;i < nums.length;i++){
                var numsY;
                if(max_nums===0){
                    numsY = y_height;
                }else{
                    numsY = (y_height-40)-(y_height-40)/max_nums*nums[i]+40;
                }

                var numsX = i*((cwidth-60)/5)+80;
                ctx.beginPath();
                // 画出折线上的方块
                ctx.moveTo(numsX-2,numsY);
                ctx.lineTo(numsX,numsY-2);
                ctx.lineTo(numsX+2,numsY);
                ctx.lineTo(numsX,numsY+2);
                ctx.fill();
                ctx.font = "10px scans-serif";
                ctx.fillStyle = "#666";
                //折线上的点值
                var text = ctx.measureText(nums[i]);
                ctx.fillText(nums[i],numsX-text.width+20,numsY-8);
                //绘制纵坐标
                var a_width = ctx.measureText('电流/A').width;
                ctx.fillText('电流/A',60-a_width,14);

                //控制小数位数
                var a_num = new Number((nums.length-i)*(max_nums/4));
                var a_value = a_num.toFixed(2);

                var t_width = ctx.measureText(a_value).width;

                if(i==0){

                }else{
                    if(max_nums ===0){
                        ctx.fillText(a_value,60-t_width-2,y_height);
                    }else{
                        ctx.fillText(a_value,60-t_width-2,(y_height-40)/max_nums*(i-1)*(max_nums/4)+40+2);
                    }

                }

                //绘制横坐标并判断
                if (i < 5){
                    ctx.fillText(datas[i],i*((cwidth-70)/5)+60,y_height+20);
                    ctx.fillText(datas2[i],i*((cwidth-70)/5)+60,y_height+36);
                }else if(i == 5) {
                    return;
                }

                ctx.closePath();
                ctx.stroke();
            }
        }
        drawBorder();
        drawLine();
        drawBlock();
    }

    function data_change(){

        if(a_data.length < 5){

            a_data.push(current_value);
        }else{
            for(var i =0;i<a_data.length;i++){
                if(i<4){
                    a_data[i] = a_data[i+1];
                }else if(i == 4){
                    a_data[4] =  current_value;
                }
            }
        }
        clearCanvas();
        draw_a();
    }

    /*清除画布内容，重绘*/
    function clearCanvas()
    {
        var c=document.getElementById('can2');
        var cxt=c.getContext("2d");
        cxt.clearRect(0,0, c.width,c.height);
    }
}



/**功率实时数据折线图**/
var st_p = setInterval(start_p,1000);

function start_p(){
    if(power_value != null){
        p_charts();
        clearInterval(st_p);
    }else{
        return false;
    }
}


function p_charts(){

    setInterval(data_change,1000);

    var p_data = new Array();

    var datas = new  Array();
    var datas2 = new  Array();

    var can3 = document.getElementById('can3');

    var ctx = can3.getContext("2d");

    function draw_p(){
        /*给画布设置宽度与高度*/
        var c1_width = document.getElementById('power_data').clientWidth;
        var c1_height = document.getElementById('power_data').clientHeight-20;

        can3.setAttribute('width',c1_width);
        can3.setAttribute('height',c1_height);

        var cwidth = document.getElementById('can3').clientWidth;
        var cheight = document.getElementById('can3').clientHeight;

        var y_height = cheight-40;

        //画出坐标线
        function drawBorder(){
            ctx.beginPath();
            ctx.moveTo(60,20);
            ctx.lineTo(60,y_height);
            ctx.moveTo(60,y_height);
            ctx.lineTo(cwidth-10,y_height);
            ctx.strokeStyle = "#C0C0C0";
            ctx.closePath();
            ctx.stroke();
        }

        var nums = p_data;
        var max_nums = Math.max.apply(null,nums);

        /*获取当前时间*/
        var time = new Date();

        var m = time.getMonth() + 1;
        var dt = time.getFullYear() + "-" + m + "-"
            + time.getDate();
        var dt2 = time.getHours() + " " + ":"+" "+
            + time.getMinutes() +" " +":" + " " + time.getSeconds();

        if(datas.length<5){
            datas.push(dt);
            datas2.push(dt2);
        }else{
            for(var i =0;i<datas.length;i++){
                if(i<4){
                    datas[i] = datas[i+1];
                    datas2[i] = datas2[i+1];
                }else if(i == 4){
                    datas[4] = dt;
                    datas2[4] = dt2;
                }
            }
        }


        //画出折线
        function drawLine(){
            for (var i = 0;i < nums.length-1;i++){
                //起始坐标
                var numsY;
                var numsNY;
                if(max_nums === 0){
                    numsY = y_height;
                }else{
                    numsY = (y_height-40)-(y_height-40)/max_nums*nums[i]+40;
                }

                var numsX = i*((cwidth-60)/5)+80;
                //终止坐标
                if(max_nums === 0){
                    numsNY = y_height;
                }else{
                    numsNY = (y_height-40)-(y_height-40)/max_nums*nums[i+1]+40;
                }

                var numsNX = (i+1)*((cwidth-60)/5)+80;

                ctx.beginPath();
                ctx.moveTo(numsX,numsY);
                ctx.lineTo(numsNX,numsNY);
                ctx.lineWidth = 2;
                ctx.strokeStyle = "#f08e80";
                ctx.closePath();
                ctx.stroke();
            }
        }
        //绘制折线点的菱形和数值，横坐标值，纵坐标值
        function drawBlock(){
            for (var i = 0;i < nums.length;i++){
                var numsY;
                if(max_nums === 0){
                    numsY = y_height;
                }else{
                    numsY = (y_height-40)-(y_height-40)/max_nums*nums[i]+40;
                }

                var numsX = i*((cwidth-60)/5)+80;
                ctx.beginPath();
                // 画出折线上的方块
                ctx.moveTo(numsX-2,numsY);
                ctx.lineTo(numsX,numsY-2);
                ctx.lineTo(numsX+2,numsY);
                ctx.lineTo(numsX,numsY+2);
                ctx.fill();
                ctx.font = "10px scans-serif";
                ctx.fillStyle = "#666";
                //折线上的点值
                var text = ctx.measureText(nums[i]);
                ctx.fillText(nums[i],numsX-text.width+20,numsY-8);
                //绘制纵坐标
                var p_width = ctx.measureText('功率/kW').width;
                ctx.fillText('功率/kW',60-p_width,14);

                //控制小数位数
                var p_num = new Number((nums.length-i)*(max_nums/4));
                var p_value = p_num.toFixed(2);

                var t_width = ctx.measureText(p_value).width;

                if(i==0){

                }else{
                    if(max_nums === 0){
                        ctx.fillText(p_value,60-t_width-2,y_height);
                    }else{
                        ctx.fillText(p_value,60-t_width-2,(y_height-40)/max_nums*(i-1)*(max_nums/4)+40+2);
                    }
                }

                //绘制横坐标并判断
                if (i < 5){
                    ctx.fillText(datas[i],i*((cwidth-70)/5)+60,y_height+20);
                    ctx.fillText(datas2[i],i*((cwidth-70)/5)+60,y_height+36);
                }else if(i == 5) {
                    return;
                }

                ctx.closePath();
                ctx.stroke();
            }
        }
        drawBorder();
        drawLine();
        drawBlock();
    }

    function data_change(){

        if(p_data.length < 5){

            p_data.push(power_value);
        }else{
            for(var i =0;i<p_data.length;i++){
                if(i<4){
                    p_data[i] = p_data[i+1];
                }else if(i == 4){
                    p_data[4] = power_value;
                }
            }

        }
        clearCanvas();
        draw_p();
    }

    /*清除画布内容，重绘*/
    function clearCanvas()
    {
        var c=document.getElementById('can3');
        var cxt=c.getContext("2d");
        cxt.clearRect(0,0, c.width,c.height);
    }
}


/**电能实时数据折线图**/
var st_e = setInterval(start_e,1000);

function start_e(){
    if(power_value != null){
        e_charts();
        clearInterval(st_e);
    }else{
        return false;
    }
}


function e_charts(){

    setInterval(data_change,1000);

    var e_data = new Array();

    var datas = new  Array();
    var datas2 = new  Array();


    var can4 = document.getElementById('can4');


    var ctx = can4.getContext("2d");

    function draw_e(){
        /*给画布设置宽度与高度*/
        var c1_width = document.getElementById('energy_data').clientWidth;
        var c1_height = document.getElementById('energy_data').clientHeight-20;

        can4.setAttribute('width',c1_width);
        can4.setAttribute('height',c1_height);

        var cwidth = document.getElementById('can4').clientWidth;
        var cheight = document.getElementById('can4').clientHeight;

        var y_height = cheight-40;

        //画出坐标线
        function drawBorder(){
            ctx.beginPath();
            ctx.moveTo(60,20);
            ctx.lineTo(60,y_height);
            ctx.moveTo(60,y_height);
            ctx.lineTo(cwidth-10,y_height);
            ctx.strokeStyle = "#C0C0C0";
            ctx.closePath();
            ctx.stroke();
        }

        var nums = e_data;
        var max_nums = Math.max.apply(null,nums);

        /*获取当前时间*/
        var time = new Date();

        var m = time.getMonth() + 1;
        var dt = time.getFullYear() + "-" + m + "-"
            + time.getDate();
        var dt2 = time.getHours() + " " + ":"+" "+
            + time.getMinutes() +" " +":" + " " + time.getSeconds();

        if(datas.length<5){
            datas.push(dt);
            datas2.push(dt2);
        }else{
            for(var i =0;i<datas.length;i++){
                if(i<4){
                    datas[i] = datas[i+1];
                    datas2[i] = datas2[i+1];
                }else if(i == 4){
                    datas[4] = dt;
                    datas2[4] = dt2;
                }
            }
        }


        //画出折线
        function drawLine(){
            for (var i = 0;i < nums.length-1;i++){
                //起始坐标
                var numsY;
                var numsNY;
                if(max_nums === 0){
                    numsY = y_height;
                }else{
                    numsY = (y_height-40)-(y_height-40)/max_nums*nums[i]+40;
                }

                var numsX = i*((cwidth-60)/5)+80;
                //终止坐标
                if(max_nums === 0){
                    numsNY = y_height;
                }else{
                    numsNY = (y_height-40)-(y_height-40)/max_nums*nums[i+1]+40;
                }

                var numsNX = (i+1)*((cwidth-60)/5)+80;
                ctx.beginPath();
                ctx.moveTo(numsX,numsY);
                ctx.lineTo(numsNX,numsNY);
                ctx.lineWidth = 2;
                ctx.strokeStyle = "#30BB99";
                ctx.closePath();
                ctx.stroke();
            }
        }
        //绘制折线点的菱形和数值，横坐标值，纵坐标值
        function drawBlock(){
            for (var i = 0;i < nums.length;i++){
                var numsY;
                if(max_nums === 0){
                    numsY = y_height;
                }else{
                    numsY = (y_height-40)-(y_height-40)/max_nums*nums[i]+40;
                }

                var numsX = i*((cwidth-60)/5)+80;

                ctx.beginPath();
                // 画出折线上的方块
                ctx.moveTo(numsX-2,numsY);
                ctx.lineTo(numsX,numsY-2);
                ctx.lineTo(numsX+2,numsY);
                ctx.lineTo(numsX,numsY+2);
                ctx.fill();
                ctx.font = "10px scans-serif";
                ctx.fillStyle = "#666";
                //折线上的点值
                var text = ctx.measureText(nums[i]);
                ctx.fillText(nums[i],numsX-text.width+20,numsY-8);
                //绘制纵坐标
                var v_width = ctx.measureText('电能/kWh').width;
                ctx.fillText('电能/kWh',60-v_width,14);

                //控制小数位数
                var e_num = new Number((nums.length-i)*(max_nums/4));
                var e_value = e_num.toFixed(2);

                var t_width = ctx.measureText(e_value).width;

                if(i==0){

                }else{
                    if(max_nums === 0){
                        ctx.fillText(e_value,60-t_width-2,y_height);
                    }else{
                        ctx.fillText(e_value,60-t_width-2,(y_height-40)/max_nums*(i-1)*(max_nums/4)+40+2);
                    }
                }

                //绘制横坐标并判断
                if (i < 5){
                    ctx.fillText(datas[i],i*((cwidth-70)/5)+60,y_height+20);
                    ctx.fillText(datas2[i],i*((cwidth-70)/5)+60,y_height+36);
                }else if(i == 5) {
                    return;
                }

                ctx.closePath();
                ctx.stroke();
            }
        }
        drawBorder();
        drawLine();
        drawBlock();
    }

    function data_change(){

        if(e_data.length < 5){

            e_data.push(energy_value);
        }else{
            for(var i =0;i<e_data.length;i++){
                if(i<4){
                    e_data[i] = e_data[i+1];
                }else if(i == 4){
                    e_data[4] = energy_value;
                }
            }

        }
        clearCanvas();
        draw_e();
    }

    /*清除画布内容，重绘*/
    function clearCanvas()
    {
        var c=document.getElementById('can4');
        var cxt=c.getContext("2d");
        cxt.clearRect(0,0, c.width,c.height);
    }
}




/**电源数据页**/
function outlet_data(){
    websocket.send('outlet_data');
}

var page2 = document.getElementById('page2');

page2.onclick = function() {
    menu_page();
    document.getElementById('page_two').style.display = 'block';
    page2.className = 'on';

    search1.style.display = 'block';

    clearInterval(page1_setinterval);
    clearInterval(page3_setinterval);

    page = 'outlet_page';
    sen(page);
};


function outlet_do1(){

    var ou1_arr = ws_data.split("st");

    for(var i=1;i<ou1_arr.length;i++){
        ou1_arr[i] = 'st'+ou1_arr[i];

        var arr2 = ou1_arr[i].split(" ");

        var switch_status = arr2[0].split("=")[1];
        var realvol = arr2[1].split("=")[1];
        var cur = arr2[2].split("=")[1];
        var power = arr2[3].split("=")[1];
        var energy = arr2[4].split("=")[1];

        var table = document.getElementById('alternatecolor');

        var oneRow = table.insertRow();
        var cell1= oneRow.insertCell();
        var cell2=oneRow.insertCell();
        var cell3=oneRow.insertCell();
        var cell4=oneRow.insertCell();
        var cell5=oneRow.insertCell();
        var cell6=oneRow.insertCell();
        var cell7=oneRow.insertCell();

        cell1.className = 'ult_name';

        cell1.innerHTML = 'outlet'+i;

        if(switch_status == 1){
            cell2.innerHTML = "开";
        }else if(switch_status == 0){
            cell2.innerHTML = "关";
        }

        cell3.innerHTML = realvol;
        cell4.innerHTML = cur;
        cell5.innerHTML = power;
        cell6.innerHTML = energy;
        cell7.innerHTML = "<a class='detail' href='#'>查看</a>";
    }

}

/*首次之后执行*/
function outlet_do2(){
    var ou1_arr = ws_data.split("st");

    for(var i=1;i<ou1_arr.length;i++){
        ou1_arr[i] = 'st'+ou1_arr[i];

        var arr2 = ou1_arr[i].split(" ");

        var switch_status = arr2[0].split("=")[1];
        var realvol = arr2[1].split("=")[1];
        var cur = arr2[2].split("=")[1];
        var power = arr2[3].split("=")[1];
        var energy = arr2[4].split("=")[1];


        var table = document.getElementById('alternatecolor');

        table.rows[i].cells[0].innerHTML = 'outlet'+i;

        if(switch_status == 1){
            table.rows[i].cells[1].innerHTML = '开';
        }else if(switch_status == 0){
            table.rows[i].cells[1].innerHTML = '关';
        }

        table.rows[i].cells[2].innerHTML = realvol;
        table.rows[i].cells[3].innerHTML = cur;
        table.rows[i].cells[4].innerHTML = power;
        table.rows[i].cells[5].innerHTML = energy;
        table.rows[i].cells[6].innerHTML = '<a class="detail" href="#">查看</a>';
    }

}


function outlet_do(){

    var table = document.getElementById('alternatecolor');

    if(table.rows.length > 1){
        outlet_do2();
    }else{
        outlet_do1();
    }
}


function one_outlet_history(){
    var outlet_tab = document.getElementById('alternatecolor');
    var olt = outlet_tab.getElementsByClassName('detail');
    var olt_name = outlet_tab.getElementsByClassName('ult_name');

    for(var i=0;i<olt.length;i++){
        olt[i].a = i;

        olt[i].onclick =function(){

            clearInterval(page2_setinterval);

            outlet_text = olt_name[this.a].textContent;
            var outlet_content = outlet_text.split('outlet')[1];

            websocket.send('outlet_history:outlet='+outlet_content);


            document.getElementById('el_his').style.display = 'block';
            document.getElementById('top_div').style.display = 'block';
        }
    }
}

function every_outlet_history(){
    var table = document.getElementById('every_outlet');

    if(table.getElementsByClassName('outlet_hshead').length == 0){
        var oneRow1 = table.insertRow();

        oneRow1.innerHTML = ' <tr>'
        +'<th>编号</th>'
        +'<th>插座名称</th>'
        +'<th>时间</th>'
        +'<th>电能值(kWh)</th>'
        +'</tr>';

        oneRow1.className = 'outlet_hshead';
    }

    var every_history1 = ws_data.replace("branch ","");
    var every_history = every_history1.split("&");

    for(var i=0;i<every_history.length-1;i++){
        var every_hstdata = every_history[i].split(" ");

        var time_ym =  every_hstdata[0];
        var time_hs1 =  every_hstdata[1];
        var branch_power = every_hstdata[2];

        if(time_hs1 != undefined){
            var time_hs2 = time_hs1.replace(/-/g,":");
        }

        var oneRow2 = table.insertRow();

        var cell1= oneRow2.insertCell();
        var cell2= oneRow2.insertCell();
        var cell3= oneRow2.insertCell();
        var cell4= oneRow2.insertCell();

        outlet_history_num = outlet_history_num + 1 ;

        cell1.textContent = outlet_history_num;
        cell2.textContent = outlet_text;
        cell3.textContent = time_ym+' '+time_hs2;
        cell4.textContent =  branch_power;
    }

}


function outlet_historydo(){
    var every_outlet = document.getElementById('every_outlet');

    every_outlet.innerHTML = '';

    outlet_history_num = 0;
}


/**每个设备电能历史数据的分页**/
var outlet_prev_button = document.getElementById('outlet_prev');
var outlet_next_button = document.getElementById('outlet_next');

/*上一页*/
outlet_prev_button.onclick = function(){

    websocket.send('outlet_history:prev_page');
};

/*下一页*/
outlet_next_button.onclick = function(){

    websocket.send('outlet_history:next_page');
};

/**关闭历史数据弹框**/
document.getElementById('every_button').onclick = function(){
    document.getElementById('top_div').style.display = 'none';
    document.getElementById('el_his').style.display = 'none';

    page2_setinterval = setInterval(outlet_data,1000);
};




/**环境管理**/
function environment_data1(){
    websocket.send('envi_data');
}

function edit_temupfl(){
    var env = document.getElementById('environment_data');
    var edi = env.getElementsByClassName('edit');

    for(var i=0;i<edi.length;i++){
        edi[i].a = i;

        edi[i].onclick = function(){
            for(var i=0;i<1;i++){
               if(document.getElementById('tuplimit')){
                   document.getElementById('tuplimit').disabled=true;
                   document.getElementById('tlowlimit').disabled=true;
               }
               if(document.getElementById('humi_uplimit')){
                   document.getElementById('humi_uplimit').disabled =true;
               }
            }
            if(this.a < 1){
                document.getElementById('tuplimit').disabled=false;
                document.getElementById('tlowlimit').disabled=false;
            }else{
                document.getElementById('humi_uplimit').disabled=false;
            }
        }
    }
}

var page3 = document.getElementById('page3');

page3.onclick = function(){
    menu_page();

    document.getElementById('page_three').style.display = 'block';
    page3.className = 'on';

    search1.style.display = 'block';

    clearInterval(page1_setinterval);
    clearInterval(page2_setinterval);

    page = 'environment';
    sen(page);
};

function environment_give(){
    var  environm_arr = ws_data.split(" ");

    var tuplimit = environm_arr[0];
    var tlowlimit = environm_arr[1];
    var humi_uplimit = environm_arr[2];
    var tem_switc = environm_arr[3];
    var humi_switch = environm_arr[4];
    var smoke_switch = environm_arr[5];
    var door_switch = environm_arr[6];
    var out_switch = environm_arr[7];

    var tuplimit_data = tuplimit.split("=")[1];
    var tlowlimit_data = tlowlimit.split("=")[1];
    var humi_uplimit_data = humi_uplimit.split("=")[1];
    temp_switch_data = tem_switc.split("=")[1];
    hum_switch_data = humi_switch.split("=")[1];
    smoke_switch_data = smoke_switch.split("=")[1];
    door_switch_data = door_switch.split("=")[1];
    out_switch_data = out_switch.split("=")[1];


    if(temp_switch_data == 1 || hum_switch_data == 1 || smoke_switch_data == 1 || door_switch_data == 1 || out_switch_data == 1){
        clearInterval(page3_setinterval);
        page3_setinterval = setInterval(environment_data1,1000);
    }else{
        websocket.send('envi_data');
    }

    if(temp_switch_data == 1){
        document.getElementById('uplimit').innerHTML = '<input id="tuplimit" class="ceil" disabled="disabled" type="text" name="ceil" value='+tuplimit_data+'>';
        document.getElementById('lowlimit').innerHTML = '<input id="tlowlimit" class="floor" disabled="disabled" type="text" name="ceil" value='+tlowlimit_data+'>';
        document.getElementById('tem_edit').innerHTML = '<b style="color: dodgerblue;font-size: 14px;">'+'编辑'+'</b>';
        document.getElementById('sub_tem').innerHTML ='<input id="sub_temp" type="button" value="保存"/>'
        document.getElementById('t_switch').textContent = '开';

        if(document.getElementById('sub_temp') != undefined){
            temp_sub();
        }
    }else if(temp_switch_data == 0){
        document.getElementById('uplimit').innerHTML = '<span>'+'——'+'</span>';
        document.getElementById('lowlimit').innerHTML = '<span>'+'——'+'</span>';
        document.getElementById('tem_edit').innerHTML = '<span>'+'——'+'</span>';
        document.getElementById('sub_tem').innerHTML = '<span>'+'——'+'</span>';
        document.getElementById('t_switch').textContent = '关';
    }


    if(hum_switch_data == 1){
        document.getElementById('hm_uplimit').innerHTML = '<input id="humi_uplimit" class="ceil" disabled="disabled" type="text" name="ceil" value='+humi_uplimit_data+'>';
        document.getElementById('sub_hm').innerHTML = '<input id="sub_humi" type="button" value="保存"/>';
        document.getElementById('edit_humi').innerHTML = '<b style="color: dodgerblue;font-size: 14px;">'+'编辑'+'</b>';
        document.getElementById('h_switch').textContent = '开';

        if(document.getElementById('sub_humi') != undefined){
            humi_sub();
        }
    }else if(hum_switch_data == 0){
        document.getElementById('hm_uplimit').innerHTML = '<span>'+'——'+'</span>';
        document.getElementById('sub_hm').innerHTML = '<span>'+'——'+'</span>';
        document.getElementById('edit_humi').innerHTML = '<span>'+'——'+'</span>';
        document.getElementById('h_switch').textContent = '关';
    }


    if(smoke_switch_data == 1){
        document.getElementById('s_switch').textContent = '开';
    }else if(smoke_switch_data == 0){
        document.getElementById('s_switch').textContent = '关';
    }

    if(door_switch_data == 1){
        document.getElementById('d_switch').textContent = '开';
    }else if(door_switch_data == 0){
        document.getElementById('d_switch').textContent = '关';
    }


    if(out_switch_data == 1){
        document.getElementById('o_switch').textContent = '开';
    }else if(out_switch_data == 0){
        document.getElementById('o_switch').textContent = '关';
    }


    edit_temupfl();
}


function environment_timely(){
    var  ty_arr = ws_data.split(" ");

    var t_emperature = ty_arr[0];
    var h_umidity = ty_arr[1];
    var t_status = ty_arr[2];
    var h_status = ty_arr[3];
    var s_status = ty_arr[4];
    var d_status = ty_arr[5];
    var o_status = ty_arr[6];

    var t_emperature_data = t_emperature.split("=")[1];
    var h_umidity_data = h_umidity.split("=")[1];
    var t_status_data = t_status.split("=")[1];
    var h_status_data = h_status.split("=")[1];
    var s_status_data = s_status.split("=")[1];
    var d_status_data = d_status.split("=")[1];
    var o_status_data = o_status.split("=")[1];

    if(temp_switch_data == 1){
        document.getElementById('temperature').textContent = t_emperature_data;

        if(t_status_data == 0){
            document.getElementById('temp_status').textContent = '正常';
        }else if(t_status_data == 1){
            document.getElementById('temp_status').textContent = '告警';
        }
    }else if(temp_switch_data == 0){
        document.getElementById('temperature').textContent = '——';
        document.getElementById('temp_status').textContent = '——';
    }


    if(hum_switch_data == 1){
        document.getElementById('humidity').textContent = h_umidity_data;

        if(h_status_data == 0){
            document.getElementById('humi_status').textContent = '正常';
        }else if(h_status_data == 1){
            document.getElementById('humi_status').textContent = '告警';
        }
    }else if(hum_switch_data == 0){
        document.getElementById('humidity').textContent = '——';
        document.getElementById('humi_status').textContent = '——';
    }


    if(smoke_switch_data == 1){
        if(s_status_data == 1){
            document.getElementById('smoke_status').textContent = '正常';
        }else if(s_status_data == 0){
            document.getElementById('smoke_status').textContent = '告警';
        }
    }else if(smoke_switch_data == 0){
        document.getElementById('smoke_status').textContent = '——';
    }


    if(door_switch_data == 1){
        if(d_status_data == 1){
            document.getElementById('door_status').textContent = '开';
        }else if(d_status_data == 0){
            document.getElementById('door_status').textContent = '关';
        }
    }else if(door_switch_data == 0){
        document.getElementById('door_status').textContent = '——';
    }


    if(out_switch_data == 1){
        if(o_status_data == 1){
            document.getElementById('out_status').textContent = '开';
        }else if(o_status_data == 0){
            document.getElementById('out_status').textContent = '关';
        }
    }else if(out_switch_data == 0){
        document.getElementById('out_status').textContent = '——';
    }

}


var tsh = document.getElementById('t_switch');
tsh.onclick = function(){
    websocket.send('temp_switch');
};

function tem_switch(){
    if(ws_data == 't_switch ok'){
        if(temp_switch_data == 1){
            temp_switch_data = 0;
        }else{
            temp_switch_data = 1;
        }
    }else{
        alert('温度传感器的开关切换失败');
    }

    if(temp_switch_data == 1){
        document.getElementById('t_switch').textContent = '开';
    }else if(temp_switch_data == 0){
        document.getElementById('t_switch').textContent = '关';
    }
}

var hsh = document.getElementById('h_switch');
hsh.onclick = function(){
    websocket.send('humi_switch');
};

function hum_switch(){
    if(ws_data == 'h_switch ok'){
        if(hum_switch_data == 1){
            hum_switch_data = 0;
        }else{
            hum_switch_data = 1;
        }
    }else{
        alert('湿度传感器的开关切换失败');
    }

    if(hum_switch_data == 1){
        document.getElementById('h_switch').textContent = '开';
    }else if(hum_switch_data == 0){
        document.getElementById('h_switch').textContent = '关';
    }
}


var ssth = document.getElementById('s_switch');
ssth.onclick = function(){
    websocket.send('smoke_switch');
};

function smo_switch(){
    if(ws_data == 's_switch ok'){
        if(smoke_switch_data == 1){
            smoke_switch_data = 0;
        }else{
            smoke_switch_data = 1;
        }
    }else{
        alert('烟雾传感器的开关切换失败');
    }

    if(smoke_switch_data == 1){
        document.getElementById('s_switch').textContent = '开';
    }else if(smoke_switch_data == 0){
        document.getElementById('s_switch').textContent = '关';
    }
}

var door = document.getElementById('d_switch');
door.onclick = function(){
    websocket.send('door_switch');
};

function dr_switch(){
    if(ws_data == 'd_switch ok'){
        if(door_switch_data == 1){
            door_switch_data = 0;
        }else{
            door_switch_data = 1;
        }
    }else{
        alert('温度传感器的开关切换失败');
    }

    if(door_switch_data == 1){
        document.getElementById('d_switch').textContent = '开';
    }else if(door_switch_data == 0){
        document.getElementById('d_switch').textContent = '关';
    }
}


var out = document.getElementById('o_switch');
out.onclick = function(){
    websocket.send('out_switch');
};

function ou_switch(){
    if(ws_data == 'o_switch ok'){
        if(out_switch_data == 1){
            out_switch_data = 0;
        }else{
            out_switch_data = 1;
        }
    }else{
        alert('温度传感器的开关切换失败');
    }

    if(out_switch_data == 1){
        document.getElementById('o_switch').textContent = '开';
    }else if(out_switch_data == 0){
        document.getElementById('o_switch').textContent = '关';
    }
}

function temp_sub() {
    var sub_temp = document.getElementById('sub_temp');
    sub_temp.onclick = function () {
        var tem_up = document.getElementById('tuplimit').value;
        var tem_low = document.getElementById('tlowlimit').value;

        var reg1 = /^-?\d+\.?\d*$/;
        var reg2 = /^-?\d+\.?\d*$/;
        if (reg1.test(tem_up) && reg2.test(tem_low) && tem_up.length != 0 && tem_low.length != 0) {
            if(tem_up<=80 && tem_up>= -20 && tem_low<=80 && tem_low>=-20){
                if(tem_up > tem_low){
                    document.getElementById('tuplimit').disabled = true;
                    document.getElementById('tlowlimit').disabled = true;

                    websocket.send('set_pdu:temp_uplimit=' + tem_up + ':temp_lowlimit=' + tem_low);
                }else{
                    alert('上限值不能小于下限值');
                    return false;
                }

            }else{
                alert('可设置温度范围为-20~80');
                return false;
            }

        } else {
            alert('请输入合法的温度度上限值和下限值');
            return false;
        }
    };
}

function humi_sub(){

    var sub_humi = document.getElementById('sub_humi');

    sub_humi.onclick = function(){
        var humi_up = document.getElementById('humi_uplimit').value;

        var reg = new RegExp("^[0-9]*$");
        if(reg.test(humi_up) && humi_up.length != 0){
            if(humi_up>=0 && humi_up<=100){
                document.getElementById('humi_uplimit').disabled =true;

                websocket.send('set_pdu:humi_uplimit='+humi_up);
            }else{
               alert('湿度范围为0~100');
                return false;
            }

        }else{
            alert('请输入合法的湿度上限值');
            return false;
        }
    };
}



/**历史告警记录**/
var alarm_history_num = 0;
var page4 = document.getElementById('page4');

page4.onclick = function(){
    menu_page();
    document.getElementById('page_four').style.display = 'block';
    page4.className = 'on';

    search1.style.display = 'block';

    alarm_clean();

    clearInterval(page1_setinterval);
    clearInterval(page2_setinterval);
    clearInterval(page3_setinterval);

    page = 'alarm_data';

    sen(page);
};

function alarm_clean(){
    var table = document.getElementById('alternatecolor2');

    table.innerHTML = '';

    alarm_history_num = 0;
}


function alarm_data(){
    var table = document.getElementById('alternatecolor2');

    if(table.getElementsByClassName('altable_head').length == 0){
        var oneRow1 = table.insertRow();

        oneRow1.innerHTML = ' <tr>'
        +'<th>序号</th>'
        +'<th>告警时间</th>'
        +'<th>告警类型</th>'
        +'<th>告警来源</th>'
        +'<th>告警阀值</th>'
        +'<th>告警值</th>'
        +'<th>是否消除</th>'
        +'</tr>';

        oneRow1.className = 'altable_head';
        oneRow1.style.backgroundColor = '#C3DDE0';
    }

    var alarm_history1 = ws_data.replace("alarm ","");


    var alarm_history = alarm_history1.split("&");

    for(var i=0;i<alarm_history.length-1;i++){
        var alarm_hisdata = alarm_history[i].split(" ");

        var time_ym = alarm_hisdata[0];
        var time_hs1 = alarm_hisdata[1];
        var alarm_type = alarm_hisdata[2];
        var alarm_room = alarm_hisdata[3];
        var alarm_limit = alarm_hisdata[4];
        var alarm_value = alarm_hisdata[5];
        var is_clear = alarm_hisdata[6];


        if(time_hs1 != undefined){
            var time_hs2 = time_hs1.replace(/-/g,":");
        }

        var oneRow2 = table.insertRow();
        oneRow2.style.backgroundColor = '#D4E3E5';

        var cell1= oneRow2.insertCell();
        var cell2= oneRow2.insertCell();
        var cell3= oneRow2.insertCell();
        var cell4= oneRow2.insertCell();
        var cell5= oneRow2.insertCell();
        var cell6= oneRow2.insertCell();
        var cell7= oneRow2.insertCell();

        alarm_history_num = alarm_history_num + 1 ;

        cell1.textContent =  alarm_history_num;
        cell2.textContent = time_ym+' '+time_hs2;


        if(alarm_type == 1){
            cell3.textContent = '过压';
        }
        if(alarm_type == 2){
            cell3.textContent = '欠压';
        }
        if(alarm_type == 3){
            cell3.textContent = '过流';
        }
        if(alarm_type == 4){
            cell3.textContent = '过功率';
        }
        if(alarm_type == 5){
            cell3.textContent = '漏电流超限';
        }
        if(alarm_type == 6){
            cell3.textContent = '超高温';
        }
        if(alarm_type == 7){
            cell3.textContent = '超低温';
        }
        if(alarm_type == 8){
            cell3.textContent = '过湿';
        }
        if(alarm_type == 9){
            cell3.textContent = 'SPD故障';
        }

        if(parseInt(alarm_room) < 21){
           cell4.textContent = 'outlet'+alarm_room;
        }else if(parseInt(alarm_room) > 20){
           cell4.textContent = '无';
        }

        cell5.textContent = alarm_limit;
        cell6.textContent = alarm_value;


        if(is_clear == 0){
            cell7.textContent = '是';
        }
        if(is_clear == 1){
            cell7.textContent = '否';
        }

    }
}





/**设备管理**/
var page5 = document.getElementById('page5');

page5.onclick = function(){
    menu_page();
    document.getElementById('page_five').style.display = 'block';
    page5.className = 'on';

    search1.style.display = 'none';

    clearInterval(page1_setinterval);
    clearInterval(page2_setinterval);
    clearInterval(page3_setinterval);

    page = 'device_manage';

    sen(page);
};


function device_data(){
    var dev1 = document.getElementById('equipment_name1');
    var dev2 = document.getElementById('equipment_name2');


    if(dev1.options.length > 0){
        dev1.options.length = 0;
    }

    if(dev2.options.length > 0){
        dev2.options.length = 0;
    }


    for(var i=0;i<pdu_arr.length;i++){
        dev1.options.add(new Option(pdu_arr[i],pdu_arr[i]));
        dev2.options.add(new Option(pdu_arr[i],pdu_arr[i]));
    }

    var dev_arr = ws_data.split(" ");


    var location = dev_arr[0].split("=")[1];

    var mode = dev_arr[1].split("=")[1];

    document.getElementById('location').value = location;

    var m =document.getElementById('mode');

    if(m.options.length > 0){
        m.options.length = 0;
    }

    if(mode == 1){
        m.options.add(new Option('Master','Master'));
        m.options.add(new Option('Slave','Slave'));
    }else if(mode == 0){
        m.options.add(new Option('Slave','Slave'));
        m.options.add(new Option('Master','Master'));
    }
}


var sub_locat = document.getElementById('sub_locat');

sub_locat.onclick = function(){
    var eqn = document.getElementById('equipment_name1');
    var indexed = eqn.selectedIndex;
    var p_id = eqn.options[indexed].value;

    var loc = document.getElementById('location').value;

    websocket.send('set_pdu:id='+p_id+':location='+loc);
};


var sub_mode = document.getElementById('sub_mode');

sub_mode.onclick = function(){
    var m = document.getElementById('mode');
    var indexed = m.selectedIndex;
    var mode_name = m.options[indexed].value;

    if(mode_name == 'Master'){
        websocket.send('set_pdu:mode=1');
    }else if(mode_name == 'Slave'){
        websocket.send('set_pdu:mode=0');
    }
};



var upload = document.getElementById('uploadfile');

var reader = null;
var step = 1024;
var cuLoaded = 0;
var file = null;
var enableRead = true;
var total = 0;
var startTime = null;
var eachsize = 0;
var stop_continue;

upload.onchange = function () {
    file = this.files[0];
    total = file.size;

    var file_name = file.name;
    var file_last = file_name.split(".");
    var file_last2 = file_last[file_last.length-1];

    /*判断文件类型*/
    if(file_last2 == 'bin'){
        document.getElementById('upload').disabled = false;
    }else{
        alert('该文件不是bin文件');
        return false;
    }
};


document.getElementById('upload').onclick = function(){
    stop_continue = 1;

    document.getElementById('upload_progress').style.display = 'block';
    document.getElementById('upload_top_div').style.display = 'block';
    websocket.send('startpack:appsize='+total);
};

function do_upload(){
    if(stop_continue == 1){
        bindReader();
    }else{
        return false;
    }
}


function bindReader() {
    startTime = new Date();
    enableRead = true;
    reader = new FileReader();

    reader.onload = function (e) {
        console.info('读取总数：' + e.loaded);

        eachsize = e.loaded;
        if(eachsize < step){
            loadSuccess_last(e.loaded);
        }else{
            loadSuccess(e.loaded);
        }

    };

    readBlob();
}

function loadSuccess(loaded) {

    var blob2 = reader.result;

    var blob3 = new Blob(['datapack:size='+eachsize+':data=',blob2], {
        type: 'text/plain'
    });

    websocket.send(blob3);

    var percent1 = (cuLoaded / total) * 100;
    var percent = percent1.toFixed(1);

    document.getElementById('Status').innerText = percent + '%';
    document.getElementById('progressOne').value = percent;
}

function loadSuccess_last() {

    var blob2 = reader.result;

    var blob3 = new Blob(['endpack:size='+eachsize+':data=',blob2], {
        type: 'text/plain'
    });

    websocket.send(blob3);


    var percent1 = (cuLoaded / total) * 100;
    var percent = Math.round(percent1);

    document.getElementById('Status').innerText = percent;
    document.getElementById('progressOne').value = percent;

}

function readBlob() {

    var blob1 = file.slice(cuLoaded, cuLoaded + step);

    reader.readAsArrayBuffer(blob1);
}

/*升级的暂停继续*/
var stop_up = document.getElementById('stop_upload');

stop_up.onclick = function(){
    if(stop_continue == 1){
        stop_up.textContent = '继续';
        stop_continue = 0;
    }else if(stop_continue == 0){
        stop_up.textContent = '暂停';
        stop_continue = 1;

        do_upload();
    }
};
/*取消升级*/
var res_up = document.getElementById('res_upload');

res_up.onclick = function(){
    location.reload();
};





/**电源管理**/
var page6 = document.getElementById('page6');

page6.onclick = function(){
    menu_page();
    document.getElementById('page_six').style.display = 'block';
    page6.className = 'on';

    search1.style.display = 'block';

    clearInterval(page1_setinterval);
    clearInterval(page2_setinterval);
    clearInterval(page3_setinterval);

    page = 'power_manage';

    sen(page);
};


function power_do(){
    var table = document.getElementById('alternatecolor3');

    if(table.rows.length > 1){
        switch_arr.splice(0,switch_arr.length);
        power_do2();
    }else{
        power_do1();
    }
}

var switch_arr = new Array();
var power_numsend;

function power_do1(){
    var p_arr = ws_data.split("tp");

    for(var i=1;i<p_arr.length;i++){
        p_arr[i] = 'tp'+p_arr[i];

        var p_arr2 = p_arr[i].split(" ");

        var oulet_type = p_arr2[0].split("=")[1];
        var status = p_arr2[1].split("=")[1];
        var vol_up = p_arr2[2].split("=")[1];
        var vol_low = p_arr2[3].split("=")[1];
        var cur_up = p_arr2[4].split("=")[1];
        var power_up = p_arr2[5].split("=")[1];
        var on_time = p_arr2[6].split("=")[1];

        switch_arr.push(status);


        var table = document.getElementById('el_config');
        var oneRow = table.insertRow();
        var cell1= oneRow.insertCell();
        var cell2=oneRow.insertCell();
        var cell3=oneRow.insertCell();
        var cell4=oneRow.insertCell();
        var cell5=oneRow.insertCell();
        var cell6=oneRow.insertCell();
        var cell7=oneRow.insertCell();
        var cell8=oneRow.insertCell();
        var cell9=oneRow.insertCell();
        var cell10=oneRow.insertCell();
        var cell11=oneRow.insertCell();


        cell1.innerHTML = '<td><input style="border: none;" class="power_num" disabled="disabled" type="text" value=outlet'+i+ '> </td>';
        cell2.innerHTML = '<td><input class="type" disabled="disabled" type="text"  value='+oulet_type+ '> </td>';
        cell3.innerHTML = '<td><input class="uceil" disabled="disabled" type="text" value='+vol_up+'> </td>';
        cell4.innerHTML = '<td><input class="ufloor" disabled="disabled" type="text" value='+vol_low+'> </td>';
        cell5.innerHTML = '<td><input class="aceil" disabled="disabled" type="text" value='+cur_up+'> </td>';
        cell6.innerHTML = '<td><input class="pceil" disabled="disabled" type="text" value='+power_up+'> </td>';
        cell7.innerHTML = '<td><input class="opentime" disabled="disabled" type="text" value='+on_time+'> </td>';
        cell8.innerHTML = '<td><input class="sub_power" style="font-size: 12px;" type="button" value="保存"/> </td>';

        if(status == 1){
            cell9.innerHTML = '<button class="outlet_switch" style="padding: 3px;">开</button>';
        }else if(status == 0){
            cell9.innerHTML = '<button class="outlet_switch" style="padding: 3px;">关</button>';
        }

        cell10.innerHTML = '<a class="edit" href="#">编辑</a>';
        /**预约电源行程**/
        cell11.innerHTML = '<button class="appoint_button" name='+"outlet"+i+'>预约电源行程</button>';
    }


    var el = document.getElementById('el_config');
    var ed = el.getElementsByClassName('edit');

    var type = el.getElementsByClassName('type');
    var uceil = el.getElementsByClassName('uceil');
    var ufloor = el.getElementsByClassName('ufloor');
    var aceil = el.getElementsByClassName('aceil');
    var pceil = el.getElementsByClassName('pceil');
    var opentime = el.getElementsByClassName('opentime');

    for(var j=0;j<ed.length;j++){
        ed[j].a = j;

        ed[j].onclick = function(){

            for(var j=0;j<ed.length;j++){
                type[j].disabled=true;
                uceil[j].disabled=true;
                ufloor[j].disabled=true;
                aceil[j].disabled=true;
                pceil[j].disabled=true;
                opentime[j].disabled=true;
            }
            type[this.a].disabled=false;
            uceil[this.a].disabled=false;
            ufloor[this.a].disabled=false;
            aceil[this.a].disabled=false;
            pceil[this.a].disabled=false;
            opentime[this.a].disabled=false;
        }
    }


    var s_power = el.getElementsByClassName('sub_power');
    var pnum = el.getElementsByClassName('power_num');

    for(var k=0;k<s_power.length;k++){
        s_power[k].b=k;

        s_power[k].onclick = function() {

            for (var k = 0; k < s_power.length; k++) {
                pnum[k].disabled = true;
                type[k].disabled = true;
                uceil[k].disabled = true;
                ufloor[k].disabled = true;
                aceil[k].disabled = true;
                pceil[k].disabled = true;
                opentime[k].disabled = true;
            }

            var power_num = pnum[this.b].value.split("outlet")[1];
            var out_type = type[this.b].value;
            var vol_uplimit = uceil[this.b].value;
            var vol_lowlimit = ufloor[this.b].value;
            var cur_limit = aceil[this.b].value;
            var power_limit = pceil[this.b].value;
            var on_time = opentime[this.b].value;

            /*对输入的值进行验证*/
            var reg1 = /^[A-Za-z0-9]{3}$/;
            var reg2 = /^-?\d+\.?\d*$/;
            if (out_type != '' && vol_uplimit != '' && vol_lowlimit != '' && cur_limit != '' && power_limit != '' && on_time != '') {
                if (reg1.test(out_type)) {
                    if (reg2.test(vol_uplimit) && vol_uplimit>=0 && vol_uplimit<=400) {
                        if (reg2.test(vol_lowlimit) && vol_uplimit>=0 && vol_uplimit<=400) {
                            if (reg2.test(cur_limit) && cur_limit>=0 && cur_limit<=32) {
                                if (reg2.test(power_limit) && power_limit>=0 && power_limit<=16) {
                                    if (reg2.test(on_time) && on_time>=0 && on_time<=60) {
                                        websocket.send('set_outlet:outlet='+power_num+':out_type='+out_type+':vol_uplimit='+vol_uplimit+':vol_lowlimit='+vol_lowlimit+':cur_limit='+cur_limit+':power_limit='+power_limit+':on_time='+on_time);
                                    } else {
                                        alert('上电时间为大于等于0，小于等于60的数字类型');
                                        page = 'power_manage';
                                        sen(page);
                                    }
                                } else {
                                    alert('功率上限为大于等于0，小于等于16的数字类型');
                                    page = 'power_manage';
                                    sen(page);
                                }
                            } else {
                                alert('电流上限为大于等于0，小于等于32的数字类型');
                                page = 'power_manage';
                                sen(page);
                            }
                        } else {
                            alert('电压下限为大于等于0，小于等于400的数字类型');
                            page = 'power_manage';
                            sen(page);
                        }
                    } else {
                        alert('电压上限为大于等于0，小于等于400的数字类型');
                        page = 'power_manage';
                        sen(page);
                    }
                } else {
                    alert('插座类型只能为数字或字母组成的啊三位字符');
                    page = 'power_manage';
                    sen(page);
                }
            } else {
                alert('输入不能为空');
                page = 'power_manage';
                sen(page);
            }

        }
    }


    var outlet_switch = el.getElementsByClassName('outlet_switch');

    for(var n=0;n<outlet_switch.length;n++){
        outlet_switch[n].c = n;

        outlet_switch[n].onclick = function(){
            power_numsend = this.c;

            var power_num = pnum[this.c].value.split("outlet")[1];

            websocket.send('outlet_switch:outlet='+power_num);
        }
    }

}


function power_do2(){
    var p_arr = ws_data.split("tp");

    for(var i=1;i<p_arr.length;i++){
        p_arr[i] = 'tp'+p_arr[i];

        var p_arr2 = p_arr[i].split(" ");

        var oulet_type = p_arr2[0].split("=")[1];
        var status = p_arr2[1].split("=")[1];
        var vol_up = p_arr2[2].split("=")[1];
        var vol_low = p_arr2[3].split("=")[1];
        var cur_up = p_arr2[4].split("=")[1];
        var power_up = p_arr2[5].split("=")[1];
        var on_time = p_arr2[6].split("=")[1];

        switch_arr.push(status);

        var table = document.getElementById('alternatecolor3');

        table.rows[i].cells[0].innerHTML = '<input style="border: none;" class="power_num" disabled="disabled" type="text" value=outlet'+i+ '>';
        table.rows[i].cells[1].innerHTML = '<input class="type" disabled="disabled" type="text"  value='+oulet_type+'>';
        table.rows[i].cells[2].innerHTML = '<input class="uceil" disabled="disabled" type="text" value='+vol_up+'>';
        table.rows[i].cells[3].innerHTML = '<input class="ufloor" disabled="disabled" type="text" value='+vol_low+'>';
        table.rows[i].cells[4].innerHTML = '<input class="aceil" disabled="disabled" type="text" value='+cur_up+'>';
        table.rows[i].cells[5].innerHTML = '<input class="pceil" disabled="disabled" type="text" value='+power_up+'>';
        table.rows[i].cells[6].innerHTML = '<input class="opentime" disabled="disabled" type="text" value='+on_time+'>';
        table.rows[i].cells[7].innerHTML = '<input class="sub_power" style="font-size: 12px;" type="button" value="保存"/>';

        if(status == 1){
            table.rows[i].cells[8].innerHTML = '<button class="outlet_switch" style="padding: 3px;">开</button>';
        }else if(status == 0){
            table.rows[i].cells[8].innerHTML = '<button class="outlet_switch" style="padding: 3px;">关</button>';
        }

        table.rows[i].cells[9].innerHTML = '<a class="edit" href="#">编辑</a>';
        /**预约电源行程**/
        table.rows[i].cells[10].innerHTML = '<button class="appoint_button" name='+"outlet"+i+'>预约电源行程</button>';

    }


    var el = document.getElementById('el_config');
    var ed = el.getElementsByClassName('edit');

    var type = el.getElementsByClassName('type');
    var uceil = el.getElementsByClassName('uceil');
    var ufloor = el.getElementsByClassName('ufloor');
    var aceil = el.getElementsByClassName('aceil');
    var pceil = el.getElementsByClassName('pceil');
    var opentime = el.getElementsByClassName('opentime');

    for(var j=0;j<ed.length;j++){
        ed[j].a = j;

        ed[j].onclick = function(){

            for(var j=0;j<ed.length;j++){
                type[j].disabled=true;
                uceil[j].disabled=true;
                ufloor[j].disabled=true;
                aceil[j].disabled=true;
                pceil[j].disabled=true;
                opentime[j].disabled=true;
            }
            type[this.a].disabled=false;
            uceil[this.a].disabled=false;
            ufloor[this.a].disabled=false;
            aceil[this.a].disabled=false;
            pceil[this.a].disabled=false;
            opentime[this.a].disabled=false;
        }
    }



    var s_power = el.getElementsByClassName('sub_power');
    var pnum = el.getElementsByClassName('power_num');

    for(var k=0;k<s_power.length;k++){
        s_power[k].b=k;

        s_power[k].onclick = function(){

           for(var k=0;k<s_power.length;k++){
                pnum[k].disabled=true;
                type[k].disabled=true;
                uceil[k].disabled=true;
                ufloor[k].disabled=true;
                aceil[k].disabled=true;
                pceil[k].disabled=true;
                opentime[k].disabled=true;
            }

            var power_num = pnum[this.b].value.split("outlet")[1];
            var out_type = type[this.b].value;
            var vol_uplimit = uceil[this.b].value;
            var vol_lowlimit = ufloor[this.b].value;
            var cur_limit = aceil[this.b].value;
            var power_limit = pceil[this.b].value;
            var on_time = opentime[this.b].value;


            /*对输入的值进行验证*/
            var reg1 = /^[A-Za-z0-9]{3}$/;
            var reg2 = /^-?\d+\.?\d*$/;
            if (out_type != '' && vol_uplimit != '' && vol_lowlimit != '' && cur_limit != '' && power_limit != '' && on_time != '') {
                if (reg1.test(out_type)) {
                    if (reg2.test(vol_uplimit) && vol_uplimit>=0 && vol_uplimit<=400) {
                        if (reg2.test(vol_lowlimit) && vol_uplimit>=0 && vol_uplimit<=400) {
                            if (reg2.test(cur_limit) && cur_limit>=0 && cur_limit<=32) {
                                if (reg2.test(power_limit) && power_limit>=0 && power_limit<=16) {
                                    if (reg2.test(on_time) && on_time>=0 && on_time<=60) {
                                        websocket.send('set_outlet:outlet='+power_num+':out_type='+out_type+':vol_uplimit='+vol_uplimit+':vol_lowlimit='+vol_lowlimit+':cur_limit='+cur_limit+':power_limit='+power_limit+':on_time='+on_time);
                                    } else {
                                        alert('上电时间为大于等于0，小于等于60的数字类型');
                                        page = 'power_manage';
                                        sen(page);
                                    }
                                } else {
                                    alert('功率上限为大于等于0，小于等于16的数字类型');
                                    page = 'power_manage';
                                    sen(page);
                                }
                            } else {
                                alert('电流上限为大于等于0，小于等于32的数字类型');
                                page = 'power_manage';
                                sen(page);
                            }
                        } else {
                            alert('电压下限为大于等于0，小于等于400的数字类型');
                            page = 'power_manage';
                            sen(page);
                        }
                    } else {
                        alert('电压上限为大于等于0，小于等于400的数字类型');
                        page = 'power_manage';
                        sen(page);
                    }
                } else {
                    alert('插座类型只能为数字或字母组成的啊三位字符');
                    page = 'power_manage';
                    sen(page);
                }
            } else {
                alert('输入不能为空');
                page = 'power_manage';
                sen(page);
            }
        }
    }


    var outlet_switch = el.getElementsByClassName('outlet_switch');

    for(var n=0;n<outlet_switch.length;n++){
        outlet_switch[n].c = n;

        outlet_switch[n].onclick = function(){
            power_numsend = this.c;

           var power_num = pnum[this.c].value.split("outlet")[1];

            websocket.send('outlet_switch:outlet='+power_num);
        }
    }

}


/*开关实时切换*/
function swit_success(){
    var el = document.getElementById('el_config');
    var outlet_switch = el.getElementsByClassName('outlet_switch');

    if(outlet_switch[power_numsend].innerHTML == '关'){
        outlet_switch[power_numsend].innerHTML = '开';
    }else{
        outlet_switch[power_numsend].innerHTML = '关';
    }
}


/**电源行程预约**/
function each_appointment(){
    var el_config_tab = document.getElementById('el_config');
    var appo = el_config_tab.getElementsByClassName('appoint_button');

    for(var i=0;i<appo.length;i++){
        appo[i].a = i;

        appo[i].onclick =function(){
            var outlet_num = this.name;

            outlet_appoint_do();

            websocket.send('outlet_order:list');

            document.getElementById('appointment').style.display = 'block';

            document.getElementById('appoint_outlet').value = outlet_num;

            document.getElementById('top_div').style.display = 'block';
        }
    }
}


/**电源行程预约列表**/
function every_outlet_appointment(){
    var table = document.getElementById('every_appointment');

    if(table.getElementsByClassName('outlet_hshead').length == 0){
        var oneRow1 = table.insertRow();

        oneRow1.innerHTML = ' <tr>'
        +'<th>插座名称</th>'
        +'<th>是否每天重复</th>'
        +'<th>时间</th>'
        +'<th>动作</th>'
        +'<th>删除操作</th>'
        +'</tr>';

        oneRow1.className = 'outlet_hshead';
    }

    var every_appoint = ws_data.split("out");

    for(var i=1;i<every_appoint.length;i++){

        every_appoint[i] = 'out'+every_appoint[i];

        var every_appoint2 = every_appoint[i].split(" ");

        var outlet = every_appoint2[0].split("=")[1];
        var repeat = every_appoint2[1].split("=")[1];

        /*时间处理*/
        var time1 = every_appoint2[2].split("=")[1];
        var time2 = every_appoint2[3];
        var date = time1+" "+time2;

        var action = every_appoint2[4].split("=")[1];
        var num = every_appoint2[5].split("=")[1];

        var oneRow2 = table.insertRow();

        var cell1= oneRow2.insertCell();
        var cell2= oneRow2.insertCell();
        var cell3= oneRow2.insertCell();
        var cell4= oneRow2.insertCell();
        var cell5= oneRow2.insertCell();


        cell1.textContent = 'outlet'+outlet;

        if(repeat == 1){
            cell2.textContent = '是';
        }else if(repeat == 0){
            cell2.textContent = '否';
        }

        cell3.textContent = date;

        if(action == 1){
            cell4.textContent = '闭合';
        }else if(action == 0){
            cell4.textContent = '断开';
        }


        cell5.innerHTML = '<button type="button" onclick="appoint_delete('+num+')">删除</button>';
    }

}

function outlet_appoint_do(){
    var every_appointment = document.getElementById('every_appointment');

    every_appointment.innerHTML = '';

}

/**电源行程预约设置**/
go_appointment();
function go_appointment(){

    var appoint_sub = document.getElementById('appoint_sub');
    appoint_sub.onclick = function(){

        /*插座序号*/
        var outlet_num = document.getElementById('appoint_outlet').value;
        var outlet = outlet_num.split('outlet')[1];

        /*是否每天重复*/
        var repeat = document.getElementById('appoint_repeat').value;

        /*动作*/
        var action = document.getElementById('appoint_action').value;

        /*时间*/
        var date = document.getElementById('appoint_date').value;

        var re = /^(\d){4}\-(\d){1,2}\-(\d){1,2}\s(\d){1,2}\-(\d){1,2}\-(\d){1,2}$/;

        if(date !== ''){

            if(re.test(date)){

                websocket.send('outlet_order:outlet='+outlet+':repeat='+repeat+':action='+action+':date='+date);

            }else{

                alert('时间格式不合法！应该为:年-月-日 时-分-秒');
                return false;
            }

        }else{

            alert('时间不能为空');
            return false;
        }

    }
}

/**删除预约**/
function appoint_delete(num){

  websocket.send('outlet_order:delete_num='+num);

}




/**网络管理**/
var  page7 = document.getElementById('page7');

page7.onclick = function(){
    menu_page();
    document.getElementById('page_seven').style.display = 'block';
    page7.className = 'on';

    search1.style.display = 'none';

    var show = document.getElementById("pctime");
    var st= document.getElementById('stop');

    if(intime == null){
        intime =  setInterval(timely,1000);
    }

    st.onclick = function(){
        clearInterval(intime);
        intime = null;
        show.disabled = false;
    };

    clearInterval(page1_setinterval);
    clearInterval(page2_setinterval);
    clearInterval(page3_setinterval);

    page = 'netconfig';

    sen(page);
};


function timely(){
    var show = document.getElementById('pctime');

    var time = new Date();

    var m = time.getMonth() + 1;
    var t = time.getFullYear() + "-" + m + "-"
        + time.getDate() + " " + time.getHours() + ":"
        + time.getMinutes() + ":" + time.getSeconds();
    show.value = t;
}


function internet_data(){
    var netpdu_name = document.getElementById('netpdu_name');
    var timepdu_name = document.getElementById('timepdu_name');

    if(netpdu_name.options.length > 0){
        netpdu_name.options.length = 0;
    }

    if(timepdu_name.options.length > 0){
        timepdu_name.options.length = 0;
    }


    for(var i=0;i<pdu_arr.length;i++){
        netpdu_name.options.add(new Option(pdu_arr[i],pdu_arr[i]));
        timepdu_name.options.add(new Option(pdu_arr[i],pdu_arr[i]));
    }


    var in_arr = ws_data.split(" ");

    var mac = in_arr[0].split("=")[1];
    var ip = in_arr[1].split("=")[1];
    var netmask = in_arr[2].split("=")[1];
    var gateway = in_arr[3].split("=")[1];
    var dnsserver = in_arr[4].split("=")[1];
    var serverport = in_arr[5].split("=")[1];


    document.getElementById('mac').value = mac;
    document.getElementById('ip').value = ip;
    document.getElementById('netmask').value = netmask;
    document.getElementById('gateway').value = gateway;
    document.getElementById('dnsserver').value = dnsserver;
    document.getElementById('serverport').value = serverport;

}


var sub_net= document.getElementById('sub_internet');

sub_net.onclick = function(){

    var netn = document.getElementById('netpdu_name');
    var indexed = netn.selectedIndex;
    var n_id = netn.options[indexed].value;

    var mac1 = document.getElementById('mac').value;
    var ip = document.getElementById('ip').value;
    var netmask = document.getElementById('netmask').value;
    var gateway = document.getElementById('gateway').value;
    var dns = document.getElementById('dnsserver').value;
    var port = document.getElementById('serverport').value;

    if(mac1 == ''){
        alert('Mac地址不能为空!');
        return false;
    }
    if(ip == ''){
        alert('IP地址不能为空!');
        return false;
    }
    if(netmask == ''){
        alert('子网掩码不能为空!');
        return false;
    }
    if(gateway == ''){
        alert('网关不能为空!');
        return false;
    }

    if(ip == netmask || netmask == gateway  || ip == gateway)
    {
       alert('IP地址与子网掩码、网关地址不能相同'); //3个地址不能相同
    }
    var static_ip_arr = new Array;
    var static_mask_arr = new Array;
    var static_gw_arr = new Array;

    static_ip_arr = ip.split(".");
    static_mask_arr = netmask.split(".");
    static_gw_arr = gateway.split(".");

    var res0 = parseInt(static_ip_arr[0]) & parseInt(static_mask_arr[0]);
    var res1 = parseInt(static_ip_arr[1]) & parseInt(static_mask_arr[1]);
    var res2 = parseInt(static_ip_arr[2]) & parseInt(static_mask_arr[2]);
    var res3 = parseInt(static_ip_arr[3]) & parseInt(static_mask_arr[3]);

    var res0_gw = parseInt(static_gw_arr[0]) & parseInt(static_mask_arr[0]);
    var res1_gw = parseInt(static_gw_arr[1]) & parseInt(static_mask_arr[1]);
    var res2_gw = parseInt(static_gw_arr[2]) & parseInt(static_mask_arr[2]);
    var res3_gw = parseInt(static_gw_arr[3]) & parseInt(static_mask_arr[3]);

    /**MAC地址16进制转为十进制**/
    var mac_arr = mac1.split("-");

    for(var i=0;i<mac_arr.length;i++){

        mac_arr[i] = parseInt(mac_arr[i],16);
    }

    var mac='';
    for(var j=0;j<mac_arr.length;j++){
        if(mac !== ''){
            mac = mac + '.'+mac_arr[j];
        }else{

            mac= mac_arr[j];
        }
    }


    if(res0==res0_gw && res1==res1_gw && res2==res2_gw  && res3==res3_gw)
    {
        websocket.send('set_pdu:id='+n_id+':mac='+mac+':ip='+ip+':netmask='+netmask+':gateway='+gateway+':dns='+dns+':port='+port);

    }else{
        alert('IP地址与子网掩码、网关地址不匹配');
        return false;
    }
};



var sub_time = document.getElementById('sub_time');

sub_time.onclick = function(){

    document.getElementById("pctime").disabled = true;

    if(intime == null){
        intime = setInterval(timely, 1000);
    }


    var tpdn = document.getElementById('timepdu_name');
    var indexed = tpdn.selectedIndex;
    var t_id = tpdn.options[indexed].value;

    var time1 = document.getElementById('pctime').value;
    var time2 = time1.split(" ")[0];
    var time3 = time1.split(" ")[1];

    var time4 = time3.replace(/:/g,"-");
    var time = time2+' '+time4;

    websocket.send('set_pdu:id='+t_id+':sync_date='+time);
};


function is_time(){
    var time1 = document.getElementById('pctime').value;

    var re = /^(\d+)\-(\d+)\-(\d+)\s(\d+):(\d+):(\d+)$/;

    if(re.test(time1) == true){

    }else{
        alert('时间格式不合法');
    }
}




/**用户管理**/
var page8 = document.getElementById('page8');

page8.onclick = function(){
    menu_page();
    document.getElementById('admin_content').style.display = 'block';
    page8.className = 'on';

    search1.style.display = 'none';

    clearInterval(page1_setinterval);
    clearInterval(page2_setinterval);
    clearInterval(page3_setinterval);

    if(this_admin_type == 2 || this_admin_type == null || this_admin_type == 0){

    document.getElementById('admin_content').innerHTML = '<span style="font-size: 16px;font-weight: 800;color: red">您没有权限访问该模块！</span>';

    }else if(this_admin_type == 1){
        admin_do();

        search1.style.display = 'none';

        clearInterval(page1_setinterval);
        clearInterval(page2_setinterval);
        clearInterval(page3_setinterval);

        page = 'username';
        sen(page);
    }
};

function admin_do(){
    var admin_tb = document.getElementById('alternatecolor4');

    admin_tb.innerHTML = '';
}


function admin_data(){
    var admin_table = document.getElementById('alternatecolor4');

    var oneRow1 = admin_table.insertRow();

    oneRow1.innerHTML = '<tr>'
    +'<th>用户名</th>'
    +'<th>用户状态</th>'
    +'<th>操作</th>'
    +'</tr>';

    oneRow1.style.backgroundColor = '#C3DDE0';


     var admin_arr1 = ws_data.split("name");

    for(var i=1;i<admin_arr1.length;i++){
        admin_arr1[i] = 'name'+ admin_arr1[i];

        var admin_arr2 = admin_arr1[i].split(" ");

        var username = admin_arr2[0].split("=")[1];
        var usertype = admin_arr2[1].split("=")[1];


        var oneRow2 = admin_table.insertRow();
        oneRow2.style.backgroundColor = '#D4E3E5';

        var cell1= oneRow2.insertCell();
        cell1.className = 'u_name';

        var cell2=oneRow2.insertCell();
        var cell3=oneRow2.insertCell();

        cell1.textContent = username;
        cell2.textContent = usertype;

        if(usertype == 1){
            cell3.innerHTML = '<a style="display:none" href="#" class="admin_del">删除</a>';
        }else if(usertype == 2){
            cell3.innerHTML = '<a href="#" class="admin_del">删除</a>';
        }
    }
}

function admin_del(){
    var tb = document.getElementById('alternatecolor4');
    var del = tb.getElementsByClassName('admin_del');
    var username = tb.getElementsByClassName('u_name');

    for(var i=0;i<del.length;i++){
       del[i].a = i;

        del[i].onclick = function(){
            var username1 = username[this.a].textContent;
				
            websocket.send('delete_user:name='+username1);
        }
    }
}


function add_admin(){
    var username = document.getElementById('username').value;
    var password = document.getElementById('password').value;


   var u1 = document.getElementById('alternatecolor4');
   var u_names = u1.getElementsByClassName('u_name');

    var reg = /^[A-Za-z0-9]{2,20}$/;
    var reg2 = /^[A-Za-z0-9]{6,16}$/;

    if(username != ''){
        if(password != ''){
            if(reg.test(username)){
                if(reg2.test(password)){
                    for(var i=0;i<u_names.length;i++){
                        var user_text = u_names[i].textContent;

                        if(username == user_text){
                            alert('用户名已存在');
                            return false;
                        }else{
                            websocket.send('add_user:name='+username+':password='+password);
                            document.getElementById('add').style.display = 'none';
                        }
                    }
                }else{
                    alert('密码为6-16位的字符');
                    return false;
                }
            }else{
                alert('用户名由字母或数字组成，长度为2到20位');
                return false;
            }
        }else{
            alert('密码不能为空');
            return false;
        }
    }else{
        alert('用户名不能为空');
        return false;
    }
}

/**页面切换**/
function menu_page(){
    document.getElementById('page_one').style.display = 'none';
    document.getElementById('page_two').style.display = 'none';
    document.getElementById('page_three').style.display = 'none';
    document.getElementById('page_four').style.display = 'none';
    document.getElementById('page_five').style.display = 'none';
    document.getElementById('page_six').style.display = 'none';
    document.getElementById('page_seven').style.display = 'none';
    document.getElementById('admin_content').style.display = 'none';

    document.getElementById('page1').className = '';
    document.getElementById('page2').className = '';
    document.getElementById('page3').className = '';
    document.getElementById('page4').className = '';
    document.getElementById('page5').className = '';
    document.getElementById('page6').className = '';
    document.getElementById('page7').className = '';
    document.getElementById('page8').className = '';
}


/**历史数据**/
document.getElementById('top_div').onclick = function(){
    document.getElementById('power_history').style.display = 'none';
    document.getElementById('top_div').style.display = 'none';
    document.getElementById('el_his').style.display = 'none';
    document.getElementById('appointment').style.display = 'none';

    if(document.getElementById('page_one').style.display == 'block' ||document.getElementById('page_one').style.display==''){

        page1_setinterval = setInterval(pdu_data,1000);
    }else if(document.getElementById('page_two').style.display == 'block'){

        page2_setinterval = setInterval(outlet_data,1000);
    }
};