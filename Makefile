V1 := 100
V2 := -TomatoEgg
all:
	make src-rt-6.x
	make src-rt

src-rt-6.x:
	make -C release/src-rt-6.x w1800rz V1=$(V1) V2=$(V2)
src-rt:
	make -C release/src-rt n64b V1=$(V1) V2=$(V2)
	make -C release/src-rt n60b V1=$(V1) V2=$(V2)

clean:
	make -C release/src clean
	make -C release/src-rt clean
	make -C release/src-rt-6.x clean
	git clean -f -x -d
	git reset --hard
