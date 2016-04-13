#!/bin/bash
# My first script

# Os argumentos da shell estão acessíveis através do "$".
# $0 é o primeiro argumento (nome do script)
# Depois de $9, devem-se usar brackets: ${10}

#NÃO PODE HAVER ESPAÇOS ENTRE O IGUAL E OS ARGUMENTOS

NAME=$1

make $NAME

OUT=$?

if [ $OUT -eq 0 ]
then ./$NAME
else echo "COMPILATION ERROR"
fi
