@ECHO OFF

c:
chdir \cygwin\bin
SET MAKE_MODE=UNIX
SET DISPLAY=localhost:0

bash --login -i
