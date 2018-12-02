from nltk.tree import *
import pickle


propDict = pickle.load(open("propDict", "rb"))
constDict = pickle.load(open("constDict", "rb"))

def getText(idxa, idxb, npList):
    return 0


def getSeq (tree):
    if not isinstance(tree, Tree):
        return [tree]
    result = []
    for child in tree:
        result.extend(getSeq (child))
    return result

def getSeqPos (tree):
    if not isinstance(tree[0], Tree):
        return [tree]
    result = []
    for child in tree:
        result.extend(getSeqPos (child))
    return result


def parseNPsIdx(tree):
    def idxRange(subTree):
        if not isinstance(subTree, Tree):
            return [subTree[0], subTree[0] + 1]
        rangeList = [idxRange(child) for child in subTree]
        return [rangeList[0][0], rangeList[-1][-1]]
    
    def parseNP (idxTree):
        if not isinstance(idxTree, Tree):
            return []

        if idxTree.label() == "WHNP":
            seqPos = getSeqPos(idxTree)
            if True in [leaf.label()[0] == 'N' for leaf in seqPos]:
                return [[idxRange(idxTree), idxTree]]
            else:
                return []

        if idxTree.label() != "NP" or "NP" in [child.label() for child in idxTree]:
            subRes = []
            for child in idxTree:
                subRes.extend(parseNP(child))
            return subRes  
        # print(idxTree)
        return [[idxRange(idxTree), idxTree]]
    
    def countLeave(originT, cnt):
        if not isinstance(originT, Tree):
            res = [cnt[0], originT]
            cnt[0] += 1
            return res
        return ParentedTree(originT.label(), [countLeave(child, cnt) for child in originT])
    
    ''' get NP nodes for curNode'''
    def getNodes (curNode, nodes):
        res = []
        if not isinstance(curNode, Tree):
            return []

        if "NP" in curNode.label():
            for i in range(len(nodes)):
                if curNode is nodes[i]:
                    return [i]
            for child in curNode:
                if "NP" in child.label():
                    for i in range(len(nodes)):
                        if child is nodes[i]:
                            res.append(i)
                            break
        else:
            for child in curNode:
                res.extend(getNodes(child, nodes))
        
        return res

    ''' get NP nodes related to curNode'''
    def getRalated (curNode, nodes):
        brothers = list(filter(lambda node : not node is curNode, [child for child in curNode.parent()]))
        res = []
        for brother in brothers:
            res.extend(getNodes(brother, nodes))
        if "NP" in curNode.parent().label():
            res.extend(getRalated(curNode.parent(), nodes))
        return res

    def parseRalation(nodes):
        res = []
        for i in range(len(nodes)):
            res.extend([[i, num] for num in getRalated(nodes[i], nodes)])
        return res

    npNodes = list(map(lambda np : np[1], parseNP(countLeave(tree, [0]))))
    # print(parseRalation(npNodes))
    return [list(map(lambda np : np[0], parseNP(countLeave(tree, [0])))), parseRalation(npNodes)]

def getNPIdx (npList):
    res = []
    for i in range(len(npList)):
        if isinstance(npList[i], list):
            res.append(i)
    return res


def getOffset (npList):
    res = []
    cur = 0
    for x in npList:
        if isinstance(x, list):
            res.append(cur)
            cur += len(x)
        else:
            cur += 1
    return res


def expandList (lst):
    res = []
    for element in lst:
        if isinstance(element, list):
            res.extend(element)
        else:
            res.append(element)
    return res


def resolveConflict(funcs):

    def condition(i, j, score):
        def result(func):
            # print(func)
            if ((func[0][0] <= j and func[0][0] >= i) or (func[0][1] <= j and func[0][1] >= i)) and func[2] < score:
                return False
            return True
        return result
    
    max = 999
    while True:
        curMax = 0
        maxFunc = None
        for func in funcs:
            if func[2] > curMax and func[2] < max:
                curMax = func[2]
                maxFunc = func
        if maxFunc is None:
            break
        funcs = list(filter(condition(*maxFunc[0], curMax), funcs))
        max = curMax
    
    return funcs
        

