#!/bin/bash

python train.py
python parse.py < ../syntax/d1_valid.synt > ../result/valid.txt
python parse.py < ../syntax/d1_test.synt > ../result/test.txt