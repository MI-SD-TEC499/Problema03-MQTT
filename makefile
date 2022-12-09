# compilação e link com o código da lib
build: raspberry.c lcd.c
	gcc raspberry.c lcd.c lcd.h -o sensores -lwiringPi
