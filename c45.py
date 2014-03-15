#!/usr/bin/env python
# -*- coding: utf-8 -*-
__date__= 'Mar 16, 2014 '
__author__= 'mkfsn'

import sys
from math import log

class Node():
    item = 2
    value = ""
    decision = []

    def __init__(self, item = 2, num = 0, value = ""):
        self.decision = [{}]*num
        self.item     = item
        self.value    = value

    def __str__(self):
        return "(item: {0}, value: {1}, decision: {2})".format(self.item,
                self.value, self.decision)

class C45():
    __tree = {}
    __database = []
    level = 0

    def __init__(self):
        pass

    def __transfer(self, attr, value):
        if attr == 0:
            return ['S', 'M', 'O'].index(value)
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
            res = int(value)/40000
            return size-1 if res>size else res
    
    def __attr_len(self, attr):
        length = [3, 11, 4, 11, 6]
        return length[attr]

    def __func_info(self, val_list):
        s = sum(val_list)
        return sum([(-1)*(num*1.0/s)*log((num*1.0/s),2) if num else 0.0 for num in val_list])

    def __decode(self, string):
        #marital_status, num_children_at_home, member_card, age, year_income
        element = [ 'O', '0', 'Basic', '0', '0' ]
        for e in string.strip().strip('{}').split(','):
            k, v = e.split(' ')
            element[ int(k) ] = v
        return element

    def __build(self, dataset, flag):

        if len(dataset) < 1:
            return Node(2, 0, 'Basic')
        if not sum(flag):
            print dataset
            print
            return Node(2, 0 , 'Basic')

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
        new_flag = [ 0 if idx==k else v for k,v in enumerate(flag) ]
        for k, n in enumerate(node.decision):
            new_dataset = [e for e in dataset if self.__transfer(idx, e[idx])==k]
            self.level += 1
            n['child'] = self.__build( new_dataset, new_flag )
            self.level -= 1
        return node


    def learn(self, filename):
        with open(filename, 'r') as f:
            self.__database = [ self.__decode(l) for l in f ]
        self.__tree['child'] = self.__build(self.__database, [1, 1, 0, 1, 1])

    def test(self, filename):
        pass

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
    c.test( test_file )


if __name__ == '__main__':
    main()
