import os
import ast
from nltk import *

def parseAnswer (answerText):
    resultList = []
    prev = 0
    cnt = 0
    for i in range(len(answerText)):
        if answerText[i] == '(' or answerText[i] == '[':
            if cnt == 0:
                resultList.append(answerText[prev:i])
                prev = i + 1
            cnt += 1
        elif answerText[i] == ')' or answerText[i] == ']':
            if cnt == 1:
                resultList.append(answerText[prev:i])
                return resultList
            cnt -= 1
        elif answerText[i] == ',' and cnt == 1:
            resultList.append(answerText[prev:i])
            prev = i + 1
    return [answerText]



def parseList (list):
    prev = 0
    res = []
    brCnt = 0
    for i in range(len(list)):
        if list[i] == '(':
            brCnt += 1
        elif list[i] == ',' and brCnt == 0:
            res.append(list[prev:i])
            prev = i + 1
        elif list[i] == ')':
            brCnt -= 1
    res.append(list[prev:])
    return res


def extractType (text, typeDict):
    if text[0] == '(' and text[-1] == ')':
        elementList = parseList(text[1:-1])
        # print(elementList)
        for element in elementList:
            extractType(element, typeDict)
        return "props"

    answer = parseAnswer(text)
    if len(answer) == 1:
        return "obj"
    
    name = answer[0]
    type = []
    for element in answer[1:]:
        type.append(extractType(element, typeDict))
    try:
        if typeDict[name] != type: 
            print("diffrent type of " + name + ", ")
            print("which are: ")
            print(typeDict[name])
            print(type)
    except KeyError:
        typeDict[name] = type

    return "props"

def printAnswers():
    try:
        while True:
            parsedText = input().replace(' ', '')
            answer = parseAnswer(parsedText)[2]
            print(answer)
    except EOFError: pass

def printDict():
    typeDict = {}
    try:
        while True:
            parsedText = input().replace(' ', '')
            answer = parseAnswer(parsedText)[2]
            # print(answer)
            # print("answer: \n" + answer)
            extractType(answer, typeDict)
            # print(typeDict)
    except EOFError: 
        for head in typeDict:
            print(head + ": ")
            print(typeDict[head])


if __name__ == '__main__':
    while True:
        try:
            str = input()
            print(str[str.find("answer"):])
        except EOFError:
            exit(0)