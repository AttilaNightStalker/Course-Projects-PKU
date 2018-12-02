import pickle
from nltk import *
from Tools import *

def maxScore (candidates):
    maxIdx = None
    max = 0
    for i in range(len(candidates)):
        if candidates[i][2] > max:
            max = candidates[i][2]
            maxIdx = i
    return candidates.pop(maxIdx)

def seperated (tp1, tp2):
    return tp1[0] > tp2[1] or tp1[1] < tp2[0]

def getPropList (candidates):
    result = []
    while len(candidates) > 0:
        winner = maxScore (candidates)
        candidates = list(filter(lambda choice: seperated(choice[1], winner[1]) and choice[0] != winner[0], candidates))
        result.append(winner[:-1])
    return result


def parseSeq (ptree):
    if not isinstance(ptree[0], Tree):
        return [[ptree.label(), ptree[0]]]
    
    seq = []
    for child in ptree:
        seq.extend(parseSeq (child))
    return seq


def main():
    searchTree = pickle.load(open("PathTree", "rb"))
    check (searchTree)

    while True:
        try:
            treeStr = input()
        except EOFError: exit(0)
        if treeStr == "":
            break
        ptree = ParentedTree.fromstring(treeStr)
        transForm(ptree)

        verbs = getVerbs(ptree)

        verbprops = []
        for verb in verbs:
            for child in searchTree:
                if child.label() == [verb.label()[0], getBrothers(verb)]:
                    candidates = getArgs(verb, child)
                    propList = getPropList(candidates)
                    list = [["V", verb.label()[1]]]
                    list.extend(propList)
                    verbprops.append(list)
                    # print(list)
                    break
        
        seq = parseSeq (ptree)
        seqlen = len(seq)
        verbcnt = len(verbprops)

        verbline = ["-" for i in range(seqlen)]
        for i in range(seqlen):
            if "V" in seq[i][0][0]:
                verbline[i] = seq[i][1]

        printedData = [verbline]
        for i in range(verbcnt):
            curcol = ["*" for i in range(seqlen)]
            for prop in verbprops[i]:
                curcol[prop[1][0]] = "(" + prop[0] + curcol[prop[1][0]]
                curcol[prop[1][1]] = curcol[prop[1][1]] + prop[0] + ")"
            printedData.append(curcol)
        
        for i in range(seqlen):
            for j in range(len(printedData)):
                print(printedData[j][i], end = "\t")
            print("")
        print("")
        # break

if __name__ == '__main__':
    main()
