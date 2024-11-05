start=`date +%s`
./main < input1.txt
end=`date +%s`
echo Execution time `expr $end - $start`