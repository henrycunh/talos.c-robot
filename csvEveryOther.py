import csv
datasheet = open("rawData.csv", "w")
datasheet.write("X,Y\n")
with open('viewData.csv') as data:
    reader = csv.DictReader(data)
    i = 0
    for linha in reader:
        if i % 2 == 0:
            datasheet.write(linha['X'] + "," + linha['Y']+"\n")
        i += 1
datasheet.close()