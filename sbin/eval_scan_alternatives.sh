#! /bin/sh

cmake . && make
./cmake-build-debug/bench-bucket-putget-scan-1 > ./tmp/bench-bucket-putget-scan-1.csv
./cmake-build-debug/bench-bucket-putget-scan-1-parallel > ./tmp/bench-bucket-putget-scan-1-parallel.csv 
./cmake-build-debug/bench-bucket-putget-scan-2-cache > ./tmp/bench-bucket-putget-scan-2-cache.csv   
./cmake-build-debug/bench-bucket-putget-scan-3-cache > ./tmp/bench-bucket-putget-scan-3-cache.csv   
./cmake-build-debug/bench-bucket-putget-scan-4-cache > ./tmp/bench-bucket-putget-scan-4-cache.csv  
./cmake-build-debug/bench-bucket-putget-scan-1-cache > ./tmp/bench-bucket-putget-scan-1-cache.csv    
./cmake-build-debug/bench-bucket-putget-scan-2 > ./tmp/bench-bucket-putget-scan-2.csv      
./cmake-build-debug/bench-bucket-putget-scan-3 > ./tmp/bench-bucket-putget-scan-3.csv         
./cmake-build-debug/bench-bucket-putget-scan-4 > ./tmp/bench-bucket-putget-scan-4.csv
./cmake-build-debug/bench-bucket-putget-bsearch > ./tmp/bench-bucket-putget-bsearch.csv   

