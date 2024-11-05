echo '5500 10 1 1 1 100' | python3 test.py
g++ -std=c++17 main.cpp -o main
start=`date +%s`
./main < input1.txt
end=`date +%s`
echo Boltz time was `expr $end - $start` seconds
start=`date +%s`
./main < input2.txt
end=`date +%s`
echo Cauchy time was `expr $end - $start` seconds
start=`date +%s`
./main < input3.txt
end=`date +%s`
echo Combined time was `expr $end - $start` seconds
