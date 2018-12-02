from nltk.tree import *
from tools import *
from nltk.parse.stanford import StanfordParser
import pickle

constDict = {}

def formParas(para):
    if len(para) == 0:
        return []
    if len(para) == 1:
        return [0]
    if ord(para[0]) < ord(para[1]):
        return [0, 1]
    return [1, 0]


def getDict():
    trnSet = pickle.load(open("trnSet", "rb"))    
    tags = []
    dict = {}
    
    def regulizeName (name):
        global constDict
        try:
            if name[0] == '\'':
                res = name[1:-1].replace(" ", "_")
                constDict[res] = name
                return res
        except IndexError:
            pass
        constDict[name] = name
        return name

    for sample in trnSet:
        sentence = sample[0]
        print(sample[2])
        tagSeq = list(map(lambda prop : [prop[0], formParas(prop[1])] if prop[0] != "const" else ["const_" + regulizeName(prop[1][1]), [0]], sample[2]))
        # print(tagSeq)
        for i in range(len(sentence)):
            word = sentence[i]
            if [word, [0]] in tagSeq: 
                try:
                    if dict[word][0][0] != word:
                        print(dict[word], word)
                        pass
                except KeyError:
                    dict[word] = [[word, [0]], (2.1, 2,1)]

                # sentence[i] = "*"
                tagSeq.remove([word, [0]])

            elif ["const_" + word, [0]] in tagSeq: 
                try:
                    if dict[word][0][0] != "const_" + word:
                        print(dict[word], word)
                        pass
                except KeyError:
                    dict[word] = [["const_" + word, [0]], (2.1, 2,1)]
                tagSeq.remove(["const_" + word, [0]])   

        # for tag in tagSeq:
        #     if              
        
        try:
            sentence.remove('?')
            sentence.remove(".")
        except ValueError: pass

        tags.append([sentence, sample[1], tagSeq])


    while True:
        print("mark")
        flag = False
        for sample in tags:
            tagList = sample[2]
            tagFlags = [None] * len(sample[0])

            for tag in tagList:
                # print(tag, ": ")
                max = 2
                while True:
                    key =  selectKeySeq (tag, tags, sample[0], sample[1], max)
                    if key[1] == (0, 0):
                        break
                    keyPos = key[2]

                    keyPhrase = "_".join(key[0])

                    try:
                        existed = dict[keyPhrase]
                        if existed[0] != tag:
                            if existed[1][0] < key[1][0]:       
                                dict[keyPhrase] = [tag, key[1]]
                                flag = True
                                break        
                            max = key[1][0]
                            continue
                        break

                    except KeyError:
                        ''' check if there is overlap key phrases in the same answer '''
                        for tFlag in tagFlags[keyPos[0]:keyPos[1]]:
                            if not tFlag is None:                        
                                overlapKey = "_".join(s for s in sample[0][tFlag[0]:tFlag[1]])
                                
                                existed = dict[overlapKey]
                                if existed[1][0] < key[1][0]:
                                    del dict[overlapKey]                       
                                    flag = True
                                    for i in range(tFlag[0], tFlag[1]):
                                        tagFlags[i] = None
                                    break
                                else: 
                                    max = key[1][0]
                                    break
                        if max == key[1][0]:
                            continue
                        
                        for i in range(keyPos[0], keyPos[1]):
                            tagFlags[i] = keyPos
                        dict[keyPhrase] = [tag, key[1]]
                                
                        break
        
        if flag == False:
                break
        # break

    # print(dict["the_us"])
    print(dict)
    with open("dict", "wb") as fp:
        pickle.dump(dict, fp)    

    with open("constDict", "wb") as fp:
        pickle.dump(constDict, fp)   
    # for tag in tags:
    #     print(tag)


if __name__ == '__main__':
    getDict()



