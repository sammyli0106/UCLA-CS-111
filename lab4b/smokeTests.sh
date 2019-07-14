#!/bin/bash

# NAME: Sum Yi Li
# EMAIL: sammyli0106@gamil.com
# ID: 505146702

#check whether the program pass when using with regular arguments
./lab4b --scale="C" --period=4 --log="logfile1" <<-EOF
SCALE=F
STOP
START
OFF
EOF
ret=$?
if [ $ret -eq 0 ]; then
    echo "Program succeed : correct exit code for valid command.";
else
    echo "Program failed : incorrect exit code for valid command.";
fi

#check whether the program recognized bad(unsupported) argument and exit with the correct code, 1
echo | ./lab4b --unrecogArg &> /dev/null;
if [ $? -ne 1 ]; then
echo "Program failed : exit with wrong code.";
else
echo "Program succeed : exit with correct code.";
fi

#check whether the stop process has been logged into the log file
grep "START" logfile1 &> /dev/null;
if [ $? -eq 0 ]; then
echo "Program succeed : START command is in the log file.";
else
echo "Program failed : START command is not in the log file.";
fi

#check whether the shutdown process has been logged into the log file
grep "SHUTDOWN" logfile1 &> /dev/null;
if [ $? -eq 0 ]; then
    echo "Program succeed : SHUTDOWN command is in the log file.";
else
    echo "Program failed : SHUTDOWN command is not in the log file.";
fi

#check whether the stop process has been logged into the log file
grep "STOP" logfile1 &> /dev/null;
if [ $? -eq 0 ]; then
    echo "Program succeed : STOP command is in the log file.";
else
    echo "Program failed : STOP command is not in the log file.";
fi

# remove the created log file
rm -f logfile1;


