filename = 'car-mobile.html'
file = None
string = 'extern const char * car = '

with open(filename) as f:
    for line in f:
        line_processed = line.strip()
        line_processed = line_processed.replace("\"", "\\\"")
        string += "\"" + line_processed + " \"" + '\n'

string += "\"\";"
f.closed

headerFilename = input('Enter header filename: ')
headerFilename += '.h'
headerPath = 'c:\\Users\\IR\\Documents\\Arduino\\esp-car\\monster-shield-car\\'

with open(headerPath + headerFilename, 'w') as f:
    f.write(string)
f.closed

# print(headerFilename)