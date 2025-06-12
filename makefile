myShell: myShell.c myShell.h
	gcc -std=gnu99 -Wpedantic myShell.c -o myShell

clean:
	rm myShell