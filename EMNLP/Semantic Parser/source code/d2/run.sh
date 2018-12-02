#!/bin/bash

python getAnswer.py -sentence < data/d2_train_out.txt | python prepare.py  
python getSentences.py < data/d2_train_in.txt | java -jar ../berkeleyparser-master/BerkeleyParser-1.7.jar -gr ../../berkeleyparser-master/eng_sm6.gr > syntax/d2_train.synt
python getSentences.py < data/d2_valid_in.txt | java -jar ../berkeleyparser-master/BerkeleyParser-1.7.jar -gr ../../berkeleyparser-master/eng_sm6.gr > syntax/d2_valid.synt
python getSentences.py < data/d2_test_in.txt | java -jar ../berkeleyparser-master/BerkeleyParser-1.7.jar -gr ../../berkeleyparser-master/eng_sm6.gr > syntax/d2_test.synt

train/run.sh