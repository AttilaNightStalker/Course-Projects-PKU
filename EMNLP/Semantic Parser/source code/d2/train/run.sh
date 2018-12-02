#!/bin/bash

python train.py
python parse.py < ../syntax/d2_valid.synt > ../result/valid.txt
python parse.py < ../syntax/d2_test.synt > ../result/test.txt