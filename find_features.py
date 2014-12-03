import re

def main():

    output = []

    # write the format of the features in the file
    with open('complete_data.txt', 'w') as dest:
        dest.write('linter errors - line num - empty lines\n')

        # read a single file from the list
        f = open('log.txt', 'rw')

        for line in f:
            fileName = line.split(' ')[0]
            print fileName

            with open('data/' + fileName, 'r') as src:

                lineCount = 0;
                emptyLineCount = 0
                commentLineCount = 0
                poorlySpacedComments = 0
                linesOver80 = 0
                commasWithoutSpace = 0
                linesStartingWithTab = 0
                badParenSpacingNum = 0
                openingBraceAfterEndline = 0
                poorlySpacedOpener = 0
                endLineWhiteSpace = 0
                badSpacingBeforeComment = 0

                # loop through each line in file
                for l in src:

                    # increment number of lines
                    lineCount += 1

                    # check to see if the line is empty
                    emptyline = re.findall('\S+', l)
                    if len(emptyline) == 0:
                        emptyLineCount += 1

                    # check to see if line is a comment
                    commentLine = re.findall('//.*', l)
                    if len(commentLine) > 0:
                        commentLineCount += 1
                        # loop through each comment and see if it is well spaced
                        for i in range(len(commentLine)):
                            if len(commentLine[i]) > 2 and commentLine[i][2] != ' ':
                                poorlySpacedComments += 1
                                #print "bad comment spacing", lineNum

                    # update number of long lines
                    if len(l) > 80:
                        linesOver80 += 1

                    # check for commas without a following space
                    commaLine = re.findall(',\S', l)
                    commasWithoutSpace += len(commaLine)

                    # check for lines starting with tab
                    tabLine = re.match('\t+.*', l)
                    if tabLine != None:
                        #print "starts with tab", lineNum
                        linesStartingWithTab += 1

                    # check for uneeded spaces before or after parentheses
                    parenLine = re.findall('\(\s+|\s+\)', l)
                    badParenSpacingNum += len(parenLine)

                    # check for opening brace starting on next line
                    openBraceLine = re.match('\s*\{', l)
                    if openBraceLine != None:
                        openingBraceAfterEndline += 1
                        #print "open brace", lineNum

                    # check for poorly spaced openers
                    openerLine = re.findall('.[\{|\(]', l)
                    if len(openerLine) > 0:
                        #print len(openerLine), lineNum
                        for i in range(len(openerLine)):
                            if openerLine[i][0] != ' ':
                                poorlySpacedOpener += 1
                                #print "poor opener", lineNum

                    # check for line ending in unneeded white space
                    whiteSpaceEnd = re.match('.*\s\n', l)
                    if whiteSpaceEnd != None:
                        endLineWhiteSpace += 1
                        #print whiteSpaceEnd.group(), lineNum

                # add file name and label to output
                output.append(line.split(' ')[0].rstrip())
                output.append(line.split(' ')[1].rstrip())
                # add features to output
                output.append(str(lineCount))
                output.append(str(emptyLineCount))
                output.append(str(commentLineCount))
                output.append(str(poorlySpacedComments))
                output.append(str(linesOver80))
                output.append(str(commasWithoutSpace))
                output.append(str(linesStartingWithTab))
                output.append(str(badParenSpacingNum))
                output.append(str(openingBraceAfterEndline))
                output.append(str(poorlySpacedOpener))
                output.append(str(endLineWhiteSpace))

                # create ouptut string
                outputString = ""
                for i in range(len(output)):
                    outputString += output[i]
                    if i < len(output)-1:
                        outputString += ' '
                outputString += '\n'

                #clear output array
                output = []

                #write feature info to a finished file
                dest.write(outputString)  

if __name__ == '__main__':
    main()