#!/bin/bash

process=$(grep richieste testout.log | wc -l)
nrichieste=$(grep richieste testout.log | cut -d":" -s -f 2 | paste -sd+ | bc)
nsuccesso=$(grep successo testout.log | cut -d":" -s -f 2 | paste -sd+ | bc)

echo "processi lanciati:"$process
echo "richieste fatte:"$nrichieste
echo "operazioni con successo:"$nsuccesso
nfallite=$(( $nrichieste - $nsuccesso ))
echo "operazioni fallite:"$nfallite
if [ $nfallite = 0 ];then
  exit
fi

echo client con errori 
grep client testout.log | cut -d":" -s -f 2 

echo "con errori:"
grep KO testout.log