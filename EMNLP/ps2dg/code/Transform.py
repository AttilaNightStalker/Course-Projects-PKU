from nltk.tree import  *
import re
import pickle
import numpy as np


global theHeads

'''
match child tag list to one of child list in the group
'''
def patternMatch (group, childtags):
    childtags = list(map(lambda tag:tag.split('-')[0], childtags))

    for ans in group[1:]:
        curAnswer = list(map(lambda tag:tag.split('-')[0], ans[1]))
        if curAnswer == childtags:
            return ans[0]

    for answer in group[1:]:
        key = answer[1][answer[0]].split('-')[0]
        curAnswer = list(map(lambda tag:tag.split('-')[0], answer[1]))
        if key in childtags:
            # childKeyNum = len(list(filter(lambda tag:tag == key, childtags)))
            answerKeyNum = len(list(filter(lambda tag:tag == key, curAnswer)))
            answerKeyIdx = len(list(filter(lambda tag:tag == key, curAnswer[:answer[0] + 1])))
            # print(childKeyNum)
            # print(answerKeyNum)
            # print(answerKeyIdx)
            if answerKeyIdx == 1:
                for i in range(len(childtags)):
                    if childtags[i] == key:
                        return i
                    
            elif answerKeyIdx == answerKeyNum:
                for i in reversed(range(len(childtags))):
                    if childtags[i] == key:
                        return i
                    
            else:
                for i in range(len(childtags)):
                    if childtags[i - 1] == curAnswer[answer[0] - 1] or childtags[i + 1] == curAnswer[answer[0] + 1]:
                        return i

                for i in range(len(childtags)):
                    if childtags[i] == key:
                        return i
    
    return -1

'''
return the index of head child
'''
def head (father, childs):
    '''
    if there is only one child, then the only one is the head
    '''
    if len(childs) == 1:
        return childs[0]

    '''
    we only consider prefix of tags,
    this convert the tag list to tag prefix list
    '''
    childtags = [child.label() for child in childs]
    for group in theHeads:
        if group[0] == father:
            result = patternMatch(group, childtags)
            if result != -1:
                return childs[result]
            break

    for group in theHeads:
        if group[0].split('-')[0] == father.split('-')[0]:
            result = patternMatch(group, childtags)
            if result != -1:
                return childs[result]

    # print("+++++++++++++++++++++++++++++++")
    # print(father)
    # print(childtags)
    # decidedNum = eval(input())
    # theHeads.append([father, [decidedNum, childtags]])
    return childs[0]

'''
convert a syntax tree to a dependency tree
'''
def sTree2dTree (syntaxTree):
    if not isinstance(syntaxTree[0], Tree):
        return Tree(tuple([syntaxTree[0][0], syntaxTree.label(), syntaxTree[0][1]]), [])
    else:
        childs = [child for child in syntaxTree]
        label = syntaxTree.label()
        headChild = head(label, childs)
        result = sTree2dTree(headChild)

        for child in filter(lambda child:child is not headChild, childs):
            result.append(sTree2dTree(child))

        return result

'''
calculate how many node in a dependency tree
'''
def treeSize (tree):
    if len(tree) == 0:
        return 1
    
    size = 0
    for child in tree:
        size += treeSize(child)

    return size + 1

'''
set leaves' index in a syntax tree
'''
def setIndex (sTree, index = [0]):
    if not isinstance(sTree[0], Tree):
        index[0] += 1
        sTree[0] = tuple([index[0], sTree[0]])
        return
    
    for child in sTree:
        setIndex(child, index)

'''
return a list with each node's infomation
including its father's index
'''
def setFathers (dTree, dadList, father = 0, size = 0):
    if father == 0:
        size = treeSize(dTree)
        for i in range(size):
            dadList.append([i + 1])

    dadList[dTree.label()[0] - 1].extend([dTree.label()[2], dTree.label()[1], father])   

    for child in dTree:
        setFathers(child, dadList, dTree.label()[0], size) 

        
if __name__ == '__main__':
    # with open("FinalHeads.list", "rb") as fp:
    with open("FinalHeads.list", "rb") as fp:
        theHeads = pickle.load(fp)

    readPuzzle = open("ctb.bracketed")

    try:
        while True:
            puzzle = readPuzzle.readline()
            if (len(puzzle) == 0):
                break
            if puzzle[0] == '#':
                print(puzzle, end = "")
                puzzle = readPuzzle.readline()
            
            sTree = Tree.fromstring(puzzle)
            setIndex(sTree, [0])
            dTree = sTree2dTree(sTree)
            dadList = []
            setFathers(dTree, dadList)
            for record in dadList:
                print(record[0], "	", record[1], "	_	_	", record[2], "	_	", record[3], "	X	_	_")
            print("")
    
    except EOFError:
        pass

    # with open("FinalHeads.list", "wb") as fp:
    #     pickle.dump(theHeads, fp)
    