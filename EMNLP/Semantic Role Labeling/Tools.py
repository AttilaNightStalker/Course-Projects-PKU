from nltk import *

cnt1 = 0

''' add node range to a tree '''
def transForm (tree, left = 0):
    if not isinstance(tree[0], Tree):
        tree.set_label([tree.label(), tuple([left, left])])
        return left + 1
    else:
        originLeft = left
        for child in tree:
            left = transForm(child, left)
        tree.set_label([tree.label(), tuple([originLeft, left - 1])])
        return left

''' get left and right brothers of a node '''
def getBrothers(node):
    before = "NA"
    after = "NA"

    if node.parent() is None:
        return tuple([before, after])

    parent = node.parent()
    for i in range(len(parent)):
        if parent[i] is node:
            try:
                after = parent[i + 1].label()[0]
            except IndexError: pass
            if i - 1 < 0:
                break               
            before = parent[i - 1].label()[0]
            break
    return tuple([before, after])

''' get a list of argument path '''
def getArgs(node, searchTree):
    argList = []
    brothers = []
    nodeIdx = None
    for i in range(len(node.parent())):
        if node.parent()[i] is node:
            nodeIdx = i
        brothers.append(node.parent()[i])
    
    for child in searchTree:

        if not isinstance(child[0], Tree):
            nodePos = node.label()[1]
            curArg = [child.label(), nodePos, child[0] / child[1]]
            for arg in argList:
                if arg[0] == curArg[0] and arg[1][0] <= curArg[1][0] and arg[1][1] >= curArg[1][1]:
                    arg[1][0] = curArg[1][0]
                    arg[1][1] = curArg[1][1]
                    curArg = None
                    break
            if not curArg is None:
                argList.append(curArg)

        elif child.label()[0] == -1:
            if child.label()[1] == node.parent().label()[0] and child.label()[2] == getBrothers(node.parent()):
                argList.extend(getArgs(node.parent(), child))
            continue
        
        elif child.label()[0] == 0:
            if nodeIdx + child.label()[3] < 0:
                continue
            try:
                brother = brothers[nodeIdx + child.label()[3]]

                if child.label()[2] == getBrothers(brother) and (
                    child.label()[1] == tuple([node.parent().label()[0], brother.label()[0]])):

                    argList.extend(getArgs(brother, child))

            except IndexError:
                pass
            continue
        
        else:
            idx = child.label()[3]
            if node[idx].label()[0] == child.label()[1] and getBrothers(node[idx]) == child.label()[2]:
                argList.extend(getArgs(node[idx], child))
    return argList


''' get verb nodes list from a parented tree '''
def getVerbs (ptree):
    verbList = []
    for child in ptree:
        # print(child)
        # print("")
        if (not isinstance(child[0], Tree)):
            if child.label()[0][0] == 'V':
                verbList.append(child)
        else:
            verbList.extend(getVerbs (child))
    return verbList


''' get argument infomation from .props '''
def parseProps (reader):
    # line = input()
    line = reader.readline()
    line = line.strip('\n')
    line = line.split('\t')
    verbList = []
    idxCache = []
    for i in range(len(line) - 2):
        verbList.append([])
        idxCache.append(None)
    lineIdx = 0

    while True: 
        for i in range(len(verbList)):
            tag = line[i + 1]
            tag = tag.split('*')
            if len(tag[0]) > 0:
                if tag[0].find('V') != -1:
                    temp = [[tag[0][1:], lineIdx]]
                    temp.extend(verbList[i])
                    verbList[i] = temp
                    continue
                idxCache[i] = lineIdx
            if len(tag) > 1 and len(tag[1]) > 0:
                verbList[i].append([tag[1].split(")")[0], tuple([idxCache[i], lineIdx])])
        lineIdx += 1

        # print(verbList)
        line = reader.readline()
        line = line.strip('\n')
        if line == "":
            break
        line = line.split('\t')

    return verbList


def propFind (child, pos, propList):
    for prop in propList:
        if child.label() == prop[0] and pos == prop[1]:
            return True
    return False


''' every node is accesible from node, its count +1 '''
def countNodes (searchTree, pnode):

    brothers = []
    nodeIdx = None
    # if not searchTree.label() == "root":
    for i in range(len(pnode.parent())):
        if pnode.parent()[i] is pnode:
            nodeIdx = i
        brothers.append(pnode.parent()[i])
    
    for child in searchTree:

        if not isinstance(child[0], Tree):          
            child[1] += 1

        elif child.label()[0] == -1:
            if child.label()[1] == pnode.parent().label()[0] and child.label()[2] == getBrothers(pnode.parent()):
                countNodes(child, pnode.parent())
            continue
        
        elif child.label()[0] == 0:
            if nodeIdx + child.label()[3] < 0:
                continue
            try:
                brother = brothers[nodeIdx + child.label()[3]]

                if child.label()[2] == getBrothers(brother) and (
                    child.label()[1] == tuple([pnode.parent().label()[0], brother.label()[0]])):
                    # print("$")
                    countNodes(child, brother)

            except IndexError:
                pass
            continue
        
        else:
            idx = child.label()[3]
            try:
                if pnode[idx].label()[0] == child.label()[1] and getBrothers(pnode[idx]) == child.label()[2]:
                    countNodes(child, pnode[idx])
            except IndexError: pass


def check (searchTree):
    # print("$")
    res = True
    for child in searchTree:
        if not isinstance(child[0], Tree):
            if child[1] == 0:
                print(child)
                res = False
        elif check (child) == False:
            print(child.label())
            res = False
    if res == False:
        return False       
    return True
