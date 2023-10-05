myshell:
	gcc sh.c -o myshell -Wall

meutop:
	gcc meutop.c -o meutop -Wall

signaltester:
	gcc signaltester.c -o signaltester -Wall

clean:
	rm meutop
	rm myshell
	rm sh
	rm signaltester
