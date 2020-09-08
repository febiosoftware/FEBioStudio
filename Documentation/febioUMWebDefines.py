import sys
from html.parser import HTMLParser

class MyHTMLParser(HTMLParser):
    
    def __init__(self):
        HTMLParser.__init__(self)
        self.start = False
        self.inLink = False
        self.currentLink = ""
        self.currentString = []
        self.links = []
        self.taken = set()
        
    
    def handle_starttag(self, tag, attrs):
        if self.start:
            if tag == "a":
                self.inLink = True
                
                for attr in attrs:
                    if attr[0] == "href":
                        self.currentLink = attr[1]
                
                # Cutt off unecessary part of URL that comes after the # symbol
                self.currentLink = self.currentLink.split("#")[0]

    def handle_endtag(self, tag):
        if self.start:
            if tag == "li":
                if len(self.currentString) > 0:
                    del self.currentString[-1] 
                
            elif tag == "a":
                self.inLink = False
                self.addLink()

    def handle_data(self, data):
        if self.start:
            if self.inLink:
                splitData = data.split(":")
                
                if len(splitData) > 1:
                    self.currentString.append(splitData[1].strip().replace(" ", "_").replace("-", "_").replace(",", "").replace("’", "").replace("‘", ""))
        elif "FEBio User Manual" in data:
            self.start = True
    
    def addLink(self):
        if len(self.currentString) > 1:
            # name = ""
            # for subString in self.currentString:
                # name += subString
                # name += "_"
            
            # name = name[:-1]
            
            index = len(self.currentString) - 1
            
            name = self.currentString[index]
            index -= 1
            if not "_" in name and len(self.currentString) > 0:
                name = self.currentString[index] + "_" + name
                index -= 1
                
            while name in self.taken:
                name = self.currentString[index] + "_" + name
                index -= 1
            
            self.links.append([name,self.currentLink])
            self.taken.add(name)
    
    def writeLinks(self):
        with open("WebDefines.h", "w") as f:
            f.write("#pragma once\n\n")
            
            for link in self.links:
                link[0] = link[0].replace("Free_Format_Input_", "")
                f.write('#define ' + link[0] + ' "' + link[1] + '"\n')
    
    def printLink(self):
        if len(self.currentString) > 1:
            name = ""
            for subString in self.currentString:
                name += subString
                name += "_"
            
            name = name[:-1]
            
            print(name + ": " + self.currentLink)

if len(sys.argv) > 1:
    f = open(sys.argv[1], "r")

    parser = MyHTMLParser()
    parser.feed(f.read())

    parser.writeLinks()
else:
    print("Please provide the path to the FEBio manual table of contents html file as an argument")
