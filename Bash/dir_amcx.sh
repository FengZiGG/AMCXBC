#!/bin/bash  

if [[ -z "$1" ]] || [[ ! -d "$1" ]]; then  
    echo "The directory is empty or not exist!"  
    echo "It will use the current directory."  
    nowdir=$(pwd)  
else  
    nowdir=$(cd $1; pwd)  
fi  
echo "$nowdir"  

function SearchCfile()  
{  
    cd $1
    cfilelist=$(ls -l | grep "^-" |grep -v BB | awk '{print $9}')  
    #cfilelist=$(ls -l | grep "^-.*_BB\.cnf" | awk '{print $9}')  
    #cfilelist=$(ls -l | grep "^-" | awk '{print $9}')  
    for cfilename in $cfilelist  
    do
	echo "-------------------"
	echo $cfilename" "
	subname=`echo ${cfilename%.*}`
	doalarm 5000 AMCX_PRC_bin 0.2 $cfilename &> $subname".log"
	preproctime=`cat $subname".log" | grep "PreProc(s)"`
	time=`cat $subname".log" | grep "Time(s)"`
	ori=`cat $subname".log" | grep "ORI="`
	sci=`cat $subname".log" | grep "SCI="`
	echo $preproctime
	echo $time
	echo $ori
	echo $sci
	
	echo -ne $cfilename"," >> ~/amcx.csv
	echo -ne $preproctime","  >> ~/amcx.csv
	echo -ne $time","  >> ~/amcx.csv
	echo -ne $ori","  >> ~/amcx.csv
	echo -ne $sci  >> ~/amcx.csv
	echo "," >> ~/amcx.csv
	
    done
    dirlist=$(ls)  
    for dirname in $dirlist
    do  
        if [[ -d "$dirname" ]];then  
            cd $dirname  

            SearchCfile $(pwd)  
            cd ..  
        fi;  
    done;  
}  

SearchCfile $nowdir 
