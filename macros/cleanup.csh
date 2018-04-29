#!/bin/csh

# cleanup the scheduler garbage and log files
# should remove everything the scheduler produces
# don't use while jobs are being submitted/running

rm -rv *session.xml 
rm -rv *report 
rm -rv *condor 
rm -rv *condor.log 
rm -rv sched*list 
rm -rv *dataset
rm -rv sched*csh 
rm -rv scheduler/csh/* 
rm -rv scheduler/list/* 
rm -rv scheduler/report/* 
rm -rv ZIP*
rm -rv *package
rm -rv logs/* 
rm -rv *.so 
rm -rv *.tmp 
rm -rv *.zip
rm -rv array*
rm -rf *.log
