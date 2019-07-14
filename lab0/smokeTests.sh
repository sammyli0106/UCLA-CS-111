#!/bin/bash

# NAME: Sum Yi Li
# EMAIL: sammyli0106@gamil.com
# ID: 505146702 

#Check whether the program runs and exit successfully, copy successfully, exit 0                                    
        echo "Writing to test input file" > input.txt;
        ./lab0 --input=input.txt --output=output.txt;
        if [ $? -eq 0 ]; then
        echo "Program copy successful and exit with 0";
        else
        echo "Program fails to copy and exit with wrong code";
        fi

#Check unrecognized argument, exit 1                                                                         
        ./lab0 --wrongArg;
        if [ $? -eq 1 ]; then
        echo "Program detects unrecognized argument successful and exit with code 1";
        else
        echo "Program fails to detect unrecognized argument and exit with wrong code";
        fi

#Check bad input file which is a input file that does not exsist, exit 2                                   
        ./lab0 --input=badInput.txt;
        if [ $? -eq 2 ]; then
        echo "Program detects bad input file and exit with code 2";
        else
        echo "Program fails to detect bad input file and exit with wrong code";
        fi

#Check bad output file, exit 3                                                                                      
#First create a input file and output file                                                                     
#Create a bad output file by removing the writing permission to it                                      
        echo "Writing to test input file 2" > input2.txt;
        touch output.txt;
        chmod -w output.txt;
        ./lab0 --input=input2.txt --output=output.txt;
        if [ $? -eq 3 ]; then
        echo "Program detects bad output file and exit with code 3";
        else
        echo "Program fails to detect bad output file and exit with wrong code";
        fi

#Check segfault and catch arguments, exit 4                                                            
        ./lab0 --segfault --catch;
        if [ $? -eq 4 ]; then
        echo "Program detects segfault and catch to exit with code 4";
        else
        echo "Program fails to detect segfault and exit with wrong code";
        fi

#Remove all the created test files
	rm -f input.txt input2.txt output.txt
	
	
