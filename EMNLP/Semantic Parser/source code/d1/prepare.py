from nltk.tree import *
from nltk.parse.stanford import StanfordParser
import pickle


def addCFG(tree, cfg):
    try:
        if not isinstance(tree[0], Tree):
            return
    except IndexError:
        return
    
    subTags = [child.label() for child in tree]
    try:
        cfgList = cfg[tree.label()]
        if not subTags in cfgList:
            cfgList.append(subTags)
    except KeyError:
        cfg[tree.label()] = [subTags]
    
    for child in tree:
        addCFG(child, cfg)

def concludeCFG ():
    try:
        cfg = {}
        while True:
            tree = Tree.fromstring(input())
            addCFG(tree, cfg)
    except EOFError: 
        for head in cfg:
            print(head + " : ")
            print(cfg[head])


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


# def parseInnerNPs(tree, result, left = 0):
#     if not isinstance(tree, Tree):
#         return 1

#     if tree.label() == "WHNP":
#         seqPos = getSeqPos(tree)
#         if True in [leaf.label()[0] == 'N' for leaf in seqPos]:
#             return [list(map(lambda node : node[0], seqPos))]
    
#     if tree.label() != "NP" or "NP" in [child.label() for child in tree]:
#         subRes = []
#         for child in tree:
#             subRes.extend(parseInnerNPs(child))
#         return subRes
#     return [getSeq(tree)]
    
# def nones (tree):
#     if not isinstance(tree[0], Tree):
#         if tree.label()[0] == 'N':
#             return [[tree[0]]]
#         else:
#             return [tree[0]]
#     else:
#         res = []
#         for child in tree:
#             res.extend(nones(child))
#         return res

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
    parseRalation(npNodes)
    # print(parseRalation(npNodes))
    return list(map(lambda np : np[0], parseNP(countLeave(tree, [0]))))


# def innerNPs ():
#     try:
#         while True:
#             tree = Tree.fromstring(input())
#             print(parseInnerNPs(tree))
#     except EOFError: pass


def parseList (text):
    # print(text)
    cnt = 0
    prev = 0
    res = []
    for i in range(len(text)):
        if text[i] == '(':
            cnt += 1
        elif text[i] == ')':
            cnt -= 1
        elif text[i] == ',' and cnt == 0:
            res.append(text[prev:i])
            prev = i + 1
    res.append(text[prev:])

    # print(res)
    return res


def parseExp (text):
    # print(text)
    if text[-1] == ')':
        try:
            start = text.index('(')
        except ValueError: return None
        if start == 0:
            return list(map(parseExp, parseList(text[start + 1:-1])))
        return tuple([text[:start], list(map(parseExp, parseList(text[start + 1:-1])))])
    return text


def updateDict (prop, dict):
    para = prop[1]
    types = []
    for x in para:
        if isinstance(x, str):
            types.append(False)
        else:
            types.append(True)
    
    if True in types:
        print(prop)
        print(types)
        dict[prop[0]] = types


def regularize (answer, dict):
    def regularPara (paraList):
        # print(paraList)
        # print(3)
        regularProps(paraList)
        res = []
        try:
            i = 0
            while True:
                
                if not isinstance(paraList[i], str):
                    res.extend(paraList[i] if isinstance(paraList[i], list) else [paraList[i]]) 
                    paraList.remove(paraList[i])
                else:
                    i += 1
        except IndexError: pass
        return res
    
    def insertList (origin, inserted, idx):
        for i in range(len(inserted)):
            origin.insert(idx + i, inserted[i])
    
    def regularProps(props):    
        try:
            i = 0
            while True:
                # print(": " + str(i))
                if isinstance(props[i], list):
                    regularProps(props[i])
                    temp = props[i]
                    props.remove(props[i])
                    insertList(props, temp, i)
                    i += len(temp)
                elif isinstance(props[i], tuple) and props[i][0] != "const":
                    updateDict(props[i], dict)
                    temp = regularPara(props[i][1])
                    if len(temp) > 0:
                        insertList(props, temp, i + 1)
                        i += len(temp)
                i += 1
        except IndexError: 
            # print("err " + str(i))
            return 
    
    if not isinstance(answer[1][1], list):
        answer[1][1] = [answer[1][1]]
    regularProps(answer[1][1])


def demoIdx():
    try:
        while True:
            tree = ParentedTree.fromstring(input())
            sentence = getSeq (tree)
            npList = parseNPsIdx(tree)
            prev = None
            show = []
            for chunck in npList:
                if not prev is None:
                    show.extend(sentence[prev:chunck[0]])
                show.append(sentence[chunck[0]:chunck[1]])
                prev = chunck[1]
            show.extend(sentence[npList[-1][1]:])
            print(show)
    except EOFError: pass


def printStructured():
    try:
        while True:
            parsed = parseExp(input()[:-2])  #in answer(...) format
            # print(parsed)
            regularize(parsed, {})
            print(parsed)
    except EOFError: pass

def dumpData():
    # seq = []
    # try:
    #     while True:
    #         tree = Tree.fromstring(input())
    #         seq.append([getSeq (tree), parseNPsIdx(tree)])
    # except EOFError: pass
    
    # with open("train/1", "wb") as fp:
    #     pickle.dump(seq, fp)

    prev = pickle.load(open("train/1", "rb"))
    propDict = {}

    for sample in prev:
        str = input()
        # print(str)
        parsed = parseExp(str[:-2])
        
        # print(parsed)
        regularize(parsed, propDict)
        sample.append(parsed[1][1])

    # with open("train/trnSet", "wb") as fp:
    #     pickle.dump(prev, fp)
    
    # print(propDict)
    # with open("train/propDict", "wb") as fp:
    #     pickle.dump(propDict, fp)


if __name__ == '__main__':
    dumpData()
    # printStructured()
