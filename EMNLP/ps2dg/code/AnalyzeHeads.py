from nltk.tree import  *
import re
import pickle
import numpy as np

global sumCFG
sumCFG = [] #this is the head rule list

'''
add new element to sumCFG
'''
def addCFG (syntaxTree, leaveDepth, index = [0]):
    if not isinstance(syntaxTree[0], Tree):
        syntaxTree.set_label(tuple([leaveDepth[index[0]][1], syntaxTree.label()]))
        index[0] += 1
        return

    curChilds = []
    min = 99999
    for i in range(len(syntaxTree)):
        addCFG(syntaxTree[i], leaveDepth, index)
        if (syntaxTree[i].label()[0] < min):
            min = syntaxTree[i].label()[0]
            minIdx = i
        curChilds.append(syntaxTree[i].label()[1])

    syntaxTree.set_label(tuple([min, syntaxTree.label()]))

    for group in sumCFG:
        # if group[0] == "IP":
        #     print(group)
        if group[0] == syntaxTree.label()[1]:       
            for childs in group[1:]:
                if curChilds == childs[1]:
                    return
            
            group.append([minIdx, curChilds])
            return
    
    sumCFG.append([syntaxTree.label()[1], [minIdx, curChilds]])

'''
calculate depth of nodes in a dependency tree
each node's father is known
'''
def setDepth (indexList, index = -1, depth = 0):
    if index == -1:
        for idx in indexList:
            if idx[1] == 0:
                idx.append('x')
                setDepth(indexList, idx[0], 0)
                break
    
    for idx in indexList:
        if idx[1] == index and len(idx) == 2:
            idx[1] = depth + 1
            idx.append('x')
            # print(str(idx[0]) + ' ' + str(idx[1]))
            setDepth(indexList, idx[0], idx[1])


if __name__ == '__main__':
    readPuzzle = open("ctb.bracketed")
    readAnswer = open("ctb-malt-examples.conll")

    try:
        while True:
            puzzle = readPuzzle.readline()
            answer = readAnswer.readline()
            
            if len(answer) == 0:
                break

            if puzzle[0] != '#':
                elementDepth = []
                while len(answer) > 1:
                    added = list(map(lambda c:int(c), re.findall(r"\d\d*",answer)))
                    if not answer[0] == '#':
                        elementDepth.append(added)
                    answer = readAnswer.readline()

                setDepth(elementDepth)
                treeList = Tree.fromstring(puzzle)
                addCFG(treeList, elementDepth, [0])
                
            
    except EOFError:
        pass

    with open("Heads.list", "wb") as fp:
        pickle.dump(sumCFG, fp)

    