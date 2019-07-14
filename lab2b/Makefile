# NAME: Sum Yi Li
# EMAIL: sammyli0106@gmail.com
# ID: 505146702

default:
	gcc -Wall -Wextra -pthread -g SortedList.c lab2_list.c -o lab2_list 

tests: default
	rm -f lab2b_list.csv 
	./lab2_list --threads=1  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=24  --iterations=1000 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=24  --iterations=1000 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --threads=4 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --threads=8 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --threads=12 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
	./lab2_list --threads=16 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=s	--lists=4 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=m	--lists=8 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=s	--lists=8 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=m	--lists=16 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=m	--lists=16 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
	./lab2_list --threads=1  --iterations=1000 --sync=s	 --lists=16 >> lab2b_list.csv
	./lab2_list --threads=2  --iterations=1000 --sync=s	 --lists=16 >> lab2b_list.csv
	./lab2_list --threads=4  --iterations=1000 --sync=s  --lists=16 >> lab2b_list.csv
	./lab2_list --threads=8  --iterations=1000 --sync=s  --lists=16 >> lab2b_list.csv
	./lab2_list --threads=12  --iterations=1000 --sync=s  --lists=16 >> lab2b_list.csv

	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv

profile:
	-rm -f profile.gperf 
	LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=profile.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text lab2_list profile.gperf > profile.out
	pprof --list=list_func lab2_list profile.gperf >> profile.out
	-rm -f profile.gperf

graphs: tests
	chmod u+x lab2_list.gp
	./lab2_list.gp

dist: clean graphs
	tar -czvf lab2b-505146702.tar.gz profile.out lab2_list.c SortedList.h SortedList.c lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png README Makefile lab2b_list.csv lab2_list.gp

clean:
	rm -f *.o *.tar.gz lab2_list




