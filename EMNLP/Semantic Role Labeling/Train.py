from nltk import *
import pickle
from Tools import *


def findVerb (ptree, index):
    if not isinstance(ptree[0], Tree):
        if ptree.label()[1][0] == index:
            return ptree
        # print("666")

    for child in ptree:
        if child.label()[1][0] <= index and child.label()[1][1] >= index:
            return findVerb(child, index)

    return None

def childIdx (parent, targetChild):
    index = 0
    tagIndex = 0
    for child in parent:
        if child is targetChild:
            return tuple([index, tagIndex])
        if child.label()[0] == targetChild.label()[0]:
            tagIndex += 1
        index += 1

    return None

def listToTree (list):
    if (len(list) == 1):
        return Tree(list[0], [1, 0])
    else:
        return Tree(list[0], [listToTree(list[1:])])


def updateTree (tree, path): 
    for i in range(len(path)):
        nextNode = findNode(tree, path[i])
        if nextNode is None:
            tree.append(listToTree(path[i:]))
            return
        elif not isinstance(nextNode[0], Tree):
            nextNode[0] = nextNode[0] + 1
            return
        tree = nextNode 
        

def findNode(tree, node):
    for child in tree:
        if child.label() == node:
            return child  
    return None


''' build path tree '''
def parseRoute (ptree, verbList, routeTree = None):
    for verb in verbList:
        verbNode = findVerb(ptree, verb[0][1])
        for prop in verb[1:]:
            left = prop[1][0]
            right = prop[1][1]
            propTag = prop[0]
            routeList = [[verbNode.label()[0], getBrothers(verbNode)]]
            curNode = verbNode

            while curNode.label()[1][0] != left or curNode.label()[1][1] != right:

                ''' when current node covers the target node '''
                if curNode.label()[1][0] <= left and curNode.label()[1][1] >= right:
                    for i in range(len(curNode)):
                        # find the target node
                        if curNode[i].label()[1][0] <= left and curNode[i].label()[1][1] >= right:
                            curNode = curNode[i]
                            routeList.append([1, curNode.label()[0], getBrothers(curNode), i])
                            break   

                # ''' when next node is current node's brother (or next of next) '''  
                elif curNode.parent().label()[1][0] <= left and curNode.parent().label()[1][1] >= right and (
                    curNode.parent().label()[1][1] - curNode.parent().label()[1][0] > right - left):
                    parent = curNode.parent()
                    brotherIdx = None
                    nextNode = None
                    curIdx = None
                    i = 0
                    for child in parent:
                        if child.label()[1][0] <= left and child.label()[1][1] >= right:
                            brotherIdx = i
                            nextNode = child
                        if child is curNode:
                            curIdx = i
                        i += 1
                    routeList.append([0, tuple([parent.label()[0], nextNode.label()[0]]), getBrothers(nextNode), brotherIdx - curIdx])
                    curNode = nextNode

                # ''' 'when need to shift to father '''
                else:
                    curNode = curNode.parent()
                    routeList.append([-1, curNode.label()[0], getBrothers(curNode)])                  

            routeList.append(propTag)

            updateTree(routeTree, routeList)


def main():
    readSyntax = open("trn/train.trees")
    readProps = open("trn/trn.props")
    
    routeTree = Tree.fromstring("(root )")
    while True:

        treeStr = readSyntax.readline()
        if treeStr == "":
            break
        ptree = ParentedTree.fromstring(treeStr)
        transForm(ptree)
        verbList = parseProps(readProps)

        parseRoute(ptree, verbList, routeTree)

    readSyntax.close()
    readProps.close()  

    readSyntax = open("trn/train.trees")
    readProps = open("trn/trn.props")

    while True:
        treeStr = readSyntax.readline()
        if treeStr == "":
            break
        ptree = ParentedTree.fromstring(treeStr)
        transForm(ptree)
        verbList = parseProps(readProps)

        for verb in verbList:
            verbNode = findVerb(ptree, verb[0][1])
            for child in routeTree:
                if child.label() == [verbNode.label()[0], getBrothers(verbNode)]:
                    countNodes(child, verbNode)
                    break

    # check(routeTree)
    with open("PathTree", "wb") as fp:
        pickle.dump(routeTree, fp)


if __name__ == '__main__':

    main()

