n=$2
test=$1

for ((i=1 ; i<=$n ; i++))
do
    for ((j=1 ; j<=5; j++))
    do
        diff output/op-c$i-p$j.txt sol$test/ans_$i\_$j.txt
    done
done
