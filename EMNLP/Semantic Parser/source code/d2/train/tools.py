from nltk.tree import *


def bayesProb(tag, tags, candidate):
    cntTag = 0
    cntKeyInTag = 0
    cntKey = 0
    # print(candidate)
    for sample in tags:
        if set(candidate).issubset(set(sample[0])):
            cntKey += 1
            if tag in sample[2]:
                # if candidate == ["through"]:
                #     print(tag, " ", sample[2])
                cntKeyInTag += 1
                cntTag += 1
        elif tag in sample[2]:
            cntTag += 1

    if cntTag == 0:
        return 0
    return (cntKeyInTag * cntKeyInTag) / (cntKey * cntTag), cntKeyInTag / cntKey

def jRange(idx, segs, length):
    if len(segs) == 0:
        print("bad")
        return 0, 0
    
    for i in range(len(segs)):
        if segs[i][0] <= idx and segs[i][1] >= idx:
            try:
                return segs[i][1] + 1, segs[i + 1][1] + 1
            except IndexError:
                return segs[i][1] + 1, length
        elif segs[i][1] < idx:
            try:
                if segs[i + 1][0] > idx:
                    return idx + 1, segs[i + 1][1] + 1
            except IndexError:
                return idx + 1, length

    try:
        return idx + 1, segs[1][1] + 1
    except IndexError:
        return idx, idx

 
def selectKeySeq (tag, tags, candidates, segs, maxscore):
    maxProb = 0, 0
    target = [None, len(candidates), -1]
    

    if len(tag[1]) == 1:
        for seg in segs:
            for i in range(seg[0], seg[1]):
                for j in range(i + 1, seg[1] + 1):
                    curProb = bayesProb(tag, tags, candidates[i:j])

                    if curProb[0] < maxscore and curProb[1] >= 0.5 and (curProb[0] > maxProb[0] or (curProb[0] == maxProb[0] and (i < target[1] or j > target[2]))):
                        maxProb = curProb
                        target = [candidates[i:j], i, j]
    
    else:
        for i in range(len(candidates)):
            for j in range(*jRange(i, segs, len(candidates))):
                curProb = bayesProb(tag, tags, candidates[i:j])

                if curProb[0] < maxscore and curProb[1] >= 0.5 and (curProb[0] > maxProb[0] or (curProb[0] == maxProb[0] and (i < target[1] or j > target[2]))):
                    maxProb = curProb
                    target = [candidates[i:j], i, j]   

    return [target[0], maxProb, target[1:]]


'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
in this part are functions for parse                         ''
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

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
    parseRalation(npNodes)
    print(parseRalation(npNodes))
    return list(map(lambda np : np[0], parseNP(countLeave(tree, [0]))))

