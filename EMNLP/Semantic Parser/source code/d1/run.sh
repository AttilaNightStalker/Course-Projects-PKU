#!/bin/bash

python getAnswer.py -sentence < data/d1_train_out.txt | python prepare.py  
python getSentences.py < data/d1_train_in.txt | java -jar ../berkeleyparser-master/BerkeleyParser-1.7.jar -gr ../../berkeleyparser-master/eng_sm6.gr > syntax/d1_train.synt
python getSentences.py < data/d1_valid_in.txt | java -jar ../berkeleyparser-master/BerkeleyParser-1.7.jar -gr ../../berkeleyparser-master/eng_sm6.gr > syntax/d1_valid.synt
python getSentences.py < data/d1_test_in.txt | java -jar ../berkeleyparser-master/BerkeleyParser-1.7.jar -gr ../../berkeleyparser-master/eng_sm6.gr > syntax/d1_test.synt

train/run.sh