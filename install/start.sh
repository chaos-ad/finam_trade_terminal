#!/bin/sh
cd `dirname $0`
WINEDEBUG=-all wine ./trade_terminal.exe
