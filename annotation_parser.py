import json, sys

data = None
with open(sys.argv[1]) as json_file:
    data = json.load(json_file)

code = data['code']
anno = data['annotations']

def _off(elem):
    if 'offset' in elem:
        return "0x{:x}".format(int(elem['offset']))
    return ''

def _type(elem):
    t = elem['type']
    if t == 'syntax_highlight':
        return "hl/" + elem['syntax_highlight']
    return t

for elem in anno:
    print("{0:>6n} {1:>6n} {2:<22s} {3:<15s} '{4}'".format(elem['start'], elem['end'], _type(elem), _off(elem), code[elem['start']:elem['end']]))
