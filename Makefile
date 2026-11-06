src/output/main.exe: src/main.c
	gcc src/main.c -o src/output/main.exe

run: src/output/main.exe
	.\src\output\main.exe

clean:
	del /f /q src\output\main.exe
