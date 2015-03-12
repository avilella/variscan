# Environment variables for LastWave
export LWPATH=$HOME/softs/LastWave_2_0_4/
export LWSOURCEDIR=$LWPATH/scripts/
export ARCH=cygwin
# Putting X11R6 and LastWave in the path
export PATH=$LWPATH/bin/cygwin:/usr/X11R6/bin:$PATH

X&
twm&

xterm -sb -tn vt100 -e bash
exit
