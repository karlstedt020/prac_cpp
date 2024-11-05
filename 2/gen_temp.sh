touch heat_map_data
for m in {2..20..2}
do
    for n in {200..2400..200}
    do
        echo $n $m 1 1 1 100 | python3 test.py
        for try in {1..5}
        do
            echo $n $m >> heat_map_data
            start=`date +%s`
            ./main < input1.txt >> heat_map_data
            end=`date +%s`
            echo `expr $end - $start` >> heat_map_data
        done
    done
done