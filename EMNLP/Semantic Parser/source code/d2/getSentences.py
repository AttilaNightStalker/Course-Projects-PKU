
if __name__ == '__main__':
    try:
        while True:
            text = input()
            if not (text[0] == '[' and text[-1] == ']'):
                continue
            
            print(text[1:-1].replace(' ', '').replace(',', ' '))
    except EOFError: pass

'''
java -jar ../berkeleyparser-master/BerkeleyParser-1.7.jar -gr ../berkeleyparser-master/eng_sm6.gr
'''