#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json

def read_file(filename):    
    with open(filename) as f:
        return f.read()

if __name__ == '__main__':
    lines = read_file('service_schema.json')
    print('=' * 80)
    print(lines)
    print('=' * 80)
    jsonFile = json.loads(lines)
    compactedJson = json.dumps(jsonFile, separators=(',',':'))
    result = ('\"' + compactedJson.replace('\"','\\"') + '\";')
    print(result)
    print('=' * 80)
    print('Length: ' + str(len(result)))
    print('Size: ' + str(len(unicode(result).encode('utf-8'))))
    print('=' * 80)
