import os

def main():

    count = 0;

    # get files
    files = ['data/' + f for f in os.listdir("data")]

    # loop through each cpp file
    for f in files:
        if f.split(".")[0] == f or (f.split(".")[-1] != "cpp" and f.split(".")[-1] != "cc"):
            os.remove(f);
            count += 1

    print count

if __name__ == '__main__':
    main()