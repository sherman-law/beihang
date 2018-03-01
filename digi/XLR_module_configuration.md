## XLR module 使用手册

### 使用AT指令配置XLR module
* 重复发送"CC字符"**三次**，让模块XLR module进入AT指令模式。（注意：这里面发送的CC字符（command sequence character）并不是“CC”字母，而是CC下面的一个字节，CC位下面的默认值是**0x2B**）
* 发送命令“AT”，module将返回“OK\r”
* 查询参数，使用格式为“AT+命令”，例如“ATCC”（**注意这里AT和命令之间是没有“+”号的**）命令查询CC位下面的参数，module将返回“2B”的字符串。（注意：这里面和一般的AT指令（AT+CGSM?）查询不一样，AT和命令之间是没有“+”号的）
* 设置参数，使用格式为“AT+命令+参数”，例如“ATCC33”（**注意这里AT和命令之间是没有“+”号的，命令和参数之间也是没有等号的**）命令设置CC位下面的参数，module将返回“OK”的字符串表示设置成功，“ERROR”的字符串表示命令执行失败。（注意：这里面和一般的AT指令配置（AT+CPMS="SM","SM"）不一样，AT和命令之间是没有“+”号的，也没有“=”号，参数也没有放在双引号内）

### 使用API模式配置XLR module
* **使用AT指令**（AT指令使用请参考本文档“使用AT指令配置XLR module”章节）设置“AP”寄存器为“1”，详情请参考手册。
* API 帧格式--->帧头（0x7E） + 帧长（数据段的长度） + 数据段（帧类型+帧ID+针对不用帧类型指定的数据格式） + 校验和（checksum）。
>备注：    
校验和的计算：不包括帧头和帧长，所有的字节相加，取低8位，然后用0xff减去这个结果，得到校验和。
校验和的验证：不包括帧头和帧长，所有的字节相加（包括校验和），如果是正确的，应该是等于0xFF。


### 参考资料
[官网社区论坛链接](https://www.digi.com/blog/community/xbee-tech-tip-the-io-at-command/)    
[XLR module的官方参考手册](https://www.digi.com/resources/documentation/Digidocs/90001407/Default.htm#reference/r_cmd_cc.htm%3FTocPath%3DAT%2520commands%7CCommand%2520mode%2520options%7C_____1)
