OUTPUT=../VANETSimulator.exe
rm -f $OUTPUT
rm -f ./bin/*
g++ -std=c++0x -c *.cpp
mv *.o ./bin/
g++ ./bin/*.o -o $OUTPUT
chmod 755 $OUTPUT
$OUTPUT $1 | tee ~/workspace/SimulationResults.txt