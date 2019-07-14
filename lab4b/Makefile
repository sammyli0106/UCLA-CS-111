# NAME: Sum Yi Li
# EMAIL: sammyli0106@gmail.com
# ID: 505146702

default:
	gcc -Wall -Wextra -lmraa -lm -g -o lab4b lab4b.c

check: default
	chmod u+x smokeTests.sh
	./smokeTests.sh

clean:
	rm -f lab4b lab4b-505146702.tar.gz

dist:
	tar -czvf lab4b-505146702.tar.gz lab4b.c README Makefile smokeTests.sh




