all: myshell meutop signaltester

myshell: sh.c
	gcc sh.c -o myshell -Wall

meutop: meutop.c
	gcc meutop.c -o meutop -Wall

signaltester: signaltester.c
	gcc signaltester.c -o signaltester -Wall

clean:
	rm meutop
	rm myshell
	rm sh
	rm signaltester
