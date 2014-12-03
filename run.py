import subprocess as sub
import os, sys

def main():

    count = 0;

    # get files
    files = ['data/' + f for f in os.listdir("data")]

    # open file to write linter errors to
    with open('log.txt', 'w') as fout:

        # loop through each cpp file
        for f in files:
            print count
            count += 1

            # spawn a process that runs the linter on the file
            p = sub.Popen("python lint.py " + f, stdout=sub.PIPE, stderr=sub.PIPE, shell=True)

            # get the num errors from the linter and write it to the data file
            out, err = p.communicate()
            fout.write(f.rsplit('/')[1] + ' ' + err.split(' ')[-1])


if __name__ == '__main__':
    main()