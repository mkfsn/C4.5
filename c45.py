#!/usr/bin/env python
# -*- coding: utf-8 -*-
__date__= 'Mar 16, 2014 '
__author__= 'mkfsn'

import sys
from math import log
from collections import Counter

class Node():
    item = -1
    value = ""
    decision = []

    def __init__(self, item = 2, num = 0, value = ""):
        self.decision = [0]*num
        self.item     = item
        self.value    = value

    def __repr__(self):
        return repr({'item': self.item, 'value': self.value, 'decision':
                self.decision})

class C45():
    __tree = [] 
    __database = []
    level = 0

    def __init__(self):
        pass

    def __transfer(self, attr, value):
        if attr == 0:
            return ['S', 'M', 'Unknown'].index(value)
        elif attr == 1:
            size = self.__attr_len(attr)
            res = int(value)
            return size-1 if res>=size else res
        elif attr == 2:
            return ['Basic', 'Normal', 'Silver', 'Gold'].index(value)
        elif attr == 3:
            size = self.__attr_len(attr)
            res = int(value)/10
            return size-1 if res>size else res
        elif attr == 4:
            size = self.__attr_len(attr)
            res = int(value)/20000
            return size-1 if res>size else res
    
    def __attr_len(self, attr):
        length = [3, 11, 4, 11, 11]
        return length[attr]

    def __func_info(self, val_list):
        s = sum(val_list)
        return sum([(-1)*(num*1.0/s)*log((num*1.0/s),2) if num else 0.0 for num in val_list])

    def __decode(self, string):
        #marital_status, num_children_at_home, member_card, age, year_income
        element = [ 'Unknown', '0', 'Basic', '0', '0' ]
        for e in string.strip().strip('{}').split(','):
            k, v = e.split(' ')
            element[ int(k) ] = v
        return element

    def __build(self, dataset, flag):
        if len(dataset) < 1:
            self.__tree.append ( Node(2, 0, 'Basic') )
            return len(self.__tree)-1
        if not sum(flag):
            candidate = [e[2] for e in dataset]
            self.__tree.append( Node(2, 0 , Counter(candidate).most_common(1)[0][0]) )
            return len(self.__tree)-1

        count = [[[0]*4 for j in range(self.__attr_len(i))] for i in range(5)]
        for entry in dataset:
            decision = self.__transfer(2, entry[2])
            for i, item in enumerate(entry):
                if i == 2:
                    count[2][0][decision] += 1
                elif flag[i]:
                    count[i][self.__transfer(i, entry[i])][decision] += 1

        info = [0.0]*5
        for k, v in enumerate(count):
            if k == 2:
                info[2] = self.__func_info( count[2][0] )
            elif flag[k]:
                info[k] = sum([(sum(v1)*1.0/len(dataset))*self.__func_info(v1) for k1, v1 in enumerate(v)])

        gain = [ info[2]-v if flag[k] else -10.0 for k,v in enumerate(info) ]
        idx = gain.index(max(gain))

        node = Node(idx, self.__attr_len(idx), "")
        self.__tree.append( node )
        cur_idx = len(self.__tree)-1

        new_flag = [ 0 if idx==k else v for k,v in enumerate(flag) ]
        for k, n in enumerate(node.decision):
            new_dataset = [e for e in dataset if self.__transfer(idx, e[idx])==k]
            node.decision[k] = self.__build( new_dataset, new_flag )
        return cur_idx


    def __traversal( self, idx, entry ):
        item = self.__tree[idx].item
        value = self.__tree[idx].value
        if item == 2:
            return value
        subtree = self.__tree[idx].decision[self.__transfer(item, entry[item])]
        return  self.__traversal(subtree, entry)

    def learn(self, filename):
        with open(filename, 'r') as f:
            self.__database = [ self.__decode(l) for l in f ]
        self.__build(self.__database, [1, 1, 0, 1, 1])

    def test(self, filename):
        with open(filename, 'r') as f:
            parsed = [self.__decode(l) for l in f]
        result = [(e[2], self.__traversal(0, e)) for e in parsed]
        ratio = sum(1 if a[0]==a[1] else 0 for a in result)*100.0/len(result)
        return "{0}%".format(ratio)

def usage():
    print "Usage:"
    print "\t ./c45.py trainingfile testfile"
    print

def main():
    if len(sys.argv) < 3:
        usage()
        exit(1)

    train_file = sys.argv[1]
    test_file = sys.argv[2]
    
    c = C45()
    c.learn( train_file )
    print c.test( test_file )

if __name__ == '__main__':
    main()
