#! /bin/bash
:<<!
使用说明:
	入参：需要进行文件内容分割的文件
	作用：对目标文件内容按行进行分割存入当前目录的t*.sql文件当中
!

test ! -f $1 && echo "$1文件不存在." && exit 0
total_line=`wc -l $1`
total_line=${total_line%$1}

echo 共有$total_line行
count=1
for ((i=1;i<=$total_line;i++)) do
	current=`head -$i $1 | tail -1`
	if [ "$str" != "" ]; then
		if [ "$current" != "" ]; then
			str=$str'\n'$current
		fi
	else
		str=$current
	fi
	
	
	if [ "$current" == "" ] || [ $i == $total_line ]; then
		 echo -e "$str" > t$count.sql
		 echo "t$count.sql"生成完毕
		 str=""
		 count=$[count+1]
	fi
done
echo 解析完成,已分为$[count-1]个文件

