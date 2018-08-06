此文档说明使用makefsdata.exe文件的使用方法：

功能：编译网页WEB文件fs（或文件夹fs目录及目录下的文件）生成工程所需二进制文件fsdata.c

方法：
	1、在makefsdata.exe文件所在目录启用命令窗口（按住shift键点击鼠标右键，选择"在此处打开命令窗口"）

	2、命令行中运行makefsdata.exe文件，并添加参数“-11”(数字11)；（即: makefsdata.exe -11）

	3、回车执行，生成fsdata.c文件；

	4、删除生成文件fsdata.c中的第二行代码：#include "lwip/def.h"；



备注:
	使用makefadata.exe时生成的二进制网页文件包含响应头，响应的内容为：
	HTTP/1.1 200 OK
	Server: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)
	Content-Length: 4515
	Connection: keep-alive
	Content-type: text/html

若需改变响应头的内容请咨询zachary_chou@foxmail.com(QQ877262677)