def funcList (npList, relation, dict):

    npIdx = getNPIdx(npList)
    npOffsets = getOffset(npList)

    def findSingle (offset, chunck, index):
        res = []
        for i in range(len(chunck)):
            for j in range(i, len(chunck)):
                try:
                    keyWord = "_".join(s for s in chunck[i:j + 1])
                    key = dict[keyWord]
                    if len(key[0][1]) == 1:
                        res.append([[offset + i, offset + j], key[0][0], key[1][1], [index]])
                except KeyError:
                    continue

        return res


    def findDouble (offset, chunck, posA, posB, indexA, indexB):
        
        res = []
        for i in range(posB):
            for j in range(i, len(chunck) if i >= posA else posB): 
                try:
                    keyWord = "_".join(s for s in chunck[i:j + 1])
                    key = dict[keyWord]

                    if len(key[0][1]) == 2:
                        if key[0][1][0] < key[0][1][1]:
                            res.append([[offset + i, offset + j], key[0][0], key[1][1], [indexA, indexB]])
                        else:
                            res.append([[offset + i, offset + j], key[0][0], key[1][1], [indexB, indexA]])
                        # res.append([[offset + i, offset + j], key[0][0], key[1][1], [indexA, indexB]])
                except KeyError:
                    continue
                    
        return res

    resSeq = []
    for i in range(len(npIdx)):
        resSeq.extend(findSingle(npOffsets[i], npList[npIdx[i]], i))
        if [i, i + 1] in relation:
            if i == 0:
                expandedList = expandList(npList[0:npIdx[i + 1] + 1])
            else:
                expandedList = expandList(npList[npIdx[i]:npIdx[i + 1] + 1])
        
            resSeq.extend(findDouble(npOffsets[i], expandedList, len(npList[npIdx[i]]), len(expandedList) - len(npList[npIdx[i + 1]]), i, i + 1))
        else:
            for k in range(i):
                if [i - k - 1, i] in relation:
                    if i == len(npIdx) - 1:
                        expandedList = expandList(npList[npIdx[i]:])
                        resSeq.extend(findDouble(npOffsets[i], expandedList, len(npList[npIdx[i]]), len(expandedList), i, i - k - 1))
    

    resSeq = resolveConflict(resSeq)
    regular = regulizeFunc(resSeq)
    # print(regular)
    normal = normalizeFunc(regular, len(regular) - 1)
    # print(normal)
    return normal

def distinctList(lst):
    res = []
    for x in lst:
        if not x in res:
            res.append(x)
    return res


def regulizeFunc(funcs):
    funcs = distinctList(list(map(lambda func : [func[1], func[3]], funcs)))
    maxIdx = -1
    idxDict = {}

    for func in funcs:
        xfunc = func[1].copy()
        xfunc.sort()
        for idx in xfunc:
            if idx > maxIdx:
                idxDict[idx] = chr(len(idxDict) + ord('A'))
                maxIdx = idx
    
    delList = []
    for func in funcs:
        for i in range(len(func[1])):
            try:
                func[1][i] = idxDict[func[1][i]]
            except KeyError:
                delList.append(func)
            
    for x in delList:
        funcs.remove(x)
    
    for func in funcs:
        if func[0].split("_")[0] == "const":
            phrase = func[0].split("_")[1:]
            func[0] = "const"
            func[1].append(" ".join(phrase))

    return list(map(lambda x : tuple(x), funcs))

def normalizeFunc (funcs, tail):
    global propDict
    for i in range(tail + 1):
        try:
            form = propDict[funcs[tail - i][0]].copy()
            para = funcs[tail - i][1]
            paraSet = set(para)
            collect = []
            delList = []
            for j in range(tail - i + 1, len(funcs)):
                curPara = set(list(filter(lambda x : isinstance(x, str), funcs[j][1])))

                if len(curPara & paraSet) > 0:
                    paraSet = paraSet & curPara
                    collect.append(funcs[j])
                    delList.append(j)
                else:
                    break
            
            paraCnt = 0

            for k in range(len(form)):
                if form[k] == True:
                    form[k] = collect
                else:
                    form[k] = para[paraCnt]
                    paraCnt += 1
            
            funcs[tail - i] = tuple([funcs[tail - i][0], form])
            
            for x in delList:
                funcs.pop(tail - i + 1)
            
            if len(collect) == 0:
                funcs.pop(tail - i)
            return normalizeFunc(funcs, tail - i - 1)

        except KeyError:
            pass

    return funcs


def getExp (funcs):
    global constDict

    if isinstance(funcs, list):
        return "(" + ",".join(list(map(getExp, funcs))) + ")"
    elif isinstance(funcs, tuple):
        return funcs[0] + getExp(funcs[1])
    else:
        try:
            return constDict[funcs]
        except KeyError:
            return funcs

def parseSeq():
    try:
        while True:
            tree = ParentedTree.fromstring(input())
            sentence = getSeq (tree)
            npPos = parseNPsIdx(tree)
            relation = npPos[1]
            npPos = npPos[0]

            dict = pickle.load(open("dict", "rb"))

            prev = 0
            npList = [] # this is the sentence with chuncks of NP
            for chunck in npPos:
                npList.extend(sentence[prev:chunck[0]])
                npList.append(sentence[chunck[0]:chunck[1]])
                prev = chunck[1]
            npList.extend(sentence[npPos[-1][1]:])
            # print(npList)
            # print(relation)

            funcs = funcList(npList, relation, dict)
            # print(funcs)
            print("parse([" + ",".join(getSeq(tree)) + "],", "answer(A," + getExp(funcs) + ")).")

    except EOFError: pass

if __name__ == '__main__':
    parseSeq()