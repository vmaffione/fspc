import re
import glob


# Scan the file provided as argument looking for '#include "..."'
# and return all the matches
def get_includes(f):
    ret = []
    fin = open(f, 'r')

    while 1:
        line = fin.readline()
        if line == '':
            break
        m = re.match(r'[ \t]*#[ \t]*include [ \t]*"([a-z._]+)"', line)
        if m:
            ret.append(m.group(1))
    fin.close()

    return ret


# Some filename transformations for the sake of readability
def transform_name(s):
    ret = s.replace('.cpp', 'O').replace('.hpp', 'H').replace('.hh', 'H')

    # Transform 'xxxx_yyyy' into 'yyyy'
    m = re.match(r'[^_]+_([^_]+)', ret)
    if m:
        ret = m.group(1)

    return ret


def get_files(extensions, blacklist):
    ret = []

    # Find all the files having one of the specified extension
    for ext in extensions:
        ret = ret + glob.glob('*.' + ext)

    # Remove from the result the blacklisted files
    for b in blacklist:
        if b in ret:
            ret.remove(b)

    return ret



extensions = ['cpp', 'hpp', 'hh']
blacklist = ['test-serializer.cpp']

files = get_files(extensions, blacklist)

fout = open('deps.gv', 'w')
fout.write('digraph G {\n')

for f in files:
    includes = get_includes(f)
    left = transform_name(f)
    for i in includes:
        right = transform_name(i)
        fout.write(left + ' -> ' + right + '\n')

fout.write('}\n')
fout.close()
