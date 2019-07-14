# NAME: Sum Yi Li
# EMAIL: sammyli0106@gamil.com
# ID: 505146702

default:
	gcc -Wall -Wextra -g -o lab0 lab0.c

check:
	./smokeTests.sh

clean: #delete all the created files and return to untared state
	rm -f lab0 lab0-505146702.tar.gz

dist: 
#build the distribution tarball
	tar -czf lab0-505146702.tar.gz lab0.c Makefile README smokeTests.sh backtrace.png breakpoint.png